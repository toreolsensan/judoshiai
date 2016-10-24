#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "cairowrapper.h"

extern int gui_max_x, gui_max_y;

#define EXTRA_SPACE 6
#define SPACE_BW_LETTERS 4

gui_entry *
gui_entry_new(int id, const char *text, void *font,
	      SDL_Color color, int maxlen, gui_entry_click_callback clkcb,
	      int flags)
{
	gui_entry *entry = (gui_entry *)gui_widget_new
		(GUI_WIDGET_TYPE_ENTRY, id, sizeof(gui_entry),
		 gui_entry_default_event_handler,
		 gui_entry_default_render, gui_entry_destroy, flags);
	if (entry) {
		entry->onclick_cb = clkcb;
		entry->maxlen = maxlen;
		entry->cursor = strlen(text);
		entry->font = font;
		gui_entry_set_text(entry, text);

		SDL_Surface *s = TTF_RenderText_Solid(font, "H", color);
		entry->letter_w = s->w/* + SPACE_BW_LETTERS*/;
		entry->region.w = entry->letter_w * maxlen + EXTRA_SPACE;
		entry->region.h = s->h + EXTRA_SPACE;
		SDL_FreeSurface(s);
	}
	return entry;
}

void gui_entry_destroy(gui_widget *entry)
{
	gui_widget_destroy((gui_widget *)entry);
}

extern gui_entry *gui_entry_focused;

int gui_entry_default_render(gui_widget *w,
			     SDL_Surface *surface)
{
	gui_entry *entry = (gui_entry *)w;
	int i, len = strlen(entry->text);
	SDL_Surface *s;
	SDL_Rect rect, bar;
	int focus = gui_entry_focused == entry;
	static int last = 0;

	if (entry->region.x + entry->region.w > gui_max_x)
		gui_max_x = entry->region.x + entry->region.w;
	if (entry->region.y + entry->region.h > gui_max_y)
		gui_max_y = entry->region.y + entry->region.h;

	rect.x = entry->region.x;
	rect.y = entry->region.y + 3;
	rect.w = entry->region.w;
	rect.h = entry->region.h;

	if (!focus)
		entry->cursor = len;

	rect.y++;

	for (i = 0; i < len; i++) {
		char t[2];
		t[0] = entry->text[i];
		t[1] = 0;
		s = TTF_RenderText_Solid(entry->font, t, entry->color);
		rect.x = entry->region.x + 3 + entry->letter_w*i;
		SDL_BlitSurface(s, NULL, surface, &rect);
	}

	if (focus) {
		bar.x = entry->region.x + 2 + entry->letter_w*entry->cursor;
		bar.y = entry->region.y + 5;
		bar.w = 2;
		bar.h = entry->region.h - 10;
		SDL_FillRect(surface, &bar,
			     SDL_MapRGB(surface->format, 0, last ? 0 : 200, 0));
	}
	last = !last;

	return 0;
}

int gui_entry_default_event_handler(gui_widget *w, const SDL_Event event)
{
	gui_entry *entry = (gui_entry *)w;
	gui_button_event bevent;

	if (gui_button_create_event(event, &bevent) == SDL_FALSE)
		return -1;

	if (bevent.x >= w->region.x &&
	    bevent.x <= (w->region.x+w->region.w) &&
	    bevent.y >= w->region.y &&
	    bevent.y <= (w->region.y+w->region.h)) {
		if (bevent.type == GUI_BUTTON_PRESSED) {
			if (entry->onclick_cb != 0) {
				return entry->onclick_cb(entry, bevent);
			}
		}
	}

	return 0;
}

void gui_entry_delete_text(gui_entry *entry)
{
	if (entry) {
		entry->text[0] = 0;
	}
}

void gui_entry_set_font(gui_entry *entry, void* font)
{
	if (entry) {
		entry->font = font;
	}
}

void gui_entry_set_text(gui_entry *entry, const char* text)
{
	entry->text[0] = 0;
	if (text) {
		strncpy(entry->text, text, sizeof(entry->text));
		entry->text[sizeof(entry->text)-1] = 0;
	}
}
