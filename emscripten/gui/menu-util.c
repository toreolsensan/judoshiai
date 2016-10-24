/*
 * Copyright (C) 2006-2016 by Hannu Jokinen
 * Full copyright text is included in the software package.
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <locale.h>
#include <sys/time.h>

#ifdef WIN32
#include <process.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif

#include "menu-util.h"

void expose(void);

extern gui_widget *menubar;
gui_entry *gui_entry_focused = 0;
static int mouse_x, mouse_y;
static void *current_widget;

void gui_buttons_to_menu(gui_button *button, int hor)
{
	gui_widget* node = (gui_widget *)button;
	gui_button *b;
	int max_w = 0, max_h = 0;
	int x0 = button->region.x;
	int y0 = button->region.y;

	while (node) {
		b = (gui_button *)node;
		//printf("1 - but=%p node=%p w=%d h=%d\n", b, node, b->region.w, b->region.h);
		if (b->region.w > max_w)
			max_w = b->region.w;
		if (b->region.h > max_h)
			max_h = b->region.h;
		node = node->next;
	}

	node = (gui_widget *)button;
	while (node) {
		b = (gui_button *)node;
		//printf("2 - but=%p node=%p w=%d h=%d\n", b, node, b->region.w, b->region.h);

		b->region.x = x0;
		b->region.y = y0;

		if (!hor)
			b->region.w = max_w;
		b->region.h = max_h;
#if 0
		b->region.w += 6;
		b->region.h += 6;
		b->region.x += 3;
		b->region.y += 3;
#endif
		if (hor)
			x0 += b->region.w;
		else
			y0 += b->region.h;

		node = node->next;
	}
}

void set_submenu_visibility(gui_widget *parent, int visible)
{
	gui_widget* child = parent->first_child;
	while (child != 0) {
		if (!visible && child == (gui_widget *)gui_entry_focused)
			gui_entry_focused = 0;

		gui_widget_set_visible(child, visible);
		child = child->next;
	}
}

void hide_submenus(gui_widget *mb)
{
	gui_widget* menu = mb->first_child;
	while (menu != 0) {
		set_submenu_visibility(menu, SDL_FALSE);
		hide_submenus(menu);
		menu = menu->next;
	}
	gui_entry_focused = 0;
}

void show_menu(int yes)
{
	if (yes)
		gui_widget_set_visible(menubar, SDL_TRUE);
	else {
		hide_submenus(menubar);
		gui_widget_set_visible(menubar, SDL_FALSE);
	}
}

int menu_draw(gui_widget *w, SDL_Surface *s)
{
    return 0;
}

static Uint32 color_basic;
static Uint32 color_entry;
static Uint32 color_entry_focused;
static Uint32 color_hoover;
static Uint32 color_light;
static Uint32 color_dark;

int menu_background(gui_widget *w, SDL_Surface *s)
{
    gui_button *b = (gui_button *)w;
    static int init_done = SDL_FALSE;

    if (!init_done) {
	    color_basic = SDL_MapRGB(s->format, 200, 200, 200);
	    color_entry = SDL_MapRGB(s->format, 230, 230, 230);
	    color_entry_focused = SDL_MapRGB(s->format, 250, 250, 250);
	    color_hoover = SDL_MapRGB(s->format, 250, 250, 250);
	    color_light = SDL_MapRGB(s->format, 250, 250, 250);
	    color_dark = SDL_MapRGB(s->format, 150, 150, 150);
	    init_done = SDL_TRUE;
    }

    if (mouse_x >= w->region.x && mouse_x <= w->region.x + w->region.w &&
	mouse_y >= w->region.y && mouse_y <= w->region.y + w->region.h)
	    current_widget = w;

    if (w == current_widget)
	    SDL_FillRect(s, &w->region, color_hoover);
    else
	    SDL_FillRect(s, &w->region, color_basic);

    if (w->widget_type == GUI_WIDGET_TYPE_ENTRY) {
	    SDL_Rect rect;
	    rect.x = w->region.x + 2;
	    rect.y = w->region.y + 2;
	    rect.w = w->region.w - 4;
	    rect.h = w->region.h - 4;

	    if (w == (gui_widget *)gui_entry_focused) {
		    SDL_FillRect(s, &rect, color_entry_focused);
	    } else {
		    SDL_FillRect(s, &rect, color_entry);
	    }
    } else if (w->widget_type == GUI_WIDGET_TYPE_BUTTON) {
	    if (((gui_button *)w)->flags & GUI_BUTTONFLAG_BUTTON) {
		    SDL_Rect rect;
		    rect.x = w->region.x;
		    rect.y = w->region.y;
		    rect.w = w->region.w;
		    rect.h = 2;
		    SDL_FillRect(s, &rect, color_light);
		    rect.y = w->region.y + w->region.h - 2;
		    SDL_FillRect(s, &rect, color_dark);
		    rect.x = w->region.x;
		    rect.y = w->region.y + 1;
		    rect.w = 2;
		    rect.h = w->region.h - 2;
		    SDL_FillRect(s, &rect, color_light);
		    rect.x = w->region.x + w->region.w - 2;
		    SDL_FillRect(s, &rect, color_dark);
	    }
    }

    return 0;
}

void set_mouse_coordinates(int x, int y)
{
	mouse_x = x;
	mouse_y = y;
	if (current_widget) {
		gui_widget *w = current_widget;
		if (x < w->region.x || x > w->region.x + w->region.w ||
		    y < w->region.y || y > w->region.y + w->region.h) {
#if 0
			if (w->widget_type == GUI_WIDGET_TYPE_BUTTON) {
				gui_button *b = (gui_button *)w;
				if (b->text && b->text[0] == '>')
					set_submenu_visibility(w, SDL_FALSE);
			}
#endif
			current_widget = 0;
			expose();
		}
#if 0
		else if (!(w->flags & GUI_WIDGETFLAG_INVISIBLE) &&
			   w->first_child) {
			if (w->widget_type == GUI_WIDGET_TYPE_BUTTON) {
				gui_button *b = (gui_button *)w;
				if (b->text && b->text[0] == '>')
					set_submenu_visibility(w, SDL_TRUE);
				expose();
			}
		}
#endif
	}
}

int mouse_error_handler(gui_error_type err, struct gui_widget *w,
			const SDL_Event *ev, SDL_Surface *s, int errc)
{
    return GUI_ERROR_IGNORE;
}

int click_menu(gui_button *btn, const gui_button_event ev)
{
	if (ev.type == GUI_BUTTON_PRESSED) {
		int is_visible = 0;
		gui_widget *node = (gui_widget *)btn;
		gui_button *child = btn->first_child;

		if (child)
			is_visible = (child->flags & GUI_WIDGETFLAG_INVISIBLE) == 0;

		hide_submenus(menubar);

		if (!is_visible)
			set_submenu_visibility((gui_widget *)btn, SDL_TRUE);

		expose();
	}

	return SDL_TRUE;
}

void clear_submenus(void *node)
{
	gui_widget *parent = ((gui_widget *)node)->parent;
	gui_widget *iter = parent->first_child;

	while (iter) {
		if (iter != node && iter->first_child)
			set_submenu_visibility(iter, SDL_FALSE);
		iter = iter->next;
	}
}

int click_dummy(gui_button *btn, const gui_button_event ev)
{
	gui_widget *node = (gui_widget *)btn;
	gui_widget *child = node->first_child;
	gui_widget *parent = node->parent;
	gui_widget *iter;

	if (ev.type != GUI_BUTTON_PRESSED)
		return SDL_TRUE;

	//hide_submenus(menubar);
	//set_submenu_visibility(btn->element_base->parent->data, SDL_TRUE);

	clear_submenus(btn);

	iter = parent->first_child;
	while (iter) {
		if (iter != node && iter->first_child)
			set_submenu_visibility((gui_widget *)iter, SDL_FALSE);
		iter = iter->next;
	}

	if (child) {
		if (child->flags & GUI_WIDGETFLAG_INVISIBLE) {
			set_submenu_visibility((gui_widget *)btn, SDL_TRUE);
			if (child->widget_type == GUI_WIDGET_TYPE_ENTRY)
				gui_entry_focused = (gui_entry *)child;
		} else
			set_submenu_visibility((gui_widget *)btn, SDL_FALSE);
	}

	handle_click(btn);
	expose();

	return SDL_TRUE;
}

int click_checkbox(gui_button *btn, const gui_button_event ev)
{
	clear_submenus(btn);

	if (ev.type == GUI_BUTTON_PRESSED) {
		btn->selected = !btn->selected;
		handle_click(btn);
		expose();
	}
	return SDL_TRUE;
}

int click_radio(gui_button *btn, const gui_button_event ev)
{
	if (ev.type != GUI_BUTTON_PRESSED)
		return SDL_TRUE;

	clear_submenus(btn);

	gui_widget *parent = (gui_widget *)btn->parent;
	gui_widget *child = parent->first_child;
	while (child) {
	    gui_button *b = (gui_button *)child;
		if (b->group == btn->group) {
			if (b == btn)
				b->selected = SDL_TRUE;
			else
				b->selected = SDL_FALSE;
		}
		child = child->next;
	}

	handle_click(btn);
	expose();
	return SDL_TRUE;
}

int click_entry(gui_entry *entry, const gui_button_event ev)
{
	if (ev.type != GUI_BUTTON_PRESSED)
		return SDL_TRUE;

	clear_submenus((gui_button *)entry);

	gui_entry_focused = entry;

	expose();
	return SDL_TRUE;
}

int gui_entry_accept(SDL_Event *event)
{
	int i, len;

	if (!gui_entry_focused)
		return SDL_FALSE;

	len = strlen(gui_entry_focused->text);

	if (event->type == SDL_TEXTINPUT) {
		if (len >= gui_entry_focused->maxlen)
			return SDL_TRUE;

		for (i = gui_entry_focused->maxlen + 1;
		     i > gui_entry_focused->cursor;
		     i--)
			gui_entry_focused->text[i] = gui_entry_focused->text[i-1];

		gui_entry_focused->text[gui_entry_focused->cursor++] =
			event->text.text[0];
		expose();
		return SDL_TRUE;
	}

	if (event->type != SDL_KEYDOWN)
		return SDL_FALSE;

	int ctl = event->key.keysym.mod & SDLK_LCTRL;
	int key = event->key.keysym.sym;

	if (key == SDLK_RETURN ||
	    key == SDLK_KP_ENTER) {
		enter_pressed();
		show_menu(SDL_FALSE);
		expose();
		return SDL_TRUE;
	}

	if (key == SDLK_LEFT) {
		if (gui_entry_focused->cursor > 0)
			gui_entry_focused->cursor--;
		expose();
		return SDL_TRUE;
	}

	if (key == SDLK_RIGHT) {
		if (gui_entry_focused->cursor < len)
			gui_entry_focused->cursor++;
		expose();
		return SDL_TRUE;
	}

	if (key == SDLK_BACKSPACE) {
		if (gui_entry_focused->cursor == 0)
			return SDL_TRUE;
		for (i = gui_entry_focused->cursor-1;
		     i < gui_entry_focused->maxlen;
		     i++)
			gui_entry_focused->text[i] = gui_entry_focused->text[i+1];
		gui_entry_focused->cursor--;
		expose();
		return SDL_TRUE;
	}

	if (key == SDLK_TAB) {
		gui_widget *iter = gui_entry_focused->next;
		while (iter) {
			if (iter->widget_type == GUI_WIDGET_TYPE_ENTRY) {
				gui_entry_focused = (gui_entry *)iter;
				expose();
				return SDL_TRUE;
			}
			iter = iter->next;
		}
		return SDL_TRUE;
	}

#if 0
	if (gui_entry_focused->cursor > len)
		gui_entry_focused->cursor = len;

	if (len >= gui_entry_focused->maxlen)
		return SDL_TRUE;

	for (i = gui_entry_focused->maxlen + 1;
	     i > gui_entry_focused->cursor;
	     i--)
		gui_entry_focused->text[i] = gui_entry_focused->text[i-1];

	gui_entry_focused->text[gui_entry_focused->cursor++] = key;
	expose();
#endif
	return SDL_TRUE;
}


void gui_print_tree(gui_widget* root, int level)
{
    gui_widget* node = root;

    while (node != 0) {
        printf("%d: node=%p parent=%p x=%d y=%d w=%d h=%d\n",
	       level, node, node->parent,
	       node->region.x, node->region.y,
	       node->region.w, node->region.h);
	if (node->widget_type == GUI_WIDGET_TYPE_BUTTON)
		printf("  button=%s\n", ((gui_button *)node)->text);
	else if (node->widget_type == GUI_WIDGET_TYPE_ENTRY)
		printf("  entry=%s\n", ((gui_entry *)node)->text);

        if (node->first_child) {
	    gui_print_tree(node->first_child, level + 1);
	}

	node = node->next;
    }
}

void update_menu(SDL_Surface *surface)
{
    struct timeval tp;
    struct timezone tzp;
    static int last = 0;

    if (!gui_entry_focused)
	    return;

    gettimeofday(&tp, &tzp);

    if (tp.tv_usec > 500000 && !last) {
	    last = 1;
	    gui_entry_focused->render_cb((gui_widget *)gui_entry_focused, surface);
    } else if (tp.tv_usec < 500000 && last) {
	    last = 0;
	    gui_entry_focused->render_cb((gui_widget *)gui_entry_focused, surface);
    }
}
