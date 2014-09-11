/* -*- mode: C; c-basic-offset: 4;  -*- */

/*
 * Copyright (C) 2006-2013 by Hannu Jokinen
 * Full copyright text is included in the software package.
 */ 

#if defined(__WIN32__) || defined(WIN32)

#define  __USE_W32_SOCKETS
//#define Win32_Winsock

#include <windows.h>
#include <stdio.h>
#include <initguid.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#define in_addr_t uint32_t
#else /* UNIX */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/tcp.h>
#include <fcntl.h>

#endif /* WIN32 */

#include <time.h>
#include <locale.h>

#include <gtk/gtk.h>
#include <glib.h>
#include <curl/curl.h>

#if (GTKVER == 3)
#include <gdk/gdkkeysyms-compat.h>
#else
#include <gdk/gdkkeysyms.h>
#endif

#ifdef WIN32
#include <process.h>
//#include <glib/gwin32.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif

/* System-dependent definitions */
#ifndef WIN32
#define closesocket     close
#define SOCKET          gint
#define INVALID_SOCKET  -1
#define SOCKET_ERROR    -1
#define SOCKADDR_BTH    struct sockaddr_rc
#define AF_BTH          PF_BLUETOOTH
#define BTHPROTO_RFCOMM BTPROTO_RFCOMM
#endif

#include "judoproxy.h"
#include "language.h"
#include "binreloc.h"

#define APPLICATION_TYPE_CAMERA 100

void get_parameters(gint i);
void send_parameter(gint i, gint j);
void send_parameter_2(struct sockaddr_in addr, gint port, const gchar *param, const gchar *value);
void send_cmd_parameters(gint i);
void scan_interfaces(void);
static gint check_table(gpointer data);
static gpointer connection_thread(gpointer args);


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
gint           num_lines = 3;//NUM_LINES;
gint           display_type = HORIZONTAL_DISPLAY;//NORMAL_DISPLAY;
gboolean       mirror_display = FALSE;
gboolean       white_first = FALSE;
gboolean       red_background = FALSE;
gchar         *filename = NULL;
GtkWidget     *id_box = NULL, *name_box = NULL;
GtkWidget     *connection_table;
static GdkColor color_yellow, color_white, color_grey, color_green, color_darkgreen, 
    color_blue, color_red, color_darkred, color_black;
gboolean       connections_updated = FALSE;
GtkWidget     *notebook;

#define MY_FONT "Arial"

#define THIN_LINE     (paper_width < 700.0 ? 1.0 : paper_width/700.0)
#define THICK_LINE    (2*THIN_LINE)

#define W(_w) ((_w)*paper_width)
#define H(_h) ((_h)*paper_height)

#define BOX_HEIGHT (horiz ? paper_height/(4.0*(num_lines+0.25)*2.0) : paper_height/(4.0*(num_lines+0.25)))
//#define BOX_HEIGHT (1.4*extents.height)

#define NUM_INTERFACES 8
#define NUM_DEVS 16

struct app_type {
    gint type;
#define APP_MODE_MASTER 1
#define APP_MODE_SLAVE  2
    gint mode;
    gint tatami;
};

static struct {
    gchar *name;
    uint32_t addr;
    GtkWidget *dev_table;
    gboolean changed;
    int fdin, fdout;
    struct application {
        gchar     *text;
        GtkWidget *textw;
        uint32_t   addr;
        GtkWidget *addrw;
        GtkWidget *check;
        struct app_type apptype;
    } device[NUM_DEVS];
} iface[NUM_INTERFACES];

static gint addrcnt = 0;

#define NUM_CONNECTIONS 10
#define START_PORT 2321

static struct {
    SOCKET fd_listen, fd_conn, fd_out;
    struct sockaddr_in caller, destination;
    GtkWidget *in_addr, *out_addr;
    GtkWidget *scrolled_window;
    gboolean changed;
} connections[NUM_CONNECTIONS];

// camera parameters
#define NUM_CAMERA_PARAMS 64
enum {
    PARAM_INT = 0,
    PARAM_BOOL,
    PARAM_STR,
    PARAM_SCALE,
    PARAM_CMD,
    PARAM_NO_ARG,
    PARAM_NEW_LINE,
    PARAM_NEW_COLUMN,
    PARAM_EMPTY_LINE
};
/*
    CAM_PARAM_SHARPNESS,
    CAM_PARAM_CONTRAST,
    CAM_PARAM_BRIGHTNESS,
    CAM_PARAM_SATURATION
*/
static struct {
    gchar *name, *command_line, *dynamic;
    gint typ;
    gchar *dflt;
} camera_params[NUM_CAMERA_PARAMS] = {
    {"Preview", NULL, "preview", PARAM_CMD, NULL},
    {"Rec stop", NULL, "stop", PARAM_CMD, NULL},
    {"Rec start", NULL, "start", PARAM_CMD, NULL},
    {"Restart", NULL, "quit", PARAM_CMD, NULL},
    {"NC", NULL, NULL, PARAM_NEW_COLUMN, NULL},
    {"Halt", NULL, "halt", PARAM_CMD, NULL},

    {"NL", NULL, NULL, PARAM_EMPTY_LINE, NULL},
    {" Width* ", "V_WIDTH", NULL, PARAM_STR, "640"},
    {" Height* ", "V_HEIGHT", NULL, PARAM_STR, "360"},
    {" Bitrate* ", "V_BITRATE", NULL, PARAM_STR, "500000"},
    {" Framerate* ", "V_FPS", NULL, PARAM_STR, "25"},

    {"NL", NULL, NULL, PARAM_NEW_LINE, NULL},
    {" Sharpness ", "V_SHARPNESS", "camera0", PARAM_SCALE, "0"},
    {" Contrast ", "V_CONTRAST", "camera1", PARAM_SCALE, "0"},
    {" Brightness ", "V_BRIGHTNESS", "camera2", PARAM_SCALE, "50"},
    {" Saturation ", "V_SATURATION", "camera3", PARAM_SCALE, "0"},

    {"NL", NULL, NULL, PARAM_EMPTY_LINE, NULL},
    {" Video dir* ", "VIDEODIR", NULL, PARAM_STR, "/home/pi/video"},
    {" Tatami* ", "TATAMI", NULL, PARAM_STR, "1"},
    {" Proxy address* ", "MASTERADDRESS", NULL, PARAM_STR, "0.0.0.0"},
    {" Logo mode ", "LOGOMODE", "logo", PARAM_STR, "shiai"},

    {"NL", NULL, NULL, PARAM_NEW_LINE, NULL},
    {" Audio* ", "AUDIO", NULL, PARAM_STR, ""},
    {" Zoom mode* ", "ZOOM", NULL, PARAM_BOOL, "1"},
    {" Force zoom ", "PARAM2", "param2", PARAM_BOOL, "0"},
    {" Force no zoom ", "PARAM3", "param3", PARAM_BOOL, "0"},

    {"NL", NULL, NULL, PARAM_EMPTY_LINE, NULL},
    {" Pixel thr ", "PARAM0", "param0", PARAM_STR, "20"},
    {" Movement thr ", "PARAM1", "param1", PARAM_STR, "30"},
    {" Test mov ", "PARAM4", "param4", PARAM_BOOL, "0"},
    {" Same XY thr ", "PARAM5", "param5", PARAM_STR, "8"},

    {"NL", NULL, NULL, PARAM_NEW_LINE, NULL},
    {" Criteria ", "PARAM6", "param6", PARAM_STR, "5"},
    {" Zoom hyst ", "PARAM7", "param7", PARAM_STR, "100"}
};

static struct {
    GtkWidget *name_w, *value_w;
    gchar *value;
} camera_values[NUM_TATAMIS][NUM_CAMERA_PARAMS];

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

static void enter_callback( GtkWidget *widget,
                            GtkWidget *entry )
{
    gulong a,b,c,d;
    gint i = ptr_to_gint(entry);
    const gchar *entry_text = gtk_entry_get_text(GTK_ENTRY(connections[i].out_addr));
    g_print("TIMER IP ENTRY=%s con=%d\n", entry_text, i);
    sscanf(entry_text, "%ld.%ld.%ld.%ld", &a, &b, &c, &d);
    connections[i].destination.sin_addr.s_addr = htonl((a << 24) | (b << 16) | (c << 8) | d);
    connections[i].changed = TRUE;
}

static void camera_ip_enter_callback( GtkWidget *widget,
                                      GtkWidget *entry )
{
    gulong a,b,c,d;
    gint i = ptr_to_gint(entry);
    const gchar *entry_text = gtk_entry_get_text(GTK_ENTRY(connections[i].in_addr));
    g_print("CAM IP ENTRY=%s con=%d\n", entry_text, i);
    sscanf(entry_text, "%ld.%ld.%ld.%ld", &a, &b, &c, &d);
    connections[i].caller.sin_addr.s_addr = htonl((a << 24) | (b << 16) | (c << 8) | d);
    connections[i].changed = TRUE;
}

#define UPDATE_TEXT(_w, _t) do { g_free(_w); _w = g_strdup(_t); } while (0)

static void camera_callback_value(GtkWidget *widget,
                                  gpointer *data)
{
    gchar buf[16];
    gint i = ptr_to_gint(data)/1000; // tatami
    gint j = ptr_to_gint(data)%1000; // param num
    if (camera_params[j].typ == PARAM_STR) {
        const gchar *entry_text = gtk_entry_get_text(GTK_ENTRY(camera_values[i][j].value_w));
        UPDATE_TEXT(camera_values[i][j].value, entry_text);
        g_print("CAM ENTRY=%s tatami=%d param=%d\n", entry_text, i+1, j);
    } else if (camera_params[j].typ == PARAM_SCALE) {
        snprintf(buf, sizeof(buf), "%f", gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget)));
        UPDATE_TEXT(camera_values[i][j].value, buf);
        //g_print("SCALE tatami=%d param=%d val=%f\n", i+1, j, gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget)));
    } else if (camera_params[j].typ == PARAM_BOOL) {
        snprintf(buf, sizeof(buf), "%d", gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
        UPDATE_TEXT(camera_values[i][j].value, buf);
        //g_print("BOOL tatami=%d param=%d val=%d\n", i+1, j, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
    } else {
        g_print("CAM BUTTON tatami=%d param=%d\n", i+1, j);
    }
    send_parameter(i, j);
}

static void camera_callback_preview(GtkWidget *widget,
                                    gpointer *data)
{
    gint i = ptr_to_gint(data)/1000; // tatami
    if (connections[i].caller.sin_addr.s_addr)
        create_video_window(connections[i].caller);
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
    int i;

    putenv("UBUNTU_MENUPROXY=");

    //init_trees();

    gdk_color_parse("#FFFF00", &color_yellow);
    gdk_color_parse("#FFFFFF", &color_white);
    gdk_color_parse("#404040", &color_grey);
    gdk_color_parse("#00FF00", &color_green);
    gdk_color_parse("#007700", &color_darkgreen);
    gdk_color_parse("#0000FF", &color_blue);
    gdk_color_parse("#FF0000", &color_red);
    gdk_color_parse("#770000", &color_darkred);
    gdk_color_parse("#000000", &color_black);

    font = pango_font_description_from_string("Sans bold 12");

#ifdef WIN32
    installation_dir = g_win32_get_package_installation_directory_of_module(NULL);
#else
    gbr_init(NULL);
    installation_dir = gbr_find_prefix(NULL);
#endif

    program_path = argv[0];

    current_directory[0] = 0;

    if (current_directory[0] == 0)
        strcpy(current_directory, ".");

    conffile = g_build_filename(g_get_user_data_dir(), "judoproxy.ini", NULL);

    keyfile = g_key_file_new();
    g_key_file_load_from_file(keyfile, conffile, 0, NULL);

    now = time(NULL);
    tm = localtime(&now);
    current_year = tm->tm_year+1900;
    srand(now); //srandom(now);
    my_address = now + getpid()*10000;

    gtk_init (&argc, &argv);

    main_window = window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window), "JudoProxy");
    gtk_widget_set_size_request(window, FRAME_WIDTH, FRAME_HEIGHT);

    gchar *iconfile = g_build_filename(installation_dir, "etc", "judoproxy.png", NULL);
    gtk_window_set_default_icon_from_file(iconfile, NULL);
    g_free(iconfile);

    g_signal_connect (G_OBJECT (window), "delete_event",
                      G_CALLBACK (delete_event), NULL);
    
    g_signal_connect (G_OBJECT (window), "destroy",
                      G_CALLBACK (destroy), NULL);
    
    gtk_container_set_border_width (GTK_CONTAINER (window), 10);

    main_vbox = gtk_grid_new();
    gtk_container_set_border_width (GTK_CONTAINER (main_vbox), 1);
    gtk_container_add (GTK_CONTAINER (window), main_vbox);
    gtk_widget_show(main_vbox);

    /* menubar */
    menubar = get_menubar_menu(window);
    gtk_widget_show(menubar);

    gtk_grid_attach(GTK_GRID(main_vbox), menubar, 0, 0, 1, 1);
    gtk_widget_set_hexpand(menubar, TRUE);

    /* judoka info */
    //name_box = gtk_label_new("?");
    //gtk_box_pack_start(GTK_BOX(main_vbox), name_box, FALSE, TRUE, 0);

    //GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
    //gtk_box_pack_start(GTK_BOX(main_vbox), hbox, FALSE, TRUE, 0);

    notebook = gtk_notebook_new();
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook), TRUE);
    gtk_widget_set_hexpand(notebook, TRUE);
    gtk_widget_set_vexpand(notebook, TRUE);
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_TOP);
    /*
    GtkWidget *camera_table = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(camera_table), 10);
    gtk_grid_set_row_spacing(GTK_GRID(camera_table), 10);
    */

#define SET_CAM_PARAM(_n, _t, _p, _v) do {                              \
        set_camera_param(data_table, (j&1)?2:0, j/2+3, i, j, _n, _t, _p, _v); \
        j++; \
    } while (0)

    for (i = 0; i < NUM_TATAMIS; i++) {
        gchar buf[32];
        connections[i].scrolled_window = gtk_scrolled_window_new(NULL, NULL);;
        gtk_container_set_border_width(GTK_CONTAINER(connections[i].scrolled_window), 10);

        snprintf(buf, sizeof(buf), "T%d [--]", i+1);
        gtk_notebook_append_page(GTK_NOTEBOOK(notebook), connections[i].scrolled_window, 
                                 gtk_label_new(buf));

        GtkWidget *data_table = gtk_grid_new();
        gtk_grid_attach(GTK_GRID(data_table), gtk_label_new(_("Camera")), 0, 0, 2, 1);
        gtk_grid_attach(GTK_GRID(data_table), gtk_label_new(_("JudoTimer")), 4, 0, 2, 1);

        connections[i].in_addr = gtk_entry_new();
        connections[i].out_addr = gtk_entry_new();
        gtk_entry_set_width_chars(GTK_ENTRY(connections[i].in_addr), 15);
        gtk_entry_set_width_chars(GTK_ENTRY(connections[i].out_addr), 15);
        gtk_grid_attach(GTK_GRID(data_table), connections[i].in_addr, 2, 0, 2, 1);
        gtk_grid_attach(GTK_GRID(data_table), connections[i].out_addr, 6, 0, 2, 1);
        gtk_grid_attach(GTK_GRID(data_table), gtk_label_new(" "), 0, 1, 1, 1);

        gint j = 0, row = 2, col = 0;

        for (j = 0; camera_params[j].name; j++) {
            if (camera_params[j].typ == PARAM_CMD) {
                camera_values[i][j].name_w = gtk_button_new_with_label(camera_params[j].name);

                if (strcmp(camera_params[j].dynamic, "preview") == 0)
                    g_signal_connect(camera_values[i][j].name_w, "clicked", 
                                     G_CALLBACK(camera_callback_preview), gint_to_ptr(i*1000+j));
                else
                    g_signal_connect(camera_values[i][j].name_w, "clicked", 
                                     G_CALLBACK(camera_callback_value), gint_to_ptr(i*1000+j));

                gtk_grid_attach(GTK_GRID(data_table), camera_values[i][j].name_w, col, row, 1, 1);
                col++;
            } else if (camera_params[j].typ == PARAM_NEW_LINE) {
                row++;
                col = 0;
            } else if (camera_params[j].typ == PARAM_NEW_COLUMN) {
                col++;
            } else if (camera_params[j].typ == PARAM_EMPTY_LINE) {
                row++;
                col = 0;
                gtk_grid_attach(GTK_GRID(data_table), gtk_label_new(" "), col, row, 1, 1);
                row++;
            } else if (camera_params[j].typ == PARAM_SCALE) {
                camera_values[i][j].name_w = gtk_label_new(camera_params[j].name);
                gtk_misc_set_alignment(GTK_MISC(camera_values[i][j].name_w), 1, 0.5);

                gdouble min, max, def = atof(camera_params[j].dflt);
                if (def == 50.0) {
                    min = 0.0; max = 100.0;
                } else {
                    min = -50.0; max = 50.0;
                }
                //GtkAdjustment *adj = gtk_adjustment_new(def, min, max, 1.0, 1.0, 1.0);
                //GtkWidget *scale = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 1.0, 0);
                GtkWidget *scale = gtk_spin_button_new_with_range(min, max, 1.0);
                gtk_spin_button_set_value(GTK_SPIN_BUTTON(scale), def);
                camera_values[i][j].value_w = scale;
                g_signal_connect(camera_values[i][j].value_w, "value-changed", 
                                 G_CALLBACK(camera_callback_value), gint_to_ptr(i*1000+j));

                gtk_grid_attach(GTK_GRID(data_table), camera_values[i][j].name_w, col, row, 1, 1);
                gtk_grid_attach(GTK_GRID(data_table), camera_values[i][j].value_w, col+1, row, 1, 1);
                col += 2;
            } else if (camera_params[j].typ == PARAM_BOOL) {
                camera_values[i][j].name_w = gtk_label_new(camera_params[j].name);
                gtk_misc_set_alignment(GTK_MISC(camera_values[i][j].name_w), 1, 0.5);
                GtkWidget *check = gtk_check_button_new();
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
                                             camera_params[j].dflt[0] == '1');
                camera_values[i][j].value_w = check;
                g_signal_connect(camera_values[i][j].value_w, "toggled", 
                                 G_CALLBACK(camera_callback_value), gint_to_ptr(i*1000+j));

                gtk_grid_attach(GTK_GRID(data_table), camera_values[i][j].name_w, col, row, 1, 1);
                gtk_grid_attach(GTK_GRID(data_table), camera_values[i][j].value_w, col+1, row, 1, 1);
                col += 2;
            } else {
                camera_values[i][j].name_w = gtk_label_new(camera_params[j].name);
                gtk_misc_set_alignment(GTK_MISC(camera_values[i][j].name_w), 1, 0.5);
                camera_values[i][j].value_w = gtk_entry_new();
                gtk_entry_set_width_chars(GTK_ENTRY(camera_values[i][j].value_w), 4);
                gtk_entry_set_text(GTK_ENTRY(camera_values[i][j].value_w), camera_params[j].dflt);
                g_signal_connect(camera_values[i][j].value_w, "activate", 
                                 G_CALLBACK(camera_callback_value), gint_to_ptr(i*1000+j));
                gtk_grid_attach(GTK_GRID(data_table), camera_values[i][j].name_w, col, row, 1, 1);
                gtk_grid_attach(GTK_GRID(data_table), camera_values[i][j].value_w, col+1, row, 1, 1);
                col += 2;
            }
        }

        gtk_container_add(GTK_CONTAINER(connections[i].scrolled_window), data_table);
        g_signal_connect (connections[i].out_addr, "activate",
                          G_CALLBACK(enter_callback),
                          gint_to_ptr(i));
        g_signal_connect (connections[i].in_addr, "activate",
                          G_CALLBACK(camera_ip_enter_callback),
                          gint_to_ptr(i));
    }

    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), 0);
    gtk_grid_attach_next_to(GTK_GRID(main_vbox), notebook, NULL, GTK_POS_BOTTOM, 1, 1);


    /* timers */
        
    timer = g_timer_new();

    gtk_widget_show_all(window);

    set_preferences();
    change_language(NULL, NULL, gint_to_ptr(language));

    open_comm_socket();
    scan_interfaces();

    /* Create a bg thread using glib */
    gpointer proxy_ssdp_thread(gpointer args);
    g_snprintf(ssdp_id, sizeof(ssdp_id), "JudoProxy");

    gth = g_thread_new("Connection",
                       (GThreadFunc)connection_thread,
                       (gpointer)&run_flag); 

    gth = g_thread_new("SSDP",
                       (GThreadFunc)proxy_ssdp_thread,
                       (gpointer)&run_flag); 
    gth = gth; // make compiler happy

    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER_ALWAYS);
	
    cursor = gdk_cursor_new(GDK_HAND2);
    //gdk_window_set_cursor(GTK_WIDGET(main_window)->window, cursor);

    //g_timeout_add(100, timeout_ask_for_data, NULL);
    g_timeout_add(1000, check_table, NULL);

    /* All GTK applications must have a gtk_main(). Control ends here
     * and waits for an event to occur (like a key press or
     * mouse event). */
    gtk_main();
    
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
    return APPLICATION_TYPE_PROXY;
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

void update_connections_table(void)
{
    gint i;

    for (i = 0; i < NUM_CONNECTIONS; i++) {
        gint stars = 0;
        gchar buf[32];

        if (connections[i].destination.sin_addr.s_addr) {
            gtk_entry_set_text(GTK_ENTRY(connections[i].out_addr), inet_ntoa(connections[i].destination.sin_addr));
            if (connections[i].fd_out > 0) stars |= 2;
        } else {
            gtk_entry_set_text(GTK_ENTRY(connections[i].out_addr), "");
        }

        if (connections[i].caller.sin_addr.s_addr) {
            gtk_entry_set_text(GTK_ENTRY(connections[i].in_addr), inet_ntoa(connections[i].caller.sin_addr));
            if (connections[i].fd_conn > 0) stars |= 1;
        } else {
            gtk_entry_set_text(GTK_ENTRY(connections[i].in_addr), "");
        }

        snprintf(buf, sizeof(buf), "T%d [%c%c]", i+1, 
                 stars & 1 ? '*' : '-', stars & 2 ? '*' : '-');
        gtk_notebook_set_tab_label_text(GTK_NOTEBOOK(notebook), connections[i].scrolled_window, buf);
    }
}

static void update_column(gint i)
{
    gint j;

    if (!iface[i].changed)
        return;

    iface[i].changed = FALSE;

    for (j = 0; j < NUM_DEVS; j++) {
        if (!iface[i].device[j].text)
            continue;

        // update connection table
        if (iface[i].device[j].apptype.type == APPLICATION_TYPE_TIMER &&
            iface[i].device[j].apptype.mode == APP_MODE_MASTER) {
            gint tatami = iface[i].device[j].apptype.tatami;
            if (tatami > 0 && tatami <= NUM_TATAMIS && connections[tatami-1].fd_out < 0) {
                connections[tatami-1].destination.sin_addr.s_addr = iface[i].device[j].addr;
                connections_updated = TRUE;
            }
        } else if (iface[i].device[j].apptype.type == APPLICATION_TYPE_CAMERA) {
            gint tatami = iface[i].device[j].apptype.tatami;
            if (tatami > 0 && tatami <= NUM_TATAMIS && connections[tatami-1].fd_conn < 0) {
                connections[tatami-1].caller.sin_addr.s_addr = iface[i].device[j].addr;
                connections_updated = TRUE;
                get_parameters(tatami-1);
            }
        }
    }
}

static gint check_table(gpointer data)
{
    gint i;
    for (i = 0; i < addrcnt; i++)
        update_column(i);

    if (connections_updated) {
        update_connections_table();
        connections_updated = FALSE;
    }
    return TRUE;
}

static struct app_type get_app_type(gchar *s)
{
    struct app_type at;
    at.type = 0;
    at.mode = 0;
    at.tatami = 0;

    if (strstr(s, "Camera")) {
        at.type = APPLICATION_TYPE_CAMERA;
        gchar *p = strstr(s, "Camera ");
        if (p) at.tatami = atoi(p+7);
    } else if (strstr(s, "JudoShiai"))
        at.type = APPLICATION_TYPE_SHIAI;
    else if (strstr(s, "JudoInfo"))
        at.type = APPLICATION_TYPE_INFO;
    else if (strstr(s, "JudoTimer")) {
        at.type = APPLICATION_TYPE_TIMER;
        gchar *p = strstr(s, "tatami=");
        if (p) at.tatami = atoi(p+7);

        if (strstr(s, "master"))
            at.mode = APP_MODE_MASTER;
        else if (strstr(s, "slave"))
            at.mode = APP_MODE_SLAVE;
    }

    return at;
}

struct application *report_to_proxy(gchar *rec, struct sockaddr_in *client, gint ifnum)
{
    //g_print("===============\n%s\n--------------\n", rec);

    uint32_t addr = client->sin_addr.s_addr;
    gchar *p1, *p = strstr(rec, "Judo");
    gint i;

    if (!p)
        return NULL;

    p1 = strchr(p, '\r');
    if (!p1) p1 = strchr(p, '\n');
    if (!p1)
        return NULL;

    *p1 = 0;

    p1 = strchr(p, '/');
    if (p1) *p1 = 0;

    //g_print("RX: '%s'\n", p);

    in_addr_t mask = 0xffffffff;

    while (mask) {
        for (i = 0; i < addrcnt; i++) {
            if ((ntohl(addr) & mask) == (ntohl(iface[i].addr) & mask)) {
                gint j;
                for (j = 0; j < NUM_DEVS; j++) {
                    if (!iface[i].device[j].text) {
                        iface[i].device[j].text = g_strdup(p);
                        iface[i].device[j].addr = addr;
                        iface[i].device[j].apptype = get_app_type(p);
                        iface[i].changed = TRUE;
                        g_print("new device if=%d dev=%d ifnum=%d addr=%s type=%d/%d %s\n", 
                                i, j, ifnum, 
                                inet_ntoa(client->sin_addr),
                                iface[i].device[j].apptype.type, iface[i].device[j].apptype.tatami, 
                                iface[i].device[j].text);
                        return &(iface[i].device[j]);
                    } else if (strncmp(iface[i].device[j].text, p, 7) == 0 &&
                               iface[i].device[j].addr == addr) {
                        if (strcmp(iface[i].device[j].text, p)) {
                            g_free(iface[i].device[j].text);
                            iface[i].device[j].text = g_strdup(p);
                            iface[i].changed = TRUE;
                            g_print("updated device if=%d dev=%d ifnum=%d addr=%s %s\n", i, j, ifnum,
                                    inet_ntoa(client->sin_addr), iface[i].device[j].text);
                        }
                        struct app_type at = get_app_type(p);
                        if (memcmp(&iface[i].device[j].apptype, &at, sizeof(at))) {
                            iface[i].device[j].apptype = at;
                            iface[i].changed = TRUE;
                        }
                        return &(iface[i].device[j]);
                    }
                }
                g_print("No more space\n");
                return NULL;
            }
        }
        mask <<= 1;
    }
    g_print("Cannot find interface\n");

    return NULL;
}

#define SSDP_MULTICAST      "239.255.255.250"
#define SSDP_PORT           1900
#define URN ":urn:judoshiai:service:all:" SHIAI_VERSION
#define ST "ST" URN
#define NT "NT" URN

static gchar *ssdp_req_data = NULL;
static gint   ssdp_req_data_len = 0;
gchar ssdp_id[64];
//gboolean ssdp_notify = TRUE;

#define perror(x) do {g_print("ERROR LINE=%d\n", __LINE__); perror(x); } while (0)

gpointer proxy_ssdp_thread(gpointer args)
{
    gint ret;
    struct sockaddr_in name_out, name_in;
    struct sockaddr_in clientsock;
    static gchar inbuf[1024];
    fd_set read_fd, fds;
    guint socklen;
    struct ip_mreq mreq;
    gint i;

    ssdp_req_data = g_strdup_printf("M-SEARCH * HTTP/1.1\r\n"
                                    "HOST: 239.255.255.250:1900\r\n"
                                    "MAN: \"ssdp:discover\"\r\n"
                                    ST "\r\n"
                                    "MX:3\r\n"
                                    "\r\n");
    ssdp_req_data_len = strlen(ssdp_req_data);

    FD_ZERO(&fds);

    // receiving socket
    for (i = 0; i < addrcnt; i++) {
        g_print("Initialize %x\n", iface[i].addr);

        if ((iface[i].fdin = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET) {
            perror("SSDP socket");
            continue;
        }

        FD_SET(iface[i].fdin , &fds);

        gint reuse = 1;
        if (setsockopt(iface[i].fdin, SOL_SOCKET, SO_REUSEADDR, (void *)&reuse, sizeof(reuse)) < 0) {
            perror("setsockopt (SO_REUSEADDR)");
        }

#if 0 // SO_BINDTODEVICE requires root privileges
        struct ifreq ifr;
        memset(&ifr, 0, sizeof(ifr));
        snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", iface[i].name);
        if (setsockopt(iface[i].fdin, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) < 0) {
            perror("bindtodevice in");
        }
#endif

        memset((gchar *)&name_in, 0, sizeof(name_in));
        name_in.sin_family = AF_INET;
        name_in.sin_port = htons(SSDP_PORT);
        name_in.sin_addr.s_addr = 0;//htonl(iface[i].addr);
        if (bind(iface[i].fdin, (struct sockaddr *)&name_in, sizeof(name_in)) == -1) {
            perror("SSDP bind");
        }

        memset(&mreq, 0, sizeof(mreq));
        mreq.imr_multiaddr.s_addr = inet_addr(SSDP_MULTICAST);
        mreq.imr_interface.s_addr = iface[i].addr;
        if (setsockopt(iface[i].fdin, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) == -1) {
            perror("SSDP setsockopt");
        }

        // transmitting socket
        if ((iface[i].fdout = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET) {
            perror("SSDP socket");
            continue;
        }

#if 0 // SO_BINDTODEVICE requires root privileges
        memset(&ifr, 0, sizeof(ifr));
        snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", iface[i].name);
        if (setsockopt(iface[i].fdout, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) < 0) {
            perror("bindtodevice out");
        }
#endif
        FD_SET(iface[i].fdout , &fds);

        gint ttl = 3;
        setsockopt(iface[i].fdout, IPPROTO_IP, IP_MULTICAST_TTL, (void *)&ttl, sizeof(ttl));
    } // for

    memset((gchar*)&name_out, 0, sizeof(name_out));
    name_out.sin_family = AF_INET;
    name_out.sin_port = htons(SSDP_PORT);
    name_out.sin_addr.s_addr = inet_addr(SSDP_MULTICAST);

    for ( ; *((gboolean *)args) ; )   /* exit loop when flag is cleared */
    {
        struct application *appl;
	struct timeval timeout;
        gint len;

        timeout.tv_sec=1;
        timeout.tv_usec=0;
        read_fd = fds;

        if (select(32, &read_fd, NULL, NULL, &timeout) < 0) {
            perror("SSDP select");
            continue;
        }
        
        for (i = 0; i < addrcnt; i++) {
            if (iface[i].fdin > 0 && FD_ISSET(iface[i].fdin, &read_fd)) {
                socklen = sizeof(clientsock);
                if ((len = recvfrom(iface[i].fdin, inbuf, sizeof(inbuf)-1, 0, 
                                    (struct sockaddr *)&clientsock, &socklen)) < 0) {
                    perror("SSDP recvfrom");
                    continue;
                }
                inbuf[len] = 0;
                //g_print("RXin[%d]: '%s'\n", i, inbuf);

                if (strncmp(inbuf, "NOTIFY", 6) == 0) {
                    appl = report_to_proxy(inbuf, &clientsock, i);
                    appl = appl; // make compiler happy
                }
            }
            if (iface[i].fdout > 0 && FD_ISSET(iface[i].fdout, &read_fd)) {
                socklen = sizeof(clientsock);
                if ((len = recvfrom(iface[i].fdout, inbuf, sizeof(inbuf)-1, 0, 
                                    (struct sockaddr *)&clientsock, &socklen)) < 0) {
                    perror("SSDP recvfrom");
                    continue;
                }
                inbuf[len] = 0;
                //g_print("RXout[%d]: '%s'\n", i, inbuf);

                if (strncmp(inbuf, "HTTP/1.1 200 OK", 12) == 0) {
                    appl = report_to_proxy(inbuf, &clientsock, i);
#if 0
                    if (0 && appl && appl->apptype.type == APPLICATION_TYPE_TIMER &&
                        appl->apptype.mode == APP_MODE_MASTER) {
                        gint j;

                        for (j = 0; j < addrcnt; j++) {
                            if (i == j || iface[j].fdout <= 0)
                                continue;

                            ret = sendto(iface[j].fdout, inbuf, len, 0, 
                                         (struct sockaddr*) &name_out, 
                                         sizeof(struct sockaddr_in));
                            g_print("SSDP message forwarded to port %d\n", j);
                        
                        } // for
                    } // if
#endif
                } // if (strncmp(inbuf, "HTTP/1.1 200 OK", 12) == 0)
            }
        }

        static time_t last_time = 0;
        time_t now = time(NULL);

        if (now > last_time + 5) {
            last_time = now;

            for (i = 0; i < addrcnt; i++) {
                if (iface[i].fdout <= 0)
                    continue;

                ret = sendto(iface[i].fdout, ssdp_req_data, ssdp_req_data_len, 0, 
                             (struct sockaddr*) &name_out, 
                             sizeof(struct sockaddr_in));
                //g_print("SSDP %s REQ SEND by timeout\n\n", APPLICATION);
                if (ret != ssdp_req_data_len) {
                    perror("SSDP send req");
                }
            } // for
        }
    } // eternal for

    g_print("SSDP OUT!\n");
    g_thread_exit(NULL);    /* not required just good pratice */
    return NULL;
}


void scan_interfaces(void)
{
    addrcnt = 0;

#ifdef WIN32
    gint i;

    SOCKET sd = WSASocket(AF_INET, SOCK_DGRAM, 0, 0, 0, 0);
    if (sd == SOCKET_ERROR) {
        return 0;
    }

    INTERFACE_INFO InterfaceList[20];
    unsigned long nBytesReturned;
    if (WSAIoctl(sd, SIO_GET_INTERFACE_LIST, 0, 0, &InterfaceList,
                 sizeof(InterfaceList), &nBytesReturned, 0, 0) == SOCKET_ERROR) {
        closesocket(sd);
        return 0;
    }

    gint nNumInterfaces = nBytesReturned / sizeof(INTERFACE_INFO);

    for (i = 0; i < nNumInterfaces; ++i) {
        struct sockaddr_in *pAddress;
        pAddress = (struct sockaddr_in *) & (InterfaceList[i].iiAddress);
        g_print("Interface %s\n", inet_ntoa(pAddress->sin_addr));
        if (addrcnt < NUM_INTERFACES) {
            iface[addrcnt].name = "";
            iface[addrcnt++].addr = pAddress->sin_addr.s_addr;
        }
    }

    closesocket(sd);

#else // !WIN32

    struct if_nameindex *ifnames, *ifnm;
    struct ifreq ifr;
    struct sockaddr_in sin;
    gint   fd;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
        return;

    ifnames = if_nameindex();
    for (ifnm = ifnames; ifnm && ifnm->if_name && ifnm->if_name[0]; ifnm++) {
        strncpy (ifr.ifr_name, ifnm->if_name, IFNAMSIZ);
        if (ioctl (fd, SIOCGIFADDR, &ifr) == 0) {
            memcpy (&sin, &(ifr.ifr_addr), sizeof (sin));
            if (addrcnt < NUM_INTERFACES) {
                iface[addrcnt].name = g_strdup(ifnm->if_name);
                iface[addrcnt++].addr = sin.sin_addr.s_addr;
                g_print ("Proxy Interface %s = IP %s\n", ifnm->if_name, inet_ntoa(sin.sin_addr));
            }
        }
    }
    if_freenameindex(ifnames);
    close(fd);

#endif // WIN32
}

static gpointer connection_thread(gpointer args)
{
    gint i;
    struct sockaddr_in my_addr, node;
    static guchar inbuf[2048];
    fd_set read_fd, fds;
    gint reuse = 1;

    FD_ZERO(&read_fd);

    for (i = 0; i < NUM_CONNECTIONS; i++) {
        if ((connections[i].fd_listen = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
            perror("listen socket");
            g_thread_exit(NULL);    /* not required just good pratice */
            return NULL;
        }

        if (setsockopt(connections[i].fd_listen, SOL_SOCKET, SO_REUSEADDR, (void *)&reuse, sizeof(reuse)) < 0) {
            perror("setsockopt (SO_REUSEADDR)");
        }

        memset(&my_addr, 0, sizeof(my_addr));
        my_addr.sin_family = AF_INET;
        my_addr.sin_port = htons(START_PORT + i);
        my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

        if (bind(connections[i].fd_listen, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) < 0) {
            perror("listen bind");
            g_print("CANNOT BIND!\n");
            g_thread_exit(NULL);    /* not required just good pratice */
            return NULL;
        }

        listen(connections[i].fd_listen, 5);

        FD_SET(connections[i].fd_listen, &read_fd);
        connections[i].fd_conn = -1;
        connections[i].fd_out = -1;
    }

    for ( ; *((gboolean *)args) ; )   /* exit loop when flag is cleared */
    {
        struct timeval timeout;
        gint r, i;
                
        fds = read_fd;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;

        r = select(64, &fds, NULL, NULL, &timeout);

        for (i = 0; i < NUM_CONNECTIONS; i++) {
            if (connections[i].fd_out > 0 && connections[i].changed) {
                g_print("Closing out connection %d\n", i);
                closesocket(connections[i].fd_out);
                connections[i].fd_out = -1;
                connections_updated = TRUE;
            }

            if (connections[i].fd_out < 0 && connections[i].destination.sin_addr.s_addr) {
                if ((connections[i].fd_out = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
                    perror("out socket");
                    continue;
                }
                connections[i].changed = FALSE;

                memset(&node, 0, sizeof(node));
                node.sin_family      = AF_INET;
                node.sin_port        = htons(SHIAI_PORT+2);
                node.sin_addr.s_addr = connections[i].destination.sin_addr.s_addr;

                if (connect(connections[i].fd_out, (struct sockaddr *)&node, sizeof(node))) {
                    closesocket(connections[i].fd_out);
                    connections[i].fd_out = -1;
                    g_print("Connection out %d to %s failed!\n", i, inet_ntoa(node.sin_addr));
                } else {
                    FD_SET(connections[i].fd_out, &read_fd);
                    g_print("Out connection %d (fd=%d) to %s\n", i, 
                            connections[i].fd_out, inet_ntoa(node.sin_addr));
                    connections_updated = TRUE;
                }
            }

            if (r <= 0)
                continue;

            /* messages to receive */
            if (FD_ISSET(connections[i].fd_listen, &fds)) {
                guint alen = sizeof(connections[i].caller);
                if (connections[i].fd_conn < 0) {
                    if ((connections[i].fd_conn = accept(connections[i].fd_listen, 
                                                         (struct sockaddr *)&connections[i].caller, 
                                                         &alen)) < 0) {
                        perror("conn accept");
                        continue;
                    } else {
                        FD_SET(connections[i].fd_conn, &read_fd);
                        connections_updated = TRUE;
                        g_print("New connection in %d (fd=%d) from %s\n", i, connections[i].fd_conn,
                                inet_ntoa(connections[i].caller.sin_addr));
                    }
                } else {
                    g_print("Connection %d already in use.", i);
                }
            }

            if (connections[i].fd_conn > 0 && FD_ISSET(connections[i].fd_conn, &fds)) {
                r = recv(connections[i].fd_conn, inbuf, sizeof(inbuf), 0);
                if (r <= 0) {
                    g_print("Connection in %d fd=%d closed\n", i, connections[i].fd_conn);
                    closesocket(connections[i].fd_conn);
                    FD_CLR(connections[i].fd_conn, &read_fd);
                    connections[i].fd_conn = -1;
                    connections_updated = TRUE;
                } else {
                    if (connections[i].fd_out > 0) {
                        send(connections[i].fd_out, inbuf, r, 0);
                    }
                }
            }            

            if (connections[i].fd_out > 0 && FD_ISSET(connections[i].fd_out, &fds)) {
                r = recv(connections[i].fd_out, inbuf, sizeof(inbuf), 0);
                if (r <= 0) {
                    g_print("Connection out %d fd=%d closed\n", i, connections[i].fd_out);
                    closesocket(connections[i].fd_out);
                    FD_CLR(connections[i].fd_out, &read_fd);
                    connections[i].fd_out = -1;
                    connections_updated = TRUE;
                } else {
                    if (connections[i].fd_conn > 0) {
                        send(connections[i].fd_conn, inbuf, r, 0);
                    }
                }
            }            
        
        } // for connections


    } // outer for

    g_print("CONNECT OUT!\n");
    g_thread_exit(NULL);    /* not required just good pratice */
    return NULL;
}

#define PARAMETER_PORT_CMD_LINE 3345
#define PARAMETER_PORT_DYNAMIC  3346

void send_parameter(gint i, gint j)
{
    if (camera_params[j].dynamic)
        send_parameter_2(connections[i].caller, PARAMETER_PORT_DYNAMIC, 
                       camera_params[j].dynamic, 
                       camera_params[j].typ != PARAM_CMD ? camera_values[i][j].value : NULL);
    if (camera_params[j].command_line)
        send_parameter_2(connections[i].caller, PARAMETER_PORT_CMD_LINE, 
                       camera_params[j].command_line, camera_values[i][j].value);
}

void get_parameters(gint i)
{
    struct sockaddr_in rpi, addr = connections[i].caller;
    SOCKET fd = socket(AF_INET, SOCK_STREAM, 0);
    gchar buf[1024];

    if (fd == INVALID_SOCKET) {
        perror("out socket");
        return;
    }
    g_print("Getting parameters for tatami %d\n", i);

    memset(&rpi, 0, sizeof(rpi));
    rpi.sin_family      = AF_INET;
    rpi.sin_port        = htons(PARAMETER_PORT_CMD_LINE);
    rpi.sin_addr        = addr.sin_addr;

    if (connect(fd, (struct sockaddr *)&rpi, sizeof(rpi))) {
        closesocket(fd);
        g_print("Parameter connection to rpi %s failed!\n", inet_ntoa(rpi.sin_addr));
        return;
    }

    if (send(fd, "GET=\n", 4, 0) < 0)
        g_print("Parameter send to rpi %s failed!\n", inet_ntoa(rpi.sin_addr));

    gint r = recv(fd, buf, sizeof(buf)-1, 0);
    if (r > 0) {
        buf[r] = 0;
        g_print("REC: i=%d\n%s\n", i, buf);
        gint j;
        for (j = 0; camera_params[j].name; j++) {
            if (!camera_params[j].command_line)
                continue;

            gchar *p = strstr(buf, camera_params[j].command_line);
            if (!p)
                continue;

            p += strlen(camera_params[j].command_line);

            if (*p != '=')
                continue;

            p++;
            gchar val[64];
            gint k = 0;
            while (*p > ' ' && k < sizeof(val) - 1)
                val[k++] = *p++;
            val[k] = 0;

            UPDATE_TEXT(camera_values[i][j].value, val);
            gtk_entry_set_text(GTK_ENTRY(camera_values[i][j].value_w), val);
        }
    }
    g_print("Done\n");

    closesocket(fd);
}

void send_parameter_2(struct sockaddr_in addr, gint port, const gchar *param, const gchar *value)
{
    struct sockaddr_in rpi;
    SOCKET fd = socket(AF_INET, SOCK_STREAM, 0);
    gchar buf[128];

    if (fd == INVALID_SOCKET) {
        perror("out socket");
        return;
    }

    if (param == NULL)
        snprintf(buf, sizeof(buf), "#x=x\n");
    if (value == NULL)
        snprintf(buf, sizeof(buf), "%s\n", param);
    else
        snprintf(buf, sizeof(buf), "%s=%s\n", param, value);

    memset(&rpi, 0, sizeof(rpi));
    rpi.sin_family      = AF_INET;
    rpi.sin_port        = htons(port);
    rpi.sin_addr        = addr.sin_addr;
    g_print("sending '%s'\n", buf);

    if (connect(fd, (struct sockaddr *)&rpi, sizeof(rpi))) {
        closesocket(fd);
        g_print("Parameter connection to rpi %s failed!\n", inet_ntoa(rpi.sin_addr));
        return;
    }

    if (send(fd, buf, strlen(buf), 0) < 0)
        g_print("Parameter send to rpi %s failed!\n", inet_ntoa(rpi.sin_addr));

    closesocket(fd);
}

void send_cmd_parameters(gint i)
{
    gint j;

    for (j = 0; camera_params[j].name; j++) {
        if (camera_params[j].command_line)
            send_parameter_2(connections[i].caller, PARAMETER_PORT_CMD_LINE, 
                             camera_params[j].command_line, camera_values[i][j].value);
    }
}

