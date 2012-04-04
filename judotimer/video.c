/* -*- mode: C; c-basic-offset: 4;  -*- */

/*
 * Copyright (C) 2006-2012 by Hannu Jokinen
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

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <vlc/vlc.h>

#include "judotimer.h"

struct find_frame_internals {
    gchar *data;
    gint n, state, length;
};

static void open_media(gint num);
static void close_media(void);
static gboolean refresh_video(gpointer data);
static gchar *find_frame(gchar c, gint *len, struct find_frame_internals *vars);

#define VIDEO_BUF_LEN (1<<20)
#define NUM_VIDEO_BUFFERS 2

enum {
    PREVIOUS,
    REWIND,
    STOP,
    PLAY_PAUSE,
    FORWARD,
    NEXT,
    VIEW
};

static struct video_buffer {
    gchar *buffer;
    gint offset;
    gboolean full;
    glong start, end, duration;
} video_buffers[NUM_VIDEO_BUFFERS];
static gint current = 0;

gulong video_http_addr;
guint  video_http_port;
gchar  video_http_path[128];
static gboolean connection_ok = FALSE;
static gboolean record = TRUE;

#define play_img gtk_image_new_from_stock(GTK_STOCK_MEDIA_PLAY, GTK_ICON_SIZE_BUTTON)
#define pause_img gtk_image_new_from_stock(GTK_STOCK_MEDIA_PAUSE, GTK_ICON_SIZE_BUTTON)

static GtkWidget *playpause_button;
static GtkWidget *slider;
static gint current_media = 1;
static gint direction = 1;
static gboolean play = TRUE;
static gint fps = 10, framecnt, curpos, view_fps;
static gchar *frame_now;
static gint length_now;
static gboolean view = FALSE;

gpointer video_thread(gpointer args)
{
    SOCKET comm_fd;
    gint n;
    struct sockaddr_in node;
    static guchar buf[1000];
    fd_set read_fd, fds;
    gulong http_addr1;
    guint  http_port1;
    struct video_buffer *vb;

    for (n = 0; n < NUM_VIDEO_BUFFERS; n++)
        video_buffers[n].buffer = g_malloc(VIDEO_BUF_LEN);

    for ( ; *((gboolean *)args) ; )   /* exit loop when flag is cleared */
    {
        while (video_http_addr == 0 || video_http_port == 0)
            g_usleep(1000000);

        if ((comm_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
            perror("video socket");
            g_print("CANNOT CREATE SOCKET (%s:%d)!\n", __FUNCTION__, __LINE__);
            g_thread_exit(NULL);    /* not required just good pratice */
            return NULL;
        }

        http_addr1 = video_http_addr;
        http_port1 = video_http_port;

        memset(&node, 0, sizeof(node));
        node.sin_family      = AF_INET;
        node.sin_port        = htons(video_http_port);
        node.sin_addr.s_addr = video_http_addr;

        if (connect(comm_fd, (struct sockaddr *)&node, sizeof(node))) {
            closesocket(comm_fd);
            g_usleep(1000000);
            continue;
        }

        current = 0;
        vb = &video_buffers[current];
        vb->offset = 0;
        vb->full = FALSE;
        vb->start = time(NULL);

        g_print("Video connection OK.\n");

        /* send http request */
        n = snprintf((gchar *)buf, sizeof(buf), 
                     "GET /%s HTTP/1.0\r\n"
                     "User-Agent: JudoTimer\r\n"
                     "Connection: Close\r\n\r\n", video_http_path);
        send(comm_fd, buf, n, 0);

        connection_ok = TRUE;

        FD_ZERO(&read_fd);
        FD_SET(comm_fd, &read_fd);

        while (video_http_addr == http_addr1 && video_http_port == http_port1) {
            struct timeval timeout;
            gint r;
			
            fds = read_fd;
            timeout.tv_sec = 0;
            timeout.tv_usec = 100000;

            r = select(comm_fd+1, &fds, NULL, NULL, &timeout);

            if (r <= 0)
                continue;

            if (FD_ISSET(comm_fd, &fds)) {
                glong now = time(NULL);
                static glong view_start = 0;
                static gint view_frames = 0;
                static struct find_frame_internals view_vars;

                if (record) {
                    vb = &video_buffers[current];
                    if ((n = recv(comm_fd, vb->buffer + vb->offset, VIDEO_BUF_LEN - vb->offset, 0)) > 0) {
                        if (view) {
                            // find frames in real time
                            gint i;
                            for (i = 0; i < n; i++) {
                                gint len;
                                gchar *fr = find_frame(*(vb->buffer + vb->offset + i), &len, &view_vars);
                                if (fr) {
                                    if (frame_now) g_free(frame_now);
                                    frame_now = fr;
                                    length_now = len;

                                    view_frames++;
                                    if (view_start == 0) view_start = now;
                                    if (now > view_start + 3) {
                                        view_fps = view_frames/(now - view_start);
                                        view_frames = 0;
                                        view_start = now;
                                    }
                                }
                            }
                        } else {
                            view_frames = 0;
                            view_start = 0;
                        }

                        vb->offset += n;
                        if (vb->offset >= VIDEO_BUF_LEN) {
                            vb->offset = 0;
                            vb->full = TRUE;
                            vb->duration = now - vb->start;
                            vb->start = now;
                            //g_print("current=%d video buf len=%d\n", current, VIDEO_BUF_LEN);
                        }
                    } else 
                        break;
                } 
            }
        }

        connection_ok = FALSE;
        closesocket(comm_fd);

        g_print("Video connection NOK.\n");
    }

    for (n = 0; n < NUM_VIDEO_BUFFERS; n++)
        g_free(video_buffers[n].buffer);

    g_thread_exit(NULL);    /* not required just good pratice */
    return NULL;
}

void video_save(void)
{
    if (!connection_ok)
        return;

    record = FALSE;

    video_buffers[current].end = time(NULL);
    if (video_buffers[current].full == FALSE)
        video_buffers[current].duration = 
            video_buffers[current].end - video_buffers[current].start;

    if (++current >= NUM_VIDEO_BUFFERS)
        current = 0;

    video_buffers[current].offset = 0;
    video_buffers[current].full = FALSE;
    video_buffers[current].start = time(NULL);

    record = TRUE;
}


extern GKeyFile *keyfile;

struct url {
    GtkWidget *address, *port, *path, *start;
};

static void video_ip_address_callback(GtkWidget *widget, 
                                     GdkEvent *event,
                                     GtkWidget *data)
{
    gulong a,b,c,d;
    struct url *uri = (struct url *)data;

    if ((gulong)event == GTK_RESPONSE_OK) {
        sscanf(gtk_entry_get_text(GTK_ENTRY(uri->address)), "%ld.%ld.%ld.%ld", &a, &b, &c, &d);
        video_http_addr = htonl((a << 24) | (b << 16) | (c << 8) | d);
        g_key_file_set_string(keyfile, "preferences", "videoipaddress", 
                              gtk_entry_get_text(GTK_ENTRY(uri->address)));

        video_http_port = atoi(gtk_entry_get_text(GTK_ENTRY(uri->port)));
        g_key_file_set_integer(keyfile, "preferences", "videoipport", video_http_port);

        snprintf(video_http_path, sizeof(video_http_path), "%s", gtk_entry_get_text(GTK_ENTRY(uri->path))); 
        g_key_file_set_string(keyfile, "preferences", "videoippath", video_http_path);
    }

    g_free(uri);
    gtk_widget_destroy(widget);
}

void ask_video_ip_address( GtkWidget *w,
                           gpointer   data )
{
    gchar buf[20];
    gulong myaddr = ntohl(video_http_addr);
    gint myport = video_http_port; 
    GtkWidget *dialog, *hbox, *label;
    struct url *uri = g_malloc0(sizeof(*uri));

    dialog = gtk_dialog_new_with_buttons (_("Video server URL"),
                                          NULL,
                                          GTK_DIALOG_DESTROY_WITH_PARENT,
                                          GTK_STOCK_OK, GTK_RESPONSE_OK,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          NULL);

    sprintf(buf, "%ld.%ld.%ld.%ld", 
            (myaddr>>24)&0xff, (myaddr>>16)&0xff, 
            (myaddr>>8)&0xff, (myaddr)&0xff);
    uri->address = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(uri->address), buf);
    gtk_entry_set_width_chars(GTK_ENTRY(uri->address), 15);

    sprintf(buf, "%d", myport); 
    uri->port = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(uri->port), buf);
    gtk_entry_set_width_chars(GTK_ENTRY(uri->port), 4);

    uri->path = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(uri->path), video_http_path);
    gtk_entry_set_width_chars(GTK_ENTRY(uri->path), 6);

    uri->start = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(uri->path), "5");
    gtk_entry_set_width_chars(GTK_ENTRY(uri->path), 1);

    hbox = gtk_hbox_new(FALSE, 0);
    //gtk_container_set_border_width(GTK_CONTAINER(hbox), 10);

    gtk_box_pack_start_defaults(GTK_BOX(hbox), gtk_label_new("http://"));
    gtk_box_pack_start_defaults(GTK_BOX(hbox), uri->address);
    gtk_box_pack_start_defaults(GTK_BOX(hbox), gtk_label_new(":"));
    gtk_box_pack_start_defaults(GTK_BOX(hbox), uri->port);
    gtk_box_pack_start_defaults(GTK_BOX(hbox), gtk_label_new("/"));
    gtk_box_pack_start_defaults(GTK_BOX(hbox), uri->path);

    if (connection_ok)
        label = gtk_label_new(_("(Connection OK)"));
    else
        label = gtk_label_new(_("(Connection broken)"));
    gtk_box_pack_start_defaults(GTK_BOX(hbox), label);

    gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox);
    gtk_widget_show_all(dialog);

    g_signal_connect(G_OBJECT(dialog), "response",
                     G_CALLBACK(video_ip_address_callback), uri);
}

static GtkWindow *video_window;
static const gchar *contlen1 = "content-length:";
static const gchar *contlen2 = "CONTENT-LENGTH:";

struct frame {
    struct frame *next, *prev;
    gchar *data;
    gint length;
};

static struct frame frames;
static struct frame *fp = NULL;

static void close_media(void)
{
    struct frame *f, *p = frames.next;

    while (p) {
        f = p;
        p = p->next;
        g_free(f->data);
        g_free(f);
    }

    frames.next = frames.prev = NULL;
}

static gchar *find_frame(gchar c, gint *len, struct find_frame_internals *vars)
{
    switch (vars->state) {
    case 16: // after length found
        if (c == '\r') vars->state++;
        break;
    case 17: // after cr
        if (c == '\n') vars->state++;
        else if (c != '\r') vars->state = 16;
        break;
    case 18: // after cr nl
        if (c == '\r') vars->state++;
        else vars->state = 16;
        break;
    case 19: // after cr nl cr
        if (c == '\n') {
            // start reading data
            vars->state++;
            vars->data = g_malloc(vars->length);
            vars->n = 0;
        } else if (c == '\r') vars->state = 17;
        else vars->state = 16;
        break;
    case 20: // reading data
        if (vars->length) {
            vars->data[vars->n++] = c;
            vars->length--;
        }
        if (vars->length == 0) {
            *len = vars->n;
            vars->n = 0;
            vars->state = 0;
            return vars->data;
        }
        break;
    case 15:
        if (c >= '0' && c <= '9')
            vars->length = 10*vars->length + c - '0';
        else if (c != ' ') {
            if (vars->length > 100000) vars->state = 0;
            else vars->state = 16;
        }
        break;
    default:
        if (vars->state < 15 && (c == contlen1[vars->state] || c == contlen2[vars->state])) {
            vars->state++;
            if (vars->state == 15) {
                vars->length = 0;
            }                
        } else vars->state = 0;
    } // switch

    return NULL;
}

static void open_media(gint num) 
{
    gint last = (current + NUM_VIDEO_BUFFERS - num) % NUM_VIDEO_BUFFERS;
    struct video_buffer *vb = &video_buffers[last];
    gint ix = 0, length;
    struct frame *f = NULL, *p = NULL;
    struct find_frame_internals media_vars;

    memset(&media_vars, 0, sizeof(media_vars));
    framecnt = 0;

    if (vb->full) ix = vb->offset+1;

    while (ix != vb->offset) {
        if (ix >= VIDEO_BUF_LEN) ix = 0;

        gchar *fr = find_frame(vb->buffer[ix], &length, &media_vars);
        if (fr) {
            f = g_malloc0(sizeof(*f));
            f->data = fr;
            f->length = length;
            p = &frames;
            while (p->next) p = p->next;
            p->next = f;
            f->prev = p;
            framecnt++;
        }

        ix++;
    }

    if (vb->duration)    
        fps = framecnt/vb->duration;
    else
        fps = 10;

    fp = frames.next;
    direction = 1;
    play = TRUE;
    curpos = 1;
    gtk_button_set_image(GTK_BUTTON(playpause_button), pause_img);

    // find position 5 sec before end
    gint targetpos = framecnt - 5*fps;
    while (curpos <= framecnt && curpos < targetpos && fp->next && fp->next->length > 100) {
        fp = fp->next;
        curpos++;
    }
}

static void on_button(GtkWidget *widget, gpointer data) 
{
    gint button = (gint)data;

    view = FALSE;

    switch (button) {
    case PREVIOUS:
        gtk_button_set_image(GTK_BUTTON(playpause_button), play_img);
        direction = 1;
        play = FALSE;
        gtk_range_set_value(GTK_RANGE(slider), 0.0);
#if 0
        if (current_media < NUM_VIDEO_BUFFERS - 1) {
            current_media++;
            close_media();
            open_media(current_media);
        }
#endif
        break;
    case REWIND:
        gtk_button_set_image(GTK_BUTTON(playpause_button), pause_img);
        direction = -2;
        play = TRUE;
        break;
    case STOP:
    case PLAY_PAUSE:
        if (play) {
            gtk_button_set_image(GTK_BUTTON(playpause_button), play_img);
            play = FALSE;
        } else {
            gtk_button_set_image(GTK_BUTTON(playpause_button), pause_img);
            direction = 1;
            play = TRUE;
        }
        break;
    case FORWARD:
        gtk_button_set_image(GTK_BUTTON(playpause_button), pause_img);
        direction = 2;
        play = TRUE;
        break;
    case NEXT:
        gtk_button_set_image(GTK_BUTTON(playpause_button), play_img);
        direction = 1;
        play = FALSE;
        gtk_range_set_value(GTK_RANGE(slider), 100.0);
#if 0
        if (current_media > 0) {
            current_media--;
            close_media();
            open_media(current_media);
        }
#endif
        break;
    case VIEW:
        view = TRUE;
        break;
    }
}


static gboolean delete_event_video( GtkWidget *widget, GdkEvent  *event, gpointer   data )
{
    return FALSE;
}

static void destroy_video( GtkWidget *widget, gpointer   data )
{
    close_media();
    video_window = NULL;
    if (frame_now)
        g_free(frame_now);
    frame_now = NULL;
    view = FALSE;
}

static gboolean close_video(GtkWidget *widget, gpointer userdata)
{
    gtk_widget_destroy(userdata);
    return FALSE;
}

static gboolean expose_video(GtkWidget *widget, GdkEventExpose *event, gpointer userdata)
{
    gint width, height;
    GError *err = NULL;
    GdkPixbuf *pb;

    width = widget->allocation.width;
    height = widget->allocation.height;

    if (view) {
        if (!frame_now)
            return FALSE;

        GInputStream *stream = g_memory_input_stream_new_from_data(frame_now, length_now, NULL);
        pb = gdk_pixbuf_new_from_stream(stream, NULL, &err);
        g_input_stream_close(stream, NULL, NULL);
        //pb = gdk_pixbuf_new_from_inline(length_now, frame_now, TRUE, &err);
        //g_file_set_contents("test.jpg", frame_now, length_now, NULL);
    } else {
        if (fp == NULL || fp->length < 100)
            return FALSE;

        GInputStream *stream = g_memory_input_stream_new_from_data(fp->data, fp->length, NULL);
        pb = gdk_pixbuf_new_from_stream(stream, NULL, &err);
        g_input_stream_close(stream, NULL, NULL);
        //pb = gdk_pixbuf_new_from_inline(fp->length, fp->data, TRUE, &err);
        //g_file_set_contents("test.jpg", fp->data, fp->length, NULL);
    }

    //GdkPixbuf *pb = gdk_pixbuf_new_from_file("test.jpg", &err);
    if (!pb) {
        g_print("\nERROR %s: %s %d\n",
                err->message, __FUNCTION__, __LINE__);
        //g_file_set_contents("err.jpg", fp->data, fp->length, NULL);
        return FALSE;
    }
    gint pixwidth = gdk_pixbuf_get_width(pb);
    gint pixheight = gdk_pixbuf_get_height(pb);
    gdouble scalex = 1.0*width/pixwidth, 
        scaley = 1.0*height/pixheight,
        scale = (scalex < scaley) ? scalex : scaley;

    gdouble x = (scale*pixwidth < width) ? (width/scale - pixwidth)/2.0 : 0;
    gdouble y = (scale*pixheight < height) ? (height/scale - pixheight)/2.0 : 0;

    cairo_t *c = gdk_cairo_create(widget->window);

    cairo_scale(c, scale, scale);
    gdk_cairo_set_source_pixbuf(c, pb, x, y);
    cairo_paint(c);

    if (view) {
        gchar buf[128];
        snprintf(buf, sizeof(buf), "%dx%d", pixwidth, pixheight);
        cairo_set_source_rgb(c, 1.0, 0.0, 0.0);
        cairo_set_font_size(c, 20/scale);
        cairo_move_to(c, 30.0/scale, 30.0/scale);
        cairo_show_text(c, buf);
        cairo_move_to(c, 30.0/scale, 50.0/scale);
        snprintf(buf, sizeof(buf), "%d fps", view_fps);
        cairo_show_text(c, buf);
    }
    
    g_object_unref(pb);
    cairo_show_page(c);
    cairo_destroy(c);

    return FALSE;
}

static gboolean refresh_video(gpointer data)
{
    static gdouble lastval = -100.0;
    GtkWidget *widget;
    GdkRegion *region;

    if (video_window == NULL)
        return FALSE;

    if (view)
        goto refresh;

    if (fp == NULL || framecnt == 0)
        return TRUE;

    if (!play) {
        gdouble step = 100.0/framecnt;
        gdouble val = gtk_range_get_value(GTK_RANGE(slider));
        gdouble diff = lastval - val;
        if (diff < 0) diff = -diff;
        if (diff < 0.2)
            return TRUE;

        lastval = val;
        gdouble cur = (gdouble)curpos*step;
        if (val < cur - step*0.5) {
            while (curpos > 0 && val < cur && fp->prev && fp->prev->length > 100) {
                fp = fp->prev;
                curpos--;
                cur = (gdouble)curpos*step;
            }
        } else if (val > cur + step*0.5) {
            while (curpos <= framecnt && val > cur && fp->next && fp->next->length > 100) {
                fp = fp->next;
                curpos++;
                cur = (gdouble)curpos*step;
            }
        }
        goto refresh;
    } else lastval = -100.0;

    if (direction > 0) {
        if (fp->next && fp->next->length > 100) {
            fp = fp->next;
            curpos++;
        }
        if (direction == 2 && fp->next && fp->next->length > 100) {
            fp = fp->next;
            curpos++;
        }
        if (fp->next == NULL || fp->next->length < 100) {
            play = FALSE;
            gtk_button_set_image(GTK_BUTTON(playpause_button), play_img);
        }
    } else if (direction < 0) {
        if (fp->prev && fp->prev->length > 100) {
            fp = fp->prev;
            curpos--;
        }
        if (direction == 2 && fp->prev && fp->prev->length > 100) {
            fp = fp->prev;
            curpos--;
        }
        if (fp->prev == NULL || fp->prev->length < 100) {
            play = FALSE;
            gtk_button_set_image(GTK_BUTTON(playpause_button), play_img);
        }
    }

    gtk_range_set_value(GTK_RANGE(slider), ((gdouble)curpos/(gdouble)framecnt)*100.0);

 refresh:
    widget = GTK_WIDGET(data);
    if (widget->window) {
        region = gdk_drawable_get_clip_region(widget->window);
        gdk_window_invalidate_region(widget->window, region, TRUE);
        gdk_window_process_updates(widget->window, TRUE);
    }
    return TRUE;
}

void create_video_window(void)
{
    static GtkWindow *window;
    GtkWidget
        *vbox,
        *player_widget,
        *hbuttonbox,
        *stop_button,
        *forward_button,
        *next_button,
        *rewind_button,
        *previous_button,
        *view_button;
    gint width;
    gint height;

    gtk_window_get_size(GTK_WINDOW(main_window), &width, &height);

    window = video_window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
    gtk_window_set_title(GTK_WINDOW(window), _("Show Video"));
    if (fullscreen)
        gtk_window_fullscreen(GTK_WINDOW(window));
    else 
        gtk_widget_set_size_request(GTK_WIDGET(window), width, height);

    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    //setup vbox
    vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    //setup player widget
    player_widget = gtk_drawing_area_new();
    gtk_box_pack_start(GTK_BOX(vbox), player_widget, TRUE, TRUE, 0);

    // slider
    slider = gtk_hscale_new_with_range(0.0, 100.0, 10.0);
    gtk_box_pack_start(GTK_BOX(vbox), slider, FALSE, FALSE, 0);

    //setup controls
    //playpause_button = gtk_button_new_from_stock(GTK_STOCK_MEDIA_PLAY);
    playpause_button = gtk_button_new();
    gtk_button_set_image(GTK_BUTTON(playpause_button), pause_img);

    stop_button = gtk_button_new();
    gtk_button_set_image(GTK_BUTTON(stop_button), 
                         gtk_image_new_from_stock(GTK_STOCK_CLOSE, GTK_ICON_SIZE_BUTTON));

    previous_button = gtk_button_new();
    gtk_button_set_image(GTK_BUTTON(previous_button), 
                         gtk_image_new_from_stock(GTK_STOCK_MEDIA_PREVIOUS, GTK_ICON_SIZE_BUTTON));

    forward_button = gtk_button_new();
    gtk_button_set_image(GTK_BUTTON(forward_button), 
                         gtk_image_new_from_stock(GTK_STOCK_MEDIA_FORWARD, GTK_ICON_SIZE_BUTTON));

    next_button = gtk_button_new();
    gtk_button_set_image(GTK_BUTTON(next_button), 
                         gtk_image_new_from_stock(GTK_STOCK_MEDIA_NEXT, GTK_ICON_SIZE_BUTTON));

    rewind_button = gtk_button_new();
    gtk_button_set_image(GTK_BUTTON(rewind_button), 
                         gtk_image_new_from_stock(GTK_STOCK_MEDIA_REWIND, GTK_ICON_SIZE_BUTTON));

    view_button = gtk_button_new();
    gtk_button_set_image(GTK_BUTTON(view_button), gtk_image_new_from_stock(GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_BUTTON));

    g_signal_connect(stop_button, "clicked", G_CALLBACK(close_video), window);
    g_signal_connect(playpause_button, "clicked", G_CALLBACK(on_button), (gpointer)PLAY_PAUSE);
    g_signal_connect(previous_button, "clicked", G_CALLBACK(on_button), (gpointer)PREVIOUS);
    g_signal_connect(forward_button, "clicked", G_CALLBACK(on_button), (gpointer)FORWARD);
    g_signal_connect(next_button, "clicked", G_CALLBACK(on_button), (gpointer)NEXT);
    g_signal_connect(rewind_button, "clicked", G_CALLBACK(on_button), (gpointer)REWIND);
    g_signal_connect(view_button, "clicked", G_CALLBACK(on_button), (gpointer)VIEW);

    hbuttonbox = gtk_hbutton_box_new();
    gtk_container_set_border_width(GTK_CONTAINER(hbuttonbox), 5);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(hbuttonbox), GTK_BUTTONBOX_START);
    gtk_box_pack_start(GTK_BOX(hbuttonbox), view_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbuttonbox), previous_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbuttonbox), rewind_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbuttonbox), playpause_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbuttonbox), forward_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbuttonbox), next_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbuttonbox), stop_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbuttonbox, FALSE, FALSE, 0);

    g_signal_connect(G_OBJECT(player_widget), 
                     "expose-event", G_CALLBACK(expose_video), NULL);

    gtk_widget_show_all(GTK_WIDGET(window));

    g_signal_connect (G_OBJECT(window), "delete_event",
                      G_CALLBACK (delete_event_video), NULL);
    g_signal_connect(G_OBJECT(window), "destroy", 
                     G_CALLBACK(destroy_video), NULL);

    current_media = 1;
    open_media(current_media);

    if (fps == 0)
        fps = 2;

    g_timeout_add(1000/fps, refresh_video, window);
}
