#ifndef _BUTTON_H_
#define _BUTTON_H_

struct gui_button;

typedef enum gui_button_flags
{
    GUI_BUTTONFLAG_CHECKBOX     = 0x0100,
    GUI_BUTTONFLAG_RADIO        = 0x0200,
    GUI_BUTTONFLAG_BUTTON       = 0x0400,
} gui_button_flags;

#define GUI_BUTTON_PRESSED 0x01
#define GUI_BUTTON_RELEASED 0x02
typedef struct gui_button_event {
	int type;
	int x;
	int y;
	int mbtn;
} gui_button_event;

typedef int (*gui_button_click_callback)(struct gui_button *, const gui_button_event);

#define GUI_BUTTON_SUBTYPE_NORMAL   0
#define GUI_BUTTON_SUBTYPE_CHECKBOX 1

typedef struct gui_button {
	GUI_WIDGET_BASE_MEMBERS

	int subtype;
	int group;
	int selected;

	char *text;
	void *font;
	SDL_Surface *cached_text;
	SDL_Color color;
	int options;

	gui_button_click_callback onclick_cb;
	SDL_bool is_pressed;
} gui_button;

#define GUI_BUTTON_BASE_CHECK(btn) (btn && btn->widget_type == GUI_WIDGET_TYPE_BUTTON)

gui_button *gui_button_new(int id, const char *text, void *font,
			   SDL_Color color,
			   gui_button_click_callback clkcb,
			   int flags);
void gui_button_destroy(gui_button *button);
void gui_button_destroy2(gui_widget *widget);
SDL_bool gui_button_create_event(const SDL_Event ev, gui_button_event* bev);
void gui_button_delete_text(gui_button *button);
void gui_button_set_font(gui_button *button, void* font);
void gui_button_set_text(gui_button *button, const char* text);

static inline void gui_button_set_pressed(gui_button *button, SDL_bool state)
{
	if (GUI_BUTTON_BASE_CHECK(button))
		button->is_pressed = state;
}

static inline void gui_button_set_label(gui_button *button,
					 const char *text, void *font)
{
	if (GUI_BUTTON_BASE_CHECK(button)) {
		gui_button_delete_text(button);
		gui_button_set_font(button, font);
		gui_button_set_text(button, text);
	}
}

int gui_button_default_render(gui_widget *btn, SDL_Surface *renderer);
int gui_button_default_event_handler(gui_widget *btn, const SDL_Event event);

#endif /* _BUTTON_H_ */
