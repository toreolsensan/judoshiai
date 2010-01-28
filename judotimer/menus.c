/* -*- mode: C; c-basic-offset: 4;  -*- */

/*
 * Copyright (C) 2006-2010 by Hannu Jokinen
 * Full copyright text is included in the software package.
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "judotimer.h"

void start_help(GtkWidget *w, gpointer data);

static void about_judotimer( GtkWidget *w,
                             gpointer   data )
{
    //GtkWidget *dialog;
        
    gtk_show_about_dialog (NULL, 
                           "name", "Judotimer",
                           "title", _("About Judotimer"),
                           "copyright", "Copyright 2006-2009 Hannu Jokinen",
                           "version", SHIAI_VERSION,
                           "website", "http://www.kolumbus.fi/oh2ncp/",
                           NULL);
#if 0
    dialog = gtk_message_dialog_new (NULL,
                                     0 /*GTK_DIALOG_DESTROY_WITH_PARENT*/,
                                     GTK_MESSAGE_INFO,
                                     GTK_BUTTONS_OK,
                                     "Judotimer versio 0.5\nCopyright 2006-2007 Hannu Jokinen");

    g_signal_connect_swapped (dialog, "response",
                              G_CALLBACK (gtk_widget_destroy),
                              dialog);

    gtk_widget_show(dialog);
#endif
}


static void match_auto(GtkWidget *w,
                       gpointer   data)
{
    clock_key(GDK_0, FALSE);
}

static void match_e_2(GtkWidget *w,
                      gpointer   data)
{
    //short_pin_times = GTK_CHECK_MENU_ITEM(w)->active;
    clock_key(GDK_1, FALSE);
}

static void match_2(GtkWidget *w,
                    gpointer   data)
{
    clock_key(GDK_2, FALSE);
}

static void match_3(GtkWidget *w,
                    gpointer   data)
{
    clock_key(GDK_3, FALSE);
}

static void match_4(GtkWidget *w,
                    gpointer   data)
{
    clock_key(GDK_4, FALSE);
}

static void match_5( GtkWidget *w,
                     gpointer   data )
{
    clock_key(GDK_5, FALSE);
}

static void gs_cb( GtkWidget *w,
		   gpointer   data )
{
    clock_key(GDK_9, FALSE);
}

static void tatami_selection(GtkWidget *w,
                             gpointer   data )
{
    tatami = (gint)data;
    g_key_file_set_integer(keyfile, "preferences", "tatami", tatami);
}

static void mode_selection(GtkWidget *w,
			   gpointer   data )
{
    mode = (gint)data;
}

static GtkWidget *clock_min, *clock_sec, *osaekomi;

static void set_timers(GtkWidget *widget, 
		       GdkEvent *event,
		       GtkWidget *data)
{
    if ((gulong)event == GTK_RESPONSE_OK) {
        set_clocks(atoi(gtk_entry_get_text(GTK_ENTRY(clock_min)))*60 +
                   atoi(gtk_entry_get_text(GTK_ENTRY(clock_sec))),
                   atoi(gtk_entry_get_text(GTK_ENTRY(osaekomi))));
    }
    gtk_widget_destroy(widget);
    clock_min = clock_sec = osaekomi = NULL;
}

static void manipulate_time(GtkWidget *w,
                            gpointer   data )
{
    gint what = (gint)data;
    switch (what) {
    case 0:
        hajime_inc_func();
        break;
    case 1:
        hajime_dec_func();
        break;
    case 2:
        osaekomi_inc_func();
        break;
    case 3:
        osaekomi_dec_func();
        break;
    case 4: {
        GtkWidget *dialog, *hbox, *label;

        dialog = gtk_dialog_new_with_buttons (_("Set clocks"),
                                              NULL,
                                              GTK_DIALOG_DESTROY_WITH_PARENT,
                                              GTK_STOCK_OK, GTK_RESPONSE_OK,
                                              GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                              NULL);

        clock_min = gtk_entry_new();
        clock_sec = gtk_entry_new();
        osaekomi  = gtk_entry_new();
        gtk_entry_set_max_length(GTK_ENTRY(clock_min), 1);
        gtk_entry_set_max_length(GTK_ENTRY(clock_sec), 2);
        gtk_entry_set_max_length(GTK_ENTRY(osaekomi), 2);
        gtk_entry_set_width_chars(GTK_ENTRY(clock_min), 1);
        gtk_entry_set_width_chars(GTK_ENTRY(clock_sec), 2);
        gtk_entry_set_width_chars(GTK_ENTRY(osaekomi), 2);

        gtk_entry_set_text(GTK_ENTRY(clock_min), "0");
        gtk_entry_set_text(GTK_ENTRY(clock_sec), "00");
        gtk_entry_set_text(GTK_ENTRY(osaekomi), "00");

        hbox = gtk_hbox_new(FALSE, 5);
        gtk_container_set_border_width(GTK_CONTAINER(hbox), 10);

        label = gtk_label_new(_("Clock:"));
        gtk_box_pack_start_defaults(GTK_BOX(hbox), label);
        gtk_box_pack_start_defaults(GTK_BOX(hbox), clock_min);
        label = gtk_label_new(":");
        gtk_box_pack_start_defaults(GTK_BOX(hbox), label);
        gtk_box_pack_start_defaults(GTK_BOX(hbox), clock_sec);
        label = gtk_label_new("Osaekomi:");
        gtk_box_pack_start_defaults(GTK_BOX(hbox), label);
        gtk_box_pack_start_defaults(GTK_BOX(hbox), osaekomi);

        gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox);
        gtk_widget_show_all(dialog);

        g_signal_connect(G_OBJECT(dialog), "response",
                         G_CALLBACK(set_timers), NULL);
        break;
    }
    }
}

static void change_menu_label(GtkWidget *item, const gchar *new_text)
{
    GtkWidget *menu_label = gtk_bin_get_child(GTK_BIN(item));
    gtk_label_set_text(GTK_LABEL(menu_label), new_text); 
}

static GtkWidget *menubar, *match, *preferences, *help, *matchmenu, *preferencesmenu, *helpmenu;
static GtkWidget *separator1, *separator2, *quit;
static GtkWidget *match0, *match1, *match2, *match3, *match4, *match5, *gs;
static GtkWidget *blue_wins, *white_wins, *red_background, *full_screen, *rules_no_koka;
static GtkWidget *rules_leave_points;
static GtkWidget *tatami_sel_none, *tatami_sel_1,  *tatami_sel_2,  *tatami_sel_3,  *tatami_sel_4;
static GtkWidget *tatami_sel_5,  *tatami_sel_6,  *tatami_sel_7,  *tatami_sel_8;
static GtkWidget *node_ip, *my_ip, *manual, *about, *quick_guide;
static GtkWidget *flag_fi, *flag_se, *flag_uk, *menu_flag_fi, *menu_flag_se, *menu_flag_uk;
static GtkWidget *light, *menu_light;
static GtkWidget *inc_time, *dec_time, *inc_osaekomi, *dec_osaekomi, *clock_only, *set_time;
static GtkWidget *layout_sel_1, *layout_sel_2, *layout_sel_3, *layout_sel_4, *layout_sel_5, *layout_sel_6;
static GtkTooltips *menu_tips;
static GtkWidget *mode_normal, *mode_master, *mode_slave;
static GtkWidget *undo, *hansokumake_blue, *hansokumake_white, *clear_selection;
static GtkWidget *advertise;

static GtkWidget *get_picture(const gchar *name)
{
    gchar *file = g_build_filename(installation_dir, "etc", name, NULL);
    GtkWidget *pic = gtk_image_new_from_file(file);
    g_free(file);
    return pic;
}

static void set_menu_item_picture(GtkImageMenuItem *menu_item, gchar *name)
{
    GtkImage *image = (GtkImage *)gtk_image_menu_item_get_image(GTK_IMAGE_MENU_ITEM(menu_item));
    gchar *file = g_build_filename(installation_dir, "etc", name, NULL);
    gtk_image_set_from_file(image, file);
    g_free(file);
}

static gint light_callback(gpointer data)
{
    extern gboolean connection_ok;
    extern time_t traffic_last_rec_time;
    static gboolean last_ok = FALSE;
    static gboolean yellow_set = FALSE;
        
    if (yellow_set == FALSE && connection_ok && time(NULL) > traffic_last_rec_time + 6) {
        set_menu_item_picture(GTK_IMAGE_MENU_ITEM(menu_light), "yellowlight.png");
        yellow_set = TRUE;
        return TRUE;
    } else if (yellow_set && connection_ok && time(NULL) < traffic_last_rec_time + 6) {
        yellow_set = FALSE;
        last_ok = !connection_ok;
    }

    if (connection_ok == last_ok)
        return TRUE;

    last_ok = connection_ok;

    if (connection_ok)
        set_menu_item_picture(GTK_IMAGE_MENU_ITEM(menu_light), "greenlight.png");
    else		
        set_menu_item_picture(GTK_IMAGE_MENU_ITEM(menu_light), "redlight.png");

    return TRUE;
}

/* Returns a menubar widget made from the above menu */
GtkWidget *get_menubar_menu(GtkWidget  *window)
{
    GtkAccelGroup *group;

    menu_tips = gtk_tooltips_new ();
    group = gtk_accel_group_new ();
    menubar = gtk_menu_bar_new ();

    match       = gtk_menu_item_new_with_label (_("Match"));
    preferences = gtk_menu_item_new_with_label (_("Preferences"));
    help        = gtk_menu_item_new_with_label (_("Help"));
    undo        = gtk_menu_item_new_with_label (_("Undo!"));

    flag_fi     = get_picture("finland.png");
    flag_se     = get_picture("sweden.png");
    flag_uk     = get_picture("uk.png");
    menu_flag_fi = gtk_image_menu_item_new();
    menu_flag_se = gtk_image_menu_item_new();
    menu_flag_uk = gtk_image_menu_item_new();
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_flag_fi), flag_fi);        
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_flag_se), flag_se);        
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_flag_uk), flag_uk);        

    light      = get_picture("redlight.png");
    menu_light = gtk_image_menu_item_new();
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_light), light);

    matchmenu        = gtk_menu_new ();
    preferencesmenu  = gtk_menu_new ();
    helpmenu         = gtk_menu_new ();
        
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (match), matchmenu); 
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (preferences), preferencesmenu); 
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (help), helpmenu);
  
    gtk_menu_shell_append (GTK_MENU_SHELL (menubar), match); 
    gtk_menu_shell_append (GTK_MENU_SHELL (menubar), preferences); 
    gtk_menu_shell_append (GTK_MENU_SHELL (menubar), help);
    gtk_menu_shell_append (GTK_MENU_SHELL (menubar), menu_flag_fi); 
    gtk_menu_shell_append (GTK_MENU_SHELL (menubar), menu_flag_se); 
    gtk_menu_shell_append (GTK_MENU_SHELL (menubar), menu_flag_uk); 
    g_signal_connect(G_OBJECT(menu_flag_fi), "button_press_event",
                     G_CALLBACK(change_language), (gpointer)LANG_FI);
    g_signal_connect(G_OBJECT(menu_flag_se), "button_press_event",
                     G_CALLBACK(change_language), (gpointer)LANG_SW);
    g_signal_connect(G_OBJECT(menu_flag_uk), "button_press_event",
                     G_CALLBACK(change_language), (gpointer)LANG_EN);
    //gtk_menu_shell_append (GTK_MENU_SHELL (menubar), undo); 
    g_signal_connect(G_OBJECT(undo), "button_press_event",
                     G_CALLBACK(undo_func), (gpointer)NULL);


    gtk_menu_shell_append (GTK_MENU_SHELL (menubar), menu_light); 
    g_signal_connect(G_OBJECT(menu_light), "button_press_event",
                     G_CALLBACK(ask_node_ip_address), (gpointer)NULL);
    gtk_menu_item_set_right_justified(GTK_MENU_ITEM(menu_light), TRUE);
#if 0
    gtk_menu_item_set_right_justified(menu_flag_uk, TRUE);
    gtk_menu_item_set_right_justified(menu_flag_se, TRUE);
    gtk_menu_item_set_right_justified(menu_flag_fi, TRUE);
    gtk_menu_item_set_right_justified(help, TRUE);
#endif
  
    /* Create the Contest menu content. */
    match0 = gtk_menu_item_new_with_label(_("Match duration: automatic"));
    //match1 = gtk_check_menu_item_new_with_label(_("Short pin times"));
    match1 = gtk_menu_item_new_with_label(_("Match duration: 2 min E-juniors"));
    match2 = gtk_menu_item_new_with_label(_("Match duration: 2 min"));
    match3 = gtk_menu_item_new_with_label(_("Match duration: 3 min"));
    match4 = gtk_menu_item_new_with_label(_("Match duration: 4 min"));
    match5 = gtk_menu_item_new_with_label(_("Match duration: 5 min"));
    gs     = gtk_menu_item_new_with_label(_("Golden Score"));
    separator1 = gtk_separator_menu_item_new();
    blue_wins = gtk_menu_item_new_with_label(_("Hantei: blue wins"));
    white_wins = gtk_menu_item_new_with_label(_("Hantei: white wins"));
    hansokumake_blue = gtk_menu_item_new_with_label(_("Hansoku-make to blue"));
    hansokumake_white = gtk_menu_item_new_with_label(_("Hansoku-make to white"));
    clear_selection = gtk_menu_item_new_with_label(_("Clear selection"));
    separator2 = gtk_separator_menu_item_new();
    quit = gtk_menu_item_new_with_label(_("Quit"));

    gtk_menu_shell_append (GTK_MENU_SHELL (matchmenu), match0);
    gtk_menu_shell_append (GTK_MENU_SHELL (matchmenu), match1);
    gtk_menu_shell_append (GTK_MENU_SHELL (matchmenu), match2);
    gtk_menu_shell_append (GTK_MENU_SHELL (matchmenu), match3);
    gtk_menu_shell_append (GTK_MENU_SHELL (matchmenu), match4);
    gtk_menu_shell_append (GTK_MENU_SHELL (matchmenu), match5);
    gtk_menu_shell_append (GTK_MENU_SHELL (matchmenu), gs);
    gtk_menu_shell_append (GTK_MENU_SHELL (matchmenu), separator1);
    gtk_menu_shell_append (GTK_MENU_SHELL (matchmenu), blue_wins);
    gtk_menu_shell_append (GTK_MENU_SHELL (matchmenu), white_wins);
    //gtk_menu_shell_append (GTK_MENU_SHELL (matchmenu), hansokumake_blue);
    //gtk_menu_shell_append (GTK_MENU_SHELL (matchmenu), hansokumake_white);
    gtk_menu_shell_append (GTK_MENU_SHELL (matchmenu), clear_selection);
    gtk_menu_shell_append (GTK_MENU_SHELL (matchmenu), separator2);
    gtk_menu_shell_append (GTK_MENU_SHELL (matchmenu), quit);

    g_signal_connect(G_OBJECT(match0),      "activate", G_CALLBACK(match_auto), 0);
    g_signal_connect(G_OBJECT(match1),      "activate", G_CALLBACK(match_e_2), 0);
    g_signal_connect(G_OBJECT(match2),      "activate", G_CALLBACK(match_2), 0);
    g_signal_connect(G_OBJECT(match3),      "activate", G_CALLBACK(match_3), 0);
    g_signal_connect(G_OBJECT(match4),      "activate", G_CALLBACK(match_4), 0);
    g_signal_connect(G_OBJECT(match5),      "activate", G_CALLBACK(match_5), 0);
    g_signal_connect(G_OBJECT(gs),          "activate", G_CALLBACK(gs_cb), 0);
    g_signal_connect(G_OBJECT(blue_wins),   "activate", G_CALLBACK(voting_result), (gpointer)HANTEI_BLUE);
    g_signal_connect(G_OBJECT(white_wins),  "activate", G_CALLBACK(voting_result), (gpointer)HANTEI_WHITE);
    g_signal_connect(G_OBJECT(hansokumake_blue), "activate", G_CALLBACK(voting_result), (gpointer)HANSOKUMAKE_BLUE);
    g_signal_connect(G_OBJECT(hansokumake_white), "activate", G_CALLBACK(voting_result), (gpointer)HANSOKUMAKE_WHITE);
    g_signal_connect(G_OBJECT(clear_selection), "activate", G_CALLBACK(voting_result), (gpointer)CLEAR_SELECTION);

    g_signal_connect(G_OBJECT(quit),        "activate", G_CALLBACK(destroy/*gtk_main_quit*/), NULL);

    gtk_widget_add_accelerator(quit, "activate", group, GDK_Q, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(match0, "activate", group, GDK_0, 0, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(match1, "activate", group, GDK_1, 0, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(match2, "activate", group, GDK_2, 0, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(match3, "activate", group, GDK_3, 0, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(match4, "activate", group, GDK_4, 0, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(match5, "activate", group, GDK_5, 0, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(gs,     "activate", group, GDK_9, 0, GTK_ACCEL_VISIBLE);

    /* Create the Preferences menu content. */
    red_background  = gtk_check_menu_item_new_with_label("Red background");
    clock_only      = gtk_check_menu_item_new_with_label("View only clocks");
    layout_sel_1    = gtk_radio_menu_item_new_with_label(NULL, "");
    layout_sel_2    = gtk_radio_menu_item_new_with_label_from_widget((GtkRadioMenuItem *)layout_sel_1, "");
    layout_sel_3    = gtk_radio_menu_item_new_with_label_from_widget((GtkRadioMenuItem *)layout_sel_1, "");
    layout_sel_4    = gtk_radio_menu_item_new_with_label_from_widget((GtkRadioMenuItem *)layout_sel_1, "");
    layout_sel_5    = gtk_radio_menu_item_new_with_label_from_widget((GtkRadioMenuItem *)layout_sel_1, "");
    layout_sel_6    = gtk_radio_menu_item_new_with_label_from_widget((GtkRadioMenuItem *)layout_sel_1, "");
    full_screen     = gtk_check_menu_item_new_with_label("Full screen mode");
    rules_no_koka   = gtk_check_menu_item_new_with_label("");
    rules_leave_points = gtk_check_menu_item_new_with_label("");
    tatami_sel_none = gtk_radio_menu_item_new_with_label(NULL, "");
    tatami_sel_1    = gtk_radio_menu_item_new_with_label_from_widget((GtkRadioMenuItem *)tatami_sel_none, "");
    tatami_sel_2    = gtk_radio_menu_item_new_with_label_from_widget((GtkRadioMenuItem *)tatami_sel_none, "");
    tatami_sel_3    = gtk_radio_menu_item_new_with_label_from_widget((GtkRadioMenuItem *)tatami_sel_none, "");
    tatami_sel_4    = gtk_radio_menu_item_new_with_label_from_widget((GtkRadioMenuItem *)tatami_sel_none, "");
    tatami_sel_5    = gtk_radio_menu_item_new_with_label_from_widget((GtkRadioMenuItem *)tatami_sel_none, "");
    tatami_sel_6    = gtk_radio_menu_item_new_with_label_from_widget((GtkRadioMenuItem *)tatami_sel_none, "");
    tatami_sel_7    = gtk_radio_menu_item_new_with_label_from_widget((GtkRadioMenuItem *)tatami_sel_none, "");
    tatami_sel_8    = gtk_radio_menu_item_new_with_label_from_widget((GtkRadioMenuItem *)tatami_sel_none, "");
    node_ip         = gtk_menu_item_new_with_label("Communication node");
    my_ip           = gtk_menu_item_new_with_label("Own IP addresses");
    inc_time        = gtk_menu_item_new_with_label("");
    dec_time        = gtk_menu_item_new_with_label("");
    inc_osaekomi    = gtk_menu_item_new_with_label("");
    dec_osaekomi    = gtk_menu_item_new_with_label("");
    set_time        = gtk_menu_item_new_with_label("");
    mode_normal     = gtk_radio_menu_item_new_with_label(NULL, "");
    mode_master     = gtk_radio_menu_item_new_with_label_from_widget((GtkRadioMenuItem *)mode_normal, "");
    mode_slave      = gtk_radio_menu_item_new_with_label_from_widget((GtkRadioMenuItem *)mode_normal, "");
    advertise       = gtk_menu_item_new_with_label("");


    gtk_menu_shell_append (GTK_MENU_SHELL (preferencesmenu), red_background);
    gtk_menu_shell_append (GTK_MENU_SHELL (preferencesmenu), full_screen);
    gtk_menu_shell_append (GTK_MENU_SHELL (preferencesmenu), rules_no_koka);
    gtk_menu_shell_append (GTK_MENU_SHELL (preferencesmenu), rules_leave_points);
    gtk_menu_shell_append (GTK_MENU_SHELL (preferencesmenu), gtk_separator_menu_item_new());
    gtk_menu_shell_append (GTK_MENU_SHELL (preferencesmenu), layout_sel_1);
    gtk_menu_shell_append (GTK_MENU_SHELL (preferencesmenu), layout_sel_2);
    gtk_menu_shell_append (GTK_MENU_SHELL (preferencesmenu), layout_sel_3);
    gtk_menu_shell_append (GTK_MENU_SHELL (preferencesmenu), layout_sel_4);
    gtk_menu_shell_append (GTK_MENU_SHELL (preferencesmenu), layout_sel_6);
    gtk_menu_shell_append (GTK_MENU_SHELL (preferencesmenu), layout_sel_5);
    gtk_menu_shell_append (GTK_MENU_SHELL (preferencesmenu), gtk_separator_menu_item_new());
    gtk_menu_shell_append (GTK_MENU_SHELL (preferencesmenu), tatami_sel_none);
    gtk_menu_shell_append (GTK_MENU_SHELL (preferencesmenu), tatami_sel_1);
    gtk_menu_shell_append (GTK_MENU_SHELL (preferencesmenu), tatami_sel_2);
    gtk_menu_shell_append (GTK_MENU_SHELL (preferencesmenu), tatami_sel_3);
    gtk_menu_shell_append (GTK_MENU_SHELL (preferencesmenu), tatami_sel_4);
    gtk_menu_shell_append (GTK_MENU_SHELL (preferencesmenu), tatami_sel_5);
    gtk_menu_shell_append (GTK_MENU_SHELL (preferencesmenu), tatami_sel_6);
    gtk_menu_shell_append (GTK_MENU_SHELL (preferencesmenu), tatami_sel_7);
    gtk_menu_shell_append (GTK_MENU_SHELL (preferencesmenu), tatami_sel_8);
    gtk_menu_shell_append (GTK_MENU_SHELL (preferencesmenu), gtk_separator_menu_item_new());
    gtk_menu_shell_append (GTK_MENU_SHELL (preferencesmenu), node_ip);
    gtk_menu_shell_append (GTK_MENU_SHELL (preferencesmenu), my_ip);
    gtk_menu_shell_append (GTK_MENU_SHELL (preferencesmenu), gtk_separator_menu_item_new());
    gtk_menu_shell_append (GTK_MENU_SHELL (preferencesmenu), inc_time);
    gtk_menu_shell_append (GTK_MENU_SHELL (preferencesmenu), dec_time);
    gtk_menu_shell_append (GTK_MENU_SHELL (preferencesmenu), inc_osaekomi);
    gtk_menu_shell_append (GTK_MENU_SHELL (preferencesmenu), dec_osaekomi);
    gtk_menu_shell_append (GTK_MENU_SHELL (preferencesmenu), set_time);
    gtk_menu_shell_append (GTK_MENU_SHELL (preferencesmenu), gtk_separator_menu_item_new());
    gtk_menu_shell_append (GTK_MENU_SHELL (preferencesmenu), mode_normal);
    gtk_menu_shell_append (GTK_MENU_SHELL (preferencesmenu), mode_master);
    gtk_menu_shell_append (GTK_MENU_SHELL (preferencesmenu), mode_slave);

    gtk_menu_shell_append (GTK_MENU_SHELL (preferencesmenu), gtk_separator_menu_item_new());
    gtk_menu_shell_append (GTK_MENU_SHELL (preferencesmenu), advertise);

    g_signal_connect(G_OBJECT(red_background),  "activate", G_CALLBACK(toggle_color),         (gpointer)1);
    g_signal_connect(G_OBJECT(full_screen),     "activate", G_CALLBACK(toggle_full_screen),   (gpointer)0);
    g_signal_connect(G_OBJECT(rules_no_koka),      "activate", G_CALLBACK(toggle_rules_no_koka),    (gpointer)0);
    g_signal_connect(G_OBJECT(rules_leave_points), "activate", G_CALLBACK(toggle_rules_leave_points), (gpointer)0);
    g_signal_connect(G_OBJECT(layout_sel_1),    "activate", G_CALLBACK(select_display_layout), (gpointer)1);
    g_signal_connect(G_OBJECT(layout_sel_2),    "activate", G_CALLBACK(select_display_layout), (gpointer)2);
    g_signal_connect(G_OBJECT(layout_sel_3),    "activate", G_CALLBACK(select_display_layout), (gpointer)3);
    g_signal_connect(G_OBJECT(layout_sel_4),    "activate", G_CALLBACK(select_display_layout), (gpointer)4);
    g_signal_connect(G_OBJECT(layout_sel_5),    "activate", G_CALLBACK(select_display_layout), (gpointer)5);
    g_signal_connect(G_OBJECT(layout_sel_6),    "activate", G_CALLBACK(select_display_layout), (gpointer)6);
    g_signal_connect(G_OBJECT(tatami_sel_none), "activate", G_CALLBACK(tatami_selection),     (gpointer)0);
    g_signal_connect(G_OBJECT(tatami_sel_1),    "activate", G_CALLBACK(tatami_selection),     (gpointer)1);
    g_signal_connect(G_OBJECT(tatami_sel_2),    "activate", G_CALLBACK(tatami_selection),     (gpointer)2);
    g_signal_connect(G_OBJECT(tatami_sel_3),    "activate", G_CALLBACK(tatami_selection),     (gpointer)3);
    g_signal_connect(G_OBJECT(tatami_sel_4),    "activate", G_CALLBACK(tatami_selection),     (gpointer)4);
    g_signal_connect(G_OBJECT(tatami_sel_5),    "activate", G_CALLBACK(tatami_selection),     (gpointer)5);
    g_signal_connect(G_OBJECT(tatami_sel_6),    "activate", G_CALLBACK(tatami_selection),     (gpointer)6);
    g_signal_connect(G_OBJECT(tatami_sel_7),    "activate", G_CALLBACK(tatami_selection),     (gpointer)7);
    g_signal_connect(G_OBJECT(tatami_sel_8),    "activate", G_CALLBACK(tatami_selection),     (gpointer)8);
    g_signal_connect(G_OBJECT(node_ip),         "activate", G_CALLBACK(ask_node_ip_address),  (gpointer)0);
    g_signal_connect(G_OBJECT(my_ip),           "activate", G_CALLBACK(show_my_ip_addresses), (gpointer)0);
    g_signal_connect(G_OBJECT(inc_time),        "activate", G_CALLBACK(manipulate_time),      (gpointer)0);
    g_signal_connect(G_OBJECT(dec_time),        "activate", G_CALLBACK(manipulate_time),      (gpointer)1);
    g_signal_connect(G_OBJECT(inc_osaekomi),    "activate", G_CALLBACK(manipulate_time),      (gpointer)2);
    g_signal_connect(G_OBJECT(dec_osaekomi),    "activate", G_CALLBACK(manipulate_time),      (gpointer)3);
    g_signal_connect(G_OBJECT(set_time),        "activate", G_CALLBACK(manipulate_time),      (gpointer)4);
    g_signal_connect(G_OBJECT(mode_normal),     "activate", G_CALLBACK(mode_selection),       (gpointer)0);
    g_signal_connect(G_OBJECT(mode_master),     "activate", G_CALLBACK(mode_selection),       (gpointer)1);
    g_signal_connect(G_OBJECT(mode_slave),      "activate", G_CALLBACK(mode_selection),       (gpointer)2);
    g_signal_connect(G_OBJECT(advertise),       "activate", G_CALLBACK(toggle_advertise),   (gpointer)0);

    /* Create the Help menu content. */
    manual = gtk_menu_item_new_with_label("Manual");
    quick_guide = gtk_menu_item_new_with_label("Quick guide");
    about = gtk_menu_item_new_with_label("About");

    gtk_menu_shell_append (GTK_MENU_SHELL (helpmenu), manual);
    gtk_menu_shell_append (GTK_MENU_SHELL (helpmenu), quick_guide);
    gtk_menu_shell_append (GTK_MENU_SHELL (helpmenu), about);

    g_signal_connect(G_OBJECT(manual),          "activate", G_CALLBACK(start_help),            (gpointer)0);
    g_signal_connect(G_OBJECT(quick_guide),     "activate", G_CALLBACK(start_help),            (gpointer)1);
    g_signal_connect(G_OBJECT(about),           "activate", G_CALLBACK(about_judotimer),       (gpointer)0);

    /* Attach the new accelerator group to the window. */
    gtk_window_add_accel_group (GTK_WINDOW (window), group);

    g_timeout_add(1000, light_callback, NULL);

    /* Finally, return the actual menu bar created by the item factory. */
    return menubar;
}

void set_preferences(void)
{
    GError *error = NULL;
    gchar  *str;
    gint    i;

    if ((str = g_key_file_get_string(keyfile, "preferences", "color", &error))) {
        if (strcmp(str, "red") == 0) {
            gtk_menu_item_activate(GTK_MENU_ITEM(red_background));
            //gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(red_background), TRUE);
            //toggle_color(red_background, NULL);
        }
        g_free(str);
    }

    error = NULL;
    if (g_key_file_get_boolean(keyfile, "preferences", "fullscreen", &error)) {
        gtk_menu_item_activate(GTK_MENU_ITEM(full_screen));
    }

    error = NULL;
    if (g_key_file_get_boolean(keyfile, "preferences", "rulesnokoka", &error) || error) {
        gtk_menu_item_activate(GTK_MENU_ITEM(rules_no_koka));
    }

    error = NULL;
    if (g_key_file_get_boolean(keyfile, "preferences", "rulesleavepoints", &error) || error) {
        gtk_menu_item_activate(GTK_MENU_ITEM(rules_leave_points));
    }

    error = NULL;
    i = g_key_file_get_integer(keyfile, "preferences", "tatami", &error);
    if (!error) {
        switch (i) {
        case 0: gtk_menu_item_activate(GTK_MENU_ITEM(tatami_sel_none)); break;
        case 1: gtk_menu_item_activate(GTK_MENU_ITEM(tatami_sel_1)); break;
        case 2: gtk_menu_item_activate(GTK_MENU_ITEM(tatami_sel_2)); break;
        case 3: gtk_menu_item_activate(GTK_MENU_ITEM(tatami_sel_3)); break;
        case 4: gtk_menu_item_activate(GTK_MENU_ITEM(tatami_sel_4)); break;
        case 5: gtk_menu_item_activate(GTK_MENU_ITEM(tatami_sel_5)); break;
        case 6: gtk_menu_item_activate(GTK_MENU_ITEM(tatami_sel_6)); break;
        case 7: gtk_menu_item_activate(GTK_MENU_ITEM(tatami_sel_7)); break;
        case 8: gtk_menu_item_activate(GTK_MENU_ITEM(tatami_sel_8)); break;
        }
    }

    error = NULL;
    i = g_key_file_get_integer(keyfile, "preferences", "displaylayout", &error);
    if (!error) {
        switch (i) {
        case 1: gtk_menu_item_activate(GTK_MENU_ITEM(layout_sel_1)); break;
        case 2: gtk_menu_item_activate(GTK_MENU_ITEM(layout_sel_2)); break;
        case 3: gtk_menu_item_activate(GTK_MENU_ITEM(layout_sel_3)); break;
        case 4: gtk_menu_item_activate(GTK_MENU_ITEM(layout_sel_4)); break;
        case 5: gtk_menu_item_activate(GTK_MENU_ITEM(layout_sel_5)); break;
        case 6: gtk_menu_item_activate(GTK_MENU_ITEM(layout_sel_6)); break;
        }
    } else {
        select_display_layout(NULL, (gpointer)1);
    }

    error = NULL;
    str = g_key_file_get_string(keyfile, "preferences", "nodeipaddress", &error);
    if (!error) {
        gulong a,b,c,d;
        sscanf(str, "%ld.%ld.%ld.%ld", &a, &b, &c, &d);
        node_ip_addr = host2net((a << 24) | (b << 16) | (c << 8) | d);
        g_free(str);
    }

    error = NULL;
    i = g_key_file_get_integer(keyfile, "preferences", "language", &error);
    if (!error)
        language = i;
    else
        language = LANG_FI;
}

gboolean change_language(GtkWidget *eventbox, GdkEventButton *event, void *param)
{
    gint lang = (gint)param;
    gchar *r = NULL;

    switch (lang) {
    case LANG_FI:
        putenv("LANGUAGE=fi");
        r = setlocale(LC_ALL, "fi");
        break;        
    case LANG_SW:
        putenv("LANGUAGE=sv");
        r = setlocale(LC_ALL, "sv");
        break;        
    case LANG_EN:
        putenv("LANGUAGE=en");
        r = setlocale(LC_ALL, "en");
        break;        
    }

    gchar *dirname = g_build_filename(installation_dir, "share", "locale", NULL);
    bindtextdomain ("judoshiai", dirname);
    g_free(dirname);
    bind_textdomain_codeset ("judoshiai", "UTF-8");
    textdomain ("judoshiai");

    change_menu_label(match,        _("Contest"));
    change_menu_label(preferences,  _("Preferences"));
    change_menu_label(help,         _("Help"));
    change_menu_label(undo,         _("Undo!"));

    change_menu_label(match0, _("Contest duration: automatic"));
    change_menu_label(match1, _("Contest duration: 2 min (short pin times)"));
    change_menu_label(match2, _("Contest duration: 2 min"));
    change_menu_label(match3, _("Contest duration: 3 min"));
    change_menu_label(match4, _("Contest duration: 4 min"));
    change_menu_label(match5, _("Contest duration: 5 min"));
    change_menu_label(gs,     _("Golden Score"));

    change_menu_label(blue_wins,         _("Hantei: blue wins"));
    change_menu_label(white_wins,        _("Hantei: white wins"));
    change_menu_label(hansokumake_blue,  _("Hansoku-make to blue"));
    change_menu_label(hansokumake_white, _("Hansoku-make to white"));
    change_menu_label(clear_selection,   _("Clear selection"));

    change_menu_label(quit,         _("Quit"));

    change_menu_label(red_background, _("Red background"));
    change_menu_label(full_screen, _("Full screen mode"));
    change_menu_label(rules_no_koka, _("No koka"));
    change_menu_label(rules_leave_points, _("Leave points for GS"));
    change_menu_label(clock_only, _("View clocks only"));

    change_menu_label(layout_sel_1, _("Display layout 1"));
    change_menu_label(layout_sel_2, _("Display layout 2"));
    change_menu_label(layout_sel_3, _("Display layout 3"));
    change_menu_label(layout_sel_4, _("Display layout 4"));
    change_menu_label(layout_sel_5, _("View clocks only"));
    change_menu_label(layout_sel_6, _("Display layout 5"));

    change_menu_label(tatami_sel_none, _("Contest area not chosen"));
    change_menu_label(tatami_sel_1, _("Contest area 1"));
    change_menu_label(tatami_sel_2, _("Contest area 2"));
    change_menu_label(tatami_sel_3, _("Contest area 3"));
    change_menu_label(tatami_sel_4, _("Contest area 4"));
    change_menu_label(tatami_sel_5, _("Contest area 5"));
    change_menu_label(tatami_sel_6, _("Contest area 6"));
    change_menu_label(tatami_sel_7, _("Contest area 7"));
    change_menu_label(tatami_sel_8, _("Contest area 8"));

    change_menu_label(node_ip,      _("Communication node"));
    change_menu_label(my_ip,        _("Own IP addresses"));

    change_menu_label(inc_time,     _("Increment time"));
    change_menu_label(dec_time,     _("Decrement time"));
    change_menu_label(inc_osaekomi, _("Increment pin time"));
    change_menu_label(dec_osaekomi, _("Decrement pin time"));
    change_menu_label(set_time,     _("Set clocks"));

    change_menu_label(mode_normal, _("Normal operation"));
    change_menu_label(mode_master, _("Master mode"));
    change_menu_label(mode_slave,  _("Slave mode"));

    change_menu_label(advertise,    _("Advertise"));

    change_menu_label(manual,       _("Manual"));
    change_menu_label(quick_guide,  _("Quick guide"));
    change_menu_label(about,        _("About"));

    /* tooltips */
    gtk_tooltips_set_tip (GTK_TOOLTIPS (menu_tips), menu_flag_fi,
                          _("Change language to Finnish"), NULL);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (menu_tips), menu_flag_se,
                          _("Change language to Swedish"), NULL);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (menu_tips), menu_flag_uk,
                          _("Change language to English"), NULL);

    gtk_tooltips_set_tip (GTK_TOOLTIPS (menu_tips), match0,
                          _("Contest duration automatically from JudoShiai program"), NULL);

    change_language_1();

    g_key_file_set_integer(keyfile, "preferences", "language", lang);

    return TRUE;
}

#if 0

static gint nmenu_items = sizeof (menu_items_en) / sizeof (menu_items_en[0]);

/* Returns a menubar widget made from the above menu */
GtkWidget *get_menubar_menu_en( GtkWidget  *window )
{
    GtkItemFactory *item_factory;
    GtkAccelGroup *accel_group;

    /* Make an accelerator group (shortcut keys) */
    accel_group = gtk_accel_group_new ();

    /* Make an ItemFactory (that makes a menubar) */
    item_factory = gtk_item_factory_new (GTK_TYPE_MENU_BAR, "<main>",
                                         accel_group);

    /* This function generates the menu items. Pass the item factory,
       the number of items in the array, the array itself, and any
       callback data for the the menu items. */
    gtk_item_factory_create_items (item_factory, nmenu_items, menu_items_en, NULL);

    /* Attach the new accelerator group to the window. */
    gtk_window_add_accel_group (GTK_WINDOW (window), accel_group);

    /* Finally, return the actual menu bar created by the item factory. */
    return gtk_item_factory_get_widget (item_factory, "<main>");
}

GtkWidget *get_menubar_menu_fi( GtkWidget  *window )
{
    GtkItemFactory *item_factory;
    GtkAccelGroup *accel_group;

    /* Make an accelerator group (shortcut keys) */
    accel_group = gtk_accel_group_new ();

    /* Make an ItemFactory (that makes a menubar) */
    item_factory = gtk_item_factory_new (GTK_TYPE_MENU_BAR, "<main>",
                                         accel_group);

    /* This function generates the menu items. Pass the item factory,
       the number of items in the array, the array itself, and any
       callback data for the the menu items. */
    gtk_item_factory_create_items (item_factory, nmenu_items, menu_items_fi, NULL);

    /* Attach the new accelerator group to the window. */
    gtk_window_add_accel_group (GTK_WINDOW (window), accel_group);

    /* Finally, return the actual menu bar created by the item factory. */
    return gtk_item_factory_get_widget (item_factory, "<main>");
}

/* Popup the menu when the popup button is pressed */
static gboolean popup_cb( GtkWidget *widget,
                          GdkEvent *event,
                          GtkWidget *menu )
{
    GdkEventButton *bevent = (GdkEventButton *)event;
  
    /* Only take button presses */
    if (event->type != GDK_BUTTON_PRESS)
        return FALSE;
  
    /* Show the menu */
    gtk_menu_popup (GTK_MENU(menu), NULL, NULL,
                    NULL, NULL, bevent->button, bevent->time);
  
    return TRUE;
}

/* Same as with get_menubar_menu() but just return a button with a signal to
   call a popup menu */
GtkWidget *get_popup_menu( void )
{
    GtkItemFactory *item_factory;
    GtkWidget *button, *menu;
  
    /* Same as before but don't bother with the accelerators */
    item_factory = gtk_item_factory_new (GTK_TYPE_MENU, "<main>",
                                         NULL);
    gtk_item_factory_create_items (item_factory, nmenu_items, menu_items_en, NULL);
    menu = gtk_item_factory_get_widget (item_factory, "<main>");
  
    /* Make a button to activate the popup menu */
    button = gtk_button_new_with_label ("Popup");
    /* Make the menu popup when clicked */
    g_signal_connect (G_OBJECT(button),
                      "event",
                      G_CALLBACK(popup_cb),
                      (gpointer) menu);

    return button;
}

/* Same again but return an option menu */
GtkWidget *get_option_menu( void )
{
    GtkItemFactory *item_factory;
    GtkWidget *option_menu;
  
    /* Same again, not bothering with the accelerators */
    item_factory = gtk_item_factory_new (GTK_TYPE_OPTION_MENU, "<main>",
                                         NULL);
    gtk_item_factory_create_items (item_factory, nmenu_items, menu_items_en, NULL);
    option_menu = gtk_item_factory_get_widget (item_factory, "<main>");

    return option_menu;
}
#endif

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN 1
#include "windows.h"
#include "shellapi.h"
#endif /* WIN32 */

void start_help(GtkWidget *w, gpointer data)
{
    gchar *docfile = g_build_filename(installation_dir, "doc", 
                                      data ? "judotimer.pdf" : "judoshiai.pdf", NULL);
#ifdef WIN32
    ShellExecute(NULL, TEXT("open"), docfile, NULL, ".\\", SW_SHOWMAXIMIZED);
#else /* ! WIN32 */
    gchar *cmd;
    cmd = g_strdup_printf("if which acroread ; then PDFR=acroread ; "
                          "elif which evince ; then PDFR=evince ; "
                          "else PDFR=xpdf ; fi ; $PDFR \"%s\" &", docfile);
    g_free(cmd);
#endif /* ! WIN32 */
    g_free(docfile);
}

