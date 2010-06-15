/* -*- mode: C; c-basic-offset: 4;  -*- */

/*
 * Copyright (C) 2006-2010 by Hannu Jokinen
 * Full copyright text is included in the software package.
 */ 

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <gtk/gtk.h>

#include "judotimer.h"
#include "fmod.h"
#include "fmod_errors.h"

static FMOD_SYSTEM      *fmodsystem;
static FMOD_SOUND       *fmodsound = NULL;
static FMOD_CHANNEL     *fmodchannel = NULL;
static FMOD_RESULT       result;

static gchar *sound_file = NULL;

static void ERRCHECK(FMOD_RESULT result)
{
    if (result != FMOD_OK)
        g_print("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
}

void open_sound(void)
{
    guint version;

    result = FMOD_System_Create(&fmodsystem);
    ERRCHECK(result);

    result = FMOD_System_GetVersion(fmodsystem, &version);
    ERRCHECK(result);

    if (version < FMOD_VERSION) {
        g_print("Error! You are using an old version of FMOD %08x.  This program requires %08x\n", version, FMOD_VERSION);
    }

    result = FMOD_System_Init(fmodsystem, 32, FMOD_INIT_NORMAL, NULL);
    ERRCHECK(result);

    GError *error = NULL;
    gchar  *str;
    if ((str = g_key_file_get_string(keyfile, "preferences", "soundfile", &error))) {
        if (str[0]) {
            sound_file = str;
            result = FMOD_System_CreateSound(fmodsystem, sound_file, FMOD_SOFTWARE, 0, &fmodsound);
            ERRCHECK(result);
            if (result != FMOD_OK) {
                g_free(sound_file);
                sound_file = NULL;
            }
        } else
            g_free(str);
    }
}

void close_sound(void)
{
    if (fmodsound) {
        result = FMOD_Sound_Release(fmodsound);
        ERRCHECK(result);
    }
    result = FMOD_System_Close(fmodsystem);
    ERRCHECK(result);
    result = FMOD_System_Release(fmodsystem);
    ERRCHECK(result);
}

void play_sound(void)
{
    if (sound_file && fmodsound) {
        result = FMOD_System_PlaySound(fmodsystem, FMOD_CHANNEL_FREE, fmodsound, 0, &fmodchannel);
        ERRCHECK(result);
    }
}

void select_sound(GtkWidget *menu_item, gpointer data)
{
    GtkWidget *dialog;
    GtkFileFilter *filter, *filter_all;

    dialog = gtk_file_chooser_dialog_new(_("Choose a file"),
                                         NULL,
                                         GTK_FILE_CHOOSER_ACTION_OPEN,
                                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                         "Stop Sound", 1000,
                                         GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                         NULL);

    filter = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filter, "*.[mM][pP][1-3]");
    gtk_file_filter_add_pattern(filter, "*.[wW][aA][vV]");
    gtk_file_filter_add_pattern(filter, "*.[oO][gG][gG]");
    gtk_file_filter_set_name(filter, _("Sound Files"));
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    filter_all = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filter_all, "*");
    gtk_file_filter_set_name(filter_all, _("All Files"));
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter_all);

    gchar *dir = g_build_filename(installation_dir, "etc", NULL);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), dir);
    g_free(dir);

    gint r = gtk_dialog_run(GTK_DIALOG(dialog));

    if (r == 1000) { // stop sound
        g_free(sound_file);
        sound_file = NULL;
        if (fmodsound) {
            result = FMOD_Sound_Release(fmodsound);
            ERRCHECK(result);
            fmodsound = NULL;
        }
        g_key_file_set_string(keyfile, "preferences", "soundfile", "");
        gtk_widget_destroy(dialog);
        return;
    }

    if (r != GTK_RESPONSE_ACCEPT) {
        gtk_widget_destroy(dialog);
        return;
    }
               
    if (fmodsound) {
        result = FMOD_Sound_Release(fmodsound);
        ERRCHECK(result);
        fmodsound = NULL;
    }

    g_free(sound_file);
    sound_file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
    g_key_file_set_string(keyfile, "preferences", "soundfile", sound_file);

    gtk_widget_destroy(dialog);

    result = FMOD_System_CreateSound(fmodsystem, sound_file, FMOD_SOFTWARE, 0, &fmodsound);
    ERRCHECK(result);
}
