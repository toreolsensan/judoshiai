/* -*- mode: C; c-basic-offset: 4;  -*- */

/*
 * Copyright (C) 2006-2016 by Hannu Jokinen
 * Full copyright text is included in the software package.
 */ 

#ifndef _JUDOINFO_H_
#define _JUDOINFO_H_

#ifdef EMSCRIPTEN
#include "cairowrapper.h"
#endif

#include "comm.h"

#ifdef ENABLE_NLS
#include <libintl.h>
#define _(String) gettext(String)
#ifdef gettext_noop
#define N_(String) gettext_noop(String)
#else
#define N_(String) (String)
#endif
#else /* NLS is disabled */
#define _(String) (String)
#define N_(String) (String)
#define textdomain(String) (String)
#define gettext(String) (String)
#define dgettext(Domain,String) (String)
#define dcgettext(Domain,String,Type) (String)
#define bindtextdomain(Domain,Directory) (Domain) 
#define bind_textdomain_codeset(Domain,Codeset) (Codeset) 
#endif /* ENABLE_NLS */

#define FRAME_WIDTH  600
#define FRAME_HEIGHT 400

#define BLUE  1
#define WHITE 2
#define CMASK 3
#define GIVE_POINTS 4

#define OSAEKOMI_DSP_NO      0
#define OSAEKOMI_DSP_YES     1
#define OSAEKOMI_DSP_BLUE    2
#define OSAEKOMI_DSP_WHITE   3
#define OSAEKOMI_DSP_UNKNOWN 4

#define CLEAR_SELECTION   0
#define HANTEI_BLUE       1
#define HANTEI_WHITE      2
#define HANSOKUMAKE_BLUE  3
#define HANSOKUMAKE_WHITE 4

#define NUM_TATAMIS  10
#define NUM_LINES   10

#define NORMAL_DISPLAY     0
#define SMALL_DISPLAY      1
#define HORIZONTAL_DISPLAY 2

enum {
    BRACKET_TYPE_ERR,
    BRACKET_TYPE_PNG,
    BRACKET_TYPE_SVG,
};

struct name_data {
    gint   index;
    gchar *last;
    gchar *first;
    gchar *club;
};

struct match {
    gint   category;
    gint   number;
    gint   blue;
    gint   white;
    gint   flags;
    gint   round;
    time_t rest_end;
};

struct paint_data {
    cairo_t *c;
    gdouble  paper_width;
    gdouble  paper_height;
};

extern GtkWidget *main_window;
extern GTimer *timer;
extern gchar *installation_dir;
extern gulong my_ip_address, node_ip_addr;
extern GKeyFile *keyfile;
extern gboolean show_tatami[NUM_TATAMIS];
extern struct match match_list[NUM_TATAMIS][NUM_LINES];
extern gint language;
extern gchar *svg_file;
extern gboolean bracket_ok;
extern gint bracket_pos;
extern gint bracket_len;
extern guchar *bracket_start;
extern gint bracket_x, bracket_y, bracket_w, bracket_h;
extern gint bracket_space_w, bracket_space_h;

extern gboolean this_is_shiai(void);
extern void msg_to_queue(struct message *msg);
extern struct message *get_rec_msg(void);
extern void destroy( GtkWidget *widget, gpointer   data );
extern void set_preferences(void);

#ifdef EMSCRIPTEN
extern gui_widget *get_menubar_menu(SDL_Surface *s);
#else
extern GtkWidget *get_menubar_menu(GtkWidget  *window);
#endif
extern gpointer client_thread(gpointer args);
extern gboolean change_language(GtkWidget *eventbox, GdkEventButton *event, void *param);
extern void ask_node_ip_address( GtkWidget *w, gpointer data);
extern void show_my_ip_addresses( GtkWidget *w, gpointer data);
extern gint number_of_tatamis(void);
extern gint number_of_conf_tatamis(void);
extern void init_trees(void);
extern struct name_data *avl_get_data(gint index);
extern void avl_set_data(gint index, gchar *first, gchar *last, gchar *club);
extern void init_trees(void);
extern void refresh_window(void);
extern gint timeout_ask_for_data(gpointer data);
extern void write_matches(void);
extern void read_svg_file(void);
extern gint paint_svg(struct paint_data *pd);
#ifndef EMSCRIPTEN
extern gboolean show_bracket(void);
extern cairo_status_t bracket_read(void *closure,
                                   unsigned char *data,
                                   unsigned int length);
extern gint bracket_type(void);
extern gint first_shown_tatami(void);
#endif

/* profiling stuff */
//#define PROFILE
//extern guint64 cumul_db_time;
//extern gint    cumul_db_count;

#define NUM_PROF 32

struct profiling_data {
        guint64 time_stamp;
        const gchar *func;
        gint line;
};

#ifdef PROFILE

static __inline__ guint64 rdtsc(void) {
    guint32 lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return (guint64)hi << 32 | lo;
}

extern struct profiling_data prof_data[NUM_PROF];
extern gint num_prof_data;
extern guint64 prof_start;
extern gboolean prof_started;

#define RDTSC(_a) do { _a = rdtsc(); } while(0)

#define PROF_START do { memset(&prof_data, 0, sizeof(prof_data));       \
        num_prof_data = 0; /*cumul_db_time = 0; cumul_db_count = 0;*/	\
        prof_started = TRUE; prof_start = rdtsc(); } while (0)

#define PROF do { if (prof_started && num_prof_data < NUM_PROF) { \
            prof_data[num_prof_data].time_stamp = rdtsc();        \
            prof_data[num_prof_data].func = __FUNCTION__;         \
            prof_data[num_prof_data++].line = __LINE__; }} while (0)

static inline void prof_end(void) {
    gint i;
    guint64 stop = rdtsc(), prev = prof_start;
    g_print("PROF:\n");
    for (i = 0; i < num_prof_data; i++) {
        g_print("%s:%d: %ld\n", prof_data[i].func, prof_data[i].line, (prof_data[i].time_stamp - prev)/100);
        prev = prof_data[i].time_stamp;
    }

    //guint64 tot_time_div = (stop - prof_start)/100;
    //guint64 relative_time = cumul_db_time/tot_time_div;
    //g_print("\nDB-writes=%d db-time=%ld\n", cumul_db_count, relative_time);

}
#define PROF_END prof_end()

#else

static __inline__ guint64 rdtsc(void) {
    return 0;
}

#define RDTSC(_a) do {} while(0)
#define PROF_START do {} while(0)
#define PROF do {} while(0)
#define PROF_END do {} while(0)

#endif // profiling

#endif
