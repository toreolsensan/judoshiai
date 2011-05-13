/* -*- mode: C; c-basic-offset: 4;  -*- */

/*
 * Copyright (C) 2006-2011 by Hannu Jokinen
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

#include <gtk/gtk.h>
#include <glib/gprintf.h>

#include "judoshiai.h"


static gchar serial_device[32] = {0};
static gint  serial_baudrate = 0; // index to a table
static gboolean serial_changed = FALSE;       
gboolean serial_used = FALSE;

static void handle_character(gchar c)
{
    static gint weight = 0;
    static gint decimal = 0;
        
    if (c == '.' || c == ',') {
        decimal = 1;
    } else if (c < '0' || c > '9') {
        if (c == '\r' && weight_entry && weight < 300000) {
            struct message msg;
            memset(&msg, 0, sizeof(msg));
            msg.type = MSG_SCALE;
            msg.u.scale.weight = weight;
            put_to_rec_queue(&msg);            
        }
        weight = 0;
        decimal = 0;
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

        if (!serial_used)
            continue;

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
            if (ReadFile(hSerial, szBuff, 31, &dwBytesRead, NULL)) {
                szBuff[dwBytesRead] = 0;
                for (i = 0; i < dwBytesRead; i++)
                    handle_character(szBuff[i]);
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

        if (!serial_used)
            continue;

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
         
        newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
        newtio.c_cc[VMIN]     = 1;   /* blocking read until 1 char received */
        
        tcflush(fd, TCIFLUSH);
        tcsetattr(fd,TCSANOW,&newtio);
        
        while (*((gboolean *)args) && serial_device[0] && serial_changed == FALSE) {       /* loop for input */
            res = read(fd, buf, sizeof(buf)-1);   /* returns after 1 char has been input */
            buf[res] = 0;               /* so we can printf... */
            for (i = 0; i < res; i++)
                handle_character(buf[i]);
        }

        tcsetattr(fd,TCSANOW,&oldtio);
        close(fd);
    }

    g_thread_exit(NULL);    /* not required just good pratice */
    return NULL;
}
#endif

void set_serial_dialog(GtkWidget *w, gpointer data)
{
    gint i;
    GtkWidget *dialog, *used, *device, *baudrate, *table;

    dialog = gtk_dialog_new_with_buttons (_("Serial Communication"),
                                          main_window,
                                          GTK_DIALOG_MODAL,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_OK, GTK_RESPONSE_OK,
                                          NULL);

    table = gtk_table_new(2, 2, FALSE);

    used = gtk_check_button_new();
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(used), serial_used);

    device = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(device), 20);
    gtk_entry_set_text(GTK_ENTRY(device), serial_device[0] ? serial_device : "COM1");

    baudrate = gtk_combo_box_new_text();
    for (i = 0; i < NUM_BAUDRATES; i++)
        gtk_combo_box_append_text((GtkComboBox *)baudrate, serial_baudrates[i].text);
    gtk_combo_box_set_active((GtkComboBox *)baudrate, serial_baudrate);

    gtk_table_attach_defaults(GTK_TABLE(table), gtk_label_new(_("In Use")), 0, 1, 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(table), used, 1, 2, 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(table), gtk_label_new(_("Device")), 0, 1, 1, 2);
    gtk_table_attach_defaults(GTK_TABLE(table), device, 1, 2, 1, 2);
    gtk_table_attach_defaults(GTK_TABLE(table), gtk_label_new(_("Baudrate")), 0, 1, 2, 3);
    gtk_table_attach_defaults(GTK_TABLE(table), baudrate, 1, 2, 2, 3);

    gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(dialog)->vbox), table);
    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        strncpy(serial_device, gtk_entry_get_text(GTK_ENTRY(device)), sizeof(serial_device)-1);
        serial_baudrate = gtk_combo_box_get_active(GTK_COMBO_BOX(baudrate));
        g_key_file_set_string(keyfile, "preferences", "serialdevice", serial_device);
        g_key_file_set_integer(keyfile, "preferences", "serialbaudrate", serial_baudrate);
        serial_used = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(used));
        serial_changed = TRUE;
    }

    gtk_widget_destroy(dialog);
}

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
