/* -*- mode: C; c-basic-offset: 4;  -*- */

/*
 * Copyright (C) 2006-2010 by Hannu Jokinen
 * Full copyright text is included in the software package.
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <assert.h>

#include "sqlite3.h"
#include "judoshiai.h"

//#define TATAMI_DEBUG 3

gchar info_competition[100];
gchar info_date[100];
gchar info_place[100];
gboolean three_matches_for_two = 0;
gchar info_time[100];
gchar info_num_tatamis[20];

gchar database_name[200] = {0};

const char *db_name;

G_LOCK_DEFINE(db);

gint db_exec(const char *dbn, char *cmd, void *data, void *dbcb)
{
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;

    if (db_name == dbn)
	G_LOCK(db);

    rc = sqlite3_open(dbn, &db);
    if (rc) {
	fprintf(stderr, "Can't open database: %s\n", 
		sqlite3_errmsg(db));
	sqlite3_close(db);
	if (db_name == dbn)
	    G_UNLOCK(db);
	exit(1);
    }

    //g_print("\nSQL: %s:\n  %s\n", db_name, cmd);
    rc = sqlite3_exec(db, cmd, dbcb, data, &zErrMsg);

    if (rc != SQLITE_OK && rc != SQLITE_ABORT && zErrMsg) {
	g_print("SQL error: %s\n%s\n", zErrMsg, cmd);
    }

    if (zErrMsg)
	sqlite3_free(zErrMsg);

    sqlite3_close(db);

    if (db_name == dbn)
	G_UNLOCK(db);

    return rc;
}

void db_exec_str(void *data, void *dbcb, gchar *format, ...)
{
    va_list args;
    va_start(args, format);
    gchar *text = g_strdup_vprintf(format, args);
    va_end(args);
    db_exec(db_name, text, data, dbcb);
    g_free(text);
}

static sqlite3 *maindb;

void db_close(void)
{
    if (!maindb) {
	g_print("%s: maindb was not open!\n", __FUNCTION__);
	return;
    }

    sqlite3_close(maindb);
    maindb = NULL;
    G_UNLOCK(db);
}

gint db_open(void)
{
    if (maindb) {
	g_print("%s: maindb was already open!\n", __FUNCTION__);
	return -1;
    }

    if (!db_name) {
	g_print("%s: db_name is null!\n", __FUNCTION__);
	return -2;
    }

    G_LOCK(db);

    gint rc = sqlite3_open(db_name, &maindb);

    if (rc) {
	g_print("%s: cannot open maindb (%s)!\n", __FUNCTION__, 
		sqlite3_errmsg(maindb));
	sqlite3_close(maindb);
	maindb = NULL;
	G_UNLOCK(db);
    }

    return rc;
}

gint db_cmd(void *data, void *dbcb, gchar *format, ...)
{
    char *zErrMsg = NULL;
    va_list args;
    va_start(args, format);
    gchar *text = g_strdup_vprintf(format, args);
    va_end(args);

    if (!maindb) {
	g_print("db_cmd SQL error: maindb is null\n");
	return -1;
    }

    gint rc = sqlite3_exec(maindb, text, dbcb, data, &zErrMsg);

    if (rc != SQLITE_OK && rc != SQLITE_ABORT && zErrMsg) {
	g_print("db_cmd SQL error: %s\n%s\n", zErrMsg, text);
    }

    if (zErrMsg)
	sqlite3_free(zErrMsg);

    g_free(text);

    return rc;
}

static int db_info_cb(void *data, int argc, char **argv, char **azColName)
{
    if (strcmp(argv[0], "Competition") == 0)
        strcpy(info_competition, argv[1]);
    else if (strcmp(argv[0], "Date") == 0)
        strcpy(info_date, argv[1]);
    else if (strcmp(argv[0], "Place") == 0)
        strcpy(info_place, argv[1]);
    else if (strcmp(argv[0], "three_matches_for_two") == 0)
        three_matches_for_two = atoi(argv[1]);
    else if (strcmp(argv[0], "Time") == 0)
        strcpy(info_time, argv[1]);
    else if (strcmp(argv[0], "NumTatamis") == 0) {
        strcpy(info_num_tatamis, argv[1]);
        number_of_tatamis = atoi(info_num_tatamis);
        if (number_of_tatamis < 1)
            number_of_tatamis = 1;
        if (number_of_tatamis > NUM_TATAMIS)
            number_of_tatamis = NUM_TATAMIS;
    }

    return 0;
}

static gboolean tatami_exists, number_exists, country_exists, id_exists;

static int db_callback_tables(void *data, int argc, char **argv, char **azColName)
{
    int i;

    for (i = 0; i < argc; i++) {
        if (IS(sql)) {
            if (strstr(argv[i], "matches") && strstr(argv[i], "forcedtatami"))
                tatami_exists = TRUE;
            if (strstr(argv[i], "matches") && strstr(argv[i], "forcednumber"))
                number_exists = TRUE;
            if (strstr(argv[i], "competitors") && strstr(argv[i], "country"))
                country_exists = TRUE;
            if (strstr(argv[i], "competitors") && strstr(argv[i], "\"id\""))
                id_exists = TRUE;
        }
    }

    return 0;
}

void db_init(const char *dbname)
{
    db_matches_init();
	
    init_trees();

    db_name = dbname;

    db_exec(db_name, "SELECT * FROM \"info\"", NULL, db_info_cb);

    set_configuration();

    snprintf(hello_message.u.hello.info_competition, 
             sizeof(hello_message.u.hello.info_competition), 
             "%s", info_competition);
    snprintf(hello_message.u.hello.info_date, 
             sizeof(hello_message.u.hello.info_date), 
             "%s", info_date);
    snprintf(hello_message.u.hello.info_place, 
             sizeof(hello_message.u.hello.info_place), 
             "%s", info_place);

    read_cat_definitions();

    tatami_exists = number_exists = country_exists = id_exists = FALSE;
    db_exec(db_name, 
            "SELECT sql FROM sqlite_master", 
            NULL, db_callback_tables);
    if (!tatami_exists) {
        g_print("forcedtatami does not exist, add one\n");
        db_exec(db_name, "ALTER TABLE matches ADD \"forcedtatami\" INTEGER", NULL, NULL);
        db_exec(db_name, "UPDATE matches SET 'forcedtatami'=0", NULL, NULL);
    }
    if (!number_exists) {
        g_print("forcednumber does not exist, add one\n");
        db_exec(db_name, "ALTER TABLE matches ADD \"forcednumber\" INTEGER", NULL, NULL);
        db_exec(db_name, "UPDATE matches SET 'forcednumber'=0", NULL, NULL);
    }
    if (!country_exists) {
        g_print("country does not exist, add one\n");
        db_exec(db_name, "ALTER TABLE competitors ADD \"country\" TEXT", NULL, NULL);
    }
    if (!id_exists) {
        g_print("id does not exist, add one\n");
        db_exec(db_name, "ALTER TABLE competitors ADD \"id\" TEXT", NULL, NULL);
    }
}

void db_new(const char *dbname, 
            const gchar *competition, 
            const gchar *date,
            const gchar *place,
	    const gchar *start_time,
	    const gchar *num_tatamis)
{
    gchar buf[100];
    FILE *f;

    char *cmd1 = "CREATE TABLE competitors ("
        "\"index\" INTEGER, \"last\" TEXT, \"first\" TEXT, \"birthyear\" INTEGER, "
        "\"belt\" INTEGER, \"club\" TEXT, \"regcategory\" TEXT, "
        "\"weight\" INTEGER, \"visible\" INTEGER, "
        "\"category\" TEXT, \"deleted\" INTEGER, "
        "\"country\" TEXT, \"id\" TEXT )";
    char *cmd3 = "CREATE TABLE categories ("
        "\"index\" INTEGER, \"category\" TEXT, \"tatami\" INTEGER, "
        "\"deleted\" INTEGER, \"group\" INTEGER, \"system\" INTEGER "
        ")";
    char *cmd4 = "CREATE TABLE matches ("
        "\"category\" INTEGER, \"number\" INTEGER,"
        "\"blue\" INTEGER, \"white\" INTEGER,"
        "\"blue_score\" INTEGER, \"white_score\" INTEGER, "
        "\"blue_points\" INTEGER, \"white_points\" INTEGER, "
        "\"time\" INTEGER, \"comment\" INTEGER, \"deleted\" INTEGER, "
        "\"forcedtatami\" INTEGER, \"forcednumber\" INTEGER)";
    char *cmd5 = "CREATE TABLE \"info\" ("
        "\"item\" TEXT, \"value\" TEXT )";
	
    db_name = dbname;

    strcpy(info_competition, competition);
    strcpy(info_date, date);
    strcpy(info_place, place);
    strcpy(info_time, start_time);
    strcpy(info_num_tatamis, num_tatamis);

    if (FALSE && (f = fopen(db_name, "rb"))) {
        /* exists */
        fclose(f);

        sprintf(buf, "UPDATE \"info\" SET "
                "\"value\"=\"%s\" WHERE \"item\"=\"Competition\"",
                competition);
        db_exec(db_name, buf, NULL, NULL);

        sprintf(buf, "UPDATE \"info\" SET "
                "\"value\"=\"%s\" WHERE \"item\"=\"Date\"",
                date);
        db_exec(db_name, buf, NULL, NULL);

        sprintf(buf, "UPDATE \"info\" SET "
                "\"value\"=\"%s\" WHERE \"item\"=\"Place\"",
                place);
        db_exec(db_name, buf, NULL, NULL);

        sprintf(buf, "UPDATE \"info\" SET "
                "\"value\"=\"%s\" WHERE \"item\"=\"Time\"",
                start_time);
        db_exec(db_name, buf, NULL, NULL);

        sprintf(buf, "UPDATE \"info\" SET "
                "\"value\"=\"%s\" WHERE \"item\"=\"NumTatamis\"",
                num_tatamis);
        db_exec(db_name, buf, NULL, NULL);

        return;
    }

    db_exec(db_name, cmd1, NULL, NULL);
    db_exec(db_name, cmd3, NULL, NULL);
    db_exec(db_name, cmd4, NULL, NULL);

    db_exec(db_name, cmd5, NULL, NULL);

    sprintf(buf, "INSERT INTO \"info\" VALUES ("
            " \"Competition\", \"%s\" )", competition);
    db_exec(db_name, buf, NULL, NULL);

    sprintf(buf, "INSERT INTO \"info\" VALUES ("
            " \"Date\", \"%s\" )", date);
    db_exec(db_name, buf, NULL, NULL);

    sprintf(buf, "INSERT INTO \"info\" VALUES ("
            " \"Place\", \"%s\" )", place);
    db_exec(db_name, buf, NULL, NULL);

    sprintf(buf, "INSERT INTO \"info\" VALUES ("
            " \"three_matches_for_two\", \"0\" )");
    db_exec(db_name, buf, NULL, NULL);

    sprintf(buf, "INSERT INTO \"info\" VALUES ("
            " \"Time\", \"%s\" )", start_time);
    db_exec(db_name, buf, NULL, NULL);

    sprintf(buf, "INSERT INTO \"info\" VALUES ("
            " \"NumTatamis\", \"%s\" )", num_tatamis);
    db_exec(db_name, buf, NULL, NULL);

    number_of_tatamis = atoi(num_tatamis);
    if (number_of_tatamis < 1)
        number_of_tatamis = 1;
    if (number_of_tatamis > NUM_TATAMIS)
        number_of_tatamis = NUM_TATAMIS;
}

void db_set_info(const gchar *competition, 
		 const gchar *date,
		 const gchar *place,
		 const gchar *start_time,
		 const gchar *num_tatamis)
{
    char buffer[300];

    g_snprintf(buffer, sizeof(buffer), 
               "UPDATE \"info\" SET "
               "\"value\"=\"%s\" WHERE \"item\"=\"Competition\"", competition);
    db_exec(db_name, buffer, NULL, NULL);

    g_snprintf(buffer, sizeof(buffer), 
               "UPDATE \"info\" SET "
               "\"value\"=\"%s\" WHERE \"item\"=\"Date\"", date);
    db_exec(db_name, buffer, NULL, NULL);

    g_snprintf(buffer, sizeof(buffer), 
               "UPDATE \"info\" SET "
               "\"value\"=\"%s\" WHERE \"item\"=\"Place\"", place);
    db_exec(db_name, buffer, NULL, NULL);

    /* Time row doesn't exist in old databases. Check if this is update or insert. */
    if (info_time[0])
        g_snprintf(buffer, sizeof(buffer), 
                   "UPDATE \"info\" SET "
                   "\"value\"=\"%s\" WHERE \"item\"=\"Time\"", start_time);
    else if (start_time && start_time[0])
        g_snprintf(buffer, sizeof(buffer), 
                   "INSERT INTO \"info\" VALUES ("
                   " \"Time\", \"%s\" )", start_time);

    db_exec(db_name, buffer, NULL, NULL);

    if (info_num_tatamis[0])
        g_snprintf(buffer, sizeof(buffer), 
                   "UPDATE \"info\" SET "
                   "\"value\"=\"%s\" WHERE \"item\"=\"NumTatamis\"", 
                   num_tatamis);
    else if (num_tatamis && num_tatamis[0])
        g_snprintf(buffer, sizeof(buffer), 
                   "INSERT INTO \"info\" VALUES ("
                   " \"NumTatamis\", \"%s\" )", num_tatamis);

    db_exec(db_name, buffer, NULL, NULL);

    strcpy(info_competition, competition);
    strcpy(info_date, date);
    strcpy(info_place, place);
    strcpy(info_time, start_time);
    strcpy(info_num_tatamis, num_tatamis);

    number_of_tatamis = atoi(num_tatamis);
    if (number_of_tatamis < 1)
        number_of_tatamis = 1;
    if (number_of_tatamis > NUM_TATAMIS)
        number_of_tatamis = NUM_TATAMIS;

    update_match_pages_visibility();
}

void db_save_config(void)
{
    char buffer[1000];

    sprintf(buffer, 
            "UPDATE \"info\" SET "
            "\"value\"=\"%d\" WHERE \"item\"=\"three_matches_for_two\"",
            three_matches_for_two);

    db_exec(db_name, buffer, NULL, NULL);
}

/*********************************************************/

gchar *db_sql_command(const gchar *command)
{
    sqlite3 *db;
    char *zErrMsg = NULL;
    gint rc, row, col;
    gchar **table = NULL;
    gint rows, cols;
    gchar *ret = NULL, *tmp;

    if ((rc = sqlite3_open(db_name, &db)))
        return NULL;

    rc = sqlite3_get_table(db, command, &table, &rows, &cols, &zErrMsg);
    if (rc != SQLITE_OK) {
        if (zErrMsg) {
            ret = g_strdup(zErrMsg);
            sqlite3_free(zErrMsg);
        } else
            ret = g_strdup("UNKNOWN ERROR!");
        goto out;
    }
        
    ret = g_strdup("");
    for (row = 0; row <= rows; row++) {
        for (col = 0; col < cols; col++) {
            if (col == 0)
                tmp = g_strconcat(ret, table[row*cols + col], NULL);
            else if (col == cols - 1)
                tmp = g_strconcat(ret, " | ", table[row*cols + col], "\n", NULL);
            else
                tmp = g_strjoin(" | ", ret, table[row*cols + col], NULL);
            g_free(ret);
            ret = tmp;
        }
    }
out:
    sqlite3_free_table(table);
    sqlite3_close(db);

    return ret;
}

void db_create_cat_def_table(void)
{
    char *cmd = "CREATE TABLE \"catdef\" ("
        "\"age\" INTEGER, \"agetext\" TEXT, "
        "\"flags\" INTEGER, "
        "\"weight\" INTEGER, \"weighttext\" TEXT, "
        "\"matchtime\" INTEGER, \"pintimekoka\" INTEGER, "
        "\"pintimeyuko\" INTEGER, \"pintimewazaari\" INTEGER, \"pintimeippon\" INTEGER, "
        "\"resttime\" INTEGER, \"gstime\" INTEGER)";
    db_exec(db_name, cmd, 0, 0);
}

void db_delete_cat_def_table_data(void)
{
    char *cmd = "DELETE FROM \"catdef\"";
    db_exec(db_name, cmd, 0, 0);
}

void db_insert_cat_def_table_data_begin(void)
{
    db_open();
    db_cmd(NULL, NULL, "begin transaction");
}

void db_insert_cat_def_table_data_end(void)
{
    db_cmd(NULL, NULL, "commit");
    db_close();
}

void db_insert_cat_def_table_data(struct cat_def *def)
{
    char buf[1024];
    sprintf(buf, "INSERT INTO catdef VALUES ("
            "%d, \"%s\", %d, %d, \"%s\", %d, %d, %d, %d, %d, %d, %d)",
            def->age, def->agetext, def->gender,
            def->weights[0].weight, def->weights[0].weighttext,
            def->match_time, def->pin_time_koka, def->pin_time_yuko, 
            def->pin_time_wazaari, def->pin_time_ippon, def->rest_time, def->gs_time); 
    db_cmd(NULL, NULL, buf);
    //db_exec(db_name, buf, 0, 0);
}

void db_cat_def_table_done(void)
{
    avl_set_category_rest_times();
}

void xxdb_add_colums_to_cat_def_table(void)
{
    db_exec(db_name,
            "CREATE TABLE catdef_backup ("
            "\"age\" INTEGER, \"agetext\" TEXT, "
            "\"flags\" INTEGER, "
            "\"weight\" INTEGER, \"weighttext\" TEXT)",
            0, 0);

    db_exec(db_name,
            "INSERT INTO catdef_backup SELECT * FROM catdef",
            0, 0);

    db_exec(db_name,
            "DROP TABLE catdef",
            0, 0);

    db_create_cat_def_table();

    db_exec(db_name,
            "INSERT INTO catdef SELECT * FROM catdef_backup",
            0, 0);

    db_exec(db_name,
            "DROP TABLE catdef_backup",
            0, 0);
}

static gboolean catdef_exists, matchtime_exists, gstime_exists;

static int db_callback_catdef(void *data, int argc, char **argv, char **azColName)
{
    int i;

    for (i = 0; i < argc; i++) {
        if (IS(name)) {
            if (strcmp("catdef", argv[i]) == 0) {
                catdef_exists = TRUE;
                return 0;
            }
        } else if (IS(sql)) {
            if (strstr(argv[i], "matchtime"))
                matchtime_exists = TRUE;
            if (strstr(argv[i], "gstime"))
                gstime_exists = TRUE;
        }
    }

    return 0;
}

gboolean catdef_needs_init(void)
{
    catdef_exists = matchtime_exists = gstime_exists = FALSE;

    db_exec(db_name, 
            "SELECT name FROM sqlite_master WHERE type='table'", 
            NULL, db_callback_catdef);
	
    db_exec(db_name, 
            "SELECT sql FROM sqlite_master", 
            NULL, db_callback_catdef);
	
    if (!catdef_exists) {
        g_print("catdef does not exist, create one\n");
        db_create_cat_def_table();
        return TRUE;
    }

    if (!matchtime_exists) {
        g_print("catdef matchtime does not exist, add one\n");

        db_exec(db_name, "ALTER TABLE catdef ADD \"matchtime\" INTEGER", NULL, NULL);
        db_exec(db_name, "ALTER TABLE catdef ADD \"pintimekoka\" INTEGER", NULL, NULL);
        db_exec(db_name, "ALTER TABLE catdef ADD \"pintimeyuko\" INTEGER", NULL, NULL);
        db_exec(db_name, "ALTER TABLE catdef ADD \"pintimewazaari\" INTEGER", NULL, NULL);
        db_exec(db_name, "ALTER TABLE catdef ADD \"pintimeippon\" INTEGER", NULL, NULL);
        db_exec(db_name, "ALTER TABLE catdef ADD \"resttime\" INTEGER", NULL, NULL);

        db_exec(db_name, "UPDATE catdef SET 'matchtime'=300", NULL, NULL);
        db_exec(db_name, "UPDATE catdef SET 'matchtime'=120 WHERE \"age\"<=12", NULL, NULL);
        db_exec(db_name, "UPDATE catdef SET 'matchtime'=180 WHERE \"age\"<=16 AND \"age\">12", NULL, NULL);
        db_exec(db_name, "UPDATE catdef SET 'matchtime'=240 WHERE \"age\"<=19 AND \"age\">16", NULL, NULL);

        db_exec(db_name, "UPDATE catdef SET 'resttime'=300", NULL, NULL);

        db_exec(db_name, "UPDATE catdef SET 'pintimekoka'=10", NULL, NULL);
        db_exec(db_name, "UPDATE catdef SET 'pintimeyuko'=15", NULL, NULL);
        db_exec(db_name, "UPDATE catdef SET 'pintimewazaari'=20", NULL, NULL);
        db_exec(db_name, "UPDATE catdef SET 'pintimeippon'=25", NULL, NULL);

        db_exec(db_name, "UPDATE catdef SET 'pintimekoka'=0 WHERE \"age\"<=10", NULL, NULL);
        db_exec(db_name, "UPDATE catdef SET 'pintimeyuko'=5 WHERE \"age\"<=10", NULL, NULL);
        db_exec(db_name, "UPDATE catdef SET 'pintimewazaari'=10 WHERE \"age\"<=10", NULL, NULL);
        db_exec(db_name, "UPDATE catdef SET 'pintimeippon'=15 WHERE \"age\"<=10", NULL, NULL);
    }	

    if (!gstime_exists) {
        g_print("catdef gstime does not exist, add one\n");

        db_exec(db_name, "ALTER TABLE catdef ADD \"gstime\" INTEGER", NULL, NULL);
        db_exec(db_name, "UPDATE catdef SET 'gstime'=300", NULL, NULL);
        db_exec(db_name, "UPDATE catdef SET 'gstime'=120 WHERE \"age\"<=16", NULL, NULL);
    }

    return FALSE;
}
