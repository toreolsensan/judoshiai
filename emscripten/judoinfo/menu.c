/* -*- mode: C; c-basic-offset: 4;  -*- */

/*
 * Copyright (C) 2006-2016 by Hannu Jokinen
 * Full copyright text is included in the software package.
 */

#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <locale.h>

#ifdef WIN32
#include <process.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif

#include "judoinfo.h"
#include "language.h"
#include "menu-util.h"

extern void get_bracket(int tatami);

extern gui_widget *menubar;

enum {
	MENU_NONE,
	MENU_TATAMI,
	MENU_DISPLAY_TYPE,
	MENU_BACKGROUND,
	MENU_MIRROR,
	MENU_BRACKET,
	MENU_BRACKET_SVG
};

extern int conf_tatamis[NUM_TATAMIS];
extern int configured_tatamis;
extern const int numlines[3];
extern int num_lines;
extern int display_type;
extern gboolean red_background;
extern gboolean mirror_display;
extern gboolean show_bracket;
extern int split_pros;
extern int bracket_svg;

static gui_button *tatami_btn[NUM_TATAMIS];
static gui_button *bracket_btn;
//static gui_button *svg_btn;
static char *cookie_copy;

char *cookie_get(char *name)
{
    static char buf[16];
    int i = 0;
    char *p = strstr(cookie_copy, name);

    if (!p) return NULL;
    while (*p && *p != '=') p++;
    while (*p && (*p == '=' || *p == ' ')) p++;
    while (*p > ' ') buf[i++] = *p++;
    buf[i] = 0;
    return buf;
}

int cookie_get_num(char *name)
{
    char *p = cookie_get(name);
    if (!p) return 0;
    return atoi(p);
}

void cookie_set(char *key, char *val)
{
    char buf[128];
    time_t now;
    struct tm *tm;

    int n = snprintf(buf, sizeof(buf),
		     "document.cookie=\"%s=%s",
		     key, val);

    time(&now);
    tm = localtime(&now);
    tm->tm_year++;
    strftime(&buf[n], sizeof(buf)-n, "; expires=%a, 01 %b %Y 11:00:00 UTC\"", tm);

    emscripten_run_script(buf);
}

void cookie_set_num(char *key, int val)
{
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", val);
    cookie_set(key, buf);
}

void save_conf(void)
{
    int j;
    char buf[16];
    cookie_set_num("split_pros", split_pros);
    cookie_set_num("display", display_type);
    cookie_set_num("background", red_background);
    cookie_set_num("mirror", mirror_display);
    cookie_set_num("bracket", show_bracket);
    cookie_set_num("bracketsvg", bracket_svg);
    for (j = 0; j < NUM_TATAMIS; j++) {
	snprintf(buf, sizeof(buf), "tatami%d", j+1);
	cookie_set_num(buf, conf_tatamis[j]);
    }
}

void restore_conf(void)
{
    int j;
    char buf[16];

    char *p, *cookie = emscripten_run_script_string("document.cookie");
    if (!cookie) return;
    if (cookie_copy) free(cookie_copy);
    cookie_copy = strdup(cookie);

    split_pros = cookie_get_num("split_pros");
    if (split_pros < 10) split_pros = 25;

    display_type = cookie_get_num("display");
    red_background = cookie_get_num("background");
    mirror_display = cookie_get_num("mirror");
    show_bracket = cookie_get_num("bracket");
    bracket_svg = cookie_get_num("bracketsvg");

    configured_tatamis = 0;
    for (j = 0; j < NUM_TATAMIS; j++) {
	conf_tatamis[j] = 0;
	snprintf(buf, sizeof(buf), "tatami%d", j+1);
	conf_tatamis[j] = cookie_get_num(buf);
	if (conf_tatamis[j])
	    configured_tatamis++;
    }
}

void handle_click(gui_button *btn)
{
    char buf[64];
    int i, j;

    switch (btn->widget_id) {
    case MENU_TATAMI:
	i = ptr_to_gint(btn->custom_user_data);

	if (show_bracket) {
	    for (j = 0; j < NUM_TATAMIS; j++)
		show_tatami[j] = conf_tatamis[j] = tatami_btn[j]->selected = i == j;
	    get_bracket(i + 1);
	    expose();
	    save_conf();
	    break;
	}

	conf_tatamis[i] = tatami_btn[i]->selected;
	configured_tatamis = 0;

	for (j = 0; j < NUM_TATAMIS; j++)
	    if ((conf_tatamis[j] = tatami_btn[j]->selected))
		configured_tatamis++;

	for (j = 0; j < NUM_TATAMIS; j++) {
	    if (configured_tatamis) show_tatami[j] = conf_tatamis[j];
	    else show_tatami[j] = match_list[j][1].blue && match_list[j][1].white;
	}
	//show_bracket = 0;
	//bracket_btn->selected = 0;
	expose();
	save_conf();
	break;
    case MENU_DISPLAY_TYPE:
	display_type = ptr_to_gint(btn->custom_user_data);
	num_lines = numlines[display_type];
	expose();
	save_conf();
	break;
    case MENU_BACKGROUND:
	red_background = !red_background;
	expose();
	save_conf();
	break;
    case MENU_MIRROR:
	mirror_display = !mirror_display;
	expose();
	save_conf();
	break;
    case MENU_BRACKET:
	show_bracket = !show_bracket;
	configured_tatamis = 0;

	for (j = 0; j < NUM_TATAMIS; j++)
	    if (tatami_btn[j]->selected)
		configured_tatamis++;

	if (configured_tatamis == 0) {
	    tatami_btn[0]->selected = 1;
	    conf_tatamis[0] = 1;
	}

	int found = 0;
	for (j = 0; j < NUM_TATAMIS; j++) {
	    if (found) {
		tatami_btn[j]->selected = 0;
		show_tatami[j] = conf_tatamis[j] = 0;
	    } else if (tatami_btn[j]->selected) {
		show_tatami[j] = conf_tatamis[j] = 1;
		found = j + 1;
		get_bracket(j + 1);
	    }
	}

	for (j = 0; j < NUM_TATAMIS; j++)
	    show_tatami[j] = conf_tatamis[j];

	configured_tatamis = 1;
	expose();
	save_conf();
	break;
    case MENU_BRACKET_SVG:
	bracket_svg = !bracket_svg;
	expose();
	save_conf();
	break;
    }
}

extern time_t fsrequest;

int EMSCRIPTEN_KEEPALIVE change_to_svg_mode(int yes, int exp)
{
    fsrequest = time(NULL);
    /*svg_btn->selected =*/ bracket_svg = yes;
    if (exp)
	expose();
    save_conf();
    return 0;
}


void enter_pressed(void)
{
    if (!gui_entry_focused)
	return;
#if 0
    switch (gui_entry_focused->widget_id) {
    case MENU_SET_CLOCKS_RET:
	set_clocks_from_widgets();
	break;
    }
#endif
}

gui_button *add_button(gui_widget *parent, int id, const char *text,
		       TTF_Font *font, SDL_Color color, int flags, int hor)
{
    gui_button *b;
    b = gui_button_new(id, text, font, color,
		       (flags & GUI_BUTTONFLAG_CHECKBOX) ?
		       click_checkbox : ((flags & GUI_BUTTONFLAG_RADIO) ?
					 click_radio : click_dummy),
		       flags | GUI_WIDGETFLAG_INVISIBLE);
    b->region.x += 4;
    if (hor)
	b->region.x += parent->region.w;
    else
	b->region.y += parent->region.h;

    gui_widget_set_skinner((gui_widget *)b, menu_background);
    gui_widget_add((gui_widget *)parent, (gui_widget *)b);

    return b;
}

gui_entry *add_entry(gui_widget *parent, int id, const char *text,
		     TTF_Font *font, SDL_Color color, int maxlen, int flags, int hor)
{
    gui_entry *e;

    e = gui_entry_new(id, text, font, color, maxlen, click_entry,
		      GUI_WIDGETFLAG_INVISIBLE);
    gui_widget_set_skinner((gui_widget *)e, menu_background);
    gui_widget_add((gui_widget *)parent, (gui_widget *)e);

    e->region.x += 4;
    if (hor)
	e->region.x += parent->region.w;
    else
	e->region.y += parent->region.h;

    return e;
}

static const char *dsptypes[] = {
    "Normal", "Small", "Horizontal", NULL
};

gui_widget *get_menubar_menu(SDL_Surface *s)
{
    gui_widget *menubar = gui_widget_new(0, 0, 0, 0, menu_draw, 0, 0);
    int i;

    SDL_Color menu_color;
    menu_color.r = 0;
    menu_color.b = 0;
    menu_color.g = 0;

    TTF_Font* fnt1 = get_font(16, 0);
    TTF_Font* fnt2 = get_font(20, 0);
    gui_button *menu_pref = gui_button_new
	(0, " Preferences ", fnt1, menu_color, click_menu, 0);
    gui_button *menu_help = gui_button_new
	(0, " Help ", fnt1, menu_color, click_menu, 0);

    restore_conf();

    gui_widget_set_skinner((gui_widget *)menu_pref, menu_background);
    gui_widget_set_skinner((gui_widget *)menu_help, menu_background);

    gui_widget_add(menubar, (gui_widget *)menu_pref);
    gui_widget_add(menubar, (gui_widget *)menu_help);

    gui_buttons_to_menu(menu_pref, 1);

    gui_button *b;

    ADD_MENU(menu_pref, MENU_NONE, ">Tatami", 0);
    b->region.x = menu_pref->region.x;
    b->region.y = menu_pref->region.y + menu_pref->region.h + 4;
    gui_button *tat = b;

    ADD_MENU(menu_pref, MENU_NONE, ">Display", 0);
    gui_button *dsp = b;

    ADD_MENU(menu_pref, MENU_BACKGROUND, "Red background", GUI_BUTTONFLAG_CHECKBOX);
    b->selected = red_background;

    ADD_MENU(menu_pref, MENU_MIRROR, "Mirror", GUI_BUTTONFLAG_CHECKBOX);
    b->selected = mirror_display;

    ADD_MENU(menu_pref, MENU_BRACKET, "Bracket", GUI_BUTTONFLAG_CHECKBOX);
    b->selected = show_bracket;
    bracket_btn = b;

    //ADD_MENU(menu_pref, MENU_BRACKET_SVG, "SVG bracket", GUI_BUTTONFLAG_CHECKBOX);
    //b->selected = bracket_svg;
    //svg_btn = b;

    gui_buttons_to_menu(menu_pref->first_child, 0);

    /* Tatami */
    for (i = 0; i < NUM_TATAMIS; i++) {
	char buf[8];
	sprintf(buf, "Tatami %d", i+1);
	ADD_MENU(tat, MENU_TATAMI, buf, GUI_BUTTONFLAG_CHECKBOX);
	tatami_btn[i] = b;
	if (i == 0) {
	    b->region.x = tat->region.x + tat->region.w + 4;
	    b->region.y = tat->region.y;
	}
	b->custom_user_data = gint_to_ptr(i);
	b->selected = show_tatami[i] = conf_tatamis[i];
	b->group = 3;
    }
    gui_buttons_to_menu(tat->first_child, 0);

    /* Display layout */
    for (i = 0; dsptypes[i]; i++) {
	ADD_MENU(dsp, MENU_DISPLAY_TYPE, dsptypes[i], GUI_BUTTONFLAG_RADIO);
	if (i == 0) {
	    b->region.x = dsp->region.x + dsp->region.w + 4;
	    b->region.y = dsp->region.y;
	}
	b->custom_user_data = gint_to_ptr(i);
	if (display_type == i)
	    b->selected = 1;
	b->group = 1;
    }
    gui_buttons_to_menu(dsp->first_child, 0);

    hide_submenus(menubar);

    //gui_print_tree((gui_widget *)menubar, 0);

    return menubar;
}

void set_menu_values(void)
{
}
