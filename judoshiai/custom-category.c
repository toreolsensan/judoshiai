/* -*- mode: C; c-basic-offset: 4;  -*- */

/*
 * Copyright (C) 2006-2013 by Hannu Jokinen
 * Full copyright text is included in the software package.
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <math.h>

#include <cairo.h>
#include <cairo-pdf.h>
#include <librsvg/rsvg.h>

#include "sqlite3.h"
#include "judoshiai.h"

#define NUM_CUSTOM_BRACKETS 32
static gchar *custom_directory = NULL;
static struct custom_data *custom_brackets[NUM_CUSTOM_BRACKETS];
static gint num_custom_brackets;

struct custom_data *get_custom_table(gint table)
{
    if (table >= 0 && table < num_custom_brackets)
        return custom_brackets[table];
    return NULL;
}

gint get_custom_table_number_by_competitors(gint num_comp)
{
    gint i;
    for (i = 0; i < num_custom_brackets; i++)
        if (custom_brackets[i]->competitors_min <= num_comp &&
            custom_brackets[i]->competitors_max >= num_comp)
            return i;
    return -1;
}

void read_custom_files(void)
{
    gint i, n = 0;
    gchar *fullname;

    if (custom_directory == NULL)
        return;

    void *d = g_malloc0(sizeof(struct custom_data));
    GDir *dir = g_dir_open(custom_directory, 0, NULL);

    if (dir) {
        db_create_blob_table();
        const gchar *fname = g_dir_read_name(dir);
        while (fname) {
            char *p = strstr(fname, ".txt");
            fullname = g_build_filename(custom_directory, fname, NULL);
            if (p && p[4] == 0) {
                if (read_custom_category(fullname, d))
                    continue;

                db_write_blob(n << 8, d, sizeof(struct custom_data));

                gchar buf[64];
                snprintf(buf, sizeof(buf) - 8, "%s", fname);
                p = strstr(buf, ".txt");
                if (p) *p = 0;
                else p = buf + strlen(buf);
                i = 1;
                while (1) {
                    sprintf(p, "-%d.svg", i);
                    gchar *svgname = g_build_filename(custom_directory, buf, NULL);
                    void *data;
                    gsize datalen;
                    gboolean ok = g_file_get_contents(svgname, (gchar **)&data, &datalen, NULL);
                    g_free(svgname);
                    if (!ok)
                        break;
                    db_write_blob((n << 8) | i, data, datalen);
                    i++;
                }
                n++;
            } // if
            g_free(fullname);
            fname = g_dir_read_name(dir);
        } // while fname

        g_dir_close(dir);
    } // if dir

    g_free(d);

    read_custom_from_db();
}

void read_custom_from_db(void)
{
    gint n, page;
    int len;
    unsigned char *data;

    for (n = 0; n < NUM_CUSTOM_BRACKETS; n++) {
        db_read_blob(n << 8, (unsigned char **)&custom_brackets[n], &len);
        if (custom_brackets[n] == NULL)
            break;

        for (page = 1; page <= 10; page++) {
            db_read_blob((n << 8) | page, &data, &len);
            if (data == NULL)
                break;
            add_custom_svg((gchar *)data, len, n, page);
#if 0
            gchar buf[32];
            sprintf(buf, "%x.svg", (n << 8) | page);
            if (g_file_set_contents(buf, (gchar *)data, len, NULL))
                g_print("g_file_set_contents %s OK len=%d\n", buf, len);
            else
                g_print("g_file_set_contents %s NOK len=%d\n", buf, len);
#endif
        }
    }

    num_custom_brackets = n;
}

void select_custom_dir(GtkWidget *menu_item, gpointer data)
{
    GtkWidget *dialog;

    dialog = gtk_file_chooser_dialog_new(_("Choose a directory"),
                                         NULL,
                                         GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                         GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                         NULL);

    if (custom_directory)
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),
                                            custom_directory);
    else {
        gchar *dirname = g_build_filename(installation_dir, "custom-examples", NULL);
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), dirname);
        g_free(dirname);
    }

    if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_ACCEPT) {
        gtk_widget_destroy(dialog);
        return;
    }

    g_free(custom_directory);
    custom_directory = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
    g_key_file_set_string(keyfile, "preferences", "customdir", custom_directory);

    gtk_widget_destroy (dialog);

    read_custom_files();
}

