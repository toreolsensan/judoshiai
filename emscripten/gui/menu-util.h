#ifndef _MENU_UTIL_H_
#define _MENU_UTIL_H_

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <emscripten.h>

#include "widget.h"
#include "button.h"
#include "entry.h"

#define CHECKBOX_SIZE 12

#define ADD_MENU(_m, _id, _text, _flags) do {				\
		b = gui_button_new					\
			(_id, _text, fnt1, menu_color,			\
			 (_flags & GUI_BUTTONFLAG_CHECKBOX) ?		\
			 click_checkbox : ((_flags & GUI_BUTTONFLAG_RADIO) ? \
					   click_radio : click_dummy),	\
			 _flags | GUI_WIDGETFLAG_INVISIBLE);		\
		gui_widget_set_skinner((gui_widget *)b, menu_background); \
		gui_widget_add((gui_widget *)_m, (gui_widget *)b);	\
	} while (0)

extern gui_entry *gui_entry_focused;

void gui_buttons_to_menu(gui_button *button, int hor);
void set_submenu_visibility(gui_widget *parent, int visible);
void hide_submenus(gui_widget *mb);
void show_menu(int yes);
int menu_draw(gui_widget *w, SDL_Surface *s);
int menu_background(gui_widget *w, SDL_Surface *s);
int mouse_error_handler(gui_error_type err, struct gui_widget *w,
			const SDL_Event *ev, SDL_Surface *s, int errc);
int click_menu(gui_button *btn, const gui_button_event ev);
void clear_submenus(void *node);
int click_dummy(gui_button *btn, const gui_button_event ev);
int click_checkbox(gui_button *btn, const gui_button_event ev);
int click_radio(gui_button *btn, const gui_button_event ev);
int click_entry(gui_entry *entry, const gui_button_event ev);
void gui_print_tree(gui_widget* root, int level);
int gui_entry_accept(SDL_Event *event);
void update_menu(SDL_Surface *surface);
void set_mouse_coordinates(int x, int y);


/******************************/
/* User must implement these: */
/******************************/
extern int gui_max_x, gui_max_y;

extern void handle_click(gui_button *btn);
//extern void expose(void);
extern void enter_pressed(void);

/******************************/

#endif
