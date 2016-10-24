#ifndef _ENTRY_H_
#define _ENTRY_H_

#define GUI_ENTRY_TEXT_LEN 64

struct gui_entry;

typedef int (*gui_entry_click_callback)(struct gui_entry *, const gui_button_event);

typedef struct gui_entry {
	GUI_WIDGET_BASE_MEMBERS

	char text[GUI_ENTRY_TEXT_LEN];
	void *font;
	int cursor;
	int maxlen;
	int letter_w;
	SDL_Color color;
	gui_entry_click_callback onclick_cb;
} gui_entry;

gui_entry *gui_entry_new(int id, const char *text, void *font,
			 SDL_Color color, int maxlen,
			 gui_entry_click_callback clkcb,
			 int flags);
void gui_entry_destroy(gui_widget *entry);
void gui_entry_delete_text(gui_entry *entry);
void gui_entry_set_font(gui_entry *entry, void* font);
void gui_entry_set_text(gui_entry *entry, const char* text);

static inline void gui_entry_set_label(gui_entry *entry,
					 const char *text, void *font)
{
	gui_entry_delete_text(entry);
	gui_entry_set_font(entry, font);
	gui_entry_set_text(entry, text);
}

int gui_entry_default_render(gui_widget *btn, SDL_Surface *renderer);
int gui_entry_default_event_handler(gui_widget *btn, const SDL_Event event);

#endif /* _ENTRY_H_ */
