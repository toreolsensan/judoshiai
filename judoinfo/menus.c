/* -*- mode: C; c-basic-offset: 4;  -*- */

/*
 * Copyright (C) 2006-2011 by Hannu Jokinen
 * Full copyright text is included in the software package.
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "judoinfo.h"

void start_help(GtkWidget *w, gpointer data);


static GtkWidget *menubar, *preferences, *help, *preferencesmenu, *helpmenu;
static GtkWidget *quit, *manual;
static GtkWidget *full_screen, *normal_display, *small_display, *horizontal_display;
static GtkWidget *mirror, *whitefirst, *redbackground;
static GtkWidget *tatami_show[NUM_TATAMIS];
static GtkWidget *node_ip, *my_ip, *about;
static GtkWidget *light, *menu_light;
static GtkWidget *writefile;

gboolean show_tatami[NUM_TATAMIS];
static GtkTooltips *menu_tips;

static GtkWidget *flags[NUM_LANGS], *menu_flags[NUM_LANGS];

static const gchar *flags_files[NUM_LANGS] = {
    "finland.png", "sweden.png", "uk.png", "spain.png", "estonia.png", "ukraine.png"
};
static const gchar *lang_names[NUM_LANGS] = {
    "fi", "sv", "en", "es", "et", "uk"
};

static const gchar *help_file_names[NUM_LANGS] = {
    "judoshiai-fi.pdf", "judoshiai-en.pdf", "judoshiai-en.pdf", "judoshiai-es.pdf", "judoshiai-en.pdf",
    "judoshiai-uk.pdf"
};


extern void toggle_full_screen(GtkWidget *menu_item, gpointer data);
extern void toggle_small_display(GtkWidget *menu_item, gpointer data);
extern void toggle_mirror(GtkWidget *menu_item, gpointer data);
extern void toggle_whitefirst(GtkWidget *menu_item, gpointer data);
extern void toggle_redbackground(GtkWidget *menu_item, gpointer data);
extern void set_write_file(GtkWidget *menu_item, gpointer data);

static void about_judoinfo( GtkWidget *w,
			    gpointer   data )
{
    gtk_show_about_dialog (NULL, 
                           "name", "JudoInfo",
                           "title", _("About JudoInfo"),
                           "copyright", "Copyright 2006-2011 Hannu Jokinen",
                           "version", SHIAI_VERSION,
                           "website", "http://sourceforge.net/projects/judoshiai/",
                           NULL);
}

static void tatami_selection(GtkWidget *w,
                             gpointer   data )
{
    gchar buf[32];
    gint tatami = (gint)data;
    sprintf(buf, "tatami%d", tatami);
    show_tatami[tatami-1] = GTK_CHECK_MENU_ITEM(w)->active;
    g_key_file_set_boolean(keyfile, "preferences", buf, 
                           GTK_CHECK_MENU_ITEM(w)->active);

    refresh_window();
}

gint number_of_tatamis(void)
{
    gint i, n = 0;
    for (i = 0; i < NUM_TATAMIS; i++)
        if (show_tatami[i])
            n++;

    return n;
}

static void change_menu_label(GtkWidget *item, const gchar *new_text)
{
    GtkWidget *menu_label = gtk_bin_get_child(GTK_BIN(item));
    gtk_label_set_text(GTK_LABEL(menu_label), new_text); 
}

static GtkWidget *get_picture(const gchar *name)
{
    gchar *file = g_build_filename(installation_dir, "etc", name, NULL);
    GtkWidget *pic = gtk_image_new_from_file(file);
    g_free(file);
    return pic;
}

static void set_menu_item_picture(GtkImageMenuItem *menu_item, gchar *name)
{
    GtkImage *image = GTK_IMAGE(gtk_image_menu_item_get_image(GTK_IMAGE_MENU_ITEM(menu_item)));
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

    if (connection_ok) {
        struct message msg;
        extern gint my_address;

        msg.type = MSG_ALL_REQ;
        msg.sender = my_address;
        send_packet(&msg);

        set_menu_item_picture(GTK_IMAGE_MENU_ITEM(menu_light), "greenlight.png");
    } else		
        set_menu_item_picture(GTK_IMAGE_MENU_ITEM(menu_light), "redlight.png");

    return TRUE;
}

static GtkWidget *create_menu_item(GtkWidget *menu, void *cb, gint param)
{
    GtkWidget *w = gtk_menu_item_new_with_label("");
    gtk_menu_shell_append (GTK_MENU_SHELL(menu), w);
    g_signal_connect(G_OBJECT(w), "activate", G_CALLBACK(cb), (gpointer)param);
    return w;
}

static void create_separator(GtkWidget *menu)
{
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
}

/* Returns a menubar widget made from the above menu */
GtkWidget *get_menubar_menu(GtkWidget  *window)
{
    GtkAccelGroup *group;
    gint i;

    menu_tips = gtk_tooltips_new ();
    group = gtk_accel_group_new ();
    menubar = gtk_menu_bar_new ();

    preferences = gtk_menu_item_new_with_label (_("Preferences"));
    help        = gtk_menu_item_new_with_label (_("Help"));

    for (i = 0; i < NUM_LANGS; i++) {
        flags[i] = get_picture(flags_files[i]);
        menu_flags[i] = gtk_image_menu_item_new();
        gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_flags[i]), flags[i]);        
        gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM(menu_flags[i]), TRUE);
    }

    light      = get_picture("redlight.png");
    menu_light = gtk_image_menu_item_new();
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_light), light);
    gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM(menu_light), TRUE);

    preferencesmenu  = gtk_menu_new ();
    helpmenu         = gtk_menu_new ();
        
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (preferences), preferencesmenu); 
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (help), helpmenu);
  
    gtk_menu_shell_append (GTK_MENU_SHELL (menubar), preferences); 
    gtk_menu_shell_append (GTK_MENU_SHELL (menubar), help);

    for (i = 0; i < NUM_LANGS; i++) {
        gtk_menu_shell_append(GTK_MENU_SHELL(menubar), menu_flags[i]); 
        g_signal_connect(G_OBJECT(menu_flags[i]), "button_press_event",
                         G_CALLBACK(change_language), (gpointer)i);
    }

    gtk_menu_shell_append (GTK_MENU_SHELL (menubar), menu_light); 
    gtk_menu_item_set_right_justified(GTK_MENU_ITEM(menu_light), TRUE);
    g_signal_connect(G_OBJECT(menu_light), "button_press_event",
                     G_CALLBACK(ask_node_ip_address), (gpointer)NULL);

  
    /* Create the Preferences menu content. */
    full_screen = gtk_check_menu_item_new_with_label("");
    gtk_menu_shell_append(GTK_MENU_SHELL(preferencesmenu), full_screen);
    g_signal_connect(G_OBJECT(full_screen), "activate", 
                     G_CALLBACK(toggle_full_screen), 0);

    normal_display     = gtk_radio_menu_item_new_with_label(NULL, "");
    small_display      = gtk_radio_menu_item_new_with_label_from_widget(GTK_RADIO_MENU_ITEM(normal_display), "");
    horizontal_display = gtk_radio_menu_item_new_with_label_from_widget(GTK_RADIO_MENU_ITEM(normal_display), "");
    create_separator(preferencesmenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(preferencesmenu), normal_display);
    gtk_menu_shell_append(GTK_MENU_SHELL(preferencesmenu), small_display);
    gtk_menu_shell_append(GTK_MENU_SHELL(preferencesmenu), horizontal_display);
    create_separator(preferencesmenu);

    g_signal_connect(G_OBJECT(normal_display), "activate", 
                     G_CALLBACK(toggle_small_display), (gpointer)NORMAL_DISPLAY);
    g_signal_connect(G_OBJECT(small_display), "activate", 
                     G_CALLBACK(toggle_small_display), (gpointer)SMALL_DISPLAY);
    g_signal_connect(G_OBJECT(horizontal_display), "activate", 
                     G_CALLBACK(toggle_small_display), (gpointer)HORIZONTAL_DISPLAY);

    mirror = gtk_check_menu_item_new_with_label("");
    gtk_menu_shell_append(GTK_MENU_SHELL(preferencesmenu), mirror);
    g_signal_connect(G_OBJECT(mirror), "activate", 
                     G_CALLBACK(toggle_mirror), 0);

    whitefirst = gtk_check_menu_item_new_with_label("");
    gtk_menu_shell_append(GTK_MENU_SHELL(preferencesmenu), whitefirst);
    g_signal_connect(G_OBJECT(whitefirst), "activate", 
                     G_CALLBACK(toggle_whitefirst), 0);

    redbackground = gtk_check_menu_item_new_with_label("");
    gtk_menu_shell_append(GTK_MENU_SHELL(preferencesmenu), redbackground);
    g_signal_connect(G_OBJECT(redbackground), "activate", 
                     G_CALLBACK(toggle_redbackground), 0);

    node_ip = create_menu_item(preferencesmenu, ask_node_ip_address, 0);
    my_ip   = create_menu_item(preferencesmenu, show_my_ip_addresses, 0);
    create_separator(preferencesmenu);

    for (i = 0; i < NUM_TATAMIS; i++) {
        tatami_show[i] = gtk_check_menu_item_new_with_label("");
        gtk_menu_shell_append(GTK_MENU_SHELL(preferencesmenu), tatami_show[i]);
        g_signal_connect(G_OBJECT(tatami_show[i]), "activate", 
                         G_CALLBACK(tatami_selection), (gpointer)(i+1));
    }

    create_separator(preferencesmenu);
    writefile = gtk_menu_item_new_with_label("");
    gtk_menu_shell_append(GTK_MENU_SHELL(preferencesmenu), writefile);
    g_signal_connect(G_OBJECT(writefile), "activate", 
                     G_CALLBACK(set_write_file), 0);

    create_separator(preferencesmenu);
    quit    = create_menu_item(preferencesmenu, destroy, 0);

    /* Create the Help menu content. */
    manual = create_menu_item(helpmenu, start_help, 0);
    about  = create_menu_item(helpmenu, about_judoinfo, 0);

    /* Attach the new accelerator group to the window. */
    gtk_widget_add_accelerator(quit, "activate", group, GDK_Q, GDK_CONTROL_MASK, 
                               GTK_ACCEL_VISIBLE);
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
    gboolean b;

    error = NULL;
    if (g_key_file_get_boolean(keyfile, "preferences", "fullscreen", &error)) {
        gtk_menu_item_activate(GTK_MENU_ITEM(full_screen));
    }

    error = NULL;
    i = g_key_file_get_integer(keyfile, "preferences", "displaytype", &error);
    if (!error) {
        switch (i) {
        case NORMAL_DISPLAY:
            gtk_menu_item_activate(GTK_MENU_ITEM(normal_display));
            break;
        case SMALL_DISPLAY:
            gtk_menu_item_activate(GTK_MENU_ITEM(small_display));
            break;
        case HORIZONTAL_DISPLAY:
            gtk_menu_item_activate(GTK_MENU_ITEM(horizontal_display));
            break;
        }
    }

    error = NULL;
    if (g_key_file_get_boolean(keyfile, "preferences", "mirror", &error)) {
        gtk_menu_item_activate(GTK_MENU_ITEM(mirror));
    }

    error = NULL;
    if (g_key_file_get_boolean(keyfile, "preferences", "whitefirst", &error)) {
        gtk_menu_item_activate(GTK_MENU_ITEM(whitefirst));
    }

    error = NULL;
    if (g_key_file_get_boolean(keyfile, "preferences", "redbackground", &error)) {
        gtk_menu_item_activate(GTK_MENU_ITEM(redbackground));
    }

    for (i = 1; i <= NUM_TATAMIS; i++) {
        gchar t[10];
        sprintf(t, "tatami%d", i);
        error = NULL;
        b = g_key_file_get_boolean(keyfile, "preferences", t, &error);
        if (b && !error) {
            gtk_menu_item_activate(GTK_MENU_ITEM(tatami_show[i-1]));
            tatami_selection(tatami_show[i-1], (gpointer)i);
        }
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
    gint i;
    gchar *r = NULL;
    static gchar envbuf[32]; // this must be static for the putenv() function

    language = (gint)param;
    sprintf(envbuf, "LANGUAGE=%s", lang_names[language]);
    putenv(envbuf);
    r = setlocale(LC_ALL, lang_names[language]);

    gchar *dirname = g_build_filename(installation_dir, "share", "locale", NULL);
    bindtextdomain ("judoshiai", dirname);
    g_free(dirname);
    bind_textdomain_codeset ("judoshiai", "UTF-8");
    textdomain ("judoshiai");

    change_menu_label(preferences,  _("Preferences"));
    change_menu_label(help,         _("Help"));

    change_menu_label(quit,         _("Quit"));

    change_menu_label(full_screen, _("Full screen mode"));
    change_menu_label(normal_display, _("Normal display"));
    change_menu_label(small_display, _("Small display"));
    change_menu_label(horizontal_display, _("Horizontal display"));
    change_menu_label(mirror, _("Mirror tatami order"));
    change_menu_label(whitefirst, _("White first"));
    change_menu_label(redbackground, _("Red background"));
    change_menu_label(writefile, _("Write to file"));

    for (i = 0; i < NUM_TATAMIS; i++) {
        gchar buf[32];
        sprintf(buf, "%s %d", _("Show Tatami"), i+1);
        change_menu_label(tatami_show[i], buf);
    }

    change_menu_label(node_ip,      _("Communication node"));
    change_menu_label(my_ip,        _("Own IP addresses"));

    change_menu_label(manual,       _("Manual"));
    change_menu_label(about,        _("About"));

    /* tooltips */
    gtk_tooltips_set_tip (GTK_TOOLTIPS (menu_tips), menu_flags[LANG_FI],
                          _("Change language to Finnish"), NULL);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (menu_tips), menu_flags[LANG_SW],
                          _("Change language to Swedish"), NULL);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (menu_tips), menu_flags[LANG_EN],
                          _("Change language to English"), NULL);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (menu_tips), menu_flags[LANG_ES],
                          _("Change language to Spanish"), NULL);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (menu_tips), menu_flags[LANG_EE],
                          _("Change language to Estonian"), NULL);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (menu_tips), menu_flags[LANG_UK],
                          _("Change language to Ukrainan"), NULL);

    g_key_file_set_integer(keyfile, "preferences", "language", language);

    return TRUE;
}


#ifdef WIN32
#define WIN32_LEAN_AND_MEAN 1
#include "windows.h"
#include "shellapi.h"
#endif /* WIN32 */

void start_help(GtkWidget *w, gpointer data)
{
    gchar *docfile = g_build_filename(installation_dir, "doc", 
                                      help_file_names[language], NULL);
#ifdef WIN32
    ShellExecute(NULL, TEXT("open"), docfile, NULL, ".\\", SW_SHOWMAXIMIZED);
#else /* ! WIN32 */
    gchar *cmd;
    cmd = g_strdup_printf("if which acroread ; then PDFR=acroread ; "
                          "elif which evince ; then PDFR=evince ; "
                          "else PDFR=xpdf ; fi ; $PDFR \"%s\" &", docfile);
    system(cmd);
    g_free(cmd);
#endif /* ! WIN32 */
    g_free(docfile);
}

