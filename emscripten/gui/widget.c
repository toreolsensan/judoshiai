#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cairowrapper.h"

int gui_widget_detach(gui_widget *child)
{
	if (child != 0 && child->parent != 0) {
		gui_widget *parent = child->parent;
		gui_widget *iter;
		if (child == parent->first_child) { /* first child = special case */
			parent->first_child = child->next;
			child->next = child->parent = 0;
			return 0; /* no error */
		}
		for (iter = parent->first_child; (iter != 0); iter = iter->next) {
			if (iter->next == child) {
				iter->next = child->next;
				return 0; /* no error */
			} /* found it! */
		}/* not first child */
	}
	SDL_SetError("Child element is not contained within its designated parent.");
	return 1; /* error -- child is not a child of its designated parent! */
}

void gui_widget_add_last(gui_widget *parent, gui_widget *child)
{
	if (child != 0 && parent != 0) {
		if (child->parent != 0) {
			gui_widget_detach(child);
		}
		child->next = 0;
		if (parent->first_child == 0) {
			parent->first_child = child;
		} else {
			gui_widget *iter;
			for (iter = parent->first_child;
			     (iter->next != 0); iter = iter->next){}
			iter->next = child;
		}
		child->parent = parent;
	}
}

void gui_widget_add_first(gui_widget *parent, gui_widget *child)
{
	if (child != 0 && parent != 0) {
		if (child->parent != 0) {
			gui_widget_detach(child);
		}
		if (parent->first_child != 0) {
			child->next = parent->first_child;
		} else {
			child->next = 0;
		}
		parent->first_child = child;
		child->parent = parent;
	}
}

void gui_widget_insert_after(gui_widget *w_a, gui_widget *w_b)
{
	if (w_a && w_b && w_a->parent) {
		if (w_b->parent)
			gui_widget_detach(w_b);
		w_b->next = w_a->next;
		w_a->next = w_b;
		w_b->parent = w_a->parent;
	}
}

gui_widget *gui_widget_new(int type, int id, int w_size,
			   gui_event_handler evntcb,
			   gui_render_handler rndrcb,
			   gui_destroy_handler dstrycb,
			   int flags)
{
	if (w_size == 0)
		w_size = sizeof(gui_widget);

	gui_widget *w = SDL_malloc(w_size);

	if (w != 0) {
		memset(w, 0, w_size);
		w->widget_type = type;
		w->widget_id = id;
		w->widget_size = w_size;
		w->event_cb = evntcb;
		w->render_cb = rndrcb;
		w->destroy_cb = dstrycb;
		w->flags = flags;
	}
	return w;
}

void gui_widget_destroy(gui_widget *widget)
{
	if (widget->destroy_cb)
		widget->destroy_cb(widget);
	gui_widget_detach(widget);
	SDL_free(widget);
}

void gui_widget_add(gui_widget *parent, gui_widget *child)
{
	gui_widget_add_last(parent, child);
	child->region.x += parent->region.x;
	child->region.y += parent->region.y;
}

int gui_widget_insert_after_update_pos(gui_widget *first, gui_widget *second)
{
	if (first->parent == 0) {
		SDL_SetError("Widget lacks parent");
		return -1;
	}
	gui_widget_insert_after(first, second);
	gui_widget* parent = (gui_widget*)(first->parent);
	if (parent != 0) {
		second->region.x += parent->region.x;
		second->region.y += parent->region.y;
	}
	return 0;
}

int gui_widget_remove(gui_widget *child)
{
	gui_widget* parent = child->parent;
	if (parent) {
		child->region.x -= parent->region.x;
		child->region.y -= parent->region.y;
	}
	return gui_widget_detach(child);
}

int gui_widget_render(gui_widget *widget,
			SDL_Surface *renderer,
			gui_error_handler eh)
{
	if (widget == 0) {
		SDL_SetError("Attempt to render NULL widget");
		return -1;
	}
	if (renderer == 0) {
		SDL_SetError("Attempt to render with NULL renderer");
		return -2;
	}
	if ((widget->flags & GUI_WIDGETFLAG_INVISIBLE) != 0)
		return 0; /* don't draw invisible widgets or their children */

	if (widget->skin_cb != 0) {
		int ret = widget->skin_cb(widget, renderer);
		if (ret != 0) {
			int errorret = eh(gui_error_Preskin, widget, 0, renderer, ret);
			switch (errorret) {
			case 0:
				break;
			case 1:
				return ret;
			case 2:
				abort();
				break;
			}
		}
	}
	if (widget->render_cb != 0) {
		int ret = widget->render_cb(widget, renderer);
		if (ret != 0) {
			int errorret = eh(gui_error_Render, widget, 0, renderer, ret);
			switch (errorret) {
			case 0:
				break;
			case 1:
				return ret;
			case 2:
				abort();
				break;
			}
		}
	}

	gui_widget* child = widget->first_child;
	int cret = SDL_FALSE;
	while (child != 0 && cret == SDL_FALSE) {
		cret = gui_widget_render(child, renderer, eh);
		child = child->next;
	}

	if (widget->decor_cb != 0) {
		int ret = widget->decor_cb(widget, renderer);
		if (ret != 0) {
			int errorret = eh(gui_error_Postskin, widget, 0, renderer, ret);
			switch (errorret) {
			case 0:
				break;
			case 1:
				return ret;
			case 2:
				abort();
				break;
			}
		}
	}

	return cret;
}

void show_menu(int yes);

int gui_widget_event(gui_widget *widget, const SDL_Event ev,
		       gui_error_handler eh)
{
	if(widget == 0)
		return 0;
	if((widget->flags & GUI_WIDGETFLAG_DISABLED) ||
	   (widget->flags & GUI_WIDGETFLAG_INVISIBLE))
		return 0;

	int ret = 0;
	if (widget->event_cb != 0) {
		ret = widget->event_cb(widget, ev);
		if (ret == SDL_TRUE) /* event handled, exit */
			return ret;
		else if (ret != SDL_FALSE) {
			int errorret = eh(gui_error_Event, widget, &ev, 0, ret);
			switch (errorret) {
			case 0:
				break;
			case 1:
				return ret;
			case 2:
				abort();
				break;
			}
		}
	}

	if (ret == SDL_FALSE) {
		gui_widget* child = widget->first_child;
		while (child != 0 && ret == SDL_FALSE) {
			ret = gui_widget_event(child, ev, eh);
			child = child->next;
		}
	}
#if 0
	if (ret == SDL_FALSE) {
		show_menu(SDL_FALSE);
		expose();
	}
#endif
	return ret;
}
