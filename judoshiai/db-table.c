/* -*- mode: C; c-basic-offset: 4;  -*- */

/*
 * Copyright (C) 2006-2011 by Hannu Jokinen
 * Full copyright text is included in the software package.
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <assert.h>

#include "sqlite3.h"
#include "judoshiai.h"

G_LOCK_EXTERN(db);

static char **tablep = NULL;
static int tablerows, tablecols;
static GStaticMutex table_mutex = G_STATIC_MUTEX_INIT;

int db_get_table(char *command)
{
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;

    g_static_mutex_lock(&table_mutex);

    if (tablep) {
        g_print("ERROR: TABLE NOT CLOSED BEFORE OPENING!\n");
        sqlite3_free_table(tablep);
    }
    tablep = NULL;

    G_LOCK(db);

    rc = sqlite3_open(db_name, &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", 
                sqlite3_errmsg(db));
        G_UNLOCK(db);
        g_static_mutex_unlock(&table_mutex);
        return -1;
    }

    //g_print("\nSQL: %s:\n  %s\n", db_name, cmd);
    rc = sqlite3_get_table(db, command, &tablep, &tablerows, &tablecols, &zErrMsg);
    if (rc != SQLITE_OK && zErrMsg) {
        g_print("SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        sqlite3_close(db);
        G_UNLOCK(db);
        g_static_mutex_unlock(&table_mutex);
        return -1;
    }

    sqlite3_close(db);
    G_UNLOCK(db);

    return tablerows;
}

void db_close_table(void)
{
    if (tablep)
        sqlite3_free_table(tablep);
    else
        g_print("ERROR: TABLE CLOSED TWICE!\n");
    tablep = NULL;
    g_static_mutex_unlock(&table_mutex);
}

char *db_get_data(int row, char *name)
{
    return db_get_col_data(tablep, tablecols, row, name);
}

gchar *db_get_col_data(gchar **data, gint numcols, gint row, gchar *name)
{
    int col;

    for (col = 0; col < numcols; col++)
        if (strcmp(data[col], name) == 0)
            break;
    
    if (col >= numcols)
        return NULL;
    
    return data[(row+1)*numcols + col];
}

