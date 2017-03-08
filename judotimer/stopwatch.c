/* -*- mode: C; c-basic-offset: 4;  -*- */

/*
 * Copyright (C) 2006-2016 by Hannu Jokinen
 * Full copyright text is included in the software package.
 */

/* Include system network headers */
#if defined(__WIN32__) || defined(WIN32)

#define  __USE_W32_SOCKETS
//#define Win32_Winsock

#include <windows.h>
#include <stdio.h>
#include <initguid.h>
#include <winsock2.h>

#else /* UNIX */

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#ifndef __BEOS__
#include <arpa/inet.h>
#endif

#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>

#endif /* WIN32 */

#include <gtk/gtk.h>
#include <glib.h>

#if (GTKVER == 3)
#include <gdk/gdkkeysyms-compat.h>
#else
#include <gdk/gdkkeysyms.h>
#endif

#ifdef WIN32
//#include <glib/gwin32.h>
#endif

#include <time.h>
#include <string.h>

#include "judotimer.h"

void manipulate_time(GtkWidget *widget, gpointer data );
static void gen_random_key(void);

void check_ippon(void);
void give_osaekomi_score();
gint approve_osaekomi_score(gint who);
//static void ask_for_hantei(void);
gboolean ask_for_golden_score(void);

enum {
    I = 0, W, Y, S, L
};

enum button_responses {
    Q_GS_NO_LIMIT = 1000,
    Q_GS_1_00,
    Q_GS_1_30,
    Q_GS_2_00,
    Q_GS_2_30,
    Q_GS_3_00,
    Q_GS_4_00,
    Q_GS_5_00,
    Q_GS_AUTO,
    Q_HANTEI_BLUE,
    Q_HANTEI_WHITE
};

struct stackval {
    gchar who;
    gchar points;
};

#define STACK_DEPTH 8

static gint      osaekomi_winner;
static gboolean  action_on;
static gboolean  running;
static gboolean  oRunning;
static gdouble   startTime;
static gdouble   oStartTime;
static gdouble   elap;
static gdouble   oElap;
static gint      score;
static gint      bluepts[5], whitepts[5];
static gboolean  big_displayed;
static gdouble   match_time;
static struct stackval stackval[STACK_DEPTH];
static gint      stackdepth;

static guint     last_m_time;
static gboolean  last_m_run;
static guint     last_m_otime;
static gboolean  last_m_orun;
static guint     last_score;
static gchar     osaekomi = ' ';
static gdouble   koka = 0.0;
static gdouble   yuko = 0.0;
static gdouble   wazaari = 10.0;
static gdouble   ippon = 20.0;
static gdouble   total;
static gdouble   gs_time = 0.0;
gint             tatami;
gint             demo = 0;
gboolean         automatic;
gboolean         golden_score = FALSE;
gboolean         rest_time = FALSE;
static gint      rest_flags = 0;
gchar           *matchlist = NULL;
static gint      gs_cat = 0, gs_num = 0;
gboolean         require_judogi_ok = FALSE;
static gboolean  jcontrol;
static gint      jcontrol_flags;
static gint      last_shido_to = 0;

static void log_scores(gchar *txt, gint who)
{
    judotimer_log("%s%s: IWYS = %d%d%ds%d - %d%d%ds%d", txt,
                  ((who == BLUE && white_first) || (who == WHITE && white_first == FALSE)) ? "white" : "blue",
                  bluepts[I],
                  bluepts[W],
                  bluepts[Y],
                  bluepts[S],
                  whitepts[I],
                  whitepts[W],
                  whitepts[Y],
                  whitepts[S]);
}

void beep(char *str)
{
    if (big_displayed)
        return;

    display_big(str, 4);
    big_displayed = TRUE;

    play_sound();
}

void update_display(void)
{
    guint t, min, sec, oSec;
    gint orun, score1;
    static gint flash = 0;

    // move last second to first second i.e. 0:00 = soremade
    if (elap == 0.0) t = total;
    else if (total == 0.0) t = elap;
    else {
        t = total - elap + 0.9;
        if (t == 0 && total > elap)
            t = 1;
    }

    min = t / 60;
    sec =  t - min*60;
    oSec = oElap;

    if (t != last_m_time) {
        last_m_time = t;
        set_timer_value(min, sec/10, sec%10);
        set_competitor_window_rest_time(min, sec/10, sec%10, rest_time, rest_flags);
    }

    if (oSec != last_m_otime) {
        last_m_otime = oSec;
        set_osaekomi_value(oSec/10, oSec%10);
    }

    if (running != last_m_run) {
        last_m_run = running;
        set_timer_run_color(last_m_run, rest_time);
    }

    if (oRunning) {
        //orun = oRunning ? OSAEKOMI_DSP_YES : OSAEKOMI_DSP_NO;
	if (score >= 2) {
            if (osaekomi_winner == BLUE) orun = OSAEKOMI_DSP_BLUE;
	    else if (osaekomi_winner == WHITE) orun = OSAEKOMI_DSP_WHITE;
	    else orun = OSAEKOMI_DSP_UNKNOWN;

            if (++flash > 5)
		orun = OSAEKOMI_DSP_YES2;
            if (flash > 10)
		flash = 0;
	} else {
	    orun = OSAEKOMI_DSP_YES;
	}

        score1 = score;
    } else if (stackdepth) {
        if (stackval[stackdepth-1].who == BLUE)
            orun = OSAEKOMI_DSP_BLUE;
        else if (stackval[stackdepth-1].who == WHITE)
            orun = OSAEKOMI_DSP_WHITE;
        else
            orun = OSAEKOMI_DSP_UNKNOWN;

        if (++flash > 5)
            orun = OSAEKOMI_DSP_YES2;
        if (flash > 10)
            flash = 0;

        score1 = stackval[stackdepth-1].points;
    } else {
        orun = OSAEKOMI_DSP_NO;
        score1 = 0;
    }

    if (orun != last_m_orun) {
        last_m_orun = orun;
        set_timer_osaekomi_color(last_m_orun, score1, oRunning);
    }

    if (last_score != score1) {
        last_score = score1;
        set_score(last_score);
    }
}

void update_clock(void)
{
    gboolean show_soremade = FALSE;

    if (running) {
	elap = g_timer_elapsed(timer, NULL) - startTime;
	if (total > 0.0 && elap >= total) {
	    //running = false;
	    elap = total;
	    if (!oRunning) {
		running = FALSE;
		if (rest_time == FALSE)
		    show_soremade = TRUE;
		if (rest_time) {
		    elap = 0;
		    oRunning = FALSE;
		    oElap = 0;
		    rest_time = FALSE;
		}
	    }
	}
    }

    if (running && oRunning) {
	oElap =  g_timer_elapsed(timer, NULL) - oStartTime;
	score = 0;
	if (oElap >= yuko && !use_2017_rules)
	    score = 2;
	if (oElap >= wazaari) {
	    score = 3;
	    if (!use_2017_rules &&
		((osaekomi_winner == BLUE && bluepts[W]) ||
		 (osaekomi_winner == WHITE && whitepts[W]))) {
		running = FALSE;
		oRunning = FALSE;
		give_osaekomi_score();
		approve_osaekomi_score(0);
	    }
	}
	if (oElap >= ippon) {
	    score = 4;
	    running = FALSE;
	    oRunning = FALSE;
	    gint tmp = osaekomi_winner;
	    give_osaekomi_score();
	    if (tmp)
		approve_osaekomi_score(0);
	}
    }

    update_display();

    if (show_soremade) {
        beep("SOREMADE");
#if 0
        if (golden_score &&
            (array2int(bluepts) & 0xfffffff0) == (array2int(whitepts) & 0xfffffff0))
            ask_for_hantei();
#endif
    }

    // judogi control display for ad
    {
        static gint cnt = 0;
        if (++cnt >= 10) {
            set_competitor_window_judogi_control(jcontrol, jcontrol_flags);
            cnt = 0;
        }
    }

    if (demo) {
        if (demo == 1)
            gen_random_key();
        else {
	    static time_t nexttime = 0;
	    if (time(NULL) < nexttime)
		return;
	    nexttime = time(NULL) + demo;

            static gint last_cat = 0, last_num = 0;
            static gint res[9][2][4] = {
                {{2,1,0,0},{0,2,0,2}},
                {{0,0,3,0},{1,0,0,0}},
                {{0,1,0,0},{0,2,0,0}},
                {{3,0,0,0},{0,1,0,0}},
                {{0,0,0,2},{1,1,0,0}},
                {{2,2,0,0},{0,0,0,1}},
                {{0,4,0,0},{0,1,0,0}},
                {{3,0,0,0},{1,2,0,0}},

                {{0,1,0,1},{0,1,0,1}},
            };

            if (last_cat != current_category || last_num != current_match) {
                if (matchlist) {
                    static gint next_res = 0;
                    gint len = strlen(matchlist);
                    switch (matchlist[next_res]) {
                    case '0': send_result(res[8][0], res[8][1], 0, 0, 0, 0, 0, 1); break; // hikiwake
                    case '1': send_result(res[0][0], res[0][1], 0, 0, 0, 0, 0, 0); break; // 1st wins by ippon
                    case '2': send_result(res[0][1], res[0][0], 0, 0, 0, 0, 0, 0); break; // 2nd wins by ippon
                    case '3': send_result(res[2][1], res[2][0], 0, 0, 0, 0, 0, 0); break; // 1st wins by yuko
                    default: send_result(res[2][0], res[2][1], 0, 0, 0, 0, 0, 0);         // 2nd wins by yuko
                    }
                    if (++next_res >= len) next_res = 0;
                } else {
                    static gint next_res = 0;
                    gint ix = (next_res++ /*current_category + current_match*/) & 7;
                    send_result(res[ix][0], res[ix][1], 0, 0, 0, 0, (ix&3)==2 ? 0x100 + ix : ix, 0);
                }
                last_cat = current_category;
                last_num = current_match;
            }

        } // else
    }
}

void hajime_inc_func(gdouble fix)
{
    if (running)
        return;
    if (total == 0) {
        elap += fix;
    } else if (elap >= fix)
        elap -= fix;
    else
        elap = 0.0;
    update_display();
}

void hajime_dec_func(gdouble fix)
{
    if (running)
        return;
    if (total == 0) {
        if (elap >= fix) elap -= fix;
    } else if (elap <= (total - fix))
        elap += fix;
    else
        elap = total;
    update_display();
}

void osaekomi_inc_func(void)
{
    if (running)
        return;
    if (oElap <= (ippon - 1.0))
        oElap += 1.0;
    else
        oElap = ippon;
    oRunning = oElap > 0;

    update_display();
}

void osaekomi_dec_func(void)
{
    if (running)
        return;
    if (oElap >= 1.0)
        oElap -= 1.0;
    else
        oElap = 0;
    oRunning = oElap > 0;
    update_display();
}

void set_clocks(gint clock, gint osaekomi1)
{
    if (running)
        return;

    if ( clock >= 0 && total >= clock)
    	elap = total - (gdouble)clock;
    else if (clock >= 0)
        elap = (gdouble)clock;

    if (osaekomi1) {
        oElap = (gdouble)osaekomi1;
        oRunning = TRUE;
    }

    update_display();
}


static void toggle(void)
{
    if (running) {
        judotimer_log("Shiai clock stop");
        running = FALSE;
        elap = g_timer_elapsed(timer, NULL) - startTime;
        if (total > 0.0 && elap >= total)
            elap = total;
        if (oRunning) {
            //oRunning = FALSE;
            //oElap = 0;
            //give_osaekomi_score();
        } else {
            oElap = 0;
            score = 0;
        }
        update_display();
        video_record(FALSE);
    } else {
        if (total > 9000) // don't let clock run if dashes in display '-:--'
            return;

        if (elap == 0)
            judotimer_log("MATCH START: %s: %s - %s",
                          get_cat(), get_name(BLUE), get_name(WHITE));
        judotimer_log("Shiai clock start");
        running = TRUE;
        startTime = g_timer_elapsed(timer, NULL) - elap;
        if (oRunning) {
            oStartTime = g_timer_elapsed(timer, NULL) - oElap;
            set_comment_text("");
        }
        update_display();
        video_record(TRUE);
    }
}

int get_match_time(void)
{
    if (demo >= 2)
        return (10 + rand()%200);

    return elap + match_time;
}

static void oToggle() {
    /*
    if (running == FALSE && (elap < total || total == 0.0)) {
        if (oRunning)
            toggle();
        return;
    }
    */
    if (oRunning) {
        judotimer_log("Osaekomi clock stop after %f s", oElap);
        oRunning = FALSE;
        oElap = 0;
        set_comment_text("");
        update_display();
        if (total > 0.0 && elap >= total)
            beep("SOREMADE");
    } else {
        judotimer_log("Osaekomi clock start");
        //running = TRUE;
        osaekomi = ' ';
        oRunning = TRUE;
        oStartTime = g_timer_elapsed(timer, NULL);
        update_display();
    }
}

int clock_running(void)
{
    return action_on;
}

gboolean osaekomi_running(void)
{
    return oRunning;
}

void set_hantei_winner(gint cmd)
{
    if (cmd == CLEAR_SELECTION)
        big_displayed = FALSE;

    if (cmd == HANSOKUMAKE_BLUE)
        whitepts[I] = 1;
    else if (cmd == HANSOKUMAKE_WHITE)
        bluepts[I] = 1;

    check_ippon();
}

gboolean ask_for_golden_score(void)
{
    GtkWidget *dialog;
    gint response;

    golden_score = FALSE;
    gs_cat = gs_num = 0;

    dialog = gtk_dialog_new_with_buttons (_("Start Golden Score?"),
                                          NULL,
                                          GTK_DIALOG_MODAL,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          "Auto",           Q_GS_AUTO,
                                          _("No Limit"),    Q_GS_NO_LIMIT,
                                          "1:00 min",       Q_GS_1_00,
                                          "1:30 min",       Q_GS_1_30,
                                          "2:00 min",       Q_GS_2_00,
                                          "2:30 min",       Q_GS_2_30,
                                          "3:00 min",       Q_GS_3_00,
                                          "4:00 min",       Q_GS_4_00,
                                          "5:00 min",       Q_GS_5_00,
                                          NULL);

    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
    response = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    switch (response) {
    case Q_GS_NO_LIMIT: gs_time = 0.0; break;
    case Q_GS_1_00: gs_time = 60.0; break;
    case Q_GS_1_30: gs_time = 90.0; break;
    case Q_GS_2_00: gs_time = 120.0; break;
    case Q_GS_2_30: gs_time = 150.0; break;
    case Q_GS_3_00: gs_time = 180.0; break;
    case Q_GS_4_00: gs_time = 240.0; break;
    case Q_GS_5_00: gs_time = 300.0; break;
    case Q_GS_AUTO: gs_time = 10000.0; break;
    }

    if (response >= Q_GS_NO_LIMIT && response <= Q_GS_AUTO) {
	last_shido_to = 0;
        golden_score = TRUE;
        gs_cat = current_category;
        gs_num = current_match;
        if (response == Q_GS_AUTO)
            automatic = TRUE;
        else
            automatic = FALSE;
    }

    return golden_score;
}

#if 0
static void ask_for_hantei(void)
{
    GtkWidget *dialog;
    gint response;

    golden_score = FALSE;
    dialog = gtk_dialog_new_with_buttons (_("Hantei"),
                                          NULL,
                                          GTK_DIALOG_MODAL,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          _("Blue wins"),      Q_HANTEI_BLUE,
                                          _("White wins"),     Q_HANTEI_WHITE,
                                          NULL);
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);

    response = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    switch (response) {
    case Q_HANTEI_BLUE: voting_result(NULL, (gpointer)HANTEI_BLUE); break;
    case Q_HANTEI_WHITE: voting_result(NULL, (gpointer)HANTEI_WHITE); break;
    }
}
#endif

static gboolean asking = FALSE;
static guint key_pending = 0;
static GtkWidget *ask_area;

#define ASK_OK  0x7301
#define ASK_NOK 0x7302

static gint get_winner(gboolean final_result)
{
    // final_result is true when result is sent to judoshiai. It is up to
    // the user to ensure result is correct i.e. shidos determine the winner correctly.
    gint winner = 0;

    if (use_ger_u12_rules) {
	gint p1 = bluepts[I]*use_ger_u12_rules +
	    bluepts[W]*2 + bluepts[Y];
	gint p2 = whitepts[I]*use_ger_u12_rules +
	    whitepts[W]*2 + whitepts[Y];
	if (p1 > p2) winner = BLUE;
	else if (p1 < p2) winner = WHITE;
    } else if (bluepts[I] > whitepts[I]) winner = BLUE;
    else if (bluepts[I] < whitepts[I]) winner = WHITE;
    else if (bluepts[W] > whitepts[W]) winner = BLUE;
    else if (bluepts[W] < whitepts[W]) winner = WHITE;
    else if (bluepts[Y] > whitepts[Y]) winner = BLUE;
    else if (bluepts[Y] < whitepts[Y]) winner = WHITE;
    else if (blue_wins_voting) winner = BLUE;
    else if (white_wins_voting) winner = WHITE;
    else if (hansokumake_to_white) winner = BLUE;
    else if (hansokumake_to_blue) winner = WHITE;
    else if (use_2017_rules && !final_result) {
	if (golden_score) {
	    if (last_shido_to == BLUE &&
		(bluepts[S] > whitepts[S])) winner = WHITE;
	    else if (last_shido_to == WHITE &&
		     (whitepts[S] > bluepts[S])) winner = BLUE;
	}
    } else if (bluepts[S] || whitepts[S]) {
        if (bluepts[S] > whitepts[S]) winner = WHITE;
        else if (bluepts[S] < whitepts[S]) winner = BLUE;
    }

    return winner;
}

static GtkWindow *ask_window = NULL;
static GtkWidget *legend_widget;
static gint legend;

static gboolean delete_event_ask( GtkWidget *widget, GdkEvent  *event, gpointer   data )
{
    return FALSE;
}

static void destroy_ask( GtkWidget *widget, gpointer   data )
{
    struct message msg;
    memset(&msg, 0, sizeof(msg));
    msg.type = MSG_UPDATE_LABEL;
    msg.u.update_label.label_num = STOP_WINNER;
    /*write_tv_logo(&(msg.u.update_label));*/

    if (mode != MODE_SLAVE)
        send_label_msg(&msg);

    legend_widget = NULL;
    ask_area = NULL;
    asking = FALSE;
    ask_window = NULL;
}

static gboolean close_ask_ok(GtkWidget *widget, gpointer userdata)
{
    if (legend_widget)
        legend = gtk_combo_box_get_active(GTK_COMBO_BOX(legend_widget));
    else
        legend = 0;

    if (legend < 0) legend = 0;

    reset(ASK_OK, NULL);
    gtk_widget_destroy(userdata);
    return FALSE;
}

static gboolean close_ask_nok(GtkWidget *widget, gpointer userdata)
{
    legend = 0;
    reset(ASK_NOK, NULL);
    gtk_widget_destroy(userdata);
    return FALSE;
}

#define FIRST_BLOCK_C 0.25
#define FIRST_BLOCK_START  0.0
#define SECOND_BLOCK_START  (FIRST_BLOCK_C*height)
#define THIRD_BLOCK_START ((1.0+FIRST_BLOCK_C)*height/2.0)
#define FIRST_BLOCK_HEIGHT (FIRST_BLOCK_C*height)
#define OTHER_BLOCK_HEIGHT ((height-FIRST_BLOCK_HEIGHT)/2.0)

static gboolean expose_ask(GtkWidget *widget, GdkEventExpose *event, gpointer userdata)
{
    gint width, height, winner = get_winner(TRUE);
    const gchar *last_wname, *first_wname, *wtext = _("WINNER");

    if (!winner)
        return FALSE;

    if (winner == BLUE) {
        last_wname = saved_last1;
        first_wname = saved_first1;
    } else {
        last_wname = saved_last2;
        first_wname = saved_first2;
    }

#if (GTKVER == 3)
    width = gtk_widget_get_allocated_width(widget);
    height = gtk_widget_get_allocated_height(widget);
#else
    width = widget->allocation.width;
    height = widget->allocation.height;
#endif

    cairo_text_extents_t extents;
#if (GTKVER == 3)
    cairo_t *c = (cairo_t *)event;
#else
    cairo_t *c = gdk_cairo_create(widget->window);
#endif

    cairo_set_source_rgb(c, 0.0, 0.0, 0.0);
    cairo_rectangle(c, 0.0, 0.0, width, FIRST_BLOCK_HEIGHT);
    cairo_fill(c);

    if ((winner == BLUE && white_first) || (winner == WHITE && white_first == FALSE))
        cairo_set_source_rgb(c, 1.0, 1.0, 1.0);
    else {
        if (blue_background())
            cairo_set_source_rgb(c, 0, 0, 1.0);
        else
            cairo_set_source_rgb(c, 1.0, 0, 0);
    }

    cairo_rectangle(c, 0.0, SECOND_BLOCK_START, width, 2*OTHER_BLOCK_HEIGHT);
    cairo_fill(c);

    cairo_set_font_size(c, 0.6*FIRST_BLOCK_HEIGHT);
    cairo_set_source_rgb(c, 1.0, 1.0, 1.0);
    cairo_text_extents(c, wtext, &extents);
    cairo_move_to(c, width - 10.0 - extents.width, (FIRST_BLOCK_HEIGHT - extents.height)/2.0 - extents.y_bearing);
    cairo_show_text(c, wtext);
    cairo_text_extents(c, saved_cat, &extents);
    cairo_move_to(c, 10.0, (FIRST_BLOCK_HEIGHT - extents.height)/2.0 - extents.y_bearing);
    cairo_show_text(c, saved_cat);

    if ((winner == BLUE && white_first) || (winner == WHITE && white_first == FALSE))
        cairo_set_source_rgb(c, 0, 0, 0);
    else
        cairo_set_source_rgb(c, 1.0, 1.0, 1.0);

    cairo_set_font_size(c, 0.6*OTHER_BLOCK_HEIGHT);
    cairo_text_extents(c, last_wname, &extents);
    cairo_move_to(c, 10.0, SECOND_BLOCK_START + (OTHER_BLOCK_HEIGHT - extents.height)/2.0 - extents.y_bearing);
    cairo_show_text(c, last_wname);
    cairo_text_extents(c, first_wname, &extents);
    cairo_move_to(c, 10.0, THIRD_BLOCK_START + (OTHER_BLOCK_HEIGHT - extents.height)/2.0 - extents.y_bearing);
    cairo_show_text(c, first_wname);

#if (GTKVER != 3)
    cairo_show_page(c);
    cairo_destroy(c);
#endif

    return FALSE;
}

static gchar *legends[] =
    {"?", "(T)", "(H)", "(C)", "(L)", "SG", "HM", "KG", "T", "H", "S", "/P\\", "FG", "TT", "TH", "/HM\\", NULL};

static void create_ask_window(void)
{
    GtkWidget *vbox, *hbox, *ok, *nok, *lbl;
    gint width;
    gint height;
    gtk_window_get_size(GTK_WINDOW(main_window), &width, &height);

    GtkWindow *window = ask_window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
    gtk_window_set_keep_above(GTK_WINDOW(ask_window), TRUE);
    gtk_window_set_title(GTK_WINDOW(window), _("Start New Match?"));
    if (fullscreen ) // Clicking outside  && show_competitor_names && get_winner())
        gtk_window_fullscreen(GTK_WINDOW(window));
    else if (show_competitor_names && get_winner(TRUE))
        gtk_widget_set_size_request(GTK_WIDGET(window), width, height);

    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

#if (GTKVER == 3)
    vbox = gtk_grid_new();
#else
    vbox = gtk_vbox_new(FALSE, 1);
#endif
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 1);

    if (mode != MODE_SLAVE) {
#if (GTKVER == 3)
        hbox = gtk_grid_new();
#else
        hbox = gtk_hbox_new(FALSE, 1);
#endif
        lbl = gtk_label_new(_("Start New Match?"));
        ok = gtk_button_new_with_label(_("OK"));
        nok = gtk_button_new_with_label(_("Cancel"));

        gint i;
#if (GTKVER == 3)
        legend_widget = gtk_combo_box_text_new();
#else
        legend_widget = gtk_combo_box_new_text();
#endif
        for (i = 0; legends[i]; i++)
#if (GTKVER == 3)
            gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(legend_widget), NULL, legends[i]);
#else
            gtk_combo_box_append_text(GTK_COMBO_BOX(legend_widget), legends[i]);
#endif
#if (GTKVER == 3)
        gtk_grid_attach(GTK_GRID(hbox), lbl, 0, 0, 1, 1);
        gtk_grid_attach(GTK_GRID(hbox), ok, 1, 0, 1, 1);
        gtk_grid_attach(GTK_GRID(hbox), nok, 2, 0, 1, 1);
        gtk_grid_attach(GTK_GRID(hbox), legend_widget, 3, 0, 1, 1);

        gtk_grid_attach(GTK_GRID(vbox), hbox, 0, 0, 1, 1);
#else
        gtk_box_pack_start(GTK_BOX(hbox), lbl, FALSE, TRUE, 5);
        gtk_box_pack_start(GTK_BOX(hbox), ok, FALSE, TRUE, 5);
        gtk_box_pack_start(GTK_BOX(hbox), nok, FALSE, TRUE, 5);
        gtk_box_pack_start(GTK_BOX(hbox), legend_widget, FALSE, TRUE, 5);

        gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
#endif
    } else {
        lbl = gtk_label_new(_("- - -"));
#if (GTKVER == 3)
        gtk_grid_attach(GTK_GRID(vbox), lbl, 0, 0, 1, 1);
#else
        gtk_box_pack_start(GTK_BOX(vbox), lbl, FALSE, TRUE, 5);
#endif
    }

    if (show_competitor_names && get_winner(TRUE)) {
        ask_area = gtk_drawing_area_new();
#if (GTKVER == 3)
        gtk_widget_set_hexpand(ask_area, TRUE);
        gtk_widget_set_vexpand(ask_area, TRUE);
        gtk_grid_attach(GTK_GRID(vbox), ask_area, 0, 1, 1, 1);
        g_signal_connect(G_OBJECT(ask_area),
                         "draw", G_CALLBACK(expose_ask), NULL);
#else
        gtk_box_pack_start(GTK_BOX(vbox), ask_area, TRUE, TRUE, 5);
        g_signal_connect(G_OBJECT(ask_area),
                         "expose-event", G_CALLBACK(expose_ask), NULL);
#endif
    }

    gtk_container_add (GTK_CONTAINER (window), vbox);
    gtk_widget_show_all(GTK_WIDGET(window));

    g_signal_connect (G_OBJECT (window), "delete_event",
                      G_CALLBACK (delete_event_ask), NULL);
    g_signal_connect (G_OBJECT (window), "destroy",
                      G_CALLBACK (destroy_ask), NULL);

    if (mode != MODE_SLAVE) {
        g_signal_connect(G_OBJECT(ok),
                         "clicked", G_CALLBACK(close_ask_ok), window);
        g_signal_connect(G_OBJECT(nok),
                         "clicked", G_CALLBACK(close_ask_nok), window);
    }
    gtk_window_set_modal(GTK_WINDOW(window),TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(window),GTK_WINDOW(main_window));

}

void close_ask_window(void)
{
    close_ask_ok(NULL, ask_window);
}

void display_ask_window(gchar *name, gchar *cat, gchar winner)
{
    gchar first[32], last[32], club[32], country[32];

    if (demo) return;

    parse_name(name, first, last, club, country);

    memset(bluepts, 0, sizeof(bluepts));
    memset(whitepts, 0, sizeof(whitepts));

    if (winner == BLUE) {
        bluepts[I] = 1;
        whitepts[I] = 0;
        strncpy(saved_last1, last, sizeof(saved_last1)-1);
        strncpy(saved_first1, first, sizeof(saved_first1)-1);
    } else if (winner == WHITE) {
        bluepts[I] = 0;
        whitepts[I] = 1;
        strncpy(saved_last2, last, sizeof(saved_last2)-1);
        strncpy(saved_first2, first, sizeof(saved_first2)-1);
    }

    strncpy(saved_cat, cat, sizeof(saved_cat)-1);
    create_ask_window();
}

void reset(guint key, struct msg_next_match *msg0)
{
    gboolean asked = FALSE;

    if (key == ASK_OK) {
        if (0 && rest_time) {
            cancel_rest_time(rest_flags & MATCH_FLAG_BLUE_REST,
                             rest_flags & MATCH_FLAG_WHITE_REST);
        }
        key = key_pending;
        key_pending = 0;
        asking = FALSE;
        asked = TRUE;
    } else if (key == ASK_NOK) {
        key_pending = 0;
        asking = FALSE;
        return;
    }

    if ((running && rest_time == FALSE) || oRunning || asking)
        return;

    set_gs_text("");

    if (msg0 && (current_category != msg0->category || current_match != msg0->match))
        judotimer_log("Automatic next match %d:%d (%s - %s)",
                      msg0->category, msg0->match,
                      msg0->blue_1, msg0->white_1);

    if (key != GDK_0) {
        golden_score = FALSE;
    }

    gint bp;
    gint wp;
    bp = array2int(bluepts);
    wp = array2int(whitepts);

    if (use_2017_rules) {
	// ignore shidos
	bp &= ~0xf;
	wp &= ~0xf;
    }

    if (key == GDK_9 ||
        (demo == 0 && bp == wp && result_hikiwake == FALSE &&
         blue_wins_voting == white_wins_voting &&
         total > 0.0 && elap >= total && key != GDK_0)) {
        asking = TRUE;
        ask_for_golden_score();
        asking = FALSE;
        if (key == GDK_9 && golden_score == FALSE)
            return;
        key = GDK_9;
        match_time = elap;
        judotimer_log("Golden score starts");
        set_gs_text("GOLDEN SCORE");
    } else if (demo == 0 &&
               asked == FALSE &&
               ((bluepts[I] == 0 && whitepts[I] == 0 &&
                 elap > 0.01 && elap < total - 0.01) ||
                (running && rest_time) ||
                rules_confirm_match) &&
               key != GDK_0) {
        key_pending = key;
        asking = TRUE;
        create_ask_window();

        struct message msg1;
        gint winner = get_winner(TRUE);
        memset(&msg1, 0, sizeof(msg1));
        msg1.type = MSG_UPDATE_LABEL;
        msg1.u.update_label.label_num = START_WINNER;
        if (result_hikiwake)
            snprintf(msg1.u.update_label.text, sizeof(msg1.u.update_label.text),
                     "%s\t\t\t", _("Hikiwake"));
        else
            snprintf(msg1.u.update_label.text, sizeof(msg1.u.update_label.text),
                     "%s\t%s\t\t",
                     winner == BLUE ? saved_last1 : saved_last2,
                     winner == BLUE ? saved_first1 : saved_first2);
        strncpy(msg1.u.update_label.text2, saved_cat,
                sizeof(msg1.u.update_label.text2)-1);
        msg1.u.update_label.text3[0] = winner;
        /*write_tv_logo(&(msg1.u.update_label));*/

        if (mode != MODE_SLAVE)
            send_label_msg(&msg1);

        return;
#if 0
        dialog = gtk_dialog_new_with_buttons (_("Start New Match?"),
					      NULL,
					      GTK_DIALOG_MODAL,
					      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					      GTK_STOCK_OK, GTK_RESPONSE_OK,
					      NULL);

        GtkWidget *darea = gtk_drawing_area_new();
        gtk_container_add(GTK_CONTAINER (window), darea);

        gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
        response = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        asking = FALSE;
        if (response != GTK_RESPONSE_OK)
            return;
#endif
    }

    if (key != GDK_0 && rest_time) {
        if (rest_flags & MATCH_FLAG_BLUE_REST)
            cancel_rest_time(TRUE, FALSE);
        else if (rest_flags & MATCH_FLAG_WHITE_REST)
            cancel_rest_time(FALSE, TRUE);
    }

    rest_time = FALSE;

    if (key != GDK_0 && golden_score == FALSE) {
        // set bit if golden score
        if (gs_cat == current_category && gs_num == current_match)
            legend |= 0x100;

        if (sides_switched) {
            send_result(whitepts, bluepts,
                        white_wins_voting, blue_wins_voting,
                        hansokumake_to_white, hansokumake_to_blue, legend, result_hikiwake);
            clear_switch_sides();
        } else
            send_result(bluepts, whitepts,
                        blue_wins_voting, white_wins_voting,
                        hansokumake_to_blue, hansokumake_to_white, legend, result_hikiwake);
        match_time = 0;
        gs_cat = gs_num = 0;
    }

    if (golden_score == FALSE) {
        memset(bluepts, 0, sizeof(bluepts));
        memset(whitepts, 0, sizeof(whitepts));
    }

    blue_wins_voting = 0;
    white_wins_voting = 0;
    hansokumake_to_blue = 0;
    hansokumake_to_white = 0;
    if (key != GDK_0) result_hikiwake = 0;
    osaekomi_winner = 0;
    big_displayed = FALSE;

    running = FALSE;
    elap = 0;
    oRunning = FALSE;
    oElap = 0;

    memset(&(stackval), 0, sizeof(stackval));
    stackdepth = 0;

    if (use_2017_rules) {
        koka    = 0.000;
        yuko    = 0.000;
        wazaari = 10.000;
        ippon   = 20.000;
    } else {
        koka    = 0.000;
        yuko    = 10.000;
        wazaari = 15.000;
        ippon   = 20.000;
    }

    switch (key) {
    case GDK_0:
        if (msg0) {
            /*g_print("is-gs=%d match=%d gs=%d rep=%d flags=0x%x\n",
              golden_score, msg->match_time, msg->gs_time, msg->rep_time, msg->flags);*/
            if ((msg0->flags & MATCH_FLAG_REPECHAGE) && msg0->rep_time) {
                total = msg0->rep_time;
                golden_score = TRUE;
            } else if (golden_score)
                total = msg0->gs_time;
            else
                total = msg0->match_time;

            koka    = msg0->pin_time_koka;
            yuko    = msg0->pin_time_yuko;
            wazaari = msg0->pin_time_wazaari;
            ippon   = msg0->pin_time_ippon;

            jcontrol = FALSE;
            if (require_judogi_ok &&
                (!(msg0->flags & MATCH_FLAG_JUDOGI1_OK) ||
                 !(msg0->flags & MATCH_FLAG_JUDOGI2_OK))) {
                gchar buf[128];
                snprintf(buf, sizeof(buf), "%s: %s", _("CONTROL"),
                         !(msg0->flags & MATCH_FLAG_JUDOGI1_OK) ? get_name(BLUE) : get_name(WHITE));
                display_big(buf, 2);
                jcontrol = TRUE;
                jcontrol_flags = msg0->flags;
            } else if (msg0->rest_time) {
                gchar buf[128];
                snprintf(buf, sizeof(buf), "%s: %s", _("REST TIME"),
                         msg0->minutes & MATCH_FLAG_BLUE_REST ?
                         get_name(BLUE) :
                         get_name(WHITE));
                rest_time = TRUE;
                rest_flags = msg0->minutes;
                total = msg0->rest_time;
                startTime = g_timer_elapsed(timer, NULL);
                running = TRUE;
                display_big(buf, msg0->rest_time);
                if (current_category != msg0->category || current_match != msg0->match)
                    judotimer_log("Rest time (%d s) started for %d:%d (%s - %s)",
                                  msg0->rest_time,
                                  msg0->category, msg0->match,
                                  msg0->blue_1, msg0->white_1);
                return;
            }
        }
        break;
    case GDK_1:
        total   = golden_score ? gs_time : 120.0;
        koka    = 0.0;
        yuko    = 5.0;
        wazaari = 10.0;
        ippon   = 15.0;
        break;
    case GDK_2:
        total   = golden_score ? gs_time : 120.000;
        break;
    case GDK_3:
        total   = golden_score ? gs_time : 180.000;
        break;
    case GDK_4:
        total   = golden_score ? gs_time : 240.000;
        break;
    case GDK_5:
        total   = golden_score ? gs_time : 300.000;
        break;
    case GDK_6:
        total   = golden_score ? gs_time : 150.000;
        break;
    case GDK_7:
        total   = 10000.000;
        break;
    case GDK_9:
        if (golden_score)
            total = gs_time;
        break;
    }

    action_on = 0;

    if (key != GDK_0) {
        set_comment_text("");
        delete_big(NULL);
    }

    reset_display(key);
    update_display();

    if (key != GDK_0) {
        display_ad_window();
        if (mode != MODE_SLAVE)
            send_label(START_ADVERTISEMENT);
    }
}

static void incdecpts(gint *p, gboolean decrement)
{
    if (decrement) {
        big_displayed = FALSE;
        if (*p > 0) (*p)--;
    } else if (*p < 99)
        (*p)++;
}

#define INC FALSE
#define DEC TRUE

#define SHIDOMAX (use_2017_rules ? 3 : 4)

void check_ippon(void)
{
    if (use_ger_u12_rules) {
	if (bluepts[W] >= 2) {
	    bluepts[I]++;
	    bluepts[W] = 0;
	}
	if (whitepts[W] >= 2) {
	    whitepts[I]++;
	    whitepts[W] = 0;
	}
	if (bluepts[I] > 2)
	    bluepts[I] = 2;
	if (whitepts[I] > 2)
	    whitepts[I] = 2;
    } else {
	if (bluepts[I] > 1)
	    bluepts[I] = 1;
	if (whitepts[I] > 1)
	    whitepts[I] = 1;

	if (!use_2017_rules) {
	    if (bluepts[W] >= 2) {
		bluepts[I] = 1;
		bluepts[W] = 0;
	    }
	    if (whitepts[W] >= 2) {
		whitepts[I] = 1;
		whitepts[W] = 0;
	    }
	} else {
	    if (bluepts[L] >= 2)
		whitepts[I] = 1;
	    if (whitepts[L] >= 2)
		bluepts[I] = 1;
	}
    }

    set_points(bluepts, whitepts);

    if (use_ger_u12_rules) {
	gint p1 = bluepts[I]*use_ger_u12_rules +
	    bluepts[W]*2 + bluepts[Y];
	gint p2 = whitepts[I]*use_ger_u12_rules +
	    whitepts[W]*2 + whitepts[Y];
	if (p1 >= 2*use_ger_u12_rules ||
	    p2 >= 2*use_ger_u12_rules) {
	    if (rules_stop_ippon_2 == TRUE) {
		if (oRunning) oToggle();
		if (running) toggle();
	    }
	    beep("SOREMADE");
	}
    } else if (whitepts[I] || bluepts[I]) {
        if (rules_stop_ippon_2 == TRUE) {
	    if (oRunning) oToggle();
	    if (running) toggle();
        }
        beep("IPPON");

        gchar *name = get_name(whitepts[I] ? WHITE : BLUE);
        if (name == NULL || name[0] == 0)
            name = whitepts[I] ? "white" : "blue";

        judotimer_log("%s: %s wins by %f s Ippon)!",
                      get_cat(), name, elap);
    } else if (golden_score && get_winner(FALSE)) {
        beep(_("Golden Score"));
    } else if ((whitepts[S] >= SHIDOMAX || bluepts[S] >= SHIDOMAX) &&
               rules_stop_ippon_2) {
    	if (oRunning) oToggle();
    	if (running) toggle();
        beep("SHIDO, Hansokumake");
        gchar *name = get_name(whitepts[I] ? WHITE : BLUE);
        if (name == NULL || name[0] == 0)
            name = whitepts[I] ? "white" : "blue";

        judotimer_log("%s: %s wins by %f s Shido)!",
                      get_cat(), name, elap);
    }
}

gboolean set_osaekomi_winner(gint who)
{
    if (oRunning == 0) {
        return approve_osaekomi_score(who & CMASK);
    }

    if (who & GIVE_POINTS /*osaekomi_winner == who*/)
        return FALSE;

    osaekomi_winner = who;

    if ((who == BLUE && white_first == FALSE) ||
        ((who == WHITE && white_first == TRUE)))
        set_comment_text(_("Points going to blue"));
    else if ((who == WHITE && white_first == FALSE) ||
             ((who == BLUE && white_first == TRUE)))
        set_comment_text(_("Points going to white"));
    else
        set_comment_text("");

    return TRUE;
}

gint approve_osaekomi_score(gint who)
{
    if (stackdepth == 0)
        return FALSE;

    stackdepth--;

    if (clocks_only)
        return FALSE;

    if (who == 0)
        who = stackval[stackdepth].who;

    switch (stackval[stackdepth].points) {
    case 1: // no koka
        break;
    case 2:
        if (who == BLUE)
            incdecpts(&bluepts[Y], 0);
        else if (who == WHITE)
            incdecpts(&whitepts[Y], 0);
        break;
    case 4:
        if (who == BLUE)
            incdecpts(&bluepts[I], 0);
        else if (who == WHITE)
            incdecpts(&whitepts[I], 0);
	break;
    case 3:
        if (who == BLUE)
            incdecpts(&bluepts[W], 0);
        else if (who == WHITE)
            incdecpts(&whitepts[W], 0);
        break;
    }

    if (who == BLUE)
        log_scores("Osaekomi score to ", BLUE);
    else
        log_scores("Osaekomi score to ", WHITE);

    check_ippon();

    return TRUE;
}

void give_osaekomi_score()
{
    if (score) {
        stackval[stackdepth].who =
            osaekomi_winner;
        stackval[stackdepth].points =
            score;
        if (stackdepth < (STACK_DEPTH - 1))
            stackdepth++;
    }

    score = 0;
    osaekomi_winner = 0;
}

void clock_key(guint key, guint event_state)
{
    gboolean shift = event_state & 1;
    gboolean ctl = event_state & 4;
    static guint lastkey;
    static gdouble lasttime, now;

    now = g_timer_elapsed(timer, NULL);
    if (now < lasttime + 0.1 && key == lastkey)
        return;

    lastkey = key;
    lasttime = now;

    if (key == GDK_c && ctl) {
    	manipulate_time(main_window, (gpointer)4 );
    	return;
    }
    if (key == GDK_b && ctl) {
    	if(white_first){
    		voting_result(NULL,(gpointer)HANTEI_WHITE);
    	}else{
           	voting_result(NULL,(gpointer)HANTEI_BLUE);
    	}
    	return;
    }
    if (key == GDK_w && ctl) {
    	if(white_first){
    		voting_result(NULL,(gpointer)HANTEI_BLUE);
    	}else{
           	voting_result(NULL,(gpointer)HANTEI_WHITE);
    	}
    	return;
    }
    if (key == GDK_e && ctl) {
       	voting_result(NULL,(gpointer)CLEAR_SELECTION);
    	return;
    }

    if (key == GDK_v) {
        create_video_window();
        return;
    }

    if (key == GDK_t) {
        display_comp_window(saved_cat, saved_last1, saved_last2,
                            saved_first1, saved_first2, saved_country1, saved_country2, saved_round);
        return;

        extern GtkWidget *main_window;
#if (GTKVER == 3)
        gdk_window_invalidate_rect(gtk_widget_get_window(main_window), NULL, TRUE);
#else
        gdk_window_invalidate_rect(main_window->window, NULL, TRUE);
#endif
        return;
    }

    if (key == GDK_z && ctl) {
        update_display(); //THESE LINES CAUSE PROBLEMS!
        check_ippon();
        return;
    }

    action_on = 1;

    switch (key) {
    case GDK_0:
        automatic = TRUE;
        reset(GDK_7, NULL);
        break;
    case GDK_1:
    case GDK_2:
    case GDK_3:
    case GDK_4:
    case GDK_5:
    case GDK_6:
    case GDK_9:
        automatic = FALSE;
        reset(key, NULL);
        break;
    case GDK_space:
        toggle();
        break;
    case GDK_s:
	// sonomama removed
#if 0
        if (running) {
            running = FALSE;
            elap = g_timer_elapsed(timer, NULL) - startTime;
            if (total > 0.0 && elap >= total)
                elap = total;
            if (oRunning) {
                oElap = g_timer_elapsed(timer, NULL) - oStartTime;
                set_comment_text("SONOMAMA!");
            } else {
                oElap = 0;
                score = 0;
            }
            update_display();
        } else if (!running) {
            running = TRUE;
            startTime = g_timer_elapsed(timer, NULL) - elap;
            if (oRunning) {
                oStartTime = g_timer_elapsed(timer, NULL) - oElap;
                set_comment_text("");
            }
            update_display();
        }
#endif
        break;
    case GDK_Up:
        set_osaekomi_winner(BLUE);
        break;
    case GDK_Down:
        set_osaekomi_winner(WHITE);
        break;
    case GDK_Return:
    case GDK_KP_Enter:
        oToggle();
        if (!oRunning) {
            give_osaekomi_score();
        }
        break;
    case GDK_F5:
	if (whitepts[I] && !shift && !use_ger_u12_rules)
	    break;
        incdecpts(&whitepts[I], shift);
	log_scores("Ippon to ", WHITE);
        check_ippon();
        break;
    case GDK_F6:
	incdecpts(&whitepts[W], shift);
	log_scores("Waza-ari to ", WHITE);
        check_ippon();
        break;
    case GDK_F7:
    case GDK_l:
	if (use_2017_rules) {
	    if (shift && whitepts[L] >= 2)
		bluepts[I] = 0;
	    incdecpts(&whitepts[L], shift);
	    if (whitepts[L] >= 2) {
		whitepts[L] = 2;
                bluepts[I] = 1;
	    }
	    incdecpts(&whitepts[S], shift);
            if (whitepts[S] >= SHIDOMAX)
                bluepts[I] = 1;
	} else
	    incdecpts(&whitepts[Y], shift);
	log_scores("Yuko to ", WHITE);
        break;
    case GDK_F8:
        if (shift) {
            if (whitepts[S] >= SHIDOMAX) {
                bluepts[I] = 0;
            }
            incdecpts(&whitepts[S], DEC);
	    last_shido_to = 0;
            log_scores("Cancel shido to ", WHITE);
        } else {
            incdecpts(&whitepts[S], INC);

            if (whitepts[S] >= SHIDOMAX) {
                bluepts[I] = 1;
            }
	    last_shido_to = WHITE;
            log_scores("Shido to ", WHITE);
        }
        break;
    case GDK_F1:
	if (bluepts[I] && !shift && !use_ger_u12_rules)
	    break;
        incdecpts(&bluepts[I], shift);
	log_scores("Ippon to ", BLUE);
        check_ippon();
        break;
    case GDK_F2:
	incdecpts(&bluepts[W], shift);
	log_scores("Waza-ari to ", BLUE);
        check_ippon();
        break;
    case GDK_F3:
    case GDK_k:
	if (use_2017_rules) {
	    if (shift && bluepts[L] >= 2)
		whitepts[I] = 0;
	    incdecpts(&bluepts[L], shift);
	    if (bluepts[L] >= 2) {
		bluepts[L] = 2;
                whitepts[I] = 1;
	    }
	    incdecpts(&bluepts[S], shift);
            if (bluepts[S] >= SHIDOMAX)
                whitepts[I] = 1;
	} else
	    incdecpts(&bluepts[Y], shift);
	log_scores("Yuko to ", BLUE);
        break;
    case GDK_F4:
        if (shift) {
            if (bluepts[S] >= SHIDOMAX) {
                whitepts[I] = 0;
            }
            incdecpts(&bluepts[S], DEC);
	    last_shido_to = 0;
	    log_scores("Cancel shido to ", BLUE);
        } else {
            incdecpts(&bluepts[S], INC);
            if (bluepts[S] >= SHIDOMAX) {
                whitepts[I] = 1;
            }
	    last_shido_to = BLUE;
            log_scores("Shido to ", BLUE);
        }
        break;
    default:
        ;
    }

    /* check for shido amount of points */
    if (bluepts[S] > SHIDOMAX)
        bluepts[S] = SHIDOMAX;

    if (whitepts[S] > SHIDOMAX)
        whitepts[S] = SHIDOMAX;

    check_ippon();
}

#define NUM_KEYS 9

static void gen_random_key(void)
{
    extern int current_category;
    extern int current_match;
    /*static int last_category, last_match;*/
    /*static gboolean wait_for_new_match = FALSE;*/
    static guint /*last_time = 0, */now = 0, /*wait_set_time = 0,*/ next_time = 0;
    /*
      static guint keys[NUM_KEYS] =
      {GDK_F1, GDK_F2, GDK_F3,
      GDK_F5, GDK_F6, GDK_F7,
      GDK_2,  GDK_space, GDK_Return};
      guint key = 0;*/
    static gint /*test_state = 0,*/ last_cat = 0, last_num = 0;
    static gboolean last_win = FALSE;

    //return;

    now++;
    if (now < next_time /*last_time + 40*/)
        return;

    //last_time = now;

    if (running) {
        if (bluepts[I] == 0 && whitepts[I] == 0) {
            if (last_win) {
                clock_key(GDK_F5, 0);
                last_win = FALSE;
            } else {
                clock_key(GDK_F1, 0);
                last_win = TRUE;
            }
            next_time = now + rand()%20 + 10;
        } else {
            clock_key(GDK_space, 0);
            next_time = now + rand()%100 + 50;
        }
    } else {
        if (current_category == last_cat &&
            current_match == last_num) {
            clock_key(GDK_0, 0);
            next_time = now + rand()%60 + 40;
        } else {
            clock_key(GDK_space, 0);
            last_cat = current_category;
            last_num = current_match;
            next_time = now + rand()%1000 + 100;
        }
    }

    return;
#if 0
    switch (test_state) {
    case 0: clock_key(GDK_2, 0); break;
    case 1: clock_key(GDK_space, 0); break;
    case 2: clock_key(GDK_F1, 0); break;
    case 3: clock_key(GDK_space, 0); break;
    case 4: clock_key(GDK_2, 0); break;
    case 5: clock_key(GDK_space, 0); break;
    case 6: clock_key(GDK_F5, 0); break;
    case 7: clock_key(GDK_space, 0); break;
    }
    test_state++;
    if (test_state >= 8)
        test_state = 0;
    return;

    if (wait_for_new_match && running == 0 &&
        last_category == current_category &&
        last_match == current_match &&
        now < wait_set_time + 100)
        return;

    wait_for_new_match = FALSE;

    if (running == 0 && oRunning == 0) {
        if (action_on) {
            last_category = current_category;
            last_match = current_match;
            wait_for_new_match = TRUE;
            wait_set_time = now;
            key = GDK_0;
            //last_time += 20;
        } else
            key = GDK_space;
    } else if (oRunning) {
        if (now % 10 == 0)
            key = GDK_Return;
    } else if (stackdepth) {
        key = (rand()&1) ? GDK_Up : GDK_Down;
    } else if ((bluepts[I] || whitepts[I]) &&
               running) {
        key = GDK_space;
    } else {
        key = keys[rand() % NUM_KEYS];
    }

    if (key)
        clock_key(key, 0);

#endif
}

gchar *logfile_name = NULL;

void judotimer_log(gchar *format, ...)
{
    time_t now;
    gchar buf[256];
    va_list args;
    va_start(args, format);
    gchar *text = g_strdup_vprintf(format, args);
    va_end(args);

    now = time(NULL);

    if (logfile_name == NULL) {
        struct tm *tm = localtime(&now);
        if (tm) {
	        sprintf(buf, "judotimer_%04d%02d%02d_%02d%02d%02d.log",
	                tm->tm_year+1900,
	                tm->tm_mon+1,
	                tm->tm_mday,
	                tm->tm_hour,
	                tm->tm_min,
	                tm->tm_sec);
	        logfile_name = g_build_filename(g_get_user_data_dir(), buf, NULL);
	    }
        g_print("logfile_name=%s\n", logfile_name);
    }

    FILE *f = fopen(logfile_name, "a");
    if (f) {
        struct tm *tm = localtime(&now);

        guint t = (total > 0.0) ? (total - elap) : elap;
        guint min = t / 60;
        guint sec =  t - min*60;

#ifdef USE_ISO_8859_1
        gsize x;
        gchar *text_ISO_8859_1 =
            g_convert(text, -1, "ISO-8859-1", "UTF-8", NULL, &x, NULL);
#else
        gchar *text_ISO_8859_1 = text;
#endif
		if (tm) {
	        if (t > 610)
	            fprintf(f, "%02d:%02d:%02d [-:--] <%d-%02d> %s\n",
	                    tm->tm_hour,
	                    tm->tm_min,
	                    tm->tm_sec,
	                    current_category, current_match,
	                    text_ISO_8859_1);
	        else
	            fprintf(f, "%02d:%02d:%02d [%d:%02d] <%d-%02d> %s\n",
	                    tm->tm_hour,
	                    tm->tm_min,
	                    tm->tm_sec,
	                    min, sec,
	                    current_category, current_match,
	                    text_ISO_8859_1);
		}
#ifdef USE_ISO_8859_1
        g_free(text_ISO_8859_1);
#endif
        fclose(f);
    } else {
        g_print("Cannot open log file\n");
        perror("logfile_name");
    }

    g_free(text);
}
