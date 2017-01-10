/* -*- mode: C; c-basic-offset: 4;  -*- */

/*
 * Copyright (C) 2006-2016 by Hannu Jokinen
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

#include <glib.h>
#include "comm.h"

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

#include <sys/time.h>

extern gint websock_send_msg(gint fd, struct message *msg);
extern void handle_websock(struct jsconn *conn, char *in, gint length);
extern void serial_set_device(gchar *dev);
extern void serial_set_baudrate(gint baud);
extern void serial_set_type(gint type);

G_LOCK_DEFINE(send_mutex);
G_LOCK_DEFINE_STATIC(rec_mutex);

void send_packet(struct message *msg);

static struct message next_match_messages[NUM_TATAMIS];
static gboolean tatami_state[NUM_TATAMIS];

struct message hello_message;

#define NUM_CONNECTIONS 8
static struct jsconn connections[NUM_CONNECTIONS];

static struct {
    gulong addr;
    gint id;
    gchar ssdp_info[SSDP_INFO_LEN];
} noconnections[NUM_CONNECTIONS];

//static volatile struct message message_queue[MSG_QUEUE_LEN];
//static volatile gint msg_put = 0, msg_get = 0;
//static GStaticMutex msg_mutex = G_STATIC_MUTEX_INIT;

volatile gint msg_queue_put = 0, msg_queue_get = 0;
struct message msg_to_send[MSG_QUEUE_LEN];

void msg_received(struct message *input_msg)
{
    struct message output_msg;
    guint now, i;
    gboolean newentry = FALSE;
    gulong addr = input_msg->src_ip_addr;
    gchar  buf[16];

#if 0
    if (input_msg->type != MSG_HELLO)
        g_print("msg type = %d from %lx (my addr = %lx)\n", input_msg->type, addr, my_address);
#endif

    switch (input_msg->type) {
    case MSG_HELLO:
        break;

    case MSG_SCALE:
        break;
    } // switch
}

struct message *put_to_rec_queue(volatile struct message *msg)
{
    struct message m;
    m = *msg;

    if (m.type == MSG_SCALE) {
	char *p = strtok(m.u.scale.config, ";");
	while (p) {
	    if (!strncmp(p, "type=", 5)) {
		serial_set_type(atoi(p+5));
	    } else if (!strncmp(p, "baud=", 5)) {
		serial_set_baudrate(atoi(p+5));
	    } else if (!strncmp(p, "dev=", 4)) {
		serial_set_device(p+4);
	    }
	    p = strtok(NULL, ";");
	}
    }
}

void msg_to_queue(struct message *msg)
{
    G_LOCK(send_mutex);
    msg_to_send[msg_queue_put] = *msg;
    msg_queue_put++;
    if (msg_queue_put >= MSG_QUEUE_LEN)
        msg_queue_put = 0;
    G_UNLOCK(send_mutex);
}

void send_packet(struct message *msg)
{
    msg->sender = 102;
    msg_to_queue(msg);
}

gpointer websock_thread(gpointer args)
{
    SOCKET tmp_fd, websock_fd;
    socklen_t alen;
    struct sockaddr_in my_addr, caller;
    gint reuse = 1;
    fd_set read_fd, fds;
    struct message msg_out;
    gboolean msg_out_ready;

#ifndef WIN32
    signal(SIGPIPE, SIG_IGN);
#endif

    /* WebSock socket */
    if ((websock_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        perror("serv socket");
        g_thread_exit(NULL);    /* not required just good pratice */
        return NULL;
    }

    if (setsockopt(websock_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&reuse, sizeof(reuse)) < 0) {
        perror("setsockopt (SO_REUSEADDR)");
    }

    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(SERIAL_PORT);
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(websock_fd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) < 0) {
        perror("websock bind");
        g_print("CANNOT BIND websock!\n");
        g_thread_exit(NULL);    /* not required just good pratice */
        return NULL;
    }

    /***/

    listen(websock_fd, 5);

    FD_ZERO(&read_fd);
    FD_SET(websock_fd, &read_fd);

    for ( ; *((gboolean *)args) ; )   /* exit loop when flag is cleared */
    {
        struct timeval timeout;
        gint r, i;

        fds = read_fd;
        timeout.tv_sec = 0;
        timeout.tv_usec = 1000;

        r = select(64, &fds, NULL, NULL, &timeout);

        /* messages to send */

        // Mutex may be locked for a long time if there are network problems
        // during send. Thus we send a copy to unlock the mutex immediatelly.
        msg_out_ready = FALSE;

        G_LOCK(send_mutex);

        if (msg_queue_get != msg_queue_put) {
            msg_out = msg_to_send[msg_queue_get];
            msg_queue_get++;
            if (msg_queue_get >= MSG_QUEUE_LEN)
                msg_queue_get = 0;
	    msg_out_ready = TRUE;
        }

        G_UNLOCK(send_mutex);

        if (msg_out_ready) {
	    int ret;

            for (i = 0; i < NUM_CONNECTIONS; i++) {
                if (connections[i].fd == 0)
                    continue;

		ret = 0;
		if (connections[i].websock_ok)
		    ret = websock_send_msg(connections[i].fd, &msg_out);

                if (ret < 0) {
                    perror("sendto");
                    g_print("Node cannot send: conn=%d fd=%d\n", i, connections[i].fd);

#if defined(__WIN32__) || defined(WIN32)
		    shutdown(connections[i].fd, SD_SEND);
#endif
		    closesocket(connections[i].fd);
		    FD_CLR(connections[i].fd, &read_fd);
		    connections[i].fd = 0;
                }
            }
        }

        if (r <= 0)
            continue;

        /* messages to receive */

        if (FD_ISSET(websock_fd, &fds)) {
            alen = sizeof(caller);
            if ((tmp_fd = accept(websock_fd, (struct sockaddr *)&caller, &alen)) < 0) {
                perror("serialsock accept");
		g_print("serialsock=%d tmpfd=%d\n", websock_fd, tmp_fd);
		usleep(1000000);
                continue;
            }

            for (i = 0; i < NUM_CONNECTIONS; i++)
                if (connections[i].fd == 0)
                    break;

            if (i >= NUM_CONNECTIONS) {
                g_print("Node cannot accept new connections!\n");
                closesocket(tmp_fd);
                continue;
            }

            connections[i].fd = tmp_fd;
            connections[i].addr = caller.sin_addr.s_addr;
            connections[i].id = 0;
            connections[i].conn_type = 0;
            connections[i].websock = TRUE;
            g_print("New serialsock connection[%d]: fd=%d addr=%s\n",
                    i, tmp_fd, inet_ntoa(caller.sin_addr));
            FD_SET(tmp_fd, &read_fd);
        }

        for (i = 0; i < NUM_CONNECTIONS; i++) {
            static guchar inbuf[2000];

            if (connections[i].fd == 0)
                continue;

            if (!(FD_ISSET(connections[i].fd, &fds)))
                continue;

            r = recv(connections[i].fd, (char *)inbuf, sizeof(inbuf), 0);
            if (r > 0) {
		handle_websock(&connections[i], (gchar *)inbuf, r);
            } else {
                g_print("Connection %d fd=%d closed (r=%d, err=%s)\n",
			i, connections[i].fd, r, strerror(errno));
#if defined(__WIN32__) || defined(WIN32)
		shutdown(connections[i].fd, SD_SEND);
#endif
                closesocket(connections[i].fd);
                FD_CLR(connections[i].fd, &read_fd);
                connections[i].fd = 0;
            }
        }
    }

    g_thread_exit(NULL);    /* not required just good pratice */
    return NULL;
}
