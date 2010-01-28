/* -*- mode: C; c-basic-offset: 4;  -*- */

/*
 * Copyright (C) 2006-2010 by Hannu Jokinen
 * Full copyright text is included in the software package.
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include <cairo.h>
#include <cairo-pdf.h>

#include "sqlite3.h"
#include "judoshiai.h"

#define SIZEX 400
#define SIZEY 400
#define HISTORY_LEN 8

static gchar *history[HISTORY_LEN];
static gint history_line;
static GtkTextBuffer *buffer;

static gboolean delete_event_sql( GtkWidget *widget,
                                  GdkEvent  *event,
                                  gpointer   data )
{
    return FALSE;
}

static void destroy_sql( GtkWidget *widget,
                         gpointer   data )
{
}

#if 0
static void char_typed(GtkWidget *w, gpointer arg) 
{
    const gchar *txt = gtk_entry_get_text(GTK_ENTRY(w));
    g_print("txt=%s\n", txt);
}
#endif

static void enter_typed(GtkWidget *w, gpointer arg) 
{
    gint i;
    gchar *ret;
    const gchar *txt = gtk_entry_get_text(GTK_ENTRY(w));
        
    g_free(history[HISTORY_LEN-1]);
    for (i = HISTORY_LEN-1; i > 0; i--)
        history[i] = history[i-1];
    history[0] = g_strdup(txt);
    history_line = 0;

    ret = db_sql_command(txt);

    if (ret == NULL || ret[0] == 0)
        gtk_text_buffer_set_text(buffer, "OK", -1);
    else
        gtk_text_buffer_set_text(buffer, ret, -1);

    g_free(ret);

    gtk_entry_set_text(GTK_ENTRY(w), "");
}

static gboolean key_press(GtkWidget *widget, GdkEventKey *event, gpointer userdata)
{
    if (event->keyval == GDK_Up ||
        event->keyval == GDK_KP_Up) {
        if (history[history_line]) {
            gtk_entry_set_text(GTK_ENTRY(widget), history[history_line]);
            if (history_line < HISTORY_LEN - 1)
                history_line++;
        }
        return TRUE;
    } else if (event->keyval == GDK_Down ||
               event->keyval == GDK_KP_Down) {
        if (history_line > 0) {
            history_line--;
            gtk_entry_set_text(GTK_ENTRY(widget), history[history_line]);
        } else
            gtk_entry_set_text(GTK_ENTRY(widget), "");
        return TRUE;
    }
    return FALSE;
}

void sql_window(GtkWidget *w, gpointer data)
{
    GtkWidget *vbox, *line, *result;
    GtkWindow *window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
    gtk_window_set_title(GTK_WINDOW(window), _("SQL"));
    gtk_widget_set_size_request(GTK_WIDGET(window), SIZEX, SIZEY);

    vbox = gtk_vbox_new(FALSE, 1);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 1);
    gtk_container_add (GTK_CONTAINER (window), vbox);

    result = gtk_text_view_new();
    line = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(vbox), result, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), line, FALSE, TRUE, 0);

    gtk_widget_show_all(GTK_WIDGET(window));

    g_signal_connect (G_OBJECT (window), "delete_event",
                      G_CALLBACK (delete_event_sql), NULL);
    g_signal_connect (G_OBJECT (window), "destroy",
                      G_CALLBACK (destroy_sql), NULL);

    g_signal_connect(G_OBJECT(line), "activate", G_CALLBACK(enter_typed), data);
    g_signal_connect(G_OBJECT(line), "key-press-event", G_CALLBACK(key_press), data);

    gtk_entry_set_text(GTK_ENTRY(line), "SELECT * FROM sqlite_master");
    gtk_widget_grab_focus(GTK_WIDGET(line));
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(result));
    gtk_text_view_set_editable(GTK_TEXT_VIEW(result), FALSE);
}

