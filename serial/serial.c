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

#else /* UNIX */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>

#endif /* WIN32 */

#include <glib/gprintf.h>

#include "comm.h"

#define _(String) (String)
#define N_(String) (String)
#define textdomain(String) (String)
#define gettext(String) (String)
#define dgettext(Domain,String) (String)
#define dcgettext(Domain,String,Type) (String)
#define bindtextdomain(Domain,Directory) (Domain)
#define bind_textdomain_codeset(Domain,Codeset) (Codeset)

extern void send_packet(struct message *msg);

gboolean debug = FALSE;

#define DEV_TYPE_NORMAL   0
#define DEV_TYPE_STATHMOS 1
#define DEV_TYPE_AP1      2
#define DEV_TYPE_MYWEIGHT 3

#define NUM_BAUDRATES 5
struct baudrates {
    gint value;
    gchar *text;
};

static gchar serial_device[32] = {0};
static gint  serial_baudrate = 0; // index to a table
static gint device_type = 0;
static gboolean serial_changed = FALSE;

static gchar handle_character(gchar c)
{
    static gint weight = 0;
    static gint decimal = 0;

    if (c == '.' || c == ',') {
        decimal = 1;
    } else if (device_type == DEV_TYPE_AP1 && c == 0x06) { // ACK
        return 0x11; // DC1
    } else if (c < '0' || c > '9') {
        if ((device_type == DEV_TYPE_NORMAL   && c == '\r') ||
            (device_type == DEV_TYPE_STATHMOS && c == 0x1e) ||
            (device_type == DEV_TYPE_AP1      && (c == 'k' || c == 0x04 || c == 0x03)) ||
            (device_type == DEV_TYPE_MYWEIGHT && (c == 'k' || c == '\r'))) {

            if (weight > 5000 && weight < 300000) {
                struct message msg;
                memset(&msg, 0, sizeof(msg));
                msg.type = MSG_SCALE;
                msg.u.scale.weight = weight;
                send_packet(&msg);
		if (debug) g_print("send weight %d\n", weight);
            }
            weight = 0;
            decimal = 0;
        }
    } else if (device_type == DEV_TYPE_STATHMOS) {
        weight = 10*weight + (c - '0');
    } else if (decimal == 0) {
        weight = 10*weight + 1000*(c - '0');
    } else {
        switch (decimal) {
        case 1: weight += 100*(c - '0'); break;
        case 2: weight += 10*(c - '0'); break;
        case 3: weight += c - '0'; break;
        }
        decimal++;
    }
    //g_print("weight=%d\n", weight);
    return 0;
}

static gchar send_chars(void)
{
    static time_t last_send = 0;

    if (device_type == DEV_TYPE_NORMAL)
        return 0;

    if (time(NULL) == last_send)
        return 0;

    last_send = time(NULL);

    if (device_type == DEV_TYPE_STATHMOS)
        return 0x05;

    if (device_type == DEV_TYPE_AP1)
        return 0x05;

    if (device_type == DEV_TYPE_MYWEIGHT)
        return 0x0d;

    return 0;
}

#if defined(__WIN32__) || defined(WIN32)

struct baudrates serial_baudrates[NUM_BAUDRATES] = {
    {CBR_1200, "1200,N81"}, {CBR_9600, "9600,N81"}, {CBR_19200, "19200,N81"}, {CBR_38400, "38400,N81"},
    {CBR_115200, "115200,N81"}
};

#define ERR_SER g_print("Error %s:%d\n", __FUNCTION__, __LINE__)

gpointer serial_thread(gpointer args)
{
    gint i;
    char szBuff[32];
    DWORD dwBytesRead;

    while (*((gboolean *)args)) {
        serial_changed = FALSE;

        g_usleep(500000);

        if (serial_device[0] == 0)
            continue;

        HANDLE hSerial;
        hSerial = CreateFile(serial_device,
                             GENERIC_READ | GENERIC_WRITE,
                             0,
                             0,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL,
                             0);
        if(hSerial==INVALID_HANDLE_VALUE){
            if(GetLastError()==ERROR_FILE_NOT_FOUND){
                //serial port does not exist. Inform user.
                ERR_SER;
            }
            //some other error occurred. Inform user.
            ERR_SER;
            continue;
        }

        DCB dcbSerialParams = {0};
        dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
        if (!GetCommState(hSerial, &dcbSerialParams)) {
            //error getting state
            ERR_SER;
            continue;
        }
        dcbSerialParams.BaudRate = serial_baudrates[serial_baudrate].value;
        dcbSerialParams.ByteSize = 8;
        dcbSerialParams.StopBits = ONESTOPBIT;
        dcbSerialParams.Parity = NOPARITY;
        if(!SetCommState(hSerial, &dcbSerialParams)) {
            //error setting serial port state
            ERR_SER;
            continue;
        }

        COMMTIMEOUTS timeouts = {0};
        timeouts.ReadIntervalTimeout=50;
        timeouts.ReadTotalTimeoutConstant=50;
        timeouts.ReadTotalTimeoutMultiplier=10;
        timeouts.WriteTotalTimeoutConstant=50;
        timeouts.WriteTotalTimeoutMultiplier=10;
        if(!SetCommTimeouts(hSerial, &timeouts)) {
            //error occureed. Inform user
            ERR_SER;
            continue;
        }

        while (*((gboolean *)args) && serial_device[0] && serial_changed == FALSE) {       /* loop for input */
            gchar ch = 0, ch2 = 0;

            if (ReadFile(hSerial, szBuff, 31, &dwBytesRead, NULL)) {
                szBuff[dwBytesRead] = 0;
                for (i = 0; i < dwBytesRead; i++) {
                    ch2 = handle_character(szBuff[i]);
                    if (ch2 && ch == 0) ch = ch2;
                }
            }

            if (!ch) ch = send_chars();

            if (ch) {
                szBuff[0] = ch;
                if(!WriteFile(hSerial, szBuff, 1, &dwBytesRead, NULL))
                    g_print("Cannot write to COM!\n");
            }
        }

        CloseHandle(hSerial);
    } // while

    g_thread_exit(NULL);    /* not required just good pratice */
    return NULL;
}

#else // Linux

struct baudrates serial_baudrates[NUM_BAUDRATES] = {
    {B1200, "1200,N81"}, {B9600, "9600,N81"}, {B19200, "19200,N81"}, {B38400, "38400,N81"},
    {B115200, "115200,N81"}
};

gpointer serial_thread(gpointer args)
{
    gint fd, res, i;
    struct termios oldtio, newtio;
    gchar buf[32];

    while (*((gboolean *)args)) {
        serial_changed = FALSE;

        g_usleep(500000);

        if (serial_device[0] == 0)
            continue;

        fd = open(serial_device, O_RDWR | O_NOCTTY );
        if (fd < 0)
            continue;

        tcgetattr(fd,&oldtio); /* save current port settings */

        bzero(&newtio, sizeof(newtio));
        newtio.c_cflag = serial_baudrates[serial_baudrate].value | CRTSCTS | CS8 | CLOCAL | CREAD;
        newtio.c_iflag = IGNPAR;
        newtio.c_oflag = 0;

        /* set input mode (non-canonical, no echo,...) */
        newtio.c_lflag = 0;

        newtio.c_cc[VTIME]    = 2;   /* inter-character timer in 0.1 sec */
        newtio.c_cc[VMIN]     = 0;   /* blocking read until 1 char received */

        tcflush(fd, TCIFLUSH);
        tcsetattr(fd,TCSANOW,&newtio);

        while (*((gboolean *)args) && serial_device[0] && serial_changed == FALSE) {       /* loop for input */
            gchar ch = 0, ch2 = 0;

            res = read(fd, buf, sizeof(buf)-1);   /* returns after 1 char has been input */

            for (i = 0; i < res; i++) {
		if (debug) g_print(" %02x", buf[i]);
                ch2 = handle_character(buf[i]);
                if (ch2 && ch == 0) ch = ch2;
            }

            if (!ch) ch = send_chars();

            if (ch) {
                buf[0] = ch;
                write(fd, buf, 1);
            }
        }

        tcsetattr(fd,TCSANOW,&oldtio);
        close(fd);
    }

    g_thread_exit(NULL);    /* not required just good pratice */
    return NULL;
}
#endif

void serial_set_device(gchar *dev)
{
    strncpy(serial_device, dev, sizeof(serial_device)-1);
    serial_changed = TRUE;
}

void serial_set_baudrate(gint baud)
{
    serial_baudrate = baud;
    serial_changed = TRUE;
}

void serial_set_type(gint type)
{
    device_type = type;
    serial_changed = TRUE;
}
