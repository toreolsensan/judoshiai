#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cairowrapper.h"

extern int gui_max_x, gui_max_y;

gui_button *
gui_button_new(int id, const char *text, void *font,
	       SDL_Color color, gui_button_click_callback clkcb,
	       int flags)
{
	gui_button *button = (gui_button *)gui_widget_new
		(GUI_WIDGET_TYPE_BUTTON, id, sizeof(gui_button),
		 gui_button_default_event_handler,
		 gui_button_default_render, gui_button_destroy2, flags);
	if (button) {
		button->onclick_cb = clkcb;
		button->cached_text = 0;
		gui_button_set_font(button, font);
		gui_button_set_text(button, text);

		if (text[0] != '-') {
			if (text[0] == '>') text++;

			button->cached_text =
				TTF_RenderText_Solid(font, text, color);
			button->region.w = button->cached_text->w + 4;
			button->region.h = button->cached_text->h + 4;
		} else {
			button->region.h = 5;
		}
		if (button->flags &
		    (GUI_BUTTONFLAG_CHECKBOX | GUI_BUTTONFLAG_RADIO))
			button->region.w += button->region.h;
	}
	return button;
}

void gui_button_destroy(gui_button *button)
{
	gui_widget_destroy((gui_widget *)button);
}

void gui_button_destroy2(gui_widget *widget)
{
	gui_button *btn = (gui_button*)widget;
	if (GUI_BUTTON_BASE_CHECK(btn)) {
		gui_button_delete_text(btn);
	}
}

int gui_button_default_render(gui_widget *btn,
			       SDL_Surface *surface)
{
	gui_button *button = (gui_button *)btn;

	if (GUI_BUTTON_BASE_CHECK(button)) {
		if (button->region.x + button->region.w > gui_max_x)
			gui_max_x = button->region.x + button->region.w;
		if (button->region.y + button->region.h > gui_max_y)
			gui_max_y = button->region.y + button->region.h;

		const char *txt = button->text;

		if (txt && txt[0] != '-') {
			SDL_Rect rect;
			rect.x = button->region.x + 2;
			rect.y = button->region.y + 2;
			rect.w = button->region.w;
			rect.h = button->region.h;

			if (txt[0] == '>') txt++;

			if (!button->cached_text) {
				button->cached_text = TTF_RenderText_Solid
					(button->font,
					 txt,
					 button->color);
				if (button->region.w == 0) {
					button->region.w = button->cached_text->w + 4;
					button->region.h = button->cached_text->h + 4;
				}
			}
			SDL_BlitSurface(button->cached_text,
					NULL, surface,
					&rect);

			if (button->text[0] == '>') {
				SDL_Rect rect;
				//button->region.w += button->region.h;
				rect.x = button->region.x +
					button->region.w -
					button->region.h/2;
				rect.y = button->region.y + 4;
				rect.w = 2;
				rect.h = button->region.h - 8;
				while (rect.h > 0) {
					SDL_FillRect(surface, &rect,
						     SDL_MapRGB(surface->format, 0, 0, 0));
					rect.x++;
					rect.y++;
					rect.h -= 2;
				}
			}
		} else {
			SDL_Rect rect;
			rect.x = button->region.x + 2;
			rect.y = button->region.y + button->region.h / 2 - 1;
			rect.w = button->region.w - 4;
			rect.h = 1;
			SDL_FillRect(surface, &rect,
				     SDL_MapRGB(surface->format, 0, 0, 0));
		}

		if (button->flags &
		    (GUI_BUTTONFLAG_CHECKBOX | GUI_BUTTONFLAG_RADIO)) {
			SDL_Rect rect;
			rect.x = button->region.x +
				button->region.w -
				button->region.h + 2;
			rect.y = button->region.y + 2;
			rect.w = button->region.h - 4;
			rect.h = button->region.h - 4;
			SDL_FillRect(surface, &rect,
				     SDL_MapRGB(surface->format, 0, 0, 0));
			rect.x++; rect.y++; rect.w -= 2; rect.h -= 2;
			SDL_FillRect(surface, &rect,
				     SDL_MapRGB(surface->format, 255, 255, 255));

			if (button->selected) {
				rect.x += 3; rect.y += 3; rect.w -= 6; rect.h -= 6;
				SDL_FillRect(surface, &rect,
					     SDL_MapRGB(surface->format, 0, 0, 0));
			}
		}

		return 0;
	}
	return -1;
}

SDL_bool gui_button_create_event(const SDL_Event ev, gui_button_event* bev)
{
	if (bev == 0)
		return SDL_FALSE;

	switch (ev.type) {
        case SDL_MOUSEBUTTONDOWN:
		bev->type = GUI_BUTTON_PRESSED;
		bev->x = ev.button.x;
		bev->y = ev.button.y;
		bev->mbtn = ev.button.button;
		return SDL_TRUE;
        case SDL_MOUSEBUTTONUP:
		bev->type = GUI_BUTTON_RELEASED;
		bev->x = ev.button.x;
		bev->y = ev.button.y;
		bev->mbtn = ev.button.button;
		return SDL_TRUE;
        case SDL_FINGERDOWN:
		bev->type = GUI_BUTTON_PRESSED;
		bev->x = ev.tfinger.x;
		bev->y = ev.tfinger.y;
		bev->mbtn = ev.tfinger.fingerId;
		return SDL_TRUE;
        case SDL_FINGERUP:
		bev->type = GUI_BUTTON_RELEASED;
		bev->x = ev.tfinger.x;
		bev->y = ev.tfinger.y;
		bev->mbtn = ev.tfinger.fingerId;
		return SDL_TRUE;
        default:
		return SDL_FALSE;
	}
	return SDL_FALSE;
}

int gui_button_default_event_handler(gui_widget *btn, const SDL_Event event)
{
	gui_button *button = (gui_button*)btn;

	if (GUI_BUTTON_BASE_CHECK(button)) {
		gui_button_event bevent;
		if (gui_button_create_event(event, &bevent) == SDL_TRUE) {
			if (bevent.x >= btn->region.x &&
			    bevent.x <= (btn->region.x+btn->region.w) &&
			    bevent.y >= btn->region.y &&
			    bevent.y <= (btn->region.y+btn->region.h)) {
				if (!(((bevent.type == GUI_BUTTON_PRESSED) &&
				       button->is_pressed) ||
				      ((bevent.type == GUI_BUTTON_RELEASED) &&
				       !button->is_pressed)) ) {
					if (bevent.type == GUI_BUTTON_PRESSED)
						button->is_pressed = SDL_TRUE;
					else if (bevent.type == GUI_BUTTON_RELEASED)
						button->is_pressed = SDL_FALSE;

					if (button->onclick_cb != 0) {
						return button->onclick_cb(button, bevent);
					}
				}/* event already handled */
			}/* not on button */
		}
		return 0;
	} /* not a button */
	SDL_SetError("[gui_button_default_event_handler]: no widget provided or button is not a widget.");
	return -1;
}

void gui_button_delete_text(gui_button *button)
{
	if (button) {
		if (button->text) {
			SDL_free(button->text);
			button->text = 0;
		}
		if (button->cached_text) {
			SDL_FreeSurface(button->cached_text);
			button->cached_text = 0;
		}
	}
}

void gui_button_set_font(gui_button *button, void* font)
{
	if (button) {
		button->font = font;
		if (button->cached_text) {
			SDL_FreeSurface(button->cached_text);
			button->cached_text = 0;
		}
	}
}

void gui_button_set_text(gui_button *button, const char* text)
{
	if (button->text) {
		SDL_free(button->text);
		button->text = 0;
	}

	if (text)
		button->text = strdup(text);

	if (button->cached_text) {
		SDL_FreeSurface(button->cached_text);
		button->cached_text = 0;
	}
}
