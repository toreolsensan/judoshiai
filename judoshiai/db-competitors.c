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

extern void write_competitor(FILE *f, const gchar *first, const gchar *last, const gchar *belt, 
                             const gchar *club, const gchar *category, const gint index);

#define ADD_COMPETITORS               1
#define ADD_COMPETITORS_WITH_WEIGHTS  2
#define ADD_DELETED_COMPETITORS       4
#define PRINT_COMPETITORS             8
#define COMPETITOR_STATISTICS        16

static FILE *print_file = NULL;
static gint num_competitors;
static gint num_weighted_competitors;
static gint competitors_not_added, competitors_added;

static int db_callback(void *data, int argc, char **argv, char **azColName)
{
    int i, flags = (int)data;
    struct judoka j;
    gchar *newcat = NULL;

    for(i = 0; i < argc; i++){
        //g_print("  %s=%s", azColName[i], argv[i] ? argv[i] : "(NULL)");
        if (IS(index))
            j.index = argv[i] ? atoi(argv[i]) : 0;
        else if (IS(last))
            j.last = argv[i] ? argv[i] : "?";
        else if (IS(first))
            j.first = argv[i] ? argv[i] : "?";
        else if (IS(birthyear))
            j.birthyear = argv[i] ? atoi(argv[i]) : 0;
        else if (IS(club))
            j.club = argv[i] ? argv[i] : "?";
        else if (IS(regcategory) || IS(wclass))
            j.regcategory = argv[i] ? argv[i] : "?";
        else if (IS(belt))
            j.belt = argv[i] ? atoi(argv[i]) : 0;
        else if (IS(weight))
            j.weight = argv[i] ? atoi(argv[i]) : 0;
        else if (IS(visible))
            j.visible = argv[i] ? atoi(argv[i]) : 1;
        else if (IS(category))
            j.category = argv[i] ? argv[i] : "?";
        else if (IS(deleted))
            j.deleted = argv[i] ? atoi(argv[i]) : 0;
    }
    //g_print("\n");

    if (flags & ADD_DELETED_COMPETITORS) {
        if ((j.deleted & DELETED) && j.visible) {
            j.deleted &= ~DELETED;
            if (j.index >= current_index)
                current_index = j.index + 1;
            display_one_judoka(&j);

            avl_set_competitor(j.index, j.first, j.last);
            avl_set_competitor_status(j.index, j.deleted);
        }
        return 0;
    }

    if ((j.deleted & DELETED) || j.visible == 0)
        return 0;

    if (flags & PRINT_COMPETITORS) {
        if (print_file == NULL)
            return 1;
        const gchar *b = "?";
        if (j.belt >= 0 && j.belt < 14)
            b = belts[j.belt];
        write_competitor(print_file, j.first, j.last, b, j.club, j.category, j.index);
        return 0;
    }

    if (flags & COMPETITOR_STATISTICS) {
        num_competitors++;
        if (j.weight)
            num_weighted_competitors++;
    }

    if ((flags & ADD_COMPETITORS_WITH_WEIGHTS) ||
        (flags & ADD_COMPETITORS)) {
        GtkTreeIter iter;
        if (cleanup_import && find_iter_name(&iter, j.last, j.first, j.club)) {
            competitors_not_added++;
            return 0;
        }
        j.index = current_index++;
        j.category = "?";

        if (cleanup_import) {
            newcat = find_correct_category(j.birthyear ? current_year - j.birthyear : 0, 
                                           j.weight, 0, j.regcategory, TRUE);
            if (newcat)
                j.regcategory = newcat;
        }

        if (flags & ADD_COMPETITORS)
            j.weight = 0;
        db_add_judoka(j.index, &j);

        competitors_added++;
    } else {
        if (j.index >= current_index)
            current_index = j.index + 1;
    }

    display_one_judoka(&j);
    g_free(newcat);

    avl_set_competitor(j.index, j.first, j.last);
    avl_set_competitor_status(j.index, j.deleted);

    //find_gender(j.first);

    return 0;
}

void db_add_judoka(int num, struct judoka *j)
{
    char buffer[1000];

    sprintf(buffer, 
            "INSERT INTO competitors VALUES ("
            "%d, \"%s\", \"%s\", \"%d\", "
            "%d, \"%s\", \"%s\", "
            "%d, %d, \"%s\", %d"
            ")", 
            num, j->last, j->first, j->birthyear,
            j->belt, j->club, j->regcategory,
            j->weight, j->visible, j->category, j->deleted);

    db_exec(db_name, buffer, NULL, db_callback);

    avl_set_competitor(num, j->first, j->last);
    avl_set_competitor_status(num, j->deleted);
}

void db_update_judoka(int num, struct judoka *j)
{
    char buffer[1000];

    sprintf(buffer, 
            "UPDATE competitors SET "
            "last=\"%s\", first=\"%s\", birthyear=\"%d\", "
            "belt=\"%d\", club=\"%s\", regcategory=\"%s\", "
            "weight=\"%d\", visible=\"%d\", "
            "category=\"%s\", deleted=\"%d\" WHERE \"index\"=%d",
            j->last, j->first, j->birthyear,
            j->belt, j->club, j->regcategory,
            j->weight, j->visible, j->category, j->deleted, num);

    db_exec(db_name, buffer, NULL, db_callback);

    avl_set_competitor(num, j->first, j->last);
    avl_set_competitor_status(num, j->deleted);
}

void db_restore_removed_competitors(void)
{
    db_exec(db_name, "UPDATE competitors SET \"category\"=\"?\" WHERE \"deleted\"&1", 
            0, 
            db_callback);

    db_exec(db_name, "SELECT * FROM competitors WHERE \"deleted\"&1", 
            (void *)ADD_DELETED_COMPETITORS, 
            db_callback);

    db_exec(db_name, "UPDATE competitors SET \"deleted\"=\"deleted\"&~1 WHERE \"deleted\"=1", 
            0, 
            0);
}

void db_print_competitors(FILE *f)
{
    print_file = f;
    if (print_file == NULL)
        return;

    fprintf(print_file,
            "<td><table class=\"competitors\">"
            "<tr><td colspan=\"4\" align=\"center\"><h2>%s</h2></td></tr>\n", _T(competitor));
    fprintf(print_file, "<tr><th>%s</th><th>%s</th><th>%s</th><th>%s</th></tr>\n", 
            _T(name), _T(grade), _T(club), _T(category));

    db_exec(db_name, "SELECT * FROM competitors ORDER BY \"last\" ASC", 
            (void *)PRINT_COMPETITORS, 
            db_callback);

    fprintf(print_file, "</table></td>\n");
}

void db_read_judokas(void)
{
    char buffer[100];

    sprintf(buffer, 
            "SELECT * FROM competitors");
    db_exec(db_name, buffer, NULL, db_callback);
}

void db_read_competitor_statistics(gint *numcomp, gint *numweighted)
{
    num_competitors = num_weighted_competitors = 0;
    db_exec(db_name, "SELECT * FROM competitors", (void *)COMPETITOR_STATISTICS, db_callback);
    *numcomp = num_competitors;
    *numweighted = num_weighted_competitors;
}

void db_add_competitors(const gchar *competition, gboolean with_weight, 
			gint *added, gint *not_added)
{
    char buffer[100];

    competitors_not_added = competitors_added = 0;

    sprintf(buffer, 
            "SELECT * FROM competitors");
    db_exec(competition, buffer, 
            (void *)(with_weight?ADD_COMPETITORS_WITH_WEIGHTS:ADD_COMPETITORS), 
            db_callback);

    *added = competitors_added;
    *not_added = competitors_not_added;
}

static gboolean has_hansokumake;
 
static int db_hansokumake_cb(void *data, int argc, char **argv, char **azColName)
{
    has_hansokumake = TRUE;
    return 1;
}

gboolean db_has_hansokumake(gint competitor)
{
    char buffer[100];

    has_hansokumake = FALSE;

    sprintf(buffer, 
            "SELECT * FROM competitors WHERE \"index\"=%d AND \"deleted\"&2",
            competitor);

    db_exec(db_name, buffer, NULL, db_hansokumake_cb);

    return has_hansokumake;
}

void db_set_match_hansokumake(gint category, gint number, gint blue, gint white)
{ 
    gchar buffer[600];

    if (blue) {
        sprintf(buffer, 
                "UPDATE competitors SET \"deleted\"=%d "
                "WHERE EXISTS (SELECT * FROM matches "
                "WHERE competitors.'index'=matches.'blue' AND "
                "matches.'category'=%d AND matches.'number'=%d)", 
                HANSOKUMAKE, category, number);
        db_exec(db_name, buffer, NULL, NULL);
        sprintf(buffer, 
                "SELECT * from competitors "
                "WHERE EXISTS (SELECT * FROM matches "
                "WHERE competitors.'index'=matches.'blue' AND "
                "matches.'category'=%d AND matches.'number'=%d)", 
                category, number);
        db_exec(db_name, buffer, NULL, db_callback);
    }
    if (white) {
        sprintf(buffer, 
                "UPDATE competitors SET \"deleted\"=%d "
                "WHERE EXISTS (SELECT * FROM matches "
                "WHERE competitors.'index'=matches.'white' AND "
                "matches.'category'=%d AND matches.'number'=%d)", 
                HANSOKUMAKE, category, number);
        db_exec(db_name, buffer, NULL, NULL);
        sprintf(buffer, 
                "SELECT * from competitors "
                "WHERE EXISTS (SELECT * FROM matches "
                "WHERE competitors.'index'=matches.'white' AND "
                "matches.'category'=%d AND matches.'number'=%d)", 
                category, number);
        db_exec(db_name, buffer, NULL, db_callback);
    }
}


