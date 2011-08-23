/* -*- mode: C; c-basic-offset: 4;  -*- */

/*
 * Copyright (C) 2006-2011 by Hannu Jokinen
 * Full copyright text is included in the software package.
 */

#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <locale.h>

#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gthread.h>
#include <gdk/gdkkeysyms.h>
#ifdef WIN32
#include <glib/gwin32.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif

#include "judotimer.h"
#include "binreloc.h"
#include "fmod.h"

static void show_big(void);
static void expose_label(cairo_t *c, gint w);
static gboolean expose(GtkWidget *widget, GdkEventExpose *event, gpointer userdata);
gboolean delete_big(gpointer data);
gint language = LANG_FI;

//#define TABLE_PARAM (GTK_EXPAND)
#define TABLE_PARAM (GTK_EXPAND|GTK_FILL)

static gint dsp_layout = 2;
static gint selected_name_layout;
static const gchar *num2str[11] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "+"};
static const gchar *pts2str[6]  = {"-", "K", "Y", "W", "I", "S"};
guint current_year;
gchar *installation_dir = NULL;
GTimer *timer;
gboolean blue_wins_voting = FALSE, white_wins_voting = FALSE;
gboolean hansokumake_to_blue = FALSE, hansokumake_to_white = FALSE;
static PangoFontDescription *font;
gint mode = 0;
GKeyFile *keyfile;
gchar *conffile;

static gboolean big_dialog = FALSE;
static gchar big_text[40];
static time_t big_end;

gboolean rules_no_koka_dsp = FALSE;
gboolean rules_leave_score = FALSE;
gboolean rules_stop_ippon_2 = FALSE;
gboolean rules_confirm_match = FALSE;
GdkCursor *cursor = NULL;
gboolean sides_switched = FALSE;
gboolean white_first = FALSE;

static const gchar *num_to_str(guint num)
{
        static gchar result[10];
        if (num < 10)
                return num2str[num];

        sprintf(result, "%d", num);
        return result;
}

static const gchar *pts_to_str(guint num)
{
        if (num < 6)
                return pts2str[num];

        return ".";
}

static gboolean delete_event( GtkWidget *widget,
                              GdkEvent  *event,
                              gpointer   data )
{
        /* If you return FALSE in the "delete_event" signal handler,
         * GTK will emit the "destroy" signal. Returning TRUE means
         * you don't want the window to be destroyed.
         * This is useful for popping up 'are you sure you want to quit?'
         * type dialogs. */

        g_print ("delete event occurred\n");

        /* Change TRUE to FALSE and the main window will be destroyed with
         * a "delete_event". */

        //return TRUE;
        return FALSE;
}

extern void print_stat(void);

/* Another callback */
void destroy( GtkWidget *widget,
	      gpointer   data )
{
	gsize length;
	gchar *inidata = g_key_file_to_data (keyfile,
					     &length,
					     NULL);
        print_stat();

	g_file_set_contents(conffile, inidata, length, NULL);
	g_free(inidata);
	g_key_file_free(keyfile);

        gtk_main_quit ();
}

#define CBFUNC(_f) static gboolean cb_##_f(GtkWidget *eventbox, GdkEventButton *event, void *param)

CBFUNC(match1)
{
        return FALSE;
}

CBFUNC(match2)
{
        return FALSE;
}

CBFUNC(blue_name_1)
{
        return FALSE;
}

CBFUNC(white_name_1)
{
        return FALSE;
}

CBFUNC(blue_name_2)
{
        return FALSE;
}

CBFUNC(white_name_2)
{
        return FALSE;
}

CBFUNC(blue_club)
{
        return FALSE;
}

CBFUNC(white_club)
{
        return FALSE;
}

CBFUNC(wazaari)
{
	if (big_dialog)
		delete_big(NULL);
        return FALSE;
}

CBFUNC(yuko)
{
	if (big_dialog)
		delete_big(NULL);
        return FALSE;
}

CBFUNC(koka)
{
	if (big_dialog)
		delete_big(NULL);
        return FALSE;
}

CBFUNC(shido)
{
	if (big_dialog)
		delete_big(NULL);
        return FALSE;
}

CBFUNC(padding)
{
	if (big_dialog)
		delete_big(NULL);
        return FALSE;
}

CBFUNC(sonomama)
{
	if (big_dialog)
		delete_big(NULL);

        clock_key(GDK_s, 0);
        return FALSE;
}

CBFUNC(bw)
{
        if (set_osaekomi_winner(BLUE | GIVE_POINTS))
                return FALSE;

        clock_key(GDK_F1, (event->button == 3) ? 1 : (event->state & 1));
        return FALSE;
}

CBFUNC(by)
{
        if (set_osaekomi_winner(BLUE | GIVE_POINTS))
                return FALSE;

        clock_key(GDK_F2, (event->button == 3) ? 1 : (event->state & 1));
        return FALSE;
}

CBFUNC(bk)
{
        if (set_osaekomi_winner(BLUE | GIVE_POINTS))
                return FALSE;

        clock_key(GDK_F3, (event->button == 3) ? 1 : (event->state & 1));
        return FALSE;
}

CBFUNC(bs)
{
        if (set_osaekomi_winner(BLUE | GIVE_POINTS))
                return FALSE;

        clock_key(GDK_F4, (event->button == 3) ? 1 : (event->state & 1));
        return FALSE;
}

CBFUNC(ww)
{
        if (set_osaekomi_winner(WHITE | GIVE_POINTS))
                return FALSE;

        clock_key(GDK_F5, (event->button == 3) ? 1 : (event->state & 1));
        return FALSE;
}

CBFUNC(wy)
{
        if (set_osaekomi_winner(WHITE | GIVE_POINTS))
                return FALSE;

        clock_key(GDK_F6, (event->button == 3) ? 1 : (event->state & 1));
        return FALSE;
}

CBFUNC(wk)
{
        if (set_osaekomi_winner(WHITE | GIVE_POINTS))
                return FALSE;

        clock_key(GDK_F7, (event->button == 3) ? 1 : (event->state & 1));
        return FALSE;
}

CBFUNC(ws)
{
        if (set_osaekomi_winner(WHITE | GIVE_POINTS))
                return FALSE;

        clock_key(GDK_F8, (event->button == 3) ? 1 : (event->state & 1));
        return FALSE;
}

CBFUNC(t_min)
{
	clock_key(GDK_space, event->state & 1);
        return FALSE;
}

CBFUNC(colon)
{
	clock_key(GDK_space, event->state & 1);
        return FALSE;
}

CBFUNC(t_tsec)
{
	clock_key(GDK_space, event->state & 1);
        return FALSE;
}

CBFUNC(t_sec)
{
	clock_key(GDK_space, event->state & 1);
        return FALSE;
}

CBFUNC(o_tsec)
{
        clock_key(GDK_Return, event->state & 1);
        return FALSE;
}

CBFUNC(o_sec)
{
        clock_key(GDK_Return, event->state & 1);
        return FALSE;
}

CBFUNC(points)
{
	set_osaekomi_winner(0);
        return FALSE;
}

CBFUNC(comment)
{
        return FALSE;
}

CBFUNC(pts_to_blue)
{
        set_osaekomi_winner(BLUE);
        return FALSE;
}

CBFUNC(pts_to_white)
{
        set_osaekomi_winner(WHITE);
        return FALSE;
}

CBFUNC(cat1)
{
        return FALSE;
}

CBFUNC(cat2)
{
        return FALSE;
}

CBFUNC(gs)
{
        return FALSE;
}

CBFUNC(flag_blue)
{
        return FALSE;
}

CBFUNC(flag_white)
{
        return FALSE;
}


/* globals */
gchar *program_path;

GtkWidget     *main_vbox = NULL;
GtkWidget     *main_window = NULL;
gchar          current_directory[1024] = {0};
gint           my_address;
gboolean       clocks_only = FALSE;

static GtkWidget *darea = NULL;
static gint match1, match2, colon;
static gint blue_name_1, white_name_1;
static gint blue_name_2, white_name_2;
static gint blue_club, white_club;
static gint wazaari, yuko, koka, shido;
static gint bw, by, bk, bs;
static gint ww, wy, wk, ws;
static gint t_min, t_tsec, t_sec;
static gint o_tsec, o_sec, padding, sonomama;
static gint points, comment, cat1, cat2, gs;
static gint pts_to_blue, pts_to_white, flag_blue, flag_white;

static GdkColor color_yellow, color_white, color_grey, color_green, color_blue, color_red, color_black;
static GdkColor *bgcolor = &color_blue;

#define MY_LABEL(_x) _x

static double paper_width, paper_height;
#define W(_w) ((_w)*paper_width)
#define H(_h) ((_h)*paper_height)

#define LABEL_STATUS_EXPOSE 1
#define LABEL_STATUS_CHANGED 2
#define NUM_LABELS 40
struct label {
    gdouble x, y;
    gdouble w, h;
    gchar *text;
    gchar *text2;
    gdouble size;
    gint xalign;
    gdouble fg_r, fg_g, fg_b;
    gdouble bg_r, bg_g, bg_b;
    gboolean (*cb)(GtkWidget *, GdkEventButton *, void *);
    gchar status;
    gboolean wrap;
} labels[NUM_LABELS], defaults_for_labels[NUM_LABELS];
static gint num_labels = 0;

#define SET_COLOR(_w) do {			\
		set_fg_color(_w, 0, &fg);	\
		set_bg_color(_w, 0, &bg);	\
	} while (0)

#define GET_LABEL(_w, _l, _x, _y, _width, _height) do {		\
		labels[num_labels].x = _x;		\
		labels[num_labels].y = _y;		\
		labels[num_labels].w = _width;		\
		labels[num_labels].h = _height;		\
		labels[num_labels].cb = cb_##_w;	\
		labels[num_labels].text = g_strdup(_l); \
		labels[num_labels].text2 = 0;		\
		labels[num_labels].size = 0.0;		\
		labels[num_labels].fg_r = 1.0;		\
		labels[num_labels].fg_g = 1.0;		\
		labels[num_labels].fg_b = 1.0;		\
		labels[num_labels].bg_r = 0.0;		\
		labels[num_labels].bg_g = 0.0;		\
		labels[num_labels].bg_b = 0.0;		\
                labels[num_labels].status = 0;          \
                labels[num_labels].wrap = 0;            \
		_w = num_labels;			\
		num_labels++;				\
                printf("%d = %s\n", _w, #_w); } while (0)

static void set_fg_color(gint w, gint s, GdkColor *c)
{
	labels[w].fg_r = ((gdouble)c->red)/65536.0;
	labels[w].fg_g = ((gdouble)c->green)/65536.0;
	labels[w].fg_b = ((gdouble)c->blue)/65536.0;

        labels[w].status |= LABEL_STATUS_CHANGED;
}

static void set_bg_color(gint w, gint s, GdkColor *c)
{
	labels[w].bg_r = ((gdouble)c->red)/65536.0;
	labels[w].bg_g = ((gdouble)c->green)/65536.0;
	labels[w].bg_b = ((gdouble)c->blue)/65536.0;

        labels[w].status |= LABEL_STATUS_CHANGED;
}

static void set_text(gint w, const gchar *text)
{
	g_free(labels[w].text);
	labels[w].text = g_strdup(text);

        labels[w].status |= LABEL_STATUS_CHANGED;
}

static void set_text2(gint w, const gchar *text)
{
	g_free(labels[w].text2);
	labels[w].text2 = g_strdup(text);

        labels[w].status |= LABEL_STATUS_CHANGED;
}

static gchar *get_text(gint w)
{
	return labels[w].text;
}

void set_timer_run_color(gboolean running)
{
        GdkColor *color;

        if (running) {
		if (rest_time)
			color = &color_red;
		else
			color = &color_white;
        } else
                color = &color_yellow;

        set_fg_color(t_min, GTK_STATE_NORMAL, color);
        set_fg_color(colon, GTK_STATE_NORMAL, color);
        set_fg_color(t_tsec, GTK_STATE_NORMAL, color);
        set_fg_color(t_sec, GTK_STATE_NORMAL, color);

	expose_label(NULL, t_min);
	expose_label(NULL, colon);
	expose_label(NULL, t_tsec);
	expose_label(NULL, t_sec);
}

void set_timer_osaekomi_color(gint osaekomi_state, gint pts)
{
        GdkColor *fg = &color_white, *bg = &color_black, *color = &color_green;

	if (pts) {
		if (osaekomi_state == OSAEKOMI_DSP_BLUE ||
		    osaekomi_state == OSAEKOMI_DSP_WHITE ||
		    osaekomi_state == OSAEKOMI_DSP_UNKNOWN ||
		    osaekomi_state == OSAEKOMI_DSP_NO) {
			gint b = bk, w = wk;
			gdouble sb, sw;

			if (rules_no_koka_dsp) {
				switch (pts) {
				case 2: b = bk; w = wk; break;
				case 3: b = by; w = wy; break;
				case 4: b = bw; w = ww; break;
				}
			} else {
				switch (pts) {
				case 1: b = bk; w = wk; break;
				case 2: b = by; w = wy; break;
				case 3: b = bw; w = ww; break;
				}
			}

			sb = labels[b].size;
			sw = labels[w].size;

			if (osaekomi_state == OSAEKOMI_DSP_BLUE ||
			    osaekomi_state == OSAEKOMI_DSP_UNKNOWN)
				labels[b].size = 0.5;
			if (osaekomi_state == OSAEKOMI_DSP_WHITE ||
			    osaekomi_state == OSAEKOMI_DSP_UNKNOWN)
				labels[w].size = 0.5;

			expose_label(NULL, b);
			expose_label(NULL, w);
			labels[b].size = sb;
			labels[w].size = sw;
		}
	} else {
		expose_label(NULL, bw);
		expose_label(NULL, by);
		expose_label(NULL, bk);
		expose_label(NULL, ww);
		expose_label(NULL, wy);
		expose_label(NULL, wk);
	}

	switch (osaekomi_state) {
	case OSAEKOMI_DSP_NO:
		fg = &color_grey;
		bg = &color_black;
		break;
	case OSAEKOMI_DSP_YES:
		fg = &color_green;
		bg = &color_black;
		break;
	case OSAEKOMI_DSP_BLUE:
		fg = &color_white;
		bg = bgcolor;
		break;
	case OSAEKOMI_DSP_WHITE:
		fg = &color_black;
		bg = &color_white;
		break;
	case OSAEKOMI_DSP_UNKNOWN:
		fg = &color_white;
		bg = &color_black;
		break;
	}

        set_fg_color(points, GTK_STATE_NORMAL, fg);
        set_bg_color(points, GTK_STATE_NORMAL, bg);

        if (osaekomi_state == OSAEKOMI_DSP_YES) {
                color = &color_green;
		set_fg_color(pts_to_blue, GTK_STATE_NORMAL, &color_white);
		set_bg_color(pts_to_blue, GTK_STATE_NORMAL, bgcolor);
		set_fg_color(pts_to_white, GTK_STATE_NORMAL, &color_black);
		set_bg_color(pts_to_white, GTK_STATE_NORMAL, &color_white);
        } else {
                color = &color_grey;
		set_fg_color(pts_to_blue, GTK_STATE_NORMAL, &color_grey);
		set_bg_color(pts_to_blue, GTK_STATE_NORMAL, &color_black);
		set_fg_color(pts_to_white, GTK_STATE_NORMAL, &color_grey);
		set_bg_color(pts_to_white, GTK_STATE_NORMAL, &color_black);
	}

        set_fg_color(o_tsec, GTK_STATE_NORMAL, color);
        set_fg_color(o_sec, GTK_STATE_NORMAL, color);

	expose_label(NULL, points);
	expose_label(NULL, o_tsec);
	expose_label(NULL, o_sec);
	expose_label(NULL, pts_to_blue);
	expose_label(NULL, pts_to_white);
}

void set_timer_value(guint min, guint tsec, guint sec)
{
        if (min < 10) {
                set_text(MY_LABEL(t_min), num_to_str(min));
                set_text(MY_LABEL(t_tsec), num_to_str(tsec));
                set_text(MY_LABEL(t_sec), num_to_str(sec));
        } else {
                set_text(MY_LABEL(t_min), "-");
                set_text(MY_LABEL(t_tsec), "-");
                set_text(MY_LABEL(t_sec), "-");
        }

	expose_label(NULL, t_min);
	expose_label(NULL, t_tsec);
	expose_label(NULL, t_sec);
}

void set_osaekomi_value(guint tsec, guint sec)
{
        set_text(MY_LABEL(o_tsec), num_to_str(tsec));
        set_text(MY_LABEL(o_sec), num_to_str(sec));
	expose_label(NULL, o_tsec);
	expose_label(NULL, o_sec);
}

static void set_number(gint w, gint num)
{
	const gchar *s, *n;
	s = get_text(w);
	n = num_to_str(num);
	if ((s && n && strcmp(s, n)) || s == NULL) {
		set_text(MY_LABEL(w), n);
		expose_label(NULL, w);
	}
}

void set_points(gint blue[], gint white[])
{
	if (rules_no_koka_dsp) {
		if (blue[0] >= 2)
			set_number(bw, 1);
		else
			set_number(bw, 0);

		if (blue[0] & 1)
			set_number(by, 1);
		else
			set_number(by, 0);

		set_number(bk, blue[1]);
		set_number(bs, blue[3]);

		if (white[0] >= 2)
			set_number(ww, 1);
		else
			set_number(ww, 0);

		if (white[0] & 1)
			set_number(wy, 1);
		else
			set_number(wy, 0);

		set_number(wk, white[1]);
		set_number(ws, white[3]);
	} else {
		set_number(bw, blue[0]);
		set_number(by, blue[1]);
		set_number(bk, blue[2]);
		set_number(bs, blue[3]);
		set_number(ww, white[0]);
		set_number(wy, white[1]);
		set_number(wk, white[2]);
		set_number(ws, white[3]);
	}
}

void set_score(guint score)
{
        set_text(MY_LABEL(points), pts_to_str(score));
	expose_label(NULL, points);
}

void parse_name(const gchar *s, gchar *first, gchar *last, gchar *club, gchar *country)
{
    const gchar *p = s;
    gint i;

    *first = *last = *club = *country = 0;

    i = 0;
    while (*p >= ' ' && i < 31) {
        *last++ = *p++;
        i++;
    }
    *last = 0;
    p++;
    i = 0;
    while (*p >= ' ' && i < 31) {
        *first++ = *p++;
        i++;
    }
    *first = 0;
    p++;
    i = 0;
    while (*p >= ' ' && i < 31) {
        *country++ = *p++;
        i++;
    }
    *country = 0;
    p++;
    i = 0;
    while (*p >= ' ' && i < 31) {
        *club++ = *p++;
        i++;
    }
    *club = 0;
}

static gchar *get_name_by_layout(gchar *first, gchar *last, gchar *club, gchar *country)
{
    switch (selected_name_layout) {
    case 0:
        if (country == NULL || country[0] == 0)
            return g_strconcat(first, " ", last, ", ", club, NULL);
        else if (club == NULL || club[0] == 0)
            return g_strconcat(first, " ", last, ", ", country, NULL);
        return g_strconcat(first, " ", last, ", ", country, "/", club, NULL);
    case 1:
        if (country == NULL || country[0] == 0)
            return g_strconcat(last, ", ", first, ", ", club, NULL);
        else if (club == NULL || club[0] == 0)
            return g_strconcat(last, ", ", first, ", ", country, NULL);
        return g_strconcat(last, ", ", first, ", ", country, "/", club, NULL);
    case 2:
        if (country == NULL || country[0] == 0)
            return g_strconcat(club, "  ", last, ", ", first, NULL);
        else if (club == NULL || club[0] == 0)
            return g_strconcat(country, "  ", last, ", ", first, NULL);
        return g_strconcat(country, "/", club, "  ", last, ", ", first, NULL);
    case 3:
        return g_strconcat(country, "  ", last, ", ", first, NULL);
    case 4:
        return g_strconcat(club, "  ", last, ", ", first, NULL);
    case 5:
        return g_strconcat(country, "  ", last, NULL);
    case 6:
        return g_strconcat(club, "  ", last, NULL);
    case 7:
        return g_strconcat(last, ", ", first, NULL);
    case 8:
        return g_strdup(last);
    case 9:
        return g_strdup(country);
    case 10:
        return g_strdup(club);
    }

    return NULL;
}

void show_message(gchar *cat_1,
                  gchar *blue_1,
                  gchar *white_1,
                  gchar *cat_2,
                  gchar *blue_2,
                  gchar *white_2)
{
    gchar buf[32], *name;
    gchar *b_tmp = blue_1, *w_tmp = white_1;
    gchar b_first[32], b_last[32], b_club[32], b_country[32];
    gchar w_first[32], w_last[32], w_club[32], w_country[32];

    b_first[0] = b_last[0] = b_club[0] = b_country[0] = 0;
    w_first[0] = w_last[0] = w_club[0] = w_country[0] = 0;

    if (sides_switched) {
        blue_1 = w_tmp;
        white_1 = b_tmp;
    }

    parse_name(blue_1, b_first, b_last, b_club, b_country);
    parse_name(white_1, w_first, w_last, w_club, w_country);

    if (dsp_layout == 6) {
        g_utf8_strncpy(buf, b_first, 1);
        snprintf(b_first, sizeof(b_first), "%s.", buf);

        g_utf8_strncpy(buf, w_first, 1);
        snprintf(w_first, sizeof(w_first), "%s.", buf);
    }

    set_text(MY_LABEL(cat1), cat_1);
    set_text2(MY_LABEL(cat1), "");

    if (dsp_layout == 7) {
        // divide category on two lines
        snprintf(buf, sizeof(buf), "%s", cat_1);
        gchar *p = strrchr(buf, '-');
        if (!p)
            p = strrchr(buf, '+');
        if (!p)
            p = strrchr(buf, ' ');
        if (p) {
            set_text2(cat1, p);
            *p = 0;
            set_text(cat1, buf);
        }

        // Show flags. Country must be in IOC format.
        set_text(flag_blue, b_country);
        set_text(flag_white, w_country);
    }

    if (labels[blue_club].w > 0.01)
        set_text(MY_LABEL(blue_club), b_club);

    if (labels[white_club].w > 0.01)
        set_text(MY_LABEL(white_club), w_club);

    name = get_name_by_layout(b_first, b_last, b_club, b_country);
    set_text(MY_LABEL(blue_name_1), name);
    g_free(name);

    name = get_name_by_layout(w_first, w_last, w_club, w_country);
    set_text(MY_LABEL(white_name_1), name);
    g_free(name);

    set_text(MY_LABEL(cat2),         cat_2);

    parse_name(blue_2, b_first, b_last, b_club, b_country);
    parse_name(white_2, w_first, w_last, w_club, w_country);

    name = get_name_by_layout(b_first, b_last, b_club, b_country);
    set_text(MY_LABEL(blue_name_2), name);
    g_free(name);

    name = get_name_by_layout(w_first, w_last, w_club, w_country);
    set_text(MY_LABEL(white_name_2), name);
    g_free(name);

    expose_label(NULL, cat1);
    expose_label(NULL, blue_name_1);
    expose_label(NULL, white_name_1);
    expose_label(NULL, cat2);
    expose_label(NULL, blue_name_2);
    expose_label(NULL, white_name_2);
    expose_label(NULL, blue_club);
    expose_label(NULL, white_club);
    expose_label(NULL, flag_blue);
    expose_label(NULL, flag_white);

    if (big_dialog)
        show_big();
}

gchar *get_name(gint who)
{
        if (who == BLUE)
                return get_text(blue_name_1);
        return get_text(white_name_1);
}

gchar *get_cat(void)
{
        return get_text(cat1);
}

void set_comment_text(gchar *txt)
{
        set_text(MY_LABEL(comment), txt);
	expose_label(NULL, comment);

	if (big_dialog)
		show_big();
}

void voting_result(GtkWidget *w,
                   gpointer   data)
{
	blue_wins_voting = FALSE;
	white_wins_voting = FALSE;
	hansokumake_to_blue = FALSE;
	hansokumake_to_white = FALSE;

	switch ((gint)data) {
        case HANTEI_BLUE:
                set_text(MY_LABEL(comment), _("Blue won the voting"));
                blue_wins_voting = TRUE;
		break;
        case HANTEI_WHITE:
                set_text(MY_LABEL(comment), _("White won the voting"));
                white_wins_voting = TRUE;
		break;
	case HANSOKUMAKE_BLUE:
                set_text(MY_LABEL(comment), _("Hansokumake to blue"));
		hansokumake_to_blue = TRUE;
		break;
	case HANSOKUMAKE_WHITE:
                set_text(MY_LABEL(comment), _("Hansokumake to white"));
		hansokumake_to_white = TRUE;
		break;
	case CLEAR_SELECTION:
                set_text(MY_LABEL(comment), "");
		break;
        }
	expose_label(NULL, comment);

	set_hantei_winner((gint)data);
}

gboolean delete_big(gpointer data)
{
	big_dialog = FALSE;
	gtk_window_set_title(GTK_WINDOW(main_window), "JudoTimer");
	expose(darea, 0, 0);
        send_label(STOP_BIG);
        return FALSE;
}

static void show_big(void)
{
        cairo_text_extents_t extents;
	cairo_t *c = gdk_cairo_create(darea->window);

	cairo_set_source_rgb(c, 1.0, 1.0, 1.0);
	cairo_rectangle(c, 0, 0,
			W(1.0), H(0.2));
	cairo_fill(c);

        if (strlen(big_text) < 12)
                cairo_set_font_size(c, H(0.1));
        else
                cairo_set_font_size(c, H(0.05));
        cairo_text_extents(c, big_text, &extents);

	cairo_set_source_rgb(c, 0.0, 0.0, 0.0);
	cairo_move_to(c, W(0.5)-extents.width/2.0,
		      (H(0.2)- extents.height)/2.0 -
		      extents.y_bearing);
        cairo_show_text(c, big_text);
	cairo_show_page(c);
	cairo_destroy(c);
}

void display_big(gchar *txt, gint tmo_sec)
{
	gtk_window_set_title(GTK_WINDOW(main_window), txt);
	strncpy(big_text, txt, sizeof(big_text)-1);
	big_end = time(NULL) + tmo_sec;
	big_dialog = TRUE;
	show_big();
        send_label(START_BIG);
}

void reset_display(gint key)
{
        gint pts[4] = {0,0,0,0};

        set_timer_run_color(FALSE);
        set_timer_osaekomi_color(OSAEKOMI_DSP_NO, 0);
        set_osaekomi_value(0, 0);

        if (golden_score == FALSE || rules_leave_score == FALSE)
            set_points(pts, pts);

        set_text(MY_LABEL(comment), "");
	//expose(darea, 0, 0);
}

static void expose_label(cairo_t *c, gint w)
{
    cairo_text_extents_t extents;
    gdouble x, y, x1, y1, wi, he;
    gboolean delc = FALSE;
    gboolean two_lines = labels[w].text2 && labels[w].text2[0];
    gchar buf1[32], buf2[32];
    gchar *txt1 = labels[w].text, *txt2 = labels[w].text2;
    gdouble fsize;

    if (labels[w].w == 0.0)
        return;

    labels[w].status |= LABEL_STATUS_EXPOSE;

    if (!c) {
        delc = TRUE;
        c = gdk_cairo_create(darea->window);
    }

    cairo_save(c);

    x1 = W(labels[w].x);
    y1 = H(labels[w].y);
    wi = W(labels[w].w);
    he = H(labels[w].h);

    if (labels[w].bg_r > -0.001) {
        cairo_set_source_rgb(c, labels[w].bg_r,
                             labels[w].bg_g, labels[w].bg_b);
        cairo_rectangle(c, x1, y1, wi, he);
        cairo_clip(c);
        cairo_rectangle(c, x1, y1, wi, he);
        cairo_fill(c);
    }

    if (labels[w].size)
        fsize = H(labels[w].size * labels[w].h);
    else if (two_lines)
        fsize =  H(labels[w].h*0.4);
    else
        fsize = H(labels[w].h*0.8);

    cairo_set_font_size(c, fsize);
    cairo_text_extents(c, txt1, &extents);

    if (two_lines == FALSE && 
        labels[w].wrap && 
        extents.width > W(labels[w].w)) {
        // needs two lines
        snprintf(buf1, sizeof(buf1), "%s", labels[w].text);
        gchar *p = strrchr(buf1, '-');
        if (!p)
            p = strrchr(buf1, '+');
        if (!p)
            p = strrchr(buf1, ' ');
        if (p) {
            snprintf(buf2, sizeof(buf2), "%s", p);
            *p = 0;
            txt1 = buf1;
            txt2 = buf2;

            fsize /= 2.0;
            cairo_set_font_size(c, fsize);

            cairo_text_extents(c, txt1, &extents);
            two_lines = TRUE;
        } 
    }

    cairo_set_source_rgb(c, labels[w].fg_r,
                         labels[w].fg_g, labels[w].fg_b);
    if (labels[w].xalign < 0)
        x = W(labels[w].x);
    else if (labels[w].xalign > 0)
        x = W(labels[w].x + labels[w].w) - extents.width;
    else
        x = W(labels[w].x+labels[w].w/2.0)-extents.width/2.0-1;

    if (two_lines) {
        y = H(labels[w].y) + (H(labels[w].h/2.0)- extents.height)/2.0 - extents.y_bearing;
        cairo_move_to(c, x, y);
        cairo_show_text(c, txt1);
        cairo_text_extents(c, txt2, &extents);
        y = H(labels[w].y + labels[w].h/2.0) + (H(labels[w].h/2.0)- extents.height)/2.0 - extents.y_bearing;
        cairo_move_to(c, x, y);
        cairo_show_text(c, txt2);
    } else {
        y = H(labels[w].y) + (H(labels[w].h)- extents.height)/2.0 - extents.y_bearing;
        cairo_move_to(c, x, y);
        if (w != flag_blue && w != flag_white)
            cairo_show_text(c, txt1);
    }

    if (w == flag_blue || w == flag_white) {
        gchar buf[32];
        snprintf(buf, sizeof(buf), "%s.png", txt1);
        gchar *file = g_build_filename(installation_dir, "etc", "flags-ioc", buf, NULL);
        cairo_surface_t *image = cairo_image_surface_create_from_png(file);
        if (image && cairo_surface_status(image) == CAIRO_STATUS_SUCCESS) {
            gdouble icon_w = cairo_image_surface_get_width(image);
            gdouble icon_h = cairo_image_surface_get_height(image);
            cairo_scale(c, he/icon_h, he/icon_h);

            if (labels[w].xalign > 0)
                cairo_set_source_surface(c, image, (x1+wi)*icon_h/he - icon_w, y1*icon_h/he);
            else
                cairo_set_source_surface(c, image, x1*icon_h/he, y1*icon_h/he);

            cairo_paint(c);
        }

        cairo_surface_destroy(image);
        g_free(file);
        
    }

    cairo_restore(c);

    if (delc) {
        cairo_show_page(c);
        cairo_destroy(c);
    }
}

/* This is called when we need to draw the windows contents */
static gboolean expose(GtkWidget *widget, GdkEventExpose *event, gpointer userdata)
{
        static cairo_t *c = NULL;
	gint i;

	c = gdk_cairo_create(widget->window);

        paper_height = widget->allocation.height;
        paper_width = widget->allocation.width;

        cairo_select_font_face(c, "serif",
                               CAIRO_FONT_SLANT_NORMAL,
                               CAIRO_FONT_WEIGHT_BOLD);

	cairo_set_source_rgb(c, 0.0, 0.0, 0.0);
	cairo_rectangle(c, 0.0, 0.0, paper_width, paper_height);
	cairo_fill(c);

	cairo_show_page(c);
	cairo_destroy(c);

	for (i = 0; i < num_labels; i++) {
		expose_label(NULL, i);
	}

	if (big_dialog)
		show_big();

        return FALSE;
}

static gboolean button_pressed(GtkWidget *widget,
			       GdkEventButton *event,
			       gpointer userdata)
{
        /* single click with the right mouse button? */
        if (event->type == GDK_BUTTON_PRESS  &&
            (event->button == 1 || event->button == 3)) {
		gint x = event->x, y = event->y, i;

		for (i = 0; i < num_labels; i++) {
			if (x >= W(labels[i].x) &&
			    x <= W(labels[i].x+labels[i].w) &&
			    y >= H(labels[i].y) &&
			    y <= H(labels[i].y+labels[i].h)) {
				labels[i].cb(widget, event, userdata);
				return TRUE;
			}
		}
	}

	return FALSE;
}

static gboolean key_press(GtkWidget *widget, GdkEventKey *event, gpointer userdata)
{
        if (event->type != GDK_KEY_PRESS)
                return FALSE;

        if (event->keyval == GDK_D && (event->state & 5) == 5)
                demo = 1;
        else if (event->keyval == GDK_F && (event->state & 5) == 5)
                demo = 2;
        else
                demo = 0;

        //g_print("key=%x stat=%x\n", event->keyval, event->state);
	if (event->keyval < GDK_0 || event->keyval > GDK_9 || event->keyval == GDK_6)
		clock_key(event->keyval, event->state);

        return FALSE;
}

gboolean undo_func(GtkWidget *menu_item, GdkEventButton *event, gpointer data)
{
        clock_key(GDK_z, 4); //ctl-Z
	return TRUE;
}

gboolean send_label(gint bigdsp)
{
        struct message msg;
        static gint update_next = 0;
        gint i;
        gboolean dosend = FALSE;

        if (mode != MODE_MASTER)
                return FALSE;

        memset(&msg, 0, sizeof(msg));
        msg.type = MSG_UPDATE_LABEL;
        msg.u.update_label.label_num = -1;

        for (i = 0; i < num_labels; i++) {
                if (labels[i].status & LABEL_STATUS_EXPOSE) {
                        msg.u.update_label.expose[i] = 1;
                        labels[i].status &= ~LABEL_STATUS_EXPOSE;
                        dosend = TRUE;
                }

                if (msg.u.update_label.label_num >= 0)
                        continue;

                if (bigdsp) {
                        msg.u.update_label.label_num = bigdsp;
                        strncpy(msg.u.update_label.text, big_text,
                                sizeof(msg.u.update_label.text)-1);
                        continue;
                }

                if (labels[update_next].status & LABEL_STATUS_CHANGED) {
                        labels[update_next].status &= ~LABEL_STATUS_CHANGED;
                        msg.u.update_label.label_num = update_next;
                        msg.u.update_label.x = labels[update_next].x;
                        msg.u.update_label.y = labels[update_next].y;
                        msg.u.update_label.w = labels[update_next].w;
                        msg.u.update_label.h = labels[update_next].h;
                        strncpy(msg.u.update_label.text, labels[update_next].text,
                                sizeof(msg.u.update_label.text)-1);
                        msg.u.update_label.size = labels[update_next].size;
                        msg.u.update_label.xalign = labels[update_next].xalign;
                        msg.u.update_label.fg_r = labels[update_next].fg_r;
                        msg.u.update_label.fg_g = labels[update_next].fg_g;
                        msg.u.update_label.fg_b = labels[update_next].fg_b;
                        msg.u.update_label.bg_r = labels[update_next].bg_r;
                        msg.u.update_label.bg_g = labels[update_next].bg_g;
                        msg.u.update_label.bg_b = labels[update_next].bg_b;

                        dosend = TRUE;
                }

                if (++update_next >= num_labels)
                        update_next = 0;
        }

        if (dosend) {
                send_label_msg(&msg);
        }

        return dosend;
}

static gboolean timeout(void *param)
{
        update_clock();

	if (big_dialog && time(NULL) > big_end)
		delete_big(NULL);

        if (mode != MODE_MASTER)
                return TRUE;

        if (send_label(0)) // send second if one exists
                if (send_label(0)) // send third if second exists
                        send_label(0);

        return TRUE;
}

void update_label(struct msg_update_label *msg)
{
        gint i, w = msg->label_num;

        if (w == START_BIG || w == STOP_BIG) {
                if (w == START_BIG)
                        display_big(msg->text, 1000);
                else
                        delete_big(NULL);
        } else if (w == START_ADVERTISEMENT) {
                display_ad_window();
        } else if (w >= 0 && w < num_labels) {
                //labels[w].x = msg->x;
                //labels[w].y = msg->y;
                //labels[w].w = msg->w;
                //labels[w].h = msg->h;
                set_text(w, msg->text);
                //labels[w].size = msg->size;
                //labels[w].xalign = msg->xalign;
                labels[w].fg_r = msg->fg_r;
                labels[w].fg_g = msg->fg_g;
                labels[w].fg_b = msg->fg_b;
                labels[w].bg_r = msg->bg_r;
                labels[w].bg_g = msg->bg_g;
                labels[w].bg_b = msg->bg_b;
                expose_label(NULL, w);
        }

        for (i = 0; i < num_labels; i++) {
                if (msg->expose[i]) {
                        expose_label(NULL, i);
                }
        }

	if (big_dialog)
		show_big();
}

#if 0
static GtkWidget *get_picture(const gchar *name)
{
        gchar *file = g_build_filename(installation_dir, "etc", name, NULL);
        GtkWidget *ebox = gtk_event_box_new();
        GtkWidget *pic = gtk_image_new_from_file(file);
        gtk_widget_show(ebox);
        gtk_widget_show(pic);
        gtk_container_add(GTK_CONTAINER(ebox), pic);
        g_free(file);
        return ebox;
}
#endif

void dump_screen(void)
{
    gint i;

    printf("# num x y width height size xalign fg-red fg-green fg-blue bg-red bg-green bg-blue wrap");
    for (i = 0; i < num_labels; i++) {
        printf("%d %1.3f %1.3f %1.3f %1.3f ", i, labels[i].x, labels[i].y, labels[i].w, labels[i].h);
        printf("%1.3f %d %1.3f %1.3f %1.3f %1.3f %1.3f %1.3f %d\n", labels[i].size, labels[i].xalign, 
               labels[i].fg_r, labels[i].fg_g, labels[i].fg_b, 
               labels[i].bg_r, labels[i].bg_g, labels[i].bg_b, labels[i].wrap);
    }
}

int main( int   argc,
          char *argv[] )
{
        /* GtkWidget is the storage type for widgets */
	GtkWidget *window;
	GtkWidget *menubar;
        GdkColor   fg, bg;
        time_t     now;
        struct tm *tm;
        GThread   *gth = NULL;         /* thread id */
        gboolean   run_flag = TRUE;   /* used as exit flag for threads */
        gint i;

        judotimer_log("JudoTimer starts");

        font = pango_font_description_from_string("Sans bold 12");

        gdk_color_parse("#FFFF00", &color_yellow);
        gdk_color_parse("#FFFFFF", &color_white);
        gdk_color_parse("#404040", &color_grey);
        gdk_color_parse("#00FF00", &color_green);
        gdk_color_parse("#0000FF", &color_blue);
        gdk_color_parse("#FF0000", &color_red);
        gdk_color_parse("#000000", &color_black);

#ifdef WIN32
        installation_dir = g_win32_get_package_installation_directory(NULL, NULL);
#else
        gbr_init(NULL);
        installation_dir = gbr_find_prefix(NULL);
#endif

        program_path = argv[0];

        current_directory[0] = 0;

        if (current_directory[0] == 0)
                strcpy(current_directory, ".");
        g_print("current_directory=%s\n", current_directory);
	conffile = g_build_filename(g_get_user_data_dir(), "judotimer.ini", NULL);
	g_print("conf file = %s\n", conffile);

	keyfile = g_key_file_new();
	g_key_file_load_from_file(keyfile, conffile, 0, NULL);

        now = time(NULL);
        tm = localtime(&now);
        current_year = tm->tm_year+1900;
        srand(now); //srandom(now);
        my_address = now + getpid()*10000;

        g_print("LOCALE = %s homedir=%s configdir=%s instdir=%s\n",
                setlocale(LC_ALL, 0),
		g_get_home_dir(),
		g_get_user_config_dir(),
		installation_dir);

        g_thread_init(NULL);    /* Initialize GLIB thread support */
        gdk_threads_init();     /* Initialize GDK locks */
        gdk_threads_enter();    /* Acquire GDK locks */

        gtk_init (&argc, &argv);

        main_window = window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(main_window), "JudoTimer");
        gtk_widget_set_size_request(window, FRAME_WIDTH, FRAME_HEIGHT);

        gchar *iconfile = g_build_filename(installation_dir, "etc", "judotimer.png", NULL);
        gtk_window_set_default_icon_from_file(iconfile, NULL);
        g_free(iconfile);

        g_signal_connect (G_OBJECT (window), "delete_event",
                          G_CALLBACK (delete_event), NULL);

        g_signal_connect (G_OBJECT (window), "destroy",
                          G_CALLBACK (destroy), NULL);

        gtk_container_set_border_width (GTK_CONTAINER (window), 10);

	main_vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (main_vbox), 1);
	gtk_container_add (GTK_CONTAINER (window), main_vbox);
        gtk_widget_show(main_vbox);

        /* menubar */
	menubar = get_menubar_menu(window);
        gtk_widget_show(menubar);

        gtk_box_pack_start(GTK_BOX(main_vbox), menubar, FALSE, TRUE, 0);

	darea = gtk_drawing_area_new();
	GTK_WIDGET_SET_FLAGS(darea, GTK_CAN_FOCUS);
	gtk_widget_add_events(darea, GDK_BUTTON_PRESS_MASK);

        gtk_widget_show(darea);

	gtk_box_pack_start_defaults(GTK_BOX(main_vbox), darea);

 	g_signal_connect(G_OBJECT(darea),
			 "expose-event", G_CALLBACK(expose), NULL);
        g_signal_connect(G_OBJECT(darea),
			 "button-press-event", G_CALLBACK(button_pressed), NULL);

        /* labels */

	/*0.0          0.2            0.5          0.7
	 * | Match:     | Nimi sin     | Next:      | Nimi2 sin
	 * | Cat: xx    | Nimi valk    | Cat: yy    | Nimi2 valk
	 * | Comment
	 * |
	 */

#define SMALL_H 0.032
#define BIG_H   ((1.0 - 4*SMALL_H)/3.0)
#define BIG_H2  ((1.0 - 4*SMALL_H)/2.0)
#define BIG_START (4.0*SMALL_H)
#define BIG_W 0.125
#define BIG_W2 0.25
#define TXTW 0.12

        GET_LABEL(blue_name_1, "",  TXTW,     0.0,     0.5-TXTW, SMALL_H);
        GET_LABEL(white_name_1, "", TXTW,     SMALL_H, 0.5-TXTW, SMALL_H);
        GET_LABEL(blue_name_2, "",  0.5+TXTW, 0.0,     0.5-TXTW, SMALL_H);
        GET_LABEL(white_name_2, "", 0.5+TXTW, SMALL_H, 0.5-TXTW, SMALL_H);
        GET_LABEL(blue_club, "", 0,0,0,0);
        GET_LABEL(white_club, "", 0,0,0,0);

        GET_LABEL(match1, _("Match:"), 0.0, 0.0, TXTW, SMALL_H);
        GET_LABEL(match2, _("Next:"),  0.5, 0.0, TXTW, SMALL_H);

        GET_LABEL(wazaari, "W", 0.0,       BIG_START, BIG_W, BIG_H);
        GET_LABEL(yuko, "Y",    1.0*BIG_W, BIG_START, BIG_W, BIG_H);
        GET_LABEL(koka, "K",    2.0*BIG_W, BIG_START, BIG_W, BIG_H);
        GET_LABEL(shido, "S",   3.0*BIG_W, BIG_START, BIG_W, BIG_H);
        GET_LABEL(padding, "",   4.0*BIG_W, BIG_START, 3.0*BIG_W, BIG_H);
        GET_LABEL(sonomama, "SONO", 7.0*BIG_W, BIG_START, BIG_W, BIG_H);

        GET_LABEL(bw, "0", 0.0,       BIG_START+BIG_H, BIG_W, BIG_H);
        GET_LABEL(by, "0", 1.0*BIG_W, BIG_START+BIG_H, BIG_W, BIG_H);
        GET_LABEL(bk, "0", 2.0*BIG_W, BIG_START+BIG_H, BIG_W, BIG_H);
        GET_LABEL(bs, "0", 3.0*BIG_W, BIG_START+BIG_H, BIG_W, BIG_H);
        GET_LABEL(ww, "0", 0.0,       BIG_START+2*BIG_H, BIG_W, BIG_H);
        GET_LABEL(wy, "0", 1.0*BIG_W, BIG_START+2*BIG_H, BIG_W, BIG_H);
        GET_LABEL(wk, "0", 2.0*BIG_W, BIG_START+2*BIG_H, BIG_W, BIG_H);
        GET_LABEL(ws, "0", 3.0*BIG_W, BIG_START+2*BIG_H, BIG_W, BIG_H);

        GET_LABEL(t_min, "0",  4.0*BIG_W, BIG_START+BIG_H, BIG_W, BIG_H);
        GET_LABEL(colon, ":",  5.0*BIG_W, BIG_START+BIG_H, BIG_W, BIG_H);
        GET_LABEL(t_tsec, "0", 6.0*BIG_W, BIG_START+BIG_H, BIG_W, BIG_H);
        GET_LABEL(t_sec, "0",  7.0*BIG_W, BIG_START+BIG_H, BIG_W, BIG_H);
        GET_LABEL(o_tsec, "0", 4.0*BIG_W, BIG_START+2*BIG_H, BIG_W, BIG_H);
        GET_LABEL(o_sec, "0",  5.0*BIG_W, BIG_START+2*BIG_H, BIG_W, BIG_H);

        GET_LABEL(points, "-", 6.5*BIG_W, BIG_START+2*BIG_H, 1.5*BIG_W, BIG_H);
        GET_LABEL(pts_to_blue, "?", 6.0*BIG_W, BIG_START+2*BIG_H, 0.5*BIG_W, BIG_H/2.0);
        GET_LABEL(pts_to_white, "?", 6.0*BIG_W, BIG_START+2.5*BIG_H, 0.5*BIG_W, BIG_H/2.0);
        GET_LABEL(comment, "-", 0.0, 2.0*SMALL_H, 1.0, SMALL_H);

        GET_LABEL(cat1, "-", 0.0, SMALL_H, TXTW, SMALL_H);
        GET_LABEL(cat2, "-", 0.5, SMALL_H, TXTW, SMALL_H);
        GET_LABEL(gs, "", 0.0, 0.0, 0.0, 0.0);
        GET_LABEL(flag_blue, "", 0.0, 0.0, 0.0, 0.0);
        GET_LABEL(flag_white, "", 0.0, 0.0, 0.0, 0.0);

	labels[match1].xalign = -1;
	labels[match2].xalign = -1;
	labels[blue_name_1].xalign = -1;
	labels[white_name_1].xalign = -1;
	labels[blue_name_2].xalign = -1;
	labels[white_name_2].xalign = -1;
	labels[cat1].xalign = -1;
	labels[cat2].xalign = -1;
	labels[gs].xalign = -1;
	labels[blue_club].xalign = -1;
	labels[white_club].xalign = -1;

	labels[wazaari].size = 0.6;
	labels[yuko].size = 0.6;
	labels[koka].size = 0.6;
	labels[shido].size = 0.6;

	labels[bs].size = 0.6;
	labels[ws].size = 0.6;

	labels[pts_to_blue].size = 0.5;
	labels[pts_to_white].size = 0.5;

	labels[sonomama].size = 0.1;
	labels[sonomama].text2 = g_strdup("MAMA");

        /* colors */

        fg = color_white;
        gdk_color_parse("#000000", &bg);
        SET_COLOR(match1);
        SET_COLOR(match2);
        SET_COLOR(blue_name_1);
        SET_COLOR(white_name_1);
        SET_COLOR(blue_name_2);
        SET_COLOR(white_name_2);
        SET_COLOR(blue_club);
        SET_COLOR(white_club);
        SET_COLOR(comment);
        SET_COLOR(wazaari);
        SET_COLOR(yuko);
        SET_COLOR(koka);
        SET_COLOR(shido);
        SET_COLOR(cat1);
        SET_COLOR(cat2);
        SET_COLOR(gs);

        fg = color_yellow;
        SET_COLOR(t_min);
        SET_COLOR(colon);
        SET_COLOR(t_tsec);
        SET_COLOR(t_sec);

        fg = color_grey;
        SET_COLOR(o_tsec);
        SET_COLOR(o_sec);
        SET_COLOR(points);
	SET_COLOR(pts_to_blue);
	SET_COLOR(pts_to_white);

        fg = color_white;
        gdk_color_parse("#0000FF", &bg);
        SET_COLOR(bw);
        SET_COLOR(by);
        SET_COLOR(bk);

        gdk_color_parse("#DDD89A", &fg);
        SET_COLOR(bs);

        gdk_color_parse("#000000", &fg);
        gdk_color_parse("#FFFFFF", &bg);
        SET_COLOR(ww);
        SET_COLOR(wy);
        SET_COLOR(wk);

        gdk_color_parse("#DD6C00", &fg);
        SET_COLOR(ws);

        gdk_color_parse("#AF0000", &fg);
        gdk_color_parse("#000000", &bg);
        SET_COLOR(sonomama);


        gdk_color_parse("#000000", &fg);
        gdk_color_parse("#000000", &bg);

        for (i = 0; i < num_labels; i++)
            defaults_for_labels[i] = labels[i];

        /* signals */

#if 0
	g_signal_connect(G_OBJECT(main_window),
			 "expose-event", G_CALLBACK(expose), NULL);
#endif
	g_signal_connect(G_OBJECT(main_window),
			 "key-press-event", G_CALLBACK(key_press), NULL);

        /* timers */

        timer = g_timer_new();

        g_timeout_add(100, timeout, NULL);

        gtk_widget_show_all(window);

	set_preferences();
        change_language(NULL, NULL, (gpointer)language);

        open_comm_socket();

        /* Create a bg thread using glib */
        gth = g_thread_create((GThreadFunc)client_thread,
                              (gpointer)&run_flag, FALSE, NULL);
	gth = g_thread_create((GThreadFunc)master_thread,
			      (gpointer)&run_flag, FALSE, NULL);

        gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER_ALWAYS);

	cursor = gdk_cursor_new(GDK_HAND2);
	gdk_window_set_cursor(GTK_WIDGET(main_window)->window, cursor);

        open_sound();

        /* All GTK applications must have a gtk_main(). Control ends here
         * and waits for an event to occur (like a key press or
         * mouse event). */
        gtk_main();

        close_sound();

        gdk_threads_leave();  /* release GDK locks */
        run_flag = FALSE;     /* flag threads to stop and exit */
        //g_thread_join(gth);   /* wait for thread to exit */

        //dump_screen();
        return 0;
}

static void set_colors(void)
{
    GdkColor s_blue, s_white;

    gdk_color_parse("#DDD89A", &s_blue);
    gdk_color_parse("#DD6C00", &s_white);

    if (white_first) {
        set_fg_color(bw, GTK_STATE_NORMAL, &color_black);
        set_fg_color(by, GTK_STATE_NORMAL, &color_black);
        set_fg_color(bk, GTK_STATE_NORMAL, &color_black);
        set_fg_color(bs, GTK_STATE_NORMAL, &s_white);
        set_fg_color(ww, GTK_STATE_NORMAL, &color_white);
        set_fg_color(wy, GTK_STATE_NORMAL, &color_white);
        set_fg_color(wk, GTK_STATE_NORMAL, &color_white);
        set_fg_color(ws, GTK_STATE_NORMAL, &s_blue);

        set_bg_color(bw, GTK_STATE_NORMAL, &color_white);
        set_bg_color(by, GTK_STATE_NORMAL, &color_white);
        set_bg_color(bk, GTK_STATE_NORMAL, &color_white);
        set_bg_color(bs, GTK_STATE_NORMAL, &color_white);
        set_bg_color(ww, GTK_STATE_NORMAL, bgcolor);
        set_bg_color(wy, GTK_STATE_NORMAL, bgcolor);
        set_bg_color(wk, GTK_STATE_NORMAL, bgcolor);
        set_bg_color(ws, GTK_STATE_NORMAL, bgcolor);
    } else {
        set_fg_color(bw, GTK_STATE_NORMAL, &color_white);
        set_fg_color(by, GTK_STATE_NORMAL, &color_white);
        set_fg_color(bk, GTK_STATE_NORMAL, &color_white);
        set_fg_color(bs, GTK_STATE_NORMAL, &s_blue);
        set_fg_color(ww, GTK_STATE_NORMAL, &color_black);
        set_fg_color(wy, GTK_STATE_NORMAL, &color_black);
        set_fg_color(wk, GTK_STATE_NORMAL, &color_black);
        set_fg_color(ws, GTK_STATE_NORMAL, &s_white);

        set_bg_color(bw, GTK_STATE_NORMAL, bgcolor);
        set_bg_color(by, GTK_STATE_NORMAL, bgcolor);
        set_bg_color(bk, GTK_STATE_NORMAL, bgcolor);
        set_bg_color(bs, GTK_STATE_NORMAL, bgcolor);
        set_bg_color(ww, GTK_STATE_NORMAL, &color_white);
        set_bg_color(wy, GTK_STATE_NORMAL, &color_white);
        set_bg_color(wk, GTK_STATE_NORMAL, &color_white);
        set_bg_color(ws, GTK_STATE_NORMAL, &color_white);
    }

    if (dsp_layout == 7) {
        if (white_first) {
            set_fg_color(blue_name_1, GTK_STATE_NORMAL, &color_black);
            set_fg_color(blue_club, GTK_STATE_NORMAL, &color_black);
            set_fg_color(white_name_1, GTK_STATE_NORMAL, &color_white);
            set_fg_color(white_club, GTK_STATE_NORMAL, &color_white);

            set_bg_color(blue_name_1, GTK_STATE_NORMAL, &color_white);
            set_bg_color(blue_club, GTK_STATE_NORMAL, &color_white);
            set_bg_color(white_name_1, GTK_STATE_NORMAL, bgcolor);
            set_bg_color(white_club, GTK_STATE_NORMAL, bgcolor);
        } else {
            set_fg_color(blue_name_1, GTK_STATE_NORMAL, &color_white);
            set_fg_color(blue_club, GTK_STATE_NORMAL, &color_white);
            set_fg_color(white_name_1, GTK_STATE_NORMAL, &color_black);
            set_fg_color(white_club, GTK_STATE_NORMAL, &color_black);

            set_bg_color(blue_name_1, GTK_STATE_NORMAL, bgcolor);
            set_bg_color(blue_club, GTK_STATE_NORMAL, bgcolor);
            set_bg_color(white_name_1, GTK_STATE_NORMAL, &color_white);
            set_bg_color(white_club, GTK_STATE_NORMAL, &color_white);
        }
    }
}

void toggle_color(GtkWidget *menu_item, gpointer data)
{
        if (GTK_CHECK_MENU_ITEM(menu_item)->active) {
                bgcolor = &color_red;
		g_key_file_set_string(keyfile, "preferences", "color", "red");
        } else {
                bgcolor = &color_blue;
		g_key_file_set_string(keyfile, "preferences", "color", "blue");
	}

        set_colors();
	expose(darea, 0, 0);
}

void toggle_full_screen(GtkWidget *menu_item, gpointer data)
{
        if (GTK_CHECK_MENU_ITEM(menu_item)->active) {
                gtk_window_fullscreen(GTK_WINDOW(main_window));
		g_key_file_set_boolean(keyfile, "preferences", "fullscreen", TRUE);
        } else {
                gtk_window_unfullscreen(GTK_WINDOW(main_window));
		g_key_file_set_boolean(keyfile, "preferences", "fullscreen", FALSE);
	}
	expose(darea, 0, 0);
}

void toggle_rules_no_koka(GtkWidget *menu_item, gpointer data)
{
        if (GTK_CHECK_MENU_ITEM(menu_item)->active) {
		rules_no_koka_dsp = TRUE;
		set_text(MY_LABEL(wazaari), "I");
		set_text(MY_LABEL(yuko), "W");
		set_text(MY_LABEL(koka), "Y");
		g_key_file_set_boolean(keyfile, "preferences", "rulesnokoka", TRUE);
        } else {
		rules_no_koka_dsp = FALSE;
		set_text(MY_LABEL(wazaari), "W");
		set_text(MY_LABEL(yuko), "Y");
		set_text(MY_LABEL(koka), "K");
		g_key_file_set_boolean(keyfile, "preferences", "rulesnokoka", FALSE);
	}
	expose(darea, 0, 0);
}

void toggle_rules_leave_points(GtkWidget *menu_item, gpointer data)
{
        if (GTK_CHECK_MENU_ITEM(menu_item)->active) {
		rules_leave_score = TRUE;
		g_key_file_set_boolean(keyfile, "preferences", "rulesleavepoints", TRUE);
        } else {
		rules_leave_score = FALSE;
		g_key_file_set_boolean(keyfile, "preferences", "rulesleavepoints", FALSE);
	}
}

void toggle_rules_stop_ippon(GtkWidget *menu_item, gpointer data)
{
        if (GTK_CHECK_MENU_ITEM(menu_item)->active) {
		rules_stop_ippon_2 = TRUE;
		g_key_file_set_boolean(keyfile, "preferences", "stopippon", TRUE);
        } else {
		rules_stop_ippon_2 = FALSE;
		g_key_file_set_boolean(keyfile, "preferences", "stopippon", FALSE);
	}
}

void toggle_confirm_match(GtkWidget *menu_item, gpointer data)
{
    rules_confirm_match = GTK_CHECK_MENU_ITEM(menu_item)->active;
    g_key_file_set_boolean(keyfile, "preferences", "confirmmatch", rules_confirm_match);
}

void toggle_whitefirst(GtkWidget *menu_item, gpointer data)
{
    white_first = GTK_CHECK_MENU_ITEM(menu_item)->active;
    g_key_file_set_boolean(keyfile, "preferences", "whitefirst", white_first);
    set_colors();
    expose(darea, 0, 0);
}

void toggle_show_comp(GtkWidget *menu_item, gpointer data)
{
    show_competitor_names = GTK_CHECK_MENU_ITEM(menu_item)->active;
    g_key_file_set_boolean(keyfile, "preferences", "showcompetitornames", show_competitor_names);
}

void toggle_switch_sides(GtkWidget *menu_item, gpointer data)
{
    gchar *tmp;

    sides_switched = GTK_CHECK_MENU_ITEM(menu_item)->active;

    tmp = labels[blue_name_1].text;
    labels[blue_name_1].text = labels[white_name_1].text;
    labels[white_name_1].text = tmp;
    labels[blue_name_1].status |= LABEL_STATUS_CHANGED;
    labels[white_name_1].status |= LABEL_STATUS_CHANGED;

    tmp = labels[flag_blue].text;
    labels[flag_blue].text = labels[flag_white].text;
    labels[flag_white].text = tmp;
    labels[flag_blue].status |= LABEL_STATUS_CHANGED;
    labels[flag_white].status |= LABEL_STATUS_CHANGED;

    expose_label(NULL, blue_name_1);
    expose_label(NULL, white_name_1);
    expose_label(NULL, flag_blue);
    expose_label(NULL, flag_white);

    light_switch_sides(sides_switched);
}

static void set_position(gint lbl, gdouble x, gdouble y, gdouble w, gdouble h)
{
	labels[lbl].x = x;
	labels[lbl].y = y;
	labels[lbl].w = w;
	labels[lbl].h = h;
}

void set_gs_text(gchar *txt)
{
    set_text(gs, txt);
}

void select_display_layout(GtkWidget *menu_item, gpointer data)
{
#define NO_SHOW(x) set_position(x, 0.0, 0.0, 0.0, 0.0)
    gchar *filename;
    FILE *f;
    gint i;

    // restore default values
    for (i = 0; i < num_labels; i++) {
        labels[i].x = defaults_for_labels[i].x;
        labels[i].y = defaults_for_labels[i].y;
        labels[i].w = defaults_for_labels[i].w;
        labels[i].h = defaults_for_labels[i].h;
        labels[i].size = defaults_for_labels[i].size;
        labels[i].xalign = defaults_for_labels[i].xalign;
        labels[i].fg_r = defaults_for_labels[i].fg_r;
        labels[i].fg_g = defaults_for_labels[i].fg_g;
        labels[i].fg_b = defaults_for_labels[i].fg_b;
        labels[i].bg_r = defaults_for_labels[i].bg_r;
        labels[i].bg_g = defaults_for_labels[i].bg_g;
        labels[i].bg_b = defaults_for_labels[i].bg_b;
        labels[i].wrap = defaults_for_labels[i].wrap;
    }

    clocks_only = FALSE;
    dsp_layout = (gint)data;

    switch(dsp_layout) {
        /*if (GTK_CHECK_MENU_ITEM(menu_item)->active) {*/
    case 1:
        set_position(match1, 0.0, 0.0, TXTW, SMALL_H);
        set_position(match2, 0.5, 0.0, TXTW, SMALL_H);

        set_position(blue_name_1,  TXTW,     0.0,     0.5-TXTW, SMALL_H);
        set_position(white_name_1, TXTW,     SMALL_H, 0.5-TXTW, SMALL_H);
        set_position(blue_name_2,  0.5+TXTW, 0.0,     0.5-TXTW, SMALL_H);
        set_position(white_name_2, 0.5+TXTW, SMALL_H, 0.5-TXTW, SMALL_H);
        set_position(cat1,         0.0, SMALL_H, TXTW, SMALL_H);
        set_position(cat2,         0.5, SMALL_H, TXTW, SMALL_H);

        set_position(comment,  0.0, 2.0*SMALL_H, 1.0, SMALL_H);

        set_position(wazaari,  0.0,       BIG_START, BIG_W, BIG_H);
        set_position(yuko,     1.0*BIG_W, BIG_START, BIG_W, BIG_H);
        set_position(koka,     2.0*BIG_W, BIG_START, BIG_W, BIG_H);
        set_position(shido,    3.0*BIG_W, BIG_START, BIG_W, BIG_H);
        set_position(padding,  4.0*BIG_W, BIG_START, 3.0*BIG_W, BIG_H);
        set_position(sonomama, 7.0*BIG_W, BIG_START, BIG_W, BIG_H);

        set_position(bw, 0.0,       BIG_START+BIG_H, BIG_W, BIG_H);
        set_position(by, 1.0*BIG_W, BIG_START+BIG_H, BIG_W, BIG_H);
        set_position(bk, 2.0*BIG_W, BIG_START+BIG_H, BIG_W, BIG_H);
        set_position(bs, 3.0*BIG_W, BIG_START+BIG_H, BIG_W, BIG_H);
        set_position(ww, 0.0,       BIG_START+2*BIG_H, BIG_W, BIG_H);
        set_position(wy, 1.0*BIG_W, BIG_START+2*BIG_H, BIG_W, BIG_H);
        set_position(wk, 2.0*BIG_W, BIG_START+2*BIG_H, BIG_W, BIG_H);
        set_position(ws, 3.0*BIG_W, BIG_START+2*BIG_H, BIG_W, BIG_H);

        set_position(t_min,  4.0*BIG_W, BIG_START+BIG_H,   BIG_W, BIG_H);
        set_position(colon,  5.0*BIG_W, BIG_START+BIG_H,   BIG_W, BIG_H);
        set_position(t_tsec, 6.0*BIG_W, BIG_START+BIG_H,   BIG_W, BIG_H);
        set_position(t_sec,  7.0*BIG_W, BIG_START+BIG_H,   BIG_W, BIG_H);

        set_position(o_tsec, 4.0*BIG_W, BIG_START+2*BIG_H, BIG_W, BIG_H);
        set_position(o_sec,  5.0*BIG_W, BIG_START+2*BIG_H, BIG_W, BIG_H);

        set_position(points, 6.5*BIG_W, BIG_START+2*BIG_H, 1.5*BIG_W, BIG_H);
        set_position(pts_to_blue,  6.0*BIG_W, BIG_START+2*BIG_H, 0.5*BIG_W, BIG_H/2.0);
        set_position(pts_to_white, 6.0*BIG_W, BIG_START+2.5*BIG_H, 0.5*BIG_W, BIG_H/2.0);
        break;

    case 2:
        set_position(match1, 0.0, 0.0, 0.0, SMALL_H);
        set_position(match2, 0.5, 0.0, TXTW, SMALL_H);

        set_position(blue_name_1,  0,     BIG_START+0.3*BIG_H, 0.5, 0.2*BIG_H);
        set_position(white_name_1, 0,     BIG_START+2.5*BIG_H, 0.5, 0.2*BIG_H);
        set_position(blue_name_2,  0.5+TXTW, 0.0,     0.5-TXTW, SMALL_H);
        set_position(white_name_2, 0.5+TXTW, SMALL_H, 0.5-TXTW, SMALL_H);
        set_position(cat1,         0.0, 0.0, 0.5, 4*SMALL_H);
        set_position(cat2,         0.5, SMALL_H, TXTW, SMALL_H);

        set_position(comment,  0.0, 4.0*SMALL_H, 1.0, SMALL_H);

        set_position(wazaari,  0.0,       BIG_START+1.4*BIG_H, BIG_W, 0.2*BIG_H);
        set_position(yuko,     1.0*BIG_W, BIG_START+1.4*BIG_H, BIG_W, 0.2*BIG_H);
        set_position(koka,     2.0*BIG_W, BIG_START+1.4*BIG_H, BIG_W, 0.2*BIG_H);
        set_position(shido,    3.0*BIG_W, BIG_START+1.4*BIG_H, BIG_W, 0.2*BIG_H);
        set_position(padding,  4.0*BIG_W, BIG_START, 0, BIG_H);
        set_position(sonomama, 7.0*BIG_W, BIG_START, BIG_W, BIG_H);

        set_position(bw, 0.0,       BIG_START+0.5*BIG_H, BIG_W, 0.9*BIG_H);
        set_position(by, 1.0*BIG_W, BIG_START+0.5*BIG_H, BIG_W, 0.9*BIG_H);
        set_position(bk, 2.0*BIG_W, BIG_START+0.5*BIG_H, BIG_W, 0.9*BIG_H);
        set_position(bs, 3.0*BIG_W, BIG_START+0.5*BIG_H, BIG_W, 0.9*BIG_H);
        set_position(ww, 0.0,       BIG_START+1.6*BIG_H, BIG_W, 0.9*BIG_H);
        set_position(wy, 1.0*BIG_W, BIG_START+1.6*BIG_H, BIG_W, 0.9*BIG_H);
        set_position(wk, 2.0*BIG_W, BIG_START+1.6*BIG_H, BIG_W, 0.9*BIG_H);
        set_position(ws, 3.0*BIG_W, BIG_START+1.6*BIG_H, BIG_W, 0.9*BIG_H);

        set_position(t_min,  4.0*BIG_W, BIG_START+BIG_H,   BIG_W, BIG_H);
        set_position(colon,  5.0*BIG_W, BIG_START+BIG_H,   BIG_W, BIG_H);
        set_position(t_tsec, 6.0*BIG_W, BIG_START+BIG_H,   BIG_W, BIG_H);
        set_position(t_sec,  7.0*BIG_W, BIG_START+BIG_H,   BIG_W, BIG_H);

        set_position(o_tsec, 4.0*BIG_W, BIG_START+2*BIG_H, BIG_W, BIG_H);
        set_position(o_sec,  5.0*BIG_W, BIG_START+2*BIG_H, BIG_W, BIG_H);

        set_position(points, 6.5*BIG_W, BIG_START+2*BIG_H, 1.5*BIG_W, BIG_H);
        set_position(pts_to_blue,  6.0*BIG_W, BIG_START+2*BIG_H, 0.5*BIG_W, BIG_H/2.0);
        set_position(pts_to_white, 6.0*BIG_W, BIG_START+2.5*BIG_H, 0.5*BIG_W, BIG_H/2.0);
        break;

    case 3:
        set_position(match1, 0.0, 0.0, TXTW, 0);
        set_position(match2, 0.5, 0.0, TXTW, SMALL_H);

        set_position(blue_name_1,  0.5,  4*SMALL_H+0.3*BIG_H, 0.5, 0.2*BIG_H);
        set_position(white_name_1, 0.0,  4*SMALL_H+0.3*BIG_H, 0.5, 0.2*BIG_H);
        set_position(blue_name_2,  0.5+TXTW, 0.0,     0.5-TXTW, SMALL_H);
        set_position(white_name_2, 0.5+TXTW, SMALL_H, 0.5-TXTW, SMALL_H);
        set_position(cat1,         0.0, 0.0, 0.5, 4.0*SMALL_H);
        set_position(cat2,         0.5, SMALL_H, TXTW, SMALL_H);

        set_position(comment,  0.0, 4.0*SMALL_H+1.5*BIG_H, 1.0, SMALL_H);

        set_position(wazaari,  0.0,       BIG_START, 0, BIG_H);
        set_position(yuko,     1.0*BIG_W, BIG_START, 0, BIG_H);
        set_position(koka,     2.0*BIG_W, BIG_START, 0, BIG_H);
        set_position(shido,    3.0*BIG_W, BIG_START, 0, BIG_H);
        set_position(padding,  4.0*BIG_W, BIG_START, 0, BIG_H);
        set_position(sonomama, 7.0*BIG_W, BIG_START+1.5*BIG_H, BIG_W, 0.5*BIG_H);

        set_position(bw, 4.0*BIG_W, BIG_START+0.5*BIG_H, BIG_W, BIG_H);
        set_position(by, 5.0*BIG_W, BIG_START+0.5*BIG_H, BIG_W, BIG_H);
        set_position(bk, 6.0*BIG_W, BIG_START+0.5*BIG_H, BIG_W, BIG_H);
        set_position(bs, 7.0*BIG_W, BIG_START+0.5*BIG_H, BIG_W, BIG_H);
        set_position(ww, 0.0,       BIG_START+0.5*BIG_H, BIG_W, BIG_H);
        set_position(wy, 1.0*BIG_W, BIG_START+0.5*BIG_H, BIG_W, BIG_H);
        set_position(wk, 2.0*BIG_W, BIG_START+0.5*BIG_H, BIG_W, BIG_H);
        set_position(ws, 3.0*BIG_W, BIG_START+0.5*BIG_H, BIG_W, BIG_H);

        set_position(t_min,  0.0*BIG_W, BIG_START+2.0*BIG_H,   BIG_W, BIG_H);
        set_position(colon,  1.0*BIG_W, BIG_START+2.0*BIG_H,   BIG_W, BIG_H);
        set_position(t_tsec, 2.0*BIG_W, BIG_START+2.0*BIG_H,   BIG_W, BIG_H);
        set_position(t_sec,  3.0*BIG_W, BIG_START+2.0*BIG_H,   BIG_W, BIG_H);

        set_position(o_tsec, 4.5*BIG_W, BIG_START+2*BIG_H, BIG_W, BIG_H);
        set_position(o_sec,  5.5*BIG_W, BIG_START+2*BIG_H, BIG_W, BIG_H);

        set_position(points, 6.5*BIG_W, BIG_START+2*BIG_H, 1.5*BIG_W, BIG_H);
        set_position(pts_to_blue,  4.0*BIG_W, BIG_START+2*BIG_H, 0.5*BIG_W, BIG_H/2.0);
        set_position(pts_to_white, 4.0*BIG_W, BIG_START+2.5*BIG_H, 0.5*BIG_W, BIG_H/2.0);
        break;

    case 4:
        set_position(match1, 0.0, 0.0, TXTW, 0);
        set_position(match2, 0.5, 0.0, TXTW, SMALL_H);

        set_position(blue_name_1,  0.0,  4*SMALL_H+0.3*BIG_H, 0.5, 0.2*BIG_H);
        set_position(white_name_1, 0.5,  4*SMALL_H+0.3*BIG_H, 0.5, 0.2*BIG_H);
        set_position(blue_name_2,  0.5+TXTW, 0.0,     0.5-TXTW, SMALL_H);
        set_position(white_name_2, 0.5+TXTW, SMALL_H, 0.5-TXTW, SMALL_H);
        set_position(cat1,         0.0, 0.0, 0.5, 4.0*SMALL_H);
        set_position(cat2,         0.5, SMALL_H, TXTW, SMALL_H);

        set_position(comment,  0.0, 4.0*SMALL_H+1.5*BIG_H, 1.0, SMALL_H);

        set_position(wazaari,  0.0,       BIG_START, 0, BIG_H);
        set_position(yuko,     1.0*BIG_W, BIG_START, 0, BIG_H);
        set_position(koka,     2.0*BIG_W, BIG_START, 0, BIG_H);
        set_position(shido,    3.0*BIG_W, BIG_START, 0, BIG_H);
        set_position(padding,  4.0*BIG_W, BIG_START, 0, BIG_H);
        set_position(sonomama, 7.0*BIG_W, BIG_START+1.5*BIG_H, BIG_W, 0.5*BIG_H);

        set_position(bw, 0.0*BIG_W, BIG_START+0.5*BIG_H, BIG_W, BIG_H);
        set_position(by, 1.0*BIG_W, BIG_START+0.5*BIG_H, BIG_W, BIG_H);
        set_position(bk, 2.0*BIG_W, BIG_START+0.5*BIG_H, BIG_W, BIG_H);
        set_position(bs, 3.0*BIG_W, BIG_START+0.5*BIG_H, BIG_W, BIG_H);
        set_position(ww, 4.0*BIG_W, BIG_START+0.5*BIG_H, BIG_W, BIG_H);
        set_position(wy, 5.0*BIG_W, BIG_START+0.5*BIG_H, BIG_W, BIG_H);
        set_position(wk, 6.0*BIG_W, BIG_START+0.5*BIG_H, BIG_W, BIG_H);
        set_position(ws, 7.0*BIG_W, BIG_START+0.5*BIG_H, BIG_W, BIG_H);

        set_position(t_min,  0.0*BIG_W, BIG_START+2.0*BIG_H,   BIG_W, BIG_H);
        set_position(colon,  1.0*BIG_W, BIG_START+2.0*BIG_H,   BIG_W, BIG_H);
        set_position(t_tsec, 2.0*BIG_W, BIG_START+2.0*BIG_H,   BIG_W, BIG_H);
        set_position(t_sec,  3.0*BIG_W, BIG_START+2.0*BIG_H,   BIG_W, BIG_H);

        set_position(o_tsec, 4.5*BIG_W, BIG_START+2*BIG_H, BIG_W, BIG_H);
        set_position(o_sec,  5.5*BIG_W, BIG_START+2*BIG_H, BIG_W, BIG_H);

        set_position(points, 6.5*BIG_W, BIG_START+2*BIG_H, 1.5*BIG_W, BIG_H);
        set_position(pts_to_blue,  4.0*BIG_W, BIG_START+2*BIG_H, 0.5*BIG_W, BIG_H/2.0);
        set_position(pts_to_white, 4.0*BIG_W, BIG_START+2.5*BIG_H, 0.5*BIG_W, BIG_H/2.0);
        break;

    case 5:
        clocks_only = TRUE;
        labels[wazaari].w = 0.0;
        labels[yuko].w = 0.0;
        labels[koka].w = 0.0;
        labels[shido].w = 0.0;
        labels[padding].w = 0.0;
        labels[bw].w = 0.0;
        labels[by].w = 0.0;
        labels[bk].w = 0.0;
        labels[bs].w = 0.0;
        labels[ww].w = 0.0;
        labels[wy].w = 0.0;
        labels[wk].w = 0.0;
        labels[ws].w = 0.0;
        labels[pts_to_blue].w = 0.0;
        labels[pts_to_white].w = 0.0;

        set_position(match1, 0, 0, 0, 0);
        set_position(match2, 0, 0, 0, 0);
        set_position(blue_name_1,  0, 0, 0, 0);
        set_position(white_name_1, 0, 0, 0, 0);
        set_position(blue_name_2,  0, 0, 0, 0);
        set_position(white_name_2, 0, 0, 0, 0);
        set_position(cat1,         0, 0, 0, 0);
        set_position(cat2,         0, 0, 0, 0);
        set_position(t_min,         0.0, BIG_START,        BIG_W2, BIG_H2);
        set_position(colon,      BIG_W2, BIG_START,        BIG_W2, BIG_H2);
        set_position(t_tsec, 2.0*BIG_W2, BIG_START,        BIG_W2, BIG_H2);
        set_position(t_sec,  3.0*BIG_W2, BIG_START,        BIG_W2, BIG_H2);
        set_position(o_tsec,        0.0, BIG_START+BIG_H2, BIG_W2, BIG_H2);
        set_position(o_sec,      BIG_W2, BIG_START+BIG_H2, BIG_W2, BIG_H2);
        set_position(points, 3.0*BIG_W2, BIG_START+BIG_H2, BIG_W2, BIG_H2);
        break;

    case 6:
        set_position(match1, 0.0, 0.0, 0.0, SMALL_H);
        set_position(match2, 0.5, 0.0, TXTW, SMALL_H);

        set_position(blue_name_1,  0.01,     BIG_START+0.1*BIG_H, 0.49, 0.3*BIG_H);
        set_position(white_name_1, 0.01,     BIG_START+2.5*BIG_H, 0.49, 0.3*BIG_H);
        set_position(blue_name_2,  0.5+TXTW, 0.0,     0.5-TXTW, SMALL_H);
        set_position(white_name_2, 0.5+TXTW, SMALL_H, 0.5-TXTW, SMALL_H);
        set_position(cat1,         0.55, 4*SMALL_H, 0.325, 6*SMALL_H);
        set_position(cat2,         0.5, SMALL_H, TXTW, SMALL_H);

        set_position(comment,  0.0, 3.0*SMALL_H, 1.0, SMALL_H);

        set_position(wazaari,  0.0,       BIG_START+1.3*BIG_H, BIG_W, 0.3*BIG_H);
        set_position(yuko,     1.0*BIG_W, BIG_START+1.3*BIG_H, BIG_W, 0.3*BIG_H);
        set_position(koka,     2.0*BIG_W, BIG_START+1.3*BIG_H, BIG_W, 0.3*BIG_H);
        set_position(shido,    3.0*BIG_W, BIG_START+1.3*BIG_H, BIG_W, 0.3*BIG_H);
        set_position(padding,  4.0*BIG_W, BIG_START, 0, BIG_H);
        set_position(sonomama, 7.0*BIG_W, BIG_START, BIG_W, BIG_H);

        set_position(bw, 0.0,       BIG_START+0.4*BIG_H, BIG_W, 0.9*BIG_H);
        set_position(by, 1.0*BIG_W, BIG_START+0.4*BIG_H, BIG_W, 0.9*BIG_H);
        set_position(bk, 2.0*BIG_W, BIG_START+0.4*BIG_H, BIG_W, 0.9*BIG_H);
        set_position(bs, 3.0*BIG_W, BIG_START+0.4*BIG_H, BIG_W, 0.9*BIG_H);
        set_position(ww, 0.0,       BIG_START+1.6*BIG_H, BIG_W, 0.9*BIG_H);
        set_position(wy, 1.0*BIG_W, BIG_START+1.6*BIG_H, BIG_W, 0.9*BIG_H);
        set_position(wk, 2.0*BIG_W, BIG_START+1.6*BIG_H, BIG_W, 0.9*BIG_H);
        set_position(ws, 3.0*BIG_W, BIG_START+1.6*BIG_H, BIG_W, 0.9*BIG_H);

        set_position(t_min,  4.0*BIG_W, BIG_START+BIG_H,   BIG_W, BIG_H);
        set_position(colon,  5.0*BIG_W, BIG_START+BIG_H,   BIG_W, BIG_H);
        set_position(t_tsec, 6.0*BIG_W, BIG_START+BIG_H,   BIG_W, BIG_H);
        set_position(t_sec,  7.0*BIG_W, BIG_START+BIG_H,   BIG_W, BIG_H);

        set_position(o_tsec, 4.0*BIG_W, BIG_START+2*BIG_H, BIG_W, BIG_H);
        set_position(o_sec,  5.0*BIG_W, BIG_START+2*BIG_H, BIG_W, BIG_H);

        set_position(points, 6.5*BIG_W, BIG_START+2*BIG_H, 1.5*BIG_W, BIG_H);
        set_position(pts_to_blue,  6.0*BIG_W, BIG_START+2*BIG_H, 0.5*BIG_W, BIG_H/2.0);
        set_position(pts_to_white, 6.0*BIG_W, BIG_START+2.5*BIG_H, 0.5*BIG_W, BIG_H/2.0);
        break;

    case 7:
        filename = g_build_filename(installation_dir, "etc", "timer-custom.txt", NULL);
        f = fopen(filename, "r");
        if (f) {
            struct label lbl;
            gint num;
            gchar line[128];
            while (fgets(line, sizeof(line), f)) {
                if (line[0] == '#' || strlen(line) < 4)
                    continue;
                if (sscanf(line, "%d %lf %lf %lf %lf %lf %d %lf %lf %lf %lf %lf %lf %d",
                           &num, &lbl.x, &lbl.y, &lbl.w, &lbl.h,
                           &lbl.size, &lbl.xalign, 
                           &lbl.fg_r, &lbl.fg_g, &lbl.fg_b, 
                           &lbl.bg_r, &lbl.bg_g, &lbl.bg_b, &lbl.wrap) == 14) {
                    if (num >= 0 && num < num_labels) {
                        labels[num].x = lbl.x; 
                        labels[num].y = lbl.y; 
                        labels[num].w = lbl.w; 
                        labels[num].h = lbl.h; 
                        labels[num].size = lbl.size; 
                        labels[num].xalign = lbl.xalign; 
                        labels[num].fg_r = lbl.fg_r;
                        labels[num].fg_g = lbl.fg_g;
                        labels[num].fg_b = lbl.fg_b;
                        labels[num].bg_r = lbl.bg_r;
                        labels[num].bg_g = lbl.bg_g;
                        labels[num].bg_b = lbl.bg_b;
                        labels[num].wrap = lbl.wrap;
                    } else
                        printf("Read error in file %s, num = %d\n", filename, num);
                } else
                    printf("Read error in file %s\n", filename);
            }
            fclose(f);
        } else {

#define H1 0.25
#define H2 0.25
#define H3 (1.0 - H1 - H2)
#define H_NAME (0.6*H1)
#define H_CLUB (H1 - H_NAME)
#define H_TIME 0.25
#define H_IWYS 0.10
#define H_SCORE (H1 - 0.5*H_IWYS)
#define H_COMMENT (0.2*(H3 - H_TIME))
#define W1 0.5
#define W_TIME 0.09
#define W_COLON (0.6*W_TIME)
#define W_SEL (0.7*W_TIME)
#define W_CAT 0.28
#define W_SCORE 0.07
#define W_NAME (1 - 4.0*W_SCORE)
#define Y_IWYS (H1 - 0.5*H_IWYS)
#define Y_SCORE (H1 + 0.5*H_IWYS)
#define X_TSEC (W_CAT + W_TIME + W_SCORE)

            //labels[sonomama].size = 0.0;
            labels[gs].size = 0.4;
            labels[gs].xalign = 0;
            labels[cat1].wrap = TRUE;
            labels[cat1].size = 0.35;
            set_bg_color(blue_name_1, GTK_STATE_NORMAL, bgcolor);
            set_bg_color(blue_club, GTK_STATE_NORMAL, bgcolor);
            set_bg_color(white_name_1, GTK_STATE_NORMAL, &color_white);
            set_bg_color(white_club, GTK_STATE_NORMAL, &color_white);
            set_fg_color(white_name_1, GTK_STATE_NORMAL, &color_black);
            set_fg_color(white_club, GTK_STATE_NORMAL, &color_black);

            set_fg_color(wazaari, GTK_STATE_NORMAL, &color_black);
            set_bg_color(wazaari, GTK_STATE_NORMAL, &color_yellow);
            set_fg_color(yuko, GTK_STATE_NORMAL, &color_black);
            set_bg_color(yuko, GTK_STATE_NORMAL, &color_yellow);
            set_fg_color(koka, GTK_STATE_NORMAL, &color_black);
            set_bg_color(koka, GTK_STATE_NORMAL, &color_yellow);
            set_fg_color(shido, GTK_STATE_NORMAL, &color_black);
            set_bg_color(shido, GTK_STATE_NORMAL, &color_yellow);

            set_fg_color(cat1, GTK_STATE_NORMAL, &color_black);
            set_bg_color(cat1, GTK_STATE_NORMAL, &color_yellow);

            NO_SHOW(match1);
            NO_SHOW(match2);

            set_position(blue_name_1,  0.0, 0.0,  W_NAME, H_NAME);
            set_position(white_name_1, 0.0, H1,   W_NAME, H_NAME);
            set_position(blue_club,  0.0, H_NAME,      W_NAME, H_CLUB);
            set_position(white_club, 0.0, H1 + H_NAME, W_NAME, H_CLUB);

            NO_SHOW(blue_name_2);
            NO_SHOW(white_name_2);
            set_position(cat1, 0.0, H1+H2, W_CAT, H3);
            NO_SHOW(cat2);

            set_position(comment, W_CAT, H1 + H2, 1 - W_CAT - W_TIME, H_COMMENT);
            set_position(gs, W_CAT, H1 + H2 + H_COMMENT, 1 - W_CAT - W_TIME, H3 - H_TIME - H_COMMENT);
            set_position(sonomama, 1 - W_TIME, H1 + H2, W_TIME, H3 - H_TIME);

            set_position(wazaari, 1 - 4*W_SCORE, Y_IWYS, W_SCORE, H_IWYS);
            set_position(yuko,    1 - 3*W_SCORE, Y_IWYS, W_SCORE, H_IWYS);
            set_position(koka,    1 - 2*W_SCORE, Y_IWYS, W_SCORE, H_IWYS);
            set_position(shido,   1 - W_SCORE,   Y_IWYS, W_SCORE, H_IWYS);

            NO_SHOW(padding);

            set_position(bw, 1 - 4*W_SCORE, 0.0, W_SCORE, H_SCORE);
            set_position(by, 1 - 3*W_SCORE, 0.0, W_SCORE, H_SCORE);
            set_position(bk, 1 - 2*W_SCORE, 0.0, W_SCORE, H_SCORE);
            set_position(bs, 1 - W_SCORE,   0.0, W_SCORE, H_SCORE);

            set_position(ww, 1 - 4*W_SCORE, Y_SCORE, W_SCORE, H_SCORE);
            set_position(wy, 1 - 3*W_SCORE, Y_SCORE, W_SCORE, H_SCORE);
            set_position(wk, 1 - 2*W_SCORE, Y_SCORE, W_SCORE, H_SCORE);
            set_position(ws, 1 - W_SCORE,   Y_SCORE, W_SCORE, H_SCORE);

            set_position(t_min,  W_CAT,   1.0 - H_TIME, W_TIME, H_TIME);
            set_position(colon,  W_CAT + W_TIME, 1.0 - H_TIME, W_COLON, H_TIME);
            set_position(t_tsec, X_TSEC, 1.0 - H_TIME, W_TIME, H_TIME);
            set_position(t_sec,  X_TSEC + W_TIME, 1.0 - H_TIME, W_TIME, H_TIME);

            set_position(o_tsec, 1 - W_SEL - 3*W_TIME, 1.0 - H_TIME, W_TIME, H_TIME);
            set_position(o_sec,  1 - W_SEL - 2*W_TIME, 1.0 - H_TIME, W_TIME, H_TIME);

            set_position(pts_to_blue,  1 - W_SEL - W_TIME, 1.0 - H_TIME,     W_SEL, 0.5*H_TIME);
            set_position(pts_to_white, 1 - W_SEL - W_TIME, 1.0 - 0.5*H_TIME, W_SEL, 0.5*H_TIME);

            set_position(points,  1 - W_TIME, 1.0 - H_TIME, W_TIME, H_TIME);
        }
        break;
    }

    set_colors();
    expose(darea, 0, 0);

    g_key_file_set_integer(keyfile, "preferences", "displaylayout", (gint)data);

    /****
         if (mode == MODE_MASTER) {
         gint i;
         for (i = 0; i < num_labels; i++)
         labels[i].status = LABEL_STATUS_CHANGED | LABEL_STATUS_EXPOSE;

         display_big(" ", 4);
         }
    ****/
}

void select_name_layout(GtkWidget *menu_item, gpointer data)
{
    expose(darea, 0, 0);

    selected_name_layout = (gint)data; 
    g_key_file_set_integer(keyfile, "preferences", "namelayout", selected_name_layout);
}

void change_language_1(void)
{
        set_text(MY_LABEL(match1), _("Fight:"));
        set_text(MY_LABEL(match2), _("Next:"));
	expose(darea, 0, 0);
}

gboolean this_is_shiai(void)
{
	return FALSE;
}

gint application_type(void)
{
        return APPLICATION_TYPE_TIMER;
}

gboolean blue_background(void)
{
    return bgcolor == &color_blue;
}

