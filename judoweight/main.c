/* -*- mode: C; c-basic-offset: 4;  -*- */

/*
 * Copyright (C) 2006-2013 by Hannu Jokinen
 * Full copyright text is included in the software package.
 */ 

#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <locale.h>

#include <gtk/gtk.h>
#include <glib.h>
#include <gdk/gdkkeysyms.h>
#ifdef WIN32
#include <process.h>
//#include <glib/gwin32.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif

#include "judoweight.h"
#include "language.h"
#include "binreloc.h"

#define JUDOGI_STATUS

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
gint           language = LANG_EN;
static GtkWidget *name_box;
static GtkWidget *id_box;
static GtkWidget *weight_box;
#ifdef JUDOGI_STATUS
static GtkWidget *judogi_box;
#endif
GtkWidget *weight_entry = NULL;
static GtkWidget *confirm_box, *confirm_box2;

static struct message saved;
static gchar  *saved_id = NULL;
static GdkCursor *wait_cursor = NULL;
static GtkWidget *w_id, *w_name, *w_weight, *w_control, *w_ok, *w_confirm;

void set_display(struct msg_edit_competitor *msg)
{
    gchar buf[128];

    if (!saved_id)
        return;

    if (atoi(saved_id) != msg->index && 
        strcmp(saved_id, msg->id))
        return;
#if (GTKVER == 3)
    gdk_window_set_cursor(gtk_widget_get_window(main_window), NULL);
#else
    gdk_window_set_cursor(GTK_WIDGET(main_window)->window, NULL);
#endif
    if (msg->operation == EDIT_OP_CONFIRM) {

#if (GTKVER == 3)
        snprintf(buf, sizeof(buf), "%s %s, %s/%s: %s (%s): %d.%02d",
                 msg->last, msg->first, msg->country, msg->club, msg->category, msg->regcategory,
                 msg->weight/1000, (msg->weight%1000)/10);
        gtk_label_set_label(GTK_LABEL(confirm_box), buf);
        snprintf(buf, sizeof(buf), "%s",
                 (msg->deleted & JUDOGI_OK) ? "OK" :
                 ((msg->deleted & JUDOGI_NOK) ? "NOK" : _("WARNING: NO CONTROL")));
        gtk_label_set_label(GTK_LABEL(confirm_box2), buf);
#else
        snprintf(buf, sizeof(buf), "%s %s, %s/%s: %s (%s): %d.%02d %s",
                 msg->last, msg->first, msg->country, msg->club, msg->category, msg->regcategory,
                 msg->weight/1000, (msg->weight%1000)/10,
                 (msg->deleted & JUDOGI_OK) ? "OK" :
                 ((msg->deleted & JUDOGI_NOK) ? "NOK" : _("WARNING: NO CONTROL")));
        gtk_label_set_label(GTK_LABEL(confirm_box), buf);
#endif
        return;
    }

    saved.u.edit_competitor = *msg;

//    snprintf(buf, sizeof(buf), "%s %s, %s/%s: %s (%s)",
//             msg->last, msg->first, msg->country, msg->club, msg->category, msg->regcategory);
    snprintf(buf, sizeof(buf), "%s %s, %s/%s (%s): %s", 
             msg->last, msg->first, msg->country, msg->club, msg->beltstr, msg->regcategory);
    gtk_label_set_label(GTK_LABEL(name_box), buf);
    snprintf(buf, sizeof(buf), "%d.%02d", msg->weight/1000, (msg->weight%1000)/10);
    gtk_entry_set_text(GTK_ENTRY(weight_box), buf);
    gtk_widget_grab_focus(weight_box);

#ifdef JUDOGI_STATUS
    if (msg->deleted & JUDOGI_OK)
	gtk_combo_box_set_active(GTK_COMBO_BOX(judogi_box), 1);
    else if (msg->deleted & JUDOGI_NOK)
	gtk_combo_box_set_active(GTK_COMBO_BOX(judogi_box), 2);
    else
	gtk_combo_box_set_active(GTK_COMBO_BOX(judogi_box), 0);
#endif
}

static gboolean delete_event( GtkWidget *widget,
                              GdkEvent  *event,
                              gpointer   data )
{
    return FALSE;
}

static gboolean set_weight_on_button_pressed(GtkWidget *treeview, 
                                             GdkEventButton *event, 
                                             gpointer userdata)
{
    GtkEntry *weight = userdata;
    GdkEventKey *key = (GdkEventKey *)event;

    if (event->type == GDK_KEY_PRESS && key->keyval != ' ')
        return FALSE;

    if (weight_entry)
        gtk_entry_set_text(GTK_ENTRY(weight), gtk_button_get_label(GTK_BUTTON(weight_entry)));

    return TRUE;
}

static gint weight_grams(const gchar *s)
{
    gint weight = 0, decimal = 0;
        
    while (*s) {
        if (*s == '.' || *s == ',') {
            decimal = 1;
        } else if (*s < '0' || *s > '9') {
        } else if (decimal == 0) {
            weight = 10*weight + 1000*(*s - '0');
        } else {
            switch (decimal) {
            case 1: weight += 100*(*s - '0'); break;
            case 2: weight += 10*(*s - '0'); break;
            case 3: weight += *s - '0'; break;
            }
            decimal++;
        }
        s++;
    }
        
    return weight;
}

static void on_ok(GtkEntry *entry, gpointer user_data)  
{ 
    const gchar *weight = gtk_entry_get_text(GTK_ENTRY(weight_box)); 
#ifdef JUDOGI_STATUS
    gint judogi = gtk_combo_box_get_active(GTK_COMBO_BOX(judogi_box));
#endif
    if (saved.u.edit_competitor.index < 10)
        return;

    saved.type = MSG_EDIT_COMPETITOR;
    saved.u.edit_competitor.operation = EDIT_OP_SET_WEIGHT;
    saved.u.edit_competitor.weight = weight_grams(weight);

#ifdef JUDOGI_STATUS
    saved.u.edit_competitor.deleted &= ~(JUDOGI_OK | JUDOGI_NOK);
    if (judogi == 1)
	saved.u.edit_competitor.deleted |= JUDOGI_OK;
    else if (judogi == 2)
	saved.u.edit_competitor.deleted |= JUDOGI_NOK;
#endif
    send_packet(&saved);

    struct msg_edit_competitor *msg = &saved.u.edit_competitor;
    judoweight_log("%s %s, %s/%s: %s: %d.%02d %s", 
                   msg->last, msg->first, msg->country, msg->club, msg->regcategory,
                   msg->weight/1000, (msg->weight%1000)/10,
                   (msg->deleted & JUDOGI_OK) ? "OK" :
                   ((msg->deleted & JUDOGI_NOK) ? "NOK" : ""));

    saved.u.edit_competitor.index = 0;
    gtk_label_set_label(GTK_LABEL(name_box), "?");
    gtk_entry_set_text(GTK_ENTRY(weight_box), "?");
#ifdef JUDOGI_STATUS
    gtk_combo_box_set_active(GTK_COMBO_BOX(judogi_box), 0);
#endif
    gtk_widget_grab_focus(id_box);
#if (GTKVER == 3)
    gdk_window_set_cursor(gtk_widget_get_window(GTK_WIDGET(main_window)), wait_cursor);
#else
    gdk_window_set_cursor(GTK_WIDGET(main_window)->window, wait_cursor);
#endif
}

static void on_enter(GtkEntry *entry, gpointer user_data)  
{ 
    const gchar *the_text;
    struct message output_msg;

    the_text = gtk_entry_get_text(GTK_ENTRY(entry)); 
    g_free(saved_id);
    saved_id = g_strdup(the_text);

    memset(&output_msg, 0, sizeof(output_msg));
    output_msg.type = MSG_EDIT_COMPETITOR;
    output_msg.u.edit_competitor.operation = EDIT_OP_GET_BY_ID;
    strncpy(output_msg.u.edit_competitor.id, the_text, sizeof(output_msg.u.edit_competitor.id));
    send_packet(&output_msg);

    gtk_entry_set_text(GTK_ENTRY(entry), "");
    //gtk_widget_grab_focus(GTK_WIDGET(entry));

    gtk_label_set_label(GTK_LABEL(name_box), "?");
    gtk_entry_set_text(GTK_ENTRY(weight_box), "?");
#ifdef JUDOGI_STATUS
    gtk_combo_box_set_active(GTK_COMBO_BOX(judogi_box), 0);
#endif
#if (GTKVER == 3)
    gdk_window_set_cursor(gtk_widget_get_window(main_window), wait_cursor);
#else
    gdk_window_set_cursor(GTK_WIDGET(main_window)->window, wait_cursor);
#endif
}

void destroy( GtkWidget *widget,
	      gpointer   data )
{
    gsize length;
    gchar *inidata = g_key_file_to_data (keyfile,
                                         &length,
                                         NULL);

    g_file_set_contents(conffile, inidata, length, NULL);
    g_free(inidata);
    g_key_file_free(keyfile);

    gtk_main_quit ();
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

    putenv("UBUNTU_MENUPROXY=");

    font = pango_font_description_from_string("Sans bold 12");

#ifdef WIN32
#if (GTKVER == 3)
    installation_dir = g_win32_get_package_installation_directory_of_module(NULL);
#else
    installation_dir = g_win32_get_package_installation_directory(NULL, NULL);
#endif
#else
    gbr_init(NULL);
    installation_dir = gbr_find_prefix(NULL);
#endif

    program_path = argv[0];

    current_directory[0] = 0;

    if (current_directory[0] == 0)
        strcpy(current_directory, ".");

    conffile = g_build_filename(g_get_user_data_dir(), "judoweight.ini", NULL);

    keyfile = g_key_file_new();
    g_key_file_load_from_file(keyfile, conffile, 0, NULL);

    now = time(NULL);
    tm = localtime(&now);
    current_year = tm->tm_year+1900;
    srand(now); //srandom(now);
    my_address = now + getpid()*10000;

#if (GTKVER != 3)
    g_thread_init(NULL);    /* Initialize GLIB thread support */
    gdk_threads_init();     /* Initialize GDK locks */
    gdk_threads_enter();    /* Acquire GDK locks */ 
#endif

    gtk_init (&argc, &argv);

    wait_cursor = gdk_cursor_new(GDK_WATCH);

    main_window = window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window), "JudoWeight");
    //gtk_widget_set_size_request(window, FRAME_WIDTH, FRAME_HEIGHT);

    gchar *iconfile = g_build_filename(installation_dir, "etc", "judoweight.png", NULL);
    gtk_window_set_default_icon_from_file(iconfile, NULL);
    g_free(iconfile);

    g_signal_connect (G_OBJECT (window), "delete_event",
                      G_CALLBACK (delete_event), NULL);
    
    g_signal_connect (G_OBJECT (window), "destroy",
                      G_CALLBACK (destroy), NULL);
    
    gtk_container_set_border_width (GTK_CONTAINER (window), 10);

#if (GTKVER == 3)
    main_vbox = gtk_grid_new();
#else
    main_vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (main_vbox), 1);
#endif
    gtk_container_add (GTK_CONTAINER (window), main_vbox);
    gtk_widget_show(main_vbox);

    /* menubar */
    menubar = get_menubar_menu(window);
    gtk_widget_show(menubar);

#if (GTKVER == 3)
    gtk_grid_attach(GTK_GRID(main_vbox), menubar, 0, 0, 1, 1);
    gtk_widget_set_hexpand(menubar, TRUE);
    //gtk_widget_set_halign(menubar, GTK_ALIGN_FILL);
#else
    gtk_box_pack_start(GTK_BOX(main_vbox), menubar, FALSE, TRUE, 0);
#endif

    enum {c0 = 0, c1, c2, c3, c4, c5};

    /* */
    gint row = 0;
#if (GTKVER == 3)
    GtkWidget *table = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(table), 5);
#else
    GtkWidget *table = gtk_table_new(5, 6, FALSE);
    gtk_table_set_col_spacings(GTK_TABLE(table), 5);
    gtk_table_set_row_spacings(GTK_TABLE(table), 5);
#endif
    GtkWidget *tmp = w_id = gtk_label_new(_("ID:"));
    gtk_misc_set_alignment(GTK_MISC(tmp), 1, 0.5);
#if (GTKVER == 3)
    gtk_grid_attach(GTK_GRID(table), tmp, c1, 0, 1, 1);
    // extra space around grid to center it
    tmp = gtk_label_new(" ");
    gtk_grid_attach(GTK_GRID(table), tmp, c0, 0, 1, 1);
    gtk_widget_set_hexpand(tmp, TRUE);
    tmp = gtk_label_new(" ");
    gtk_grid_attach(GTK_GRID(table), tmp, c4, 0, 1, 1);
    gtk_widget_set_hexpand(tmp, TRUE);
#else
    gtk_table_attach_defaults(GTK_TABLE(table), tmp, 0, 1, row, row+1);
#endif
    id_box = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(id_box), 16);
#if (GTKVER == 3)
    gtk_grid_attach(GTK_GRID(table), id_box, c2, row, 1, 1);
#else
    gtk_table_attach_defaults(GTK_TABLE(table), id_box, 1, 2, row, row+1);
#endif
    row++;

    tmp = w_name = gtk_label_new(_("Name:"));
    gtk_misc_set_alignment(GTK_MISC(tmp), 1, 0.5);
#if (GTKVER == 3)
    gtk_grid_attach(GTK_GRID(table), tmp, c1, row, 1, 1);
#else
    gtk_table_attach_defaults(GTK_TABLE(table), tmp, 0, 1, row, row+1);
#endif
    name_box = gtk_label_new("?");
    //gtk_label_set_width_chars(GTK_LABEL(name_box), 60);
    gtk_misc_set_alignment(GTK_MISC(name_box), 0, 0.5);
#if (GTKVER == 3)
    gtk_grid_attach(GTK_GRID(table), name_box, c2, row, 3, 1);
#else
    gtk_table_attach_defaults(GTK_TABLE(table), name_box, 1, 4, row, row+1);
#endif
    row++;

    tmp = w_weight = gtk_label_new(_("Weight:"));
    gtk_misc_set_alignment(GTK_MISC(tmp), 1, 0.5);
#if (GTKVER == 3)
    gtk_grid_attach(GTK_GRID(table), tmp, c1, row, 1, 1);
#else
    gtk_table_attach_defaults(GTK_TABLE(table), tmp, 0, 1, row, row+1);
#endif
    //GtkWidget *whbox = gtk_hbox_new(FALSE, 5);
    GtkWidget *wbutton = weight_entry = gtk_button_new_with_label("---");
    //gtk_widget_set_size_request(GTK_WIDGET(wbutton), 70, 0);
    weight_box = gtk_entry_new();
    //gtk_widget_set_size_request(GTK_WIDGET(weight_box), 300, 30);
    gtk_entry_set_width_chars(GTK_ENTRY(weight_box), 6);
    //gtk_box_pack_start_defaults(GTK_BOX(whbox), weight_box);
    //gtk_box_pack_start_defaults(GTK_BOX(whbox), wbutton);
#if (GTKVER == 3)
    gtk_grid_attach(GTK_GRID(table), weight_box, c2, row, 1, 1);
    gtk_grid_attach(GTK_GRID(table), wbutton, c3, row, 1, 1);
#else
    gtk_table_attach_defaults(GTK_TABLE(table), weight_box, 1, 2, row, row+1);
    gtk_table_attach_defaults(GTK_TABLE(table), wbutton, 2, 3, row, row+1);
#endif
    gtk_widget_grab_focus(wbutton);
    g_signal_connect(G_OBJECT(wbutton), "button-press-event", 
                     (GCallback) set_weight_on_button_pressed, weight_box);
    g_signal_connect(G_OBJECT(wbutton), "key-press-event", 
                     (GCallback) set_weight_on_button_pressed, weight_box);
    row++;


#ifdef JUDOGI_STATUS
    tmp = w_control = gtk_label_new(_("Control:"));
    gtk_misc_set_alignment(GTK_MISC(tmp), 1, 0.5);
#if (GTKVER == 3)
    gtk_grid_attach(GTK_GRID(table), tmp, c1, row, 1, 1);
#else
    gtk_table_attach_defaults(GTK_TABLE(table), tmp, 0, 1, row, row+1);
#endif
#if (GTKVER == 3)
    judogi_box = tmp = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(tmp), NULL, "?");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(tmp), NULL, _("OK"));
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(tmp), NULL, _("NOK"));
    gtk_grid_attach(GTK_GRID(table), tmp, c2, row, 1, 1);
#else
    judogi_box = tmp = gtk_combo_box_new_text();
    gtk_combo_box_append_text((GtkComboBox *)tmp, "?");
    gtk_combo_box_append_text((GtkComboBox *)tmp, _("OK"));
    gtk_combo_box_append_text((GtkComboBox *)tmp, _("NOK"));
    gtk_table_attach_defaults(GTK_TABLE(table), tmp, 1, 2, row, row+1);
#endif
#if 0
    if (deleted & JUDOGI_OK)
        gtk_combo_box_set_active((GtkComboBox *)tmp, 1);
    else if (deleted & JUDOGI_NOK)
        gtk_combo_box_set_active((GtkComboBox *)tmp, 2);
    else
        gtk_combo_box_set_active((GtkComboBox *)tmp, 0);
#endif
    row++;
#endif

    tmp = w_ok = gtk_button_new_with_label(_("OK"));
#if (GTKVER == 3)
    gtk_grid_attach(GTK_GRID(table), tmp, c2, row, 1, 1);
#else
    gtk_table_attach_defaults(GTK_TABLE(table), tmp, 1, 2, row, row+1);
#endif
    g_signal_connect(G_OBJECT(tmp), 
                     "clicked", G_CALLBACK(on_ok), NULL);
    row++;

    tmp = w_confirm = gtk_label_new(_("Confirm:"));
    gtk_misc_set_alignment(GTK_MISC(tmp), 1, 0.5);
#if (GTKVER == 3)
    gtk_grid_attach(GTK_GRID(table), tmp, c1, row, 1, 1);
#else
    gtk_table_attach_defaults(GTK_TABLE(table), tmp, 0, 1, row, row+1);
#endif
    confirm_box = gtk_label_new("");
    confirm_box2 = gtk_label_new("");
    gtk_misc_set_alignment(GTK_MISC(confirm_box), 0, 0.5);
    gtk_misc_set_alignment(GTK_MISC(confirm_box2), 0, 0.5);
#if (GTKVER == 3)
    gtk_grid_attach(GTK_GRID(table), confirm_box, c2, row, 3, 1);
    gtk_grid_attach(GTK_GRID(table), confirm_box2, c2, row+1, 3, 1);
#else
    gtk_table_attach_defaults(GTK_TABLE(table), confirm_box, 1, 4, row, row+1);
#endif
    row++;

#if (GTKVER == 3)
    gtk_grid_attach(GTK_GRID(main_vbox), table, 0, 1, 1, 1);
#else
    gtk_box_pack_start(GTK_BOX(main_vbox), table, FALSE, TRUE, 0);
#endif
    g_signal_connect(G_OBJECT(id_box), 
                     "activate", G_CALLBACK(on_enter), id_box);

    /* timers */
        
    timer = g_timer_new();

    gtk_widget_show_all(window);

    set_preferences();
    change_language(NULL, NULL, gint_to_ptr(language));

    open_comm_socket();
	
    /* Create a bg thread using glib */
#if (GTKVER == 3)
    gth = g_thread_new("Client",
                       (GThreadFunc)client_thread,
                       (gpointer)&run_flag); 
#else
    gth = g_thread_create((GThreadFunc)client_thread,
                          (gpointer)&run_flag, FALSE, NULL); 
#endif

#if (GTKVER == 3)
    gth = g_thread_new("Serial",
                       (GThreadFunc)serial_thread,
                       (gpointer)&run_flag); 
#else
    gth = g_thread_create((GThreadFunc)serial_thread,
                          (gpointer)&run_flag, FALSE, NULL); 
#endif

    extern gpointer ssdp_thread(gpointer args);
    g_snprintf(ssdp_id, sizeof(ssdp_id), "JudoWeight");
#if (GTKVER == 3)
    gth = g_thread_new("SSDP",
                       (GThreadFunc)ssdp_thread,
                       (gpointer)&run_flag); 
#else
    gth = g_thread_create((GThreadFunc)ssdp_thread,
                          (gpointer)&run_flag, FALSE, NULL);
#endif
    gth = gth; // make compiler happy

    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER_ALWAYS);
	
    cursor = gdk_cursor_new(GDK_HAND2);
    //gdk_window_set_cursor(GTK_WIDGET(main_window)->window, cursor);

    /* All GTK applications must have a gtk_main(). Control ends here
     * and waits for an event to occur (like a key press or
     * mouse event). */
    gtk_main();
    
#if (GTKVER != 3)    
    gdk_threads_leave();  /* release GDK locks */
#endif
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
    return APPLICATION_TYPE_WEIGHT;
}

void refresh_window(void)
{
    GtkWidget *widget;
    GdkRegion *region;
    widget = GTK_WIDGET(main_window);

#if (GTKVER == 3)
    if (gtk_widget_get_window(widget)) {
        cairo_rectangle_int_t r;
        r.x = 0;
        r.y = 0;
        r.width = gtk_widget_get_allocated_width(widget);
        r.height = gtk_widget_get_allocated_height(widget);
        region = cairo_region_create_rectangle(&r);
        gdk_window_invalidate_region(gtk_widget_get_window(widget), region, TRUE);
        gdk_window_process_updates(gtk_widget_get_window(widget), TRUE);
        cairo_region_destroy(region);
    }
#else
    if (widget->window) {
        region = gdk_drawable_get_clip_region(widget->window);
        gdk_window_invalidate_region(widget->window, region, TRUE);
        gdk_window_process_updates(widget->window, TRUE);
    }
#endif
}

gchar *logfile_name = NULL;

void judoweight_log(gchar *format, ...)
{
    guint t;
    gchar buf[256];
    va_list args;
    va_start(args, format);
    gchar *text = g_strdup_vprintf(format, args);
    va_end(args);

    t = time(NULL);

    if (logfile_name == NULL) {
        struct tm *tm = localtime((time_t *)&t);
        sprintf(buf, "judoweight_%04d%02d%02d_%02d%02d%02d.log",
                tm->tm_year+1900,
                tm->tm_mon+1,
                tm->tm_mday,
                tm->tm_hour,
                tm->tm_min,
                tm->tm_sec);
        logfile_name = g_build_filename(g_get_user_data_dir(), buf, NULL);
        g_print("logfile_name=%s\n", logfile_name);
    }

    FILE *f = fopen(logfile_name, "a");
    if (f) {
        struct tm *tm = localtime((time_t *)&t);
#ifdef USE_ISO_8859_1
        gsize x;

        gchar *text_ISO_8859_1 =
            g_convert(text, -1, "ISO-8859-1", "UTF-8", NULL, &x, NULL);

        fprintf(f, "%02d:%02d:%02d %s\n",
                tm->tm_hour,
                tm->tm_min,
                tm->tm_sec,
                text_ISO_8859_1);

        g_free(text_ISO_8859_1);
#else
        fprintf(f, "%02d:%02d:%02d %s\n",
                tm->tm_hour,
                tm->tm_min,
                tm->tm_sec,
                text);
#endif
        fclose(f);
    } else {
        g_print("Cannot open log file\n");
        perror("logfile_name");
    }

    g_free(text);
}

void change_language_1(void)
{
    gtk_label_set_label(GTK_LABEL(w_id), _("ID:"));
    gtk_label_set_label(GTK_LABEL(w_name), _("Name:"));
    gtk_label_set_label(GTK_LABEL(w_weight), _("Weight:"));
    gtk_label_set_label(GTK_LABEL(w_control), _("Control:"));
    gtk_button_set_label(GTK_BUTTON(w_ok), _("OK"));
    gtk_label_set_label(GTK_LABEL(w_confirm), _("Confirm:"));
}
