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

#include "judoinfo.h"
#include "binreloc.h"

static gboolean button_pressed(GtkWidget *sheet_page, 
			       GdkEventButton *event, 
			       gpointer userdata);

gchar         *program_path;
GtkWidget     *main_vbox = NULL;
GtkWidget     *main_window = NULL;
gchar          current_directory[1024] = {0};
gint           my_address;
gchar         *installation_dir = NULL;
GTimer        *timer;
static PangoFontDescription *font;
GKeyFile      *keyfile;
gchar         *conffile;
GdkCursor     *cursor = NULL;
guint          current_year;
static GtkWidget *darea = NULL;
gint           language = LANG_FI;
gint           num_lines = NUM_LINES;
gboolean       mirror_display = FALSE;
gboolean       white_first = FALSE;
gboolean       red_background = FALSE;
gchar         *filename = NULL;

#define MY_FONT "Arial"

#define THIN_LINE     (paper_width < 700.0 ? 1.0 : paper_width/700.0)
#define THICK_LINE    (2*THIN_LINE)

#define W(_w) ((_w)*paper_width)
#define H(_h) ((_h)*paper_height)

#define BOX_HEIGHT (paper_height/(4.2*num_lines))
//#define BOX_HEIGHT (1.4*extents.height)

#define NUM_RECTANGLES 1000

static struct {
    double x1, y1, x2, y2;
    gint   tatami;
    gint   group;
    gint   category;
    gint   number;
} point_click_areas[NUM_RECTANGLES];

static gint num_rectangles;

static gboolean button_drag = FALSE;
static gchar    dragged_text[32];
static gdouble  dragged_x, dragged_y;

struct match match_list[NUM_TATAMIS][NUM_LINES];

static struct {
    gint cat;
    gint num;
} last_wins[NUM_TATAMIS];

static gboolean delete_event( GtkWidget *widget,
                              GdkEvent  *event,
                              gpointer   data )
{
    return FALSE;
}

extern void print_stat(void);

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

static gboolean refresh_graph(gpointer data)
{
    refresh_window();
    return TRUE;
}

static void paint(cairo_t *c, gdouble paper_width, gdouble paper_height, gpointer userdata)
{
    gint i;
    cairo_text_extents_t extents;
    gdouble y_pos = 0.0, colwidth = W(1.0/number_of_tatamis());
    gdouble left = 0;
    gdouble right;
    time_t now = time(NULL);
    gboolean update_later = FALSE;
    gint num_tatamis = 0;

    for (i = 0; i < NUM_TATAMIS; i++)
        if (show_tatami[i])
            num_tatamis++;

    if (mirror_display && num_tatamis)
        left = (num_tatamis - 1)*colwidth;
        
    num_rectangles = 0;

    cairo_set_font_size(c, 0.8*BOX_HEIGHT);
    cairo_text_extents(c, "Hj", &extents);

    cairo_set_source_rgb(c, 1.0, 1.0, 1.0);
    cairo_rectangle(c, 0.0, 0.0, paper_width, paper_height);
    cairo_fill(c);

    cairo_set_line_width(c, THIN_LINE);
    cairo_set_source_rgb(c, 0.0, 0.0, 0.0);

    y_pos = BOX_HEIGHT;

    for (i = 0; i < NUM_TATAMIS; i++) {
        gchar buf[30];
        gint k;

        if (!show_tatami[i])
            continue;

        right = left + colwidth;

        y_pos = BOX_HEIGHT;

        for (k = 0; k < num_lines; k++) {
            struct match *m = &match_list[i][k];
            gchar buf[20];
            gdouble e = (k == 0) ? colwidth/2 : 0.0;
			
            if (m->number >= 1000)
                break;

            struct name_data *catdata = avl_get_data(m->category);

            cairo_save(c);
            if (k == 0) {
                if (last_wins[i].cat == m->category &&
                    last_wins[i].num == m->number)
                    cairo_set_source_rgb(c, 0.7, 1.0, 0.7);
                else
                    cairo_set_source_rgb(c, 1.0, 1.0, 0.0);
            } else
                cairo_set_source_rgb(c, 1.0, 1.0, 1.0);

            cairo_rectangle(c, left, y_pos, colwidth, 4*BOX_HEIGHT);
            cairo_fill(c);
            cairo_restore(c);
			
            cairo_save(c);
            if (k == 0) {
                cairo_move_to(c, left+5, y_pos+extents.height);
                cairo_show_text(c, _("Prev. winner:"));
            } else if (m->rest_end > now && k == 1) {
                gint t = m->rest_end - now;
                sprintf(buf, "** %d:%02d **", t/60, t%60);
                cairo_save(c);
                cairo_set_source_rgb(c, 0.8, 0.0, 0.0);
                cairo_move_to(c, left+5+colwidth/2, y_pos+extents.height);
                cairo_show_text(c, buf);
                cairo_restore(c);
                update_later = TRUE;
            } else if (m->number == 1) {
                cairo_save(c);
                cairo_set_source_rgb(c, 1.0, 0.0, 0.0);
                cairo_move_to(c, left+5+colwidth/2, y_pos+extents.height);
                cairo_show_text(c, _("Category starts"));
                cairo_restore(c);
            }

            if (m->flags) {
                cairo_save(c);
                cairo_set_source_rgb(c, 0.0, 0.0, 1.0);
                cairo_move_to(c, left+5+colwidth/2, y_pos+extents.height);
                if (m->flags & MATCH_FLAG_GOLD)
                    cairo_show_text(c, _("Gold medal match"));
                else if (m->flags & MATCH_FLAG_BRONZE_A)
                    cairo_show_text(c, _("Bronze match A"));
                else if (m->flags & MATCH_FLAG_BRONZE_B)
                    cairo_show_text(c, _("Bronze match B"));
                else if (m->flags & MATCH_FLAG_SEMIFINAL_A)
                    cairo_show_text(c, _("Semifinal A"));
                else if (m->flags & MATCH_FLAG_SEMIFINAL_B)
                    cairo_show_text(c, _("Semifinal B"));
                else if (m->flags & MATCH_FLAG_SILVER)
                    cairo_show_text(c, _("Silver medal match"));
                cairo_restore(c);
            }
            cairo_select_font_face(c, MY_FONT, 0, CAIRO_FONT_WEIGHT_BOLD);
            if (k == 0) 
                cairo_move_to(c, left+5, y_pos+extents.height+BOX_HEIGHT);
            else
                cairo_move_to(c, left+5, y_pos+extents.height);

            snprintf(buf, sizeof(buf), "%s #%d", catdata ? catdata->last : "?", m->number);
            cairo_show_text(c, buf);
            //cairo_show_text(c, catdata ? catdata->last : "?");
            cairo_restore(c);

            struct name_data *j = avl_get_data(m->blue);
            if (j) {
                cairo_save(c);

#if 0 // new white first rule disables this
                if (k == 1 && m->flags & MATCH_FLAG_BLUE_DELAYED) {
                    if (m->flags & MATCH_FLAG_BLUE_REST)
                        cairo_set_source_rgb(c, 0.5, 0.5, 1.0);
                    else
                        cairo_set_source_rgb(c, 1.0, 0.5, 0.5);
                }
#else
                if (!white_first) {
                    if (red_background)
                        cairo_set_source_rgb(c, 1.0, 0.0, 0.0);
                    else
                        cairo_set_source_rgb(c, 0.0, 0.0, 1.0);
                }
#endif
                else
                    cairo_set_source_rgb(c, 1.0, 1.0, 1.0);

                if (k)
                    cairo_rectangle(c, left, y_pos+BOX_HEIGHT, 
                                    colwidth/2, 
                                    3*BOX_HEIGHT);
                cairo_fill(c);

                if (k && !white_first)
                    cairo_set_source_rgb(c, 1.0, 1.0, 1.0);
                else
                    cairo_set_source_rgb(c, 0, 0, 0);

                cairo_select_font_face(c, MY_FONT, 0, CAIRO_FONT_WEIGHT_BOLD);
                cairo_move_to(c, left+5+e, y_pos+2*BOX_HEIGHT+extents.height);
                cairo_show_text(c, j->last);
                cairo_select_font_face(c, MY_FONT, 0, CAIRO_FONT_WEIGHT_NORMAL);

                cairo_move_to(c, left+5+e, y_pos+BOX_HEIGHT+extents.height);
                cairo_show_text(c, j->first);

                cairo_move_to(c, left+5+e, y_pos+3*BOX_HEIGHT+extents.height);
                cairo_show_text(c, j->club);

                cairo_restore(c);
            }
            j = avl_get_data(m->white);
            if (j && k > 0) {
                cairo_save(c);

                if (k == 0) {
                    if (last_wins[i].cat == m->category &&
                        last_wins[i].num == m->number)
                        cairo_set_source_rgb(c, 0.7, 1.0, 0.7);
                    else
                        cairo_set_source_rgb(c, 1.0, 1.0, 0.0);
                } 
#if 0
                else if (k == 1 && m->flags & MATCH_FLAG_WHITE_DELAYED) {
                    if (m->flags & MATCH_FLAG_WHITE_REST)
                        cairo_set_source_rgb(c, 0.5, 0.5, 1.0);
                    else
                        cairo_set_source_rgb(c, 1.0, 0.5, 0.5);
                } 
#else
                else if (white_first) {
                    if (red_background)
                        cairo_set_source_rgb(c, 1.0, 0.0, 0.0);
                    else
                        cairo_set_source_rgb(c, 0.0, 0.0, 1.0);
                }
#endif
                else
                    cairo_set_source_rgb(c, 1.0, 1.0, 1.0);

                cairo_rectangle(c, left+colwidth/2, y_pos+BOX_HEIGHT, colwidth/2, 3*BOX_HEIGHT);
                cairo_fill(c);

                if (k && white_first)
                    cairo_set_source_rgb(c, 1.0, 1.0, 1.0);
                else
                    cairo_set_source_rgb(c, 0, 0, 0);

                cairo_select_font_face(c, MY_FONT, 0, CAIRO_FONT_WEIGHT_BOLD);
                cairo_move_to(c, left+5+colwidth/2, y_pos+2*BOX_HEIGHT+extents.height);
                cairo_show_text(c, j->last);

                cairo_select_font_face(c, MY_FONT, 0, CAIRO_FONT_WEIGHT_NORMAL);
                cairo_move_to(c, left+5+colwidth/2, y_pos+BOX_HEIGHT+extents.height);
                cairo_show_text(c, j->first);

                cairo_move_to(c, left+5+colwidth/2, y_pos+3*BOX_HEIGHT+extents.height);
                cairo_show_text(c, j->club);

                cairo_restore(c);
            }

            point_click_areas[num_rectangles].category = m->category;
            point_click_areas[num_rectangles].number = m->number;
            point_click_areas[num_rectangles].group = 0;
            point_click_areas[num_rectangles].tatami = i+1;
            point_click_areas[num_rectangles].x1 = left;
            point_click_areas[num_rectangles].y1 = y_pos;
            point_click_areas[num_rectangles].x2 = right;
            y_pos += 4*BOX_HEIGHT;
            point_click_areas[num_rectangles].y2 = y_pos;
            if (num_rectangles < NUM_RECTANGLES-1)
                num_rectangles++;

            cairo_move_to(c, left, y_pos);
            cairo_line_to(c, right, y_pos);
            cairo_stroke(c);
        }

        cairo_move_to(c, 10 + left, extents.height);
        sprintf(buf, "Tatami %d", i+1);
        cairo_show_text(c, buf);

        point_click_areas[num_rectangles].category = 0;
        point_click_areas[num_rectangles].number = 0;
        point_click_areas[num_rectangles].group = 0;
        point_click_areas[num_rectangles].tatami = i+1;
        point_click_areas[num_rectangles].x1 = left;
        point_click_areas[num_rectangles].y1 = y_pos;
        point_click_areas[num_rectangles].x2 = right;
        point_click_areas[num_rectangles].y2 = H(1.0);
        if (num_rectangles < NUM_RECTANGLES-1)
            num_rectangles++;

        cairo_save(c);
        cairo_set_line_width(c, THICK_LINE);
        cairo_set_source_rgb(c, 0.0, 0.0, 0.0);
        cairo_move_to(c, left, 0);
        cairo_line_to(c, left, H(1.0));
        cairo_stroke(c);
        cairo_restore(c);

        if (mirror_display)
            left -= colwidth;
        else
            left += colwidth;
    }

	
    cairo_save(c);
    cairo_set_line_width(c, THICK_LINE);

    cairo_set_source_rgb(c, 0.0, 0.0, 0.0);
    cairo_move_to(c, 0, BOX_HEIGHT);
    cairo_line_to(c, W(1.0), BOX_HEIGHT);
    cairo_stroke(c);

    cairo_set_source_rgb(c, 0.0, 0.0, 1.0);
    cairo_move_to(c, 0, 5*BOX_HEIGHT);
    cairo_line_to(c, W(1.0), 5*BOX_HEIGHT);
    cairo_move_to(c, 0, 13*BOX_HEIGHT);
    cairo_line_to(c, W(1.0), 13*BOX_HEIGHT);
    cairo_stroke(c);

    cairo_restore(c);

    if (button_drag) {
        cairo_set_line_width(c, THIN_LINE);
        cairo_text_extents(c, dragged_text, &extents);
        cairo_set_source_rgb(c, 1.0, 1.0, 1.0);
        cairo_rectangle(c, dragged_x - extents.width/2.0, dragged_y - extents.height,
                        extents.width + 4, extents.height + 4);
        cairo_fill(c);
        cairo_set_source_rgb(c, 0, 0, 0);
        cairo_rectangle(c, dragged_x - extents.width/2.0 - 1, dragged_y - extents.height - 1,
                        extents.width + 4, extents.height + 4);
        cairo_stroke(c);
        cairo_move_to(c, dragged_x - extents.width/2.0, dragged_y);
        cairo_show_text(c, dragged_text);
    }
#if 0
    if (update_later)
        g_timeout_add(5000, refresh_graph, NULL);
#endif
}

/* This is called when we need to draw the windows contents */
static gboolean expose(GtkWidget *widget, GdkEventExpose *event, gpointer userdata)
{
    cairo_t *c = gdk_cairo_create(widget->window);

    paint(c, widget->allocation.width, widget->allocation.height, userdata);

    cairo_show_page(c);
    cairo_destroy(c);

    return FALSE;
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

void toggle_small_display(GtkWidget *menu_item, gpointer data)
{
    if (GTK_CHECK_MENU_ITEM(menu_item)->active) {
        num_lines = 6;
        g_key_file_set_boolean(keyfile, "preferences", "smalldisplay", TRUE);
    } else {
        num_lines = NUM_LINES;
        g_key_file_set_boolean(keyfile, "preferences", "smalldisplay", FALSE);
    }
    expose(darea, 0, 0);
}

void toggle_mirror(GtkWidget *menu_item, gpointer data)
{
    if (GTK_CHECK_MENU_ITEM(menu_item)->active) {
        mirror_display = TRUE;
        g_key_file_set_boolean(keyfile, "preferences", "mirror", TRUE);
    } else {
        mirror_display = FALSE;
        g_key_file_set_boolean(keyfile, "preferences", "mirror", FALSE);
    }
    expose(darea, 0, 0);
}

void toggle_whitefirst(GtkWidget *menu_item, gpointer data)
{
    if (GTK_CHECK_MENU_ITEM(menu_item)->active) {
        white_first = TRUE;
        g_key_file_set_boolean(keyfile, "preferences", "whitefirst", TRUE);
    } else {
        white_first = FALSE;
        g_key_file_set_boolean(keyfile, "preferences", "whitefirst", FALSE);
    }
    expose(darea, 0, 0);
}

void toggle_redbackground(GtkWidget *menu_item, gpointer data)
{
    red_background = GTK_CHECK_MENU_ITEM(menu_item)->active;
    g_key_file_set_boolean(keyfile, "preferences", "redbackground", red_background);
    expose(darea, 0, 0);
}

int main( int   argc,
          char *argv[] )
{
    /* GtkWidget is the storage type for widgets */
    GtkWidget *window;
    GtkWidget *menubar;
    time_t     now;
    struct tm *tm;
    GThread   *gth = NULL;         /* thread id */
    gboolean   run_flag = TRUE;   /* used as exit flag for threads */

    init_trees();

    font = pango_font_description_from_string("Sans bold 12");

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

    conffile = g_build_filename(g_get_user_data_dir(), "judoinfo.ini", NULL);

    keyfile = g_key_file_new();
    g_key_file_load_from_file(keyfile, conffile, 0, NULL);

    now = time(NULL);
    tm = localtime(&now);
    current_year = tm->tm_year+1900;
    srand(now); //srandom(now);
    my_address = now + getpid()*10000;

    g_thread_init(NULL);    /* Initialize GLIB thread support */
    gdk_threads_init();     /* Initialize GDK locks */
    gdk_threads_enter();    /* Acquire GDK locks */ 

    gtk_init (&argc, &argv);

    main_window = window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window), "JudoInfo");
    gtk_widget_set_size_request(window, FRAME_WIDTH, FRAME_HEIGHT);

    gchar *iconfile = g_build_filename(installation_dir, "etc", "judoinfo.png", NULL);
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

    /* timers */
        
    timer = g_timer_new();

    /*g_timeout_add(100, timeout, NULL);*/
    g_timeout_add(2000, refresh_graph, NULL);

    gtk_widget_show_all(window);

    set_preferences();
    change_language(NULL, NULL, (gpointer)language);

    open_comm_socket();
	
    /* Create a bg thread using glib */
    gth = g_thread_create((GThreadFunc)client_thread,
                          (gpointer)&run_flag, FALSE, NULL); 

    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER_ALWAYS);
	
    cursor = gdk_cursor_new(GDK_HAND2);
    //gdk_window_set_cursor(GTK_WIDGET(main_window)->window, cursor);

    g_timeout_add(100, timeout_ask_for_data, NULL);

    /* All GTK applications must have a gtk_main(). Control ends here
     * and waits for an event to occur (like a key press or
     * mouse event). */
    gtk_main();
    
    gdk_threads_leave();  /* release GDK locks */
    run_flag = FALSE;     /* flag threads to stop and exit */
    //g_thread_join(gth);   /* wait for thread to exit */ 

    return 0;
}

gboolean this_is_shiai(void)
{
    return FALSE;
}

gint application_type(void)
{
    return APPLICATION_TYPE_INFO;
}

void refresh_window(void)
{
    GtkWidget *widget;
    GdkRegion *region;
    widget = GTK_WIDGET(main_window);
    if (widget->window) {
        region = gdk_drawable_get_clip_region(widget->window);
        gdk_window_invalidate_region(widget->window, region, TRUE);
        gdk_window_process_updates(widget->window, TRUE);
    }
}

static gint find_box(gdouble x, gdouble y)
{
    gint t;

    for (t = 0; t < num_rectangles; t++) {
        if (x >= point_click_areas[t].x1 &&
            x <= point_click_areas[t].x2 &&
            y >= point_click_areas[t].y1 &&
            y <= point_click_areas[t].y2) {
            return t;
        }
    }
    return -1;
}

static gboolean button_pressed(GtkWidget *sheet_page, 
			       GdkEventButton *event, 
			       gpointer userdata)
{
    if (event->type == GDK_BUTTON_PRESS &&  
        (event->button == 1)) {
        gdouble x = event->x, y = event->y; 
        gint t, tatami;
		    
        t = find_box(x, y);
        if (t < 0)
            return FALSE;

        tatami = point_click_areas[t].tatami;
        last_wins[tatami-1].cat = match_list[tatami-1][0].category;
        last_wins[tatami-1].num = match_list[tatami-1][0].number;

        refresh_window();

        return TRUE;
		    
    } else if (event->type == GDK_BUTTON_PRESS &&  
               (event->button == 3)) {
        struct message msg;
        extern gint my_address;

        msg.type = MSG_ALL_REQ;
        msg.sender = my_address;
        send_packet(&msg);
    }

    return FALSE;
}

void set_write_file(GtkWidget *menu_item, gpointer data)
{
    GtkWidget *dialog;
    static gchar *last_dir = NULL;

    dialog = gtk_file_chooser_dialog_new (_("Save file"),
                                          GTK_WINDOW(main_window),
                                          GTK_FILE_CHOOSER_ACTION_SAVE,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                                          NULL);

    //gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);

    if (last_dir)
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), last_dir);

    //gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), dflt);

    g_free(filename);
    filename = NULL;

    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        if (last_dir)
            g_free(last_dir);
        last_dir = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER (dialog));
    }

    gtk_widget_destroy (dialog);
}

void write_matches(void)
{
    gint t, k;

    if (!filename)
        return;

    for (t = 0; t < NUM_TATAMIS; t++)
        if (show_tatami[t])
            break;
    
    if (t >= NUM_TATAMIS)
        return;

    FILE *fout = fopen(filename, "w");
    if (!fout)
        return;

    for (k = 1; k < num_lines; k++) {
        struct match *m = &match_list[t][k];
        struct name_data *j = avl_get_data(m->category);
        if (j)
            fprintf(fout, "%s;", j->last);
        else
            fprintf(fout, ";");

        j = avl_get_data(m->blue);
        if (j)
            fprintf(fout, "%s;%s;%s;", j->last, j->first, j->club);
        else
            fprintf(fout, ";;;");

        j = avl_get_data(m->white);
        if (j)
            fprintf(fout, "%s;%s;%s\r\n", j->last, j->first, j->club);
        else
            fprintf(fout, ";;\r\n");
    }

    fclose(fout);
}
