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
#include <glib.h>
#include <glib/gstdio.h>

#include "sqlite3.h"
#include "judoshiai.h"

extern gboolean mirror_display;
extern gboolean auto_arrange;

void get_from_old_competition(GtkWidget *w, gpointer data);
void get_from_old_competition_with_weight(GtkWidget *w, gpointer data);
void start_help(GtkWidget *w, gpointer data);

static gchar *backup_directory = NULL;

void about_shiai( GtkWidget *w, gpointer data)
{
    gtk_show_about_dialog (NULL, 
                           "name", "JudoShiai",
                           "title", "About JudoShiai",
                           "copyright", "Copyright 2006-2010 Hannu Jokinen",
                           "version", SHIAI_VERSION,
                           "website", "http://sourceforge.net/projects/judoshiai/",
                           NULL);
}

void new_shiai(GtkWidget *w, gpointer data)
{
    GtkWidget *cont_name = gtk_entry_new();
    GtkWidget *cont_place = gtk_entry_new();
    GtkWidget *cont_date = gtk_entry_new();
    GtkWidget *cont_time = gtk_entry_new();
    GtkWidget *cont_num_tatamis = gtk_entry_new();
    GtkWidget *dialog;
    GtkWidget *table = gtk_table_new(2, 5, FALSE);

    dialog = gtk_file_chooser_dialog_new (_("Tournament"),
                                          NULL,
                                          GTK_FILE_CHOOSER_ACTION_SAVE,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                          NULL);

    gtk_table_attach_defaults(GTK_TABLE(table), gtk_label_new(_("Tournament:")), 0, 1, 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(table), gtk_label_new(_("Date:")), 0, 1, 1, 2);
    gtk_table_attach_defaults(GTK_TABLE(table), gtk_label_new(_("Place:")), 0, 1, 2, 3);
    gtk_table_attach_defaults(GTK_TABLE(table), gtk_label_new(_("Start time:")), 0, 1, 3, 4);
    gtk_table_attach_defaults(GTK_TABLE(table), gtk_label_new(_("Number of tatamis")), 0, 1, 4, 5);

    gtk_entry_set_text(GTK_ENTRY(cont_num_tatamis), "3");

    gtk_table_attach_defaults(GTK_TABLE(table), cont_name, 1, 2, 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(table), cont_date, 1, 2, 1, 2);
    gtk_table_attach_defaults(GTK_TABLE(table), cont_place, 1, 2, 2, 3);
    gtk_table_attach_defaults(GTK_TABLE(table), cont_time, 1, 2, 3, 4);
    gtk_table_attach_defaults(GTK_TABLE(table), cont_num_tatamis, 1, 2, 4, 5);

    gtk_widget_show_all(table);
    gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(dialog), table);

    if (database_name[0] == 0) {
        if (current_directory[0] != '.')
            gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), current_directory);
        else
            gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), g_get_home_dir());
    } else {
        gchar *dirname = g_path_get_dirname(database_name);
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), dirname);
        g_free(dirname);
    }

    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
        gchar *name;
                
        name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        if (strstr(name, ".shi") == NULL) {
            sprintf(database_name, "%s.shi", name);
        } else {
            sprintf(database_name, "%s", name);
        }
        snprintf(logfile_name, sizeof(logfile_name), "%s", database_name);
        gchar *p = strstr(logfile_name, ".shi");
        if (p)
            strcpy(p, ".log");
        else
            logfile_name[0] = 0;

        g_free (name);

        valid_ascii_string(database_name);

        db_new(database_name, 
               gtk_entry_get_text(GTK_ENTRY(cont_name)),
               gtk_entry_get_text(GTK_ENTRY(cont_date)),
               gtk_entry_get_text(GTK_ENTRY(cont_place)),
               gtk_entry_get_text(GTK_ENTRY(cont_time)),
               gtk_entry_get_text(GTK_ENTRY(cont_num_tatamis)));

        open_shiai_display();
    }
        
    gtk_widget_destroy (dialog);
}

void open_shiai_from_net(GtkWidget *w, gpointer data)
{
    GtkWidget *places[NUM_OTHERS];
    GtkWidget *dialog;
    GtkWidget *table = gtk_table_new(1, NUM_OTHERS, FALSE);
    gint i;

    dialog = gtk_file_chooser_dialog_new (_("Tournament from the net"),
                                          NULL,
                                          GTK_FILE_CHOOSER_ACTION_SAVE,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                          NULL);

    for (i = 0; i < NUM_OTHERS; i++) {
        places[i] = gtk_check_button_new_with_label(other_info(i));
        gtk_table_attach_defaults(GTK_TABLE(table), places[i], 0, 1, i, i+1);
    }

    gtk_widget_show_all(table);
    gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(dialog), table);

    if (database_name[0] == 0) {
        if (current_directory[0] != '.')
            gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), current_directory);
        else
            gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), g_get_home_dir());
    } else {
        gchar *dirname = g_path_get_dirname(database_name);
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), dirname);
        g_free(dirname);
    }

    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
        gchar *name;

        for (i = 0; i < NUM_OTHERS; i++) {
            if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(places[i])))
                break;
        }
                 
        if (i >= NUM_OTHERS)
            goto out;

        name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        if (strstr(name, ".shi") == NULL) {
            sprintf(database_name, "%s.shi", name);
        } else {
            sprintf(database_name, "%s", name);
        }
        snprintf(logfile_name, sizeof(logfile_name), "%s", database_name);
        gchar *p = strstr(logfile_name, ".shi");
        if (p)
            strcpy(p, ".log");
        else
            logfile_name[0] = 0;

        g_free (name);

        valid_ascii_string(database_name);

        if (read_file_from_net(database_name, i))
            goto out;

        open_shiai_display();
    }
out:
    gtk_widget_destroy (dialog);        
}

void open_shiai(GtkWidget *w, gpointer data)
{
    GtkWidget *dialog;
    GtkFileFilter *filter = gtk_file_filter_new();

    gtk_file_filter_add_pattern(filter, "*.shi");
    gtk_file_filter_set_name(filter, _("Contests"));

    dialog = gtk_file_chooser_dialog_new (_("Contest"),
                                          NULL,
                                          GTK_FILE_CHOOSER_ACTION_OPEN,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                          NULL);

    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    if (database_name[0] == 0) {
        if (current_directory[0] != '.')
            gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), current_directory);
        else
            gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), g_get_home_dir());
    } else {
        gchar *dirname = g_path_get_dirname(database_name);
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), dirname);
        g_free(dirname);
    }

    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
        gchar *name;

        name = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        sprintf(database_name, "%s", name);
        snprintf(logfile_name, sizeof(logfile_name), "%s", database_name);
        gchar *p = strstr(logfile_name, ".shi");
        if (p)
            strcpy(p, ".log");
        else
            logfile_name[0] = 0;

        g_free (name);

        gdk_window_set_cursor(GTK_WIDGET(dialog)->window, wait_cursor);
        open_shiai_display();
        gdk_window_set_cursor(GTK_WIDGET(dialog)->window, NULL);

        valid_ascii_string(database_name);
    }

    gtk_widget_destroy (dialog);
}

void font_dialog(GtkWidget *w, gpointer data)
{
    GtkWidget *dialog;

    dialog = gtk_font_selection_dialog_new (_("Select sheet font"));
    gtk_font_selection_dialog_set_font_name(GTK_FONT_SELECTION_DIALOG(dialog), get_font_face());

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        gchar *font = gtk_font_selection_dialog_get_font_name(GTK_FONT_SELECTION_DIALOG(dialog));
        set_font(font);
        g_print("font=%s\n", font);
    }

    gtk_widget_destroy(dialog);
}

void select_use_logo(GtkWidget *w, gpointer data)
{
    GtkWidget *dialog, *phdr;
    GtkFileFilter *filter;

    dialog = gtk_file_chooser_dialog_new(_("Choose a file"),
                                         NULL,
                                         GTK_FILE_CHOOSER_ACTION_OPEN,
                                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                         "No Logo", 1000,
                                         GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                         NULL);

    filter = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filter, "*.[pP][nN][gG]");
    gtk_file_filter_set_name(filter, _("Picture Files"));
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    phdr = gtk_check_button_new_with_label(_("Print Headers"));
    gtk_widget_show(phdr);
    gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(dialog), phdr);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(phdr), print_headers);

    gint r = gtk_dialog_run(GTK_DIALOG(dialog));

    if (r == 1000) { // no logo
        g_free(use_logo);
        use_logo = NULL;
        g_key_file_set_string(keyfile, "preferences", "logofile", "");
        gtk_widget_destroy(dialog);
        return;
    }

    if (r != GTK_RESPONSE_ACCEPT) {
        gtk_widget_destroy(dialog);
        return;
    }
               
    g_free(use_logo);
    use_logo = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
    print_headers = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(phdr));

    g_key_file_set_string(keyfile, "preferences", "logofile", use_logo);
    g_key_file_set_boolean(keyfile, "preferences", "printheaders", print_headers);

    gtk_widget_destroy(dialog);
}

void set_lang(GtkWidget *w, gpointer data)
{
    print_lang = (gint)data;
    g_key_file_set_integer(keyfile, "preferences", "printlanguage", print_lang);
}

void set_club_text(GtkWidget *w, gpointer data)
{
    club_text = (gint)data;
    g_key_file_set_integer(keyfile, "preferences", "clubtext", club_text);
}

void set_club_abbr(GtkWidget *menu_item, gpointer data)
{
    club_abbr = GTK_CHECK_MENU_ITEM(menu_item)->active;
    g_key_file_set_integer(keyfile, "preferences", "clubabbr", club_abbr);
}

void set_draw_system(GtkWidget *w, gpointer data)
{
    draw_system = (gint)data;
    g_key_file_set_integer(keyfile, "preferences", "drawsystem", draw_system);
    refresh_window();
}

void toggle_automatic_sheet_update(GtkWidget *menu_item, gpointer data)
{
    automatic_sheet_update = GTK_CHECK_MENU_ITEM(menu_item)->active;
    g_key_file_set_boolean(keyfile, "preferences", "automatic_sheet_update", automatic_sheet_update);
}

void toggle_automatic_web_page_update(GtkWidget *menu_item, gpointer data)
{
    automatic_web_page_update = GTK_CHECK_MENU_ITEM(menu_item)->active;
    if (automatic_web_page_update)
        get_output_directory();
}

void toggle_weights_in_sheets(GtkWidget *menu_item, gpointer data)
{
    weights_in_sheets = GTK_CHECK_MENU_ITEM(menu_item)->active;
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
    refresh_window();
}

void toggle_auto_arrange(GtkWidget *menu_item, gpointer data)
{
    if (GTK_CHECK_MENU_ITEM(menu_item)->active) {
        auto_arrange = TRUE;
        g_key_file_set_boolean(keyfile, "preferences", "autoarrange", TRUE);
    } else {
        auto_arrange = FALSE;
        g_key_file_set_boolean(keyfile, "preferences", "autoarrange", FALSE);
    }
}

void properties(GtkWidget *w, gpointer data)
{
    GtkWidget *dialog;
    GtkWidget *cont_name = gtk_entry_new();
    GtkWidget *cont_place = gtk_entry_new();
    GtkWidget *cont_date = gtk_entry_new();
    GtkWidget *cont_time = gtk_entry_new();
    GtkWidget *cont_num_tatamis = gtk_entry_new();
    GtkWidget *table = gtk_table_new(2, 7, FALSE);
    gint       num_comp, num_weighted;
    gchar      buf[8];

    db_read_competitor_statistics(&num_comp, &num_weighted);

    dialog = gtk_dialog_new_with_buttons (_("Tournament properties"),
                                          NULL,
                                          GTK_DIALOG_DESTROY_WITH_PARENT,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
                                          NULL);

    gtk_table_attach_defaults(GTK_TABLE(table), gtk_label_new(_("Tournament:")), 0, 1, 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(table), gtk_label_new(_("Date:")), 0, 1, 1, 2);
    gtk_table_attach_defaults(GTK_TABLE(table), gtk_label_new(_("Place:")), 0, 1, 2, 3);
    gtk_table_attach_defaults(GTK_TABLE(table), gtk_label_new(_("Start time:")), 0, 1, 3, 4);
    gtk_table_attach_defaults(GTK_TABLE(table), gtk_label_new(_("Number of tatamis:")), 0, 1, 4, 5);
    gtk_table_attach_defaults(GTK_TABLE(table), gtk_label_new(_("Competitors:")), 0, 1, 5, 6);
    gtk_table_attach_defaults(GTK_TABLE(table), gtk_label_new(_("Weighted:")), 0, 1, 6, 7);

    gtk_table_attach_defaults(GTK_TABLE(table), cont_name, 1, 2, 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(table), cont_date, 1, 2, 1, 2);
    gtk_table_attach_defaults(GTK_TABLE(table), cont_place, 1, 2, 2, 3);
    gtk_table_attach_defaults(GTK_TABLE(table), cont_time, 1, 2, 3, 4);
    gtk_table_attach_defaults(GTK_TABLE(table), cont_num_tatamis, 1, 2, 4, 5);
    sprintf(buf, "%d", num_comp);
    gtk_table_attach_defaults(GTK_TABLE(table), gtk_label_new(buf), 1, 2, 5, 6);
    sprintf(buf, "%d", num_weighted);
    gtk_table_attach_defaults(GTK_TABLE(table), gtk_label_new(buf), 1, 2, 6, 7);

    gtk_widget_show_all(table);
    gtk_container_add(GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), table);

    gtk_entry_set_text(GTK_ENTRY(cont_name), info_competition);
    gtk_entry_set_text(GTK_ENTRY(cont_date), info_date);
    gtk_entry_set_text(GTK_ENTRY(cont_place), info_place);
    gtk_entry_set_text(GTK_ENTRY(cont_time), info_time);
    gtk_entry_set_text(GTK_ENTRY(cont_num_tatamis), info_num_tatamis);

    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
        db_set_info(gtk_entry_get_text(GTK_ENTRY(cont_name)),
                    gtk_entry_get_text(GTK_ENTRY(cont_date)),
                    gtk_entry_get_text(GTK_ENTRY(cont_place)),
                    gtk_entry_get_text(GTK_ENTRY(cont_time)),
                    gtk_entry_get_text(GTK_ENTRY(cont_num_tatamis)));
    }
        
    gtk_widget_destroy (dialog);        
}

static gboolean with_weight;

static void get_competitors(void)
{
    GtkWidget *dialog;
    GtkWidget *cleanup;
    GtkFileFilter *filter = gtk_file_filter_new();
    gint added = 0, not_added = 0;

    gtk_file_filter_add_pattern(filter, "*.shi");
    gtk_file_filter_set_name(filter, _("Tournaments"));

    dialog = gtk_file_chooser_dialog_new (_("Add competitors"),
                                          NULL,
                                          GTK_FILE_CHOOSER_ACTION_OPEN,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                          NULL);

    cleanup = gtk_check_button_new_with_label(_("Clean up duplicates and update reg. categories"));
    gtk_widget_show(cleanup);
    gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(dialog), cleanup);

    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    if (database_name[0] == 0) {
        if (current_directory[0] != '.')
            gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), current_directory);
        else
            gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), g_get_home_dir());
    } else {
        gchar *dirname = g_path_get_dirname(database_name);
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), dirname);
        g_free(dirname);
    }

    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
        gchar *name;

        cleanup_import = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cleanup));

        name = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        db_add_competitors(name, with_weight, &added, &not_added);
        valid_ascii_string(name);
        g_free (name);

        if (cleanup_import)
            SHOW_MESSAGE("%d %s (%d %s).", 
                         added, _("competitors added and updated"), not_added, _("competitors already existed"));
        else
            SHOW_MESSAGE("%d %s.", added, _("competitors added unchanged"));
    }

    gtk_widget_destroy (dialog);        

}


void get_from_old_competition(GtkWidget *w, gpointer data)
{
    with_weight = FALSE;
    get_competitors();
}

void get_from_old_competition_with_weight(GtkWidget *w, gpointer data)
{
    with_weight = TRUE;
    get_competitors();
}

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN 1
#include "windows.h"
#include "shellapi.h"
#endif /* WIN32 */

#if 0
#ifndef WIN32

static int run_command( const gchar *szCmd ) {

    if ( system( szCmd ) < 0 ) {
        perror("Error launching browser");
        return -1;
    }

    return 0;
}
#endif

static void open_url(const char *szURL) {

#ifdef WIN32

    ShellExecute( NULL, TEXT("open"), szURL, NULL, ".\\", SW_SHOWMAXIMIZED );

#else /* ! WIN32 */

    gchar *pchBrowser;
    gchar *pchTemp;
    const gchar *pch;
    int rc;

    if ( ( pch = g_getenv( "BROWSER" ) ) )
        pchBrowser = g_strdup( pch );
    else {
#ifdef __APPLE__
        pchBrowser = g_strdup( "open %s" );
#else
        pchBrowser = g_strdup( "mozilla \"%s\" &");
#endif
    }

    pchTemp = g_strdup_printf(pchBrowser, szURL);
    rc = run_command(pchTemp);
    g_free(pchTemp);
    g_free(pchBrowser);

#endif /* ! WIN32 */
}
#endif

static const gchar *help_file_names[NUM_LANGS] = {
    "judoshiai-fi.pdf", "judoshiai-en.pdf", "judoshiai-en.pdf", "judoshiai-es.pdf"
};

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

void backup_shiai(GtkWidget *w, gpointer data)
{
    GtkWidget *dialog, *do_backup;

    dialog = gtk_file_chooser_dialog_new(_("Choose a directory"),
                                         NULL,
                                         GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                         GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                         NULL);

    do_backup = gtk_check_button_new_with_label(_("Do Backup"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(do_backup), TRUE);
    gtk_widget_show(do_backup);
    gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(dialog), do_backup);

    if (backup_directory)
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),
                                            backup_directory);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_ACCEPT) {
        gtk_widget_destroy(dialog);
        return;
    }
                
    g_free(backup_directory);
    backup_directory = NULL;

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(do_backup))) {
        backup_directory = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        make_backup();
    } else
        show_note(" ");

    valid_ascii_string(backup_directory);

    gtk_widget_destroy (dialog);
}

#define NUM_BACKUPS 16

void make_backup(void)
{
    static gchar buf[1024];
    static gchar *filenames[NUM_BACKUPS];
    static gint ix = 0;

    if (!backup_directory)
        return;

    time_t t = time(NULL);

    struct tm *tm = localtime((time_t *)&t);
    sprintf(buf, "shiai_%04d%02d%02d_%02d%02d%02d.shi",
            tm->tm_year+1900, 
            tm->tm_mon+1,
            tm->tm_mday,
            tm->tm_hour,
            tm->tm_min,
            tm->tm_sec);

    filenames[ix] = g_build_filename(backup_directory, buf, NULL);

    FILE *f = fopen(filenames[ix], "wb");
    if (f) {
        gint n;
        FILE *db = fopen(database_name, "rb");
        if (!db) {
            fclose(f);
            return;
        }

        while ((n = fread(buf, 1, sizeof(buf), db)) > 0) {
            fwrite(buf, 1, n, f);
        }

        fclose(db);
        fclose(f);
    } else {
        show_note("%s %s", _("CANNOT OPEN BACKUP FILE"), filenames[ix]);		
    }

    if (++ix >= NUM_BACKUPS)
        ix = 0;

    if (filenames[ix]) {
        g_unlink(filenames[ix]);
        g_free(filenames[ix]);
        filenames[ix] = NULL;
    }
}
