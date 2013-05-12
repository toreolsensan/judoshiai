/* -*- mode: C; c-basic-offset: 4;  -*- */

/*
 * Copyright (C) 2006-2013 by Hannu Jokinen
 * Full copyright text is included in the software package.
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <curl/curl.h>

#include "sqlite3.h"
#include "judoshiai.h"

#define PROTO_FTP 0
#define PROTO_HTTP 1

struct url {
    GtkWidget *address, *port, *path, *start;
    GtkWidget *user, *password, *proxy_address, *proxy_port;
    GtkWidget *do_it, *proto, *proxy_user, *proxy_password;
};

static gchar *ftp_server = NULL, *ftp_path = NULL, *proxy_host = NULL,
    *ftp_user = NULL, *ftp_password = NULL, *proxy_user, *proxy_password;
static gint ftp_port = 21, proxy_port = 8080, proto = PROTO_FTP;
static gboolean connection_ok = FALSE, ftp_update = FALSE, do_ftp = FALSE;

#define STRDUP(_d, _s) do { g_free(_d); _d = g_strdup(_s); } while (0)
#define GETSTR(_d, _s) do { g_free(_d);                                 \
        _d = g_key_file_get_string(keyfile, "preferences", _s, NULL);   \
        if (!_d) _d = g_strdup(""); } while (0)
#define GETINT(_d, _s) do { GError *x=NULL; _d = g_key_file_get_integer(keyfile, "preferences", _s, &x); } while (0)

static void ftp_to_server_callback(GtkWidget *widget, 
                                   GdkEvent *event,
                                   GtkWidget *data)
{
    struct url *uri = (struct url *)data;

    if ((gulong)event == GTK_RESPONSE_OK) {
        proto = gtk_combo_box_get_active(GTK_COMBO_BOX(uri->proto));
        if (proto < 0) proto = 0;
        g_key_file_set_integer(keyfile, "preferences", "ftpproto", proto);

        STRDUP(ftp_server, gtk_entry_get_text(GTK_ENTRY(uri->address))); 
        g_key_file_set_string(keyfile, "preferences", "ftpserver", ftp_server);

        ftp_port = atoi(gtk_entry_get_text(GTK_ENTRY(uri->port)));
        g_key_file_set_integer(keyfile, "preferences", "ftpport", ftp_port);

        STRDUP(ftp_path, gtk_entry_get_text(GTK_ENTRY(uri->path)));
        g_key_file_set_string(keyfile, "preferences", "ftppath", ftp_path);

        STRDUP(proxy_host, gtk_entry_get_text(GTK_ENTRY(uri->proxy_address))); 
        g_key_file_set_string(keyfile, "preferences", "ftpproxyaddress", proxy_host);

        proxy_port = atoi(gtk_entry_get_text(GTK_ENTRY(uri->proxy_port)));
        g_key_file_set_integer(keyfile, "preferences", "ftpproxyport", proxy_port);

        STRDUP(ftp_user, gtk_entry_get_text(GTK_ENTRY(uri->user))); 
        g_key_file_set_string(keyfile, "preferences", "ftpuser", ftp_user);

        STRDUP(ftp_password, gtk_entry_get_text(GTK_ENTRY(uri->password))); 
        g_key_file_set_string(keyfile, "preferences", "ftppassword", ftp_password);

        STRDUP(proxy_user, gtk_entry_get_text(GTK_ENTRY(uri->proxy_user))); 
        g_key_file_set_string(keyfile, "preferences", "ftpproxyuser", proxy_user);

        STRDUP(proxy_password, gtk_entry_get_text(GTK_ENTRY(uri->proxy_password))); 
        g_key_file_set_string(keyfile, "preferences", "ftpproxypassword", proxy_password);

        do_ftp = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(uri->do_it));

        ftp_update = TRUE;
    }

    g_free(uri);
    gtk_widget_destroy(widget);
}

void ftp_to_server(GtkWidget *w, gpointer data)
{
    gchar buf[128];
    GtkWidget *dialog, *hbox, *hbox1, *hbox2, *hbox3, *label;
    struct url *uri = g_malloc0(sizeof(*uri));
    GtkWidget *proxy_lbl = gtk_label_new(_("Proxy:"));

    dialog = gtk_dialog_new_with_buttons (_("Server URL"),
                                          NULL,
                                          GTK_DIALOG_DESTROY_WITH_PARENT,
                                          GTK_STOCK_OK, GTK_RESPONSE_OK,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          NULL);

    GETINT(proto, "ftpproto");
    uri->proto = gtk_combo_box_new_text();
    gtk_combo_box_append_text(GTK_COMBO_BOX(uri->proto), "ftp");
    gtk_combo_box_append_text(GTK_COMBO_BOX(uri->proto), "http");
    gtk_combo_box_set_active(GTK_COMBO_BOX(uri->proto), proto);

    GETSTR(ftp_server, "ftpserver");
    uri->address = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(uri->address), ftp_server);
    gtk_entry_set_width_chars(GTK_ENTRY(uri->address), 20);

    GETINT(ftp_port, "ftpport");
    sprintf(buf, "%d", ftp_port); 
    uri->port = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(uri->port), buf);
    gtk_entry_set_width_chars(GTK_ENTRY(uri->port), 4);

    GETSTR(ftp_path, "ftppath");
    uri->path = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(uri->path), ftp_path);
    gtk_entry_set_width_chars(GTK_ENTRY(uri->path), 16);

    GETSTR(proxy_host, "ftpproxyaddress");
    uri->proxy_address = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(uri->proxy_address), proxy_host);
    gtk_entry_set_width_chars(GTK_ENTRY(uri->proxy_address), 20);

    GETINT(proxy_port, "ftpproxyport");
    sprintf(buf, "%d", proxy_port);
    uri->proxy_port = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(uri->proxy_port), buf);
    gtk_entry_set_width_chars(GTK_ENTRY(uri->proxy_port), 4);

    GETSTR(ftp_user, "ftpuser");
    uri->user = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(uri->user), ftp_user);
    gtk_entry_set_width_chars(GTK_ENTRY(uri->user), 20);

    GETSTR(ftp_password, "ftppassword");
    uri->password = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(uri->password), ftp_password);
    gtk_entry_set_width_chars(GTK_ENTRY(uri->password), 20);

    GETSTR(proxy_user, "ftpproxyuser");
    uri->proxy_user = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(uri->proxy_user), proxy_user);
    gtk_entry_set_width_chars(GTK_ENTRY(uri->proxy_user), 20);

    GETSTR(proxy_password, "ftpproxypassword");
    uri->proxy_password = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(uri->proxy_password), proxy_password);
    gtk_entry_set_width_chars(GTK_ENTRY(uri->proxy_password), 20);

    uri->do_it = gtk_check_button_new_with_label(_("Copy to Server"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(uri->do_it), do_ftp);

    //gtk_container_set_border_width(GTK_CONTAINER(hbox), 10);

    hbox = gtk_hbox_new(FALSE, 4);
    gtk_box_pack_start(GTK_BOX(hbox), uri->proto, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new("://"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), uri->address, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new(":"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), uri->port, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new("/"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), uri->path, FALSE, FALSE, 0);

    hbox2 = gtk_hbox_new(FALSE, 4);
    gtk_box_pack_start(GTK_BOX(hbox2), gtk_label_new(_("User:")), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox2), uri->user, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(hbox2), gtk_label_new(_("Password:")), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox2), uri->password, FALSE, FALSE, 0);

    hbox1 = gtk_hbox_new(FALSE, 4);
    gtk_box_pack_start(GTK_BOX(hbox1), proxy_lbl, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox1), uri->proxy_address, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox1), gtk_label_new(":"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox1), uri->proxy_port, FALSE, FALSE, 0);

    hbox3 = gtk_hbox_new(FALSE, 4);
    gtk_box_pack_start(GTK_BOX(hbox3), gtk_label_new(_("Proxy User:")), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox3), uri->proxy_user, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(hbox3), gtk_label_new(_("Proxy Password:")), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox3), uri->proxy_password, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox, FALSE, FALSE, 4);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox2, FALSE, FALSE, 4);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox1, FALSE, FALSE, 4);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox3, FALSE, FALSE, 4);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), uri->do_it, FALSE, FALSE, 4);
    gtk_widget_show_all(dialog);

    g_signal_connect(G_OBJECT(dialog), "response",
                     G_CALLBACK(ftp_to_server_callback), uri);
}

static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
    size_t retcode = fread(ptr, size, nmemb, stream);
    return retcode;
}

static CURL *curl = NULL;
static struct curl_slist *headerlist = NULL;

static void init_curl(void)
{
    curl = curl_easy_init();
    if (!curl)
        return;
    
    /* verbose printing */
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    /* we want to use our own read function */ 
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
 
    /* enable uploading */ 
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
 
    /* user & password */
    if (ftp_user && ftp_user[0]) {
#if 0
        curl_easy_setopt(curl, CURLOPT_USERNAME, ftp_user);
        if (ftp_password && ftp_password[0])
            curl_easy_setopt(curl, CURLOPT_PASSWORD, ftp_password);
#else
        gchar pwd[64];
        snprintf(pwd, sizeof(pwd), "%s:%s", ftp_user, 
                 ftp_password?ftp_password:"");
        curl_easy_setopt(curl, CURLOPT_USERPWD, pwd);
#endif
    }

    /* proxy */
    if (proxy_host && proxy_host[0]) {
        curl_easy_setopt(curl, CURLOPT_PROXY, proxy_host);
        curl_easy_setopt(curl, CURLOPT_PROXYPORT, (long)proxy_port);
    }

    /* proxy user & password */
    if (proxy_user && proxy_user[0]) {
        gchar pwd[64];
        snprintf(pwd, sizeof(pwd), "%s:%s", proxy_user, 
                 proxy_password?proxy_password:"");
        curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, pwd);
    }

    if (ftp_port)
        curl_easy_setopt(curl, CURLOPT_PORT, (long)ftp_port);
}

static void cleanup_curl(void)
{
    /* always cleanup */ 
    if (curl) {
        curl_easy_cleanup(curl);
        curl = NULL;
    }
}

gpointer ftp_thread(gpointer args)
{
    for ( ; *((gboolean *)args) ; )   /* exit loop when flag is cleared */
    {
        time_t last_copy = 0;
        ftp_update = FALSE;

        while (!do_ftp || !ftp_server || !ftp_server[0] ||
               !current_directory[0] || current_directory[0] == '.') {
            g_usleep(1000000);
        }

        while (!ftp_update) {
            time_t copy_start = time(NULL);
            GDir *dir = g_dir_open(current_directory, 0, NULL);
            if (dir) {
                init_curl();
                const gchar *fname = g_dir_read_name(dir);
                while (fname) {
                    struct stat statbuf;
                    gchar *fullname = g_build_filename(current_directory, fname, NULL);
                    if (!g_stat(fullname, &statbuf)) {
                        if (statbuf.st_mtime >= last_copy &&
                            (statbuf.st_mode & S_IFREG)) {
                            FILE *f = fopen(fullname, "rb");
                            if (f) {
                                CURLcode res;
                                if (curl) {
                                    if (proto == PROTO_FTP) {
                                        gchar *n = g_strdup_printf("RNTO %s", fname);
                                        headerlist = curl_slist_append(headerlist, "RNFR TMPXYZ");
                                        headerlist = curl_slist_append(headerlist, n);
                                        g_free(n);
                                        /* pass in that last of FTP commands to run after the transfer */ 
                                        curl_easy_setopt(curl, CURLOPT_POSTQUOTE, headerlist);
                                    }

                                    curl_off_t fsize = statbuf.st_size;

                                    /* specify target */ 
                                    gchar *u;

                                    if (ftp_path && ftp_path[0])
                                        u = g_strdup_printf("%s://%s/%s/%s",
                                                            proto == PROTO_FTP ? "ftp" : "http", 
                                                            ftp_server, ftp_path,
                                                            proto == PROTO_FTP ? "TMPXYZ" : fname);
                                    else
                                        u = g_strdup_printf("%s://%s/%s",
                                                            proto == PROTO_FTP ? "ftp" : "http", 
                                                            ftp_server,
                                                            proto == PROTO_FTP ? "TMPXYZ" : fname);

                                    curl_easy_setopt(curl, CURLOPT_URL, u);
                                    g_free(u);
                                    
                                    /* specify which file to upload */
                                    curl_easy_setopt(curl, CURLOPT_READDATA, f);
 
                                    /* Set the size of the file to upload (optional).  If you give a *_LARGE
                                       option you MUST make sure that the type of the passed-in argument is a
                                       curl_off_t. If you use CURLOPT_INFILESIZE (without _LARGE) you must
                                       make sure that to pass in a type 'long' argument. */ 
                                    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE,
                                                     (curl_off_t)fsize);
                                
                                    //curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
                                    //curl_easy_setopt(curl, CURLOPT_FTP_USE_EPSV, 0);

                                    /* Now run off and do what you've been told! */ 
                                    res = curl_easy_perform(curl);

                                    /* Check for errors */ 
                                    if(res != CURLE_OK)
                                        g_print("curl_easy_perform() failed: %s\n",
                                                curl_easy_strerror(res));

                                    if (headerlist) {
                                        /* clean up the FTP commands list */ 
                                        curl_slist_free_all(headerlist);
                                        headerlist = NULL;
                                        curl_easy_setopt(curl, CURLOPT_POSTQUOTE, headerlist);
                                    }
                                } // if (curl)
                                fclose(f);
                            } // if (f)
                        } // if mtime >
                    } // if (!g_stat)
                    g_free(fullname);
                    fname = g_dir_read_name(dir);
                    g_usleep(1000);
                } // while fname

                g_dir_close(dir);
                last_copy = copy_start;
                cleanup_curl();
            } // if (dir)
            g_usleep(2000000);
        }
    }

    g_thread_exit(NULL);    /* not required just good pratice */
    return NULL;
}
