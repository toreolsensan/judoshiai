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

#include "judotimer.h"
#include "language.h"
#include "menu-util.h"

extern gui_widget *menubar;

enum {
	MENU_NONE,
	MENU_NEW,
	MENU_GS,
	MENU_HANTEI,
	MENU_HANSOKUMAKE,
	MENU_HIKIWAKE,
	MENU_SHOW_COMP,
	MENU_TATAMI,
	MENU_DISPLAY_LAYOUT,
	MENU_BACKGROUND,
	MENU_AUDIO,
	MENU_IPPON_STOPS_CLOCK,
	MENU_CONFIRM_NEW_CONTEST,
	MENU_NO_TEXTS,
	MENU_SET_CLOCKS,
	MENU_SET_CLOCKS_RET
};

extern gint dsp_layout;
extern int tatami;
extern int audio;
extern int matchtime, hansokumake;
extern int goldenscore, hantei;

static gui_entry *wmin, *wsec, *wosec;

static const char *matchtimetext[] = {
	"Automatic", "2 min (kids)", "2 min", "3 min", "4 min", "5 min", NULL
};
static const char *gstext[] = {
	"Auto", "No limit", "1 min", "2 min", "3 min", "4 min", "5 min", NULL
};
static double gstime[] = {
    1000.0, 0.0, 10.0, 120.0, 180.0, 240.0, 300.0
};
static const char *hanteitext[] = {
    " ", "White wins", "Blue wins"
};
static const char *hansokumaketext[] = {
    " ", "To white", "To blue"
};
static const char *audiotext[] = {
    "None", "AirHorn", "BikeHorn", "CarAlarm", "Doorbell",
    "IndutrialAlarm", "IntruderAlarm", "RedAlert", "TrainHorn", "TwoToneDoorbell",
    NULL
};

static void set_clocks_from_widgets(void)
{
    if (wmin && wsec && wosec) {
	set_clocks(atoi(wmin->text)*60 + atoi(wsec->text),
		   atoi(wosec->text));

	expose();
    }
}

void handle_click(gui_button *btn)
{
    char buf[64];

    switch (btn->widget_id) {
    case MENU_NEW:
	matchtime = ptr_to_gint(btn->custom_user_data);
	hide_submenus(menubar);
	clock_key(SDLK_0 + matchtime, 0);
	break;
    case MENU_GS:
	goldenscore = ptr_to_gint(btn->custom_user_data);
	hide_submenus(menubar);
	set_gs(gstime[goldenscore]);
	clock_key(SDLK_9, 0);
	break;
    case MENU_HANTEI:
    case MENU_HANSOKUMAKE:
    case MENU_HIKIWAKE:
	hide_submenus(menubar);
	voting_result(NULL, btn->custom_user_data);
	break;
    case MENU_SHOW_COMP:
	show_competitor_names = btn->selected;
	break;
    case MENU_TATAMI:
	tatami = ptr_to_gint(btn->custom_user_data);
	break;
    case MENU_DISPLAY_LAYOUT:
	dsp_layout = ptr_to_gint(btn->custom_user_data);
	select_display_layout(NULL, gint_to_ptr(dsp_layout));
	break;
    case MENU_BACKGROUND:
	toggle_color(btn, 0);
	break;
    case MENU_AUDIO:
	audio = ptr_to_gint(btn->custom_user_data);
	snprintf(buf, sizeof(buf), "audio = new Audio('%s.mp3')",
		 audiotext[audio]);
	emscripten_run_script(buf);
	emscripten_run_script("audio.play()");
	break;
    case MENU_IPPON_STOPS_CLOCK:
	rules_stop_ippon_2 = btn->selected;
	break;
    case MENU_CONFIRM_NEW_CONTEST:
	rules_confirm_match = btn->selected;
	break;
    case MENU_NO_TEXTS:
	no_big_text = btn->selected;
	break;
    case MENU_SET_CLOCKS:
	set_clocks_from_widgets();
	break;
    }
}

void enter_pressed(void)
{
    if (!gui_entry_focused)
	return;

    switch (gui_entry_focused->widget_id) {
    case MENU_SET_CLOCKS_RET:
	set_clocks_from_widgets();
	break;
    }
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
	gui_button *menu_file = gui_button_new
		(0, " File ", fnt1, menu_color, click_menu, 0);
	gui_button *menu_pref = gui_button_new
		(0, " Preferences ", fnt1, menu_color, click_menu, 0);
	gui_button *menu_help = gui_button_new
		(0, " Help ", fnt1, menu_color, click_menu, 0);

	gui_widget_set_skinner((gui_widget *)menu_file, menu_background);
	gui_widget_set_skinner((gui_widget *)menu_pref, menu_background);
	gui_widget_set_skinner((gui_widget *)menu_help, menu_background);

	gui_widget_add(menubar, (gui_widget *)menu_file);
	gui_widget_insert_after((gui_widget *)menu_file, (gui_widget *)menu_pref);
	gui_widget_insert_after((gui_widget *)menu_pref, (gui_widget *)menu_help);

	gui_buttons_to_menu(menu_file, 1);

	gui_button *b;

	ADD_MENU(menu_file, MENU_NONE, ">New", 0);
	b->region.x = menu_file->region.x;
	b->region.y = menu_file->region.y + menu_file->region.h + 4;
	gui_button *mt = b;

	ADD_MENU(menu_file, MENU_NONE, "-", 0);
	ADD_MENU(menu_file, MENU_NONE, ">Golden score", 0);
	gui_button *gs = b;

	ADD_MENU(menu_file, MENU_NONE, "-", 0);
	ADD_MENU(menu_file, MENU_NONE, ">Hantei", 0);
	gui_button *han = b;

	ADD_MENU(menu_file, MENU_NONE, ">Hansokumake", 0);
	gui_button *hkm = b;

	ADD_MENU(menu_file, MENU_HIKIWAKE, "Hikiwake", 0);
	ADD_MENU(menu_file, MENU_NONE, "-", 0);

	ADD_MENU(menu_file, MENU_SHOW_COMP, "Show competitors", 0);
	if (show_competitor_names)
		b->selected = SDL_TRUE;

	ADD_MENU(menu_file, MENU_NONE, ">Set clocks", 0);
	gui_button *clk = b;

	gui_buttons_to_menu(menu_file->first_child, 0);

	ADD_MENU(menu_pref, MENU_NONE, ">Tatami", 0);
	b->region.x = menu_pref->region.x;
	b->region.y = menu_pref->region.y + menu_pref->region.h + 4;
	gui_button *tat = b;

	ADD_MENU(menu_pref, MENU_NONE, ">Display layout", 0);
	gui_button *dsp = b;
	ADD_MENU(menu_pref, MENU_NONE, ">Audio", 0);
	gui_button *aud = b;
	ADD_MENU(menu_pref, MENU_BACKGROUND, "Red background", GUI_BUTTONFLAG_CHECKBOX);
	ADD_MENU(menu_pref, MENU_IPPON_STOPS_CLOCK, "Ippon stops clock", GUI_BUTTONFLAG_CHECKBOX);
	if (rules_stop_ippon_2)
		b->selected = SDL_TRUE;

	ADD_MENU(menu_pref, MENU_CONFIRM_NEW_CONTEST, "Confirm new contest", GUI_BUTTONFLAG_CHECKBOX);
	if (rules_confirm_match)
		b->selected = SDL_TRUE;

	ADD_MENU(menu_pref, MENU_NO_TEXTS, "No texts", GUI_BUTTONFLAG_CHECKBOX);
	if (no_big_text)
		b->selected = SDL_TRUE;

	gui_buttons_to_menu(menu_pref->first_child, 0);

	/* Tatami */
	for (i = 0; i < 10; i++) {
		char buf[8];
		sprintf(buf, "Tatami %d", i+1);
		ADD_MENU(tat, MENU_TATAMI, buf, GUI_BUTTONFLAG_RADIO);
		if (i == 0) {
			b->region.x = tat->region.x + tat->region.w + 4;
			b->region.y = tat->region.y;
		}
		b->custom_user_data = gint_to_ptr(i + 1);
		if (tatami == (i+1))
		    b->selected = 1;
		b->group = 3;
	}
	gui_buttons_to_menu(tat->first_child, 0);

	/* Display layout */
	for (i = 0; i < 7; i++) {
		char buf[8];
		sprintf(buf, "Layout %d", i+1);
		ADD_MENU(dsp, MENU_DISPLAY_LAYOUT, buf, GUI_BUTTONFLAG_RADIO);
		if (i == 0) {
			b->region.x = dsp->region.x + dsp->region.w + 4;
			b->region.y = dsp->region.y;
		}
		b->custom_user_data = gint_to_ptr(i + 1);
		if (dsp_layout == (i+1))
			b->selected = 1;
		b->group = 1;
	}
	gui_buttons_to_menu(dsp->first_child, 0);

	hide_submenus(menubar);

	/* Audio */
	for (i = 0; audiotext[i]; i++) {
		ADD_MENU(aud, MENU_AUDIO, audiotext[i], GUI_BUTTONFLAG_RADIO);
		if (i == 0) {
			b->region.x = aud->region.x + aud->region.w + 4;
			b->region.y = aud->region.y;
		}
		b->custom_user_data = gint_to_ptr(i);
		if (audio == i)
			b->selected = 1;
		b->group = 2;
	}
	gui_buttons_to_menu(aud->first_child, 0);

	/* Matchtime */
	for (i = 0; matchtimetext[i]; i++) {
		ADD_MENU(mt, MENU_NEW, matchtimetext[i], 0);
		if (i == 0) {
			b->region.x = mt->region.x + mt->region.w + 4;
			b->region.y = mt->region.y;
		}
		b->custom_user_data = gint_to_ptr(i);
	}
	gui_buttons_to_menu(mt->first_child, 0);

	/* Golden score */
	for (i = 0; gstext[i]; i++) {
		ADD_MENU(gs, MENU_GS, gstext[i], 0);
		if (i == 0) {
			b->region.x = gs->region.x + gs->region.w + 4;
			b->region.y = gs->region.y;
		}
		b->custom_user_data = gint_to_ptr(i);
		if (goldenscore == i)
			b->selected = 1;
		b->group = 5;
	}
	gui_buttons_to_menu(gs->first_child, 0);

	/* Hantei */
	ADD_MENU(han, MENU_HANTEI, "White wins", 0);
	b->region.x = han->region.x + han->region.w + 4;
	b->region.y = han->region.y;
	b->custom_user_data = gint_to_ptr(HANTEI_BLUE);
	ADD_MENU(han, MENU_HANTEI, "Blue wins", 0);
	b->custom_user_data = gint_to_ptr(HANTEI_WHITE);
	gui_buttons_to_menu(han->first_child, 0);

	/* Hansokumake */
	ADD_MENU(hkm, MENU_HANSOKUMAKE, "Hansokumake to white", 0);
	b->region.x = hkm->region.x + hkm->region.w + 4;
	b->region.y = hkm->region.y;
	b->custom_user_data = gint_to_ptr(HANSOKUMAKE_BLUE);
	ADD_MENU(hkm, MENU_HANSOKUMAKE, "Hansokumake to blue", 0);
	b->custom_user_data = gint_to_ptr(HANSOKUMAKE_WHITE);
	gui_buttons_to_menu(hkm->first_child, 0);

	/* Set clocks */
	gui_button *t1 = add_button((gui_widget *)clk, MENU_NONE, "Clock:",
				    fnt1, menu_color, 0, 1);

	gui_button *pad = add_button((gui_widget *)clk, MENU_NONE, " ",
				     fnt1, menu_color, 0, 1);

	wmin = add_entry((gui_widget *)clk, MENU_SET_CLOCKS_RET, "0",
			 fnt1, menu_color, 1, 0, 1);

	gui_button *t2 = add_button((gui_widget *)clk, MENU_NONE, ":",
				    fnt1, menu_color, 0, 1);

	wsec = add_entry((gui_widget *)clk, MENU_SET_CLOCKS_RET, "00",
			 fnt1, menu_color, 2, 0, 1);

	gui_button *t3 = add_button((gui_widget *)clk, MENU_NONE, "Osaekomi:",
				    fnt1, menu_color, 0, 1);

	wosec = add_entry((gui_widget *)clk, MENU_SET_CLOCKS_RET, "00",
			  fnt1, menu_color, 2, 0, 1);

	gui_buttons_to_menu(t1, 1);
	t3->region.x = t1->region.x;
	t3->region.y = t1->region.y + t1->region.h;
	gui_buttons_to_menu(t3, 1);

	int r1_w = t1->region.w + wmin->region.w + t2->region.w +
	    wsec->region.w + pad->region.w;
	int r2_w = t3->region.w + wosec->region.w;
	pad->region.w += r2_w - r1_w;
	wmin->region.x += r2_w - r1_w;
	t2->region.x += r2_w - r1_w;
	wsec->region.x += r2_w - r1_w;

	gui_button *ok = add_button((gui_widget *)clk, MENU_SET_CLOCKS, "OK",
				    fnt2, menu_color, GUI_BUTTONFLAG_BUTTON, 1);
	ok->region.x = wsec->region.x + wsec->region.w;
	ok->region.y = wsec->region.y;

	//gui_print_tree((gui_widget *)clk, 0);

	return menubar;
}

void set_menu_values(void)
{
    char buf[8];
    int t, min, sec, oSec;

    t = get_elap();
    min = t / 60;
    sec =  t - min*60;
    oSec = get_oelap();

    sprintf(buf, "%d", min);
    gui_entry_set_text(wmin, buf);
    sprintf(buf, "%02d", sec);
    gui_entry_set_text(wsec, buf);
    sprintf(buf, "%02d", oSec);
    gui_entry_set_text(wosec, buf);
}
