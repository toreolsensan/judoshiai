/* -*- mode: C; c-basic-offset: 4;  -*- */

/*
 * Copyright (C) 2006-2011 by Hannu Jokinen
 * Full copyright text is included in the software package.
 */ 

#ifndef _JUDOWEIGHT_H_
#define _JUDOWEIGHT_H_

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

#define LANG_FI 0
#define LANG_SW 1
#define LANG_EN 2
#define LANG_ES 3
#define LANG_EE 4
#define LANG_UK 5
#define LANG_IS 6
#define NUM_LANGS 7

#define FRAME_WIDTH  500
#define FRAME_HEIGHT 300

#define JUDOGI_OK     0x20
#define JUDOGI_NOK    0x40

extern GTimer *timer;
extern gchar *installation_dir;
extern gulong my_ip_address, node_ip_addr;
extern GKeyFile *keyfile;
extern gint language;
extern GtkWidget *weight_entry;
extern gint my_address;

extern gboolean this_is_shiai(void);
extern void msg_to_queue(struct message *msg);
extern struct message *get_rec_msg(void);
extern void destroy( GtkWidget *widget, gpointer   data );
extern void set_preferences(void);
extern gulong host2net(gulong a);

extern GtkWidget *get_menubar_menu(GtkWidget  *window);
extern gpointer client_thread(gpointer args);
extern gpointer serial_thread(gpointer args);
extern gboolean change_language(GtkWidget *eventbox, GdkEventButton *event, void *param);
extern void ask_node_ip_address( GtkWidget *w, gpointer data);
extern void show_my_ip_addresses( GtkWidget *w, gpointer data);

extern void set_display(struct msg_edit_competitor *msg);

#endif
