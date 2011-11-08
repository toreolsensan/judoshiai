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

#define NUM_POSITIONS 8

static struct compsys category_system;
static struct judoka j;
static gint positions[NUM_POSITIONS];

static gint my_atoi(gchar *p)
{
    if (!p)
        return 0;
    return atoi(p);
}

static int db_callback_categories(void *data, int argc, char **argv, char **azColName)
{
    gint i, flags = (int)data;

    for (i = 0; i < argc; i++) {
        //g_print(" %s=%s", azColName[i], argv[i]);
        if (IS(index))
            j.index = my_atoi(argv[i]);
        else if (IS(category))
            j.last = argv[i];
        else if (IS(tatami))
            j.belt = my_atoi(argv[i]);
        else if (IS(deleted))
            j.deleted = my_atoi(argv[i]);
        else if (IS(group))
            j.birthyear = my_atoi(argv[i]);
        else if (IS(system))
            category_system.system = my_atoi(argv[i]);
        else if (IS(numcomp))
            category_system.numcomp = my_atoi(argv[i]);
        else if (IS(table))
            category_system.table = my_atoi(argv[i]);
        else if (IS(wishsys))
            category_system.wishsys = my_atoi(argv[i]);
        else if (IS(pos1))
            positions[0] = my_atoi(argv[i]);
        else if (IS(pos2))
            positions[1] = my_atoi(argv[i]);
        else if (IS(pos3))
            positions[2] = my_atoi(argv[i]);
        else if (IS(pos4))
            positions[3] = my_atoi(argv[i]);
        else if (IS(pos5))
            positions[4] = my_atoi(argv[i]);
        else if (IS(pos6))
            positions[5] = my_atoi(argv[i]);
        else if (IS(pos7))
            positions[6] = my_atoi(argv[i]);
        else if (IS(pos8))
            positions[7] = my_atoi(argv[i]);
    }
    //g_print("\n");

    j.visible = FALSE;

    if (j.index >= current_category_index)
        current_category_index = j.index + 1;

    if (j.deleted & DELETED)
        return 0;

    if (flags & DB_GET_SYSTEM)
        return 0;

    display_one_judoka(&j);

    avl_set_category(j.index, j.last, j.belt, j.birthyear, category_system);

    return 0;
}

void db_add_category(int num, struct judoka *j)
{
    struct compsys cs;
    BZERO(cs);

    if (num < 10000) {
        g_print("%s: ERROR, num = %d\n", __FUNCTION__, num);
        return;
    }

    db_exec_str(NULL, db_callback_categories, 
            "INSERT INTO categories VALUES ("
            "%d, \"%s\", %d, %d, %d, 0, 0, 0, 0, "
            "0, 0, 0, 0, 0, 0, 0, 0)", 
                num, j->last, j->belt, j->deleted, j->birthyear);

    avl_set_category(num, j->last, j->belt, j->birthyear, cs);
}

void db_update_category(int num, struct judoka *j)
{
    if (num < 10000) {
        g_print("%s: ERROR, num = %d\n", __FUNCTION__, num);
        return;
    }

    if (j->deleted & DELETED) {
        db_exec_str(NULL, NULL,
                    "DELETE FROM categories WHERE \"index\"=%d",
                    num);
        return;
    }

    struct judoka *old = get_data(num);
    if (old) {
        db_exec_str(NULL, NULL,
                    "UPDATE competitors SET \"category\"=\"%s\" "
                    "WHERE \"category\"=\"%s\"", 
                    j->last, old->last);

        // tatami changed. remove comments to avoid conflicts.
        if (old->belt != j->belt) {
            db_exec_str(NULL, NULL, 
                        "UPDATE matches SET \"comment\"=0 "
                        "WHERE \"category\"=%d AND "
                        "(\"comment\"=%d OR \"comment\"=%d) ",
                        num, COMMENT_MATCH_1, COMMENT_MATCH_2);
        }

        free_judoka(old);
    }

    db_exec_str(NULL, db_callback_categories, 
		"UPDATE categories SET "
		"\"category\"=\"%s\", \"tatami\"=%d, \"deleted\"=%d, \"group\"=%d "
		"WHERE \"index\"=%d",
		j->last, j->belt, j->deleted, j->birthyear, num);

    struct category_data data, *data1;
    data.index = num;
    if (avl_get_by_key(categories_tree, &data, (void *)&data1) == 0) {
        strncpy(data1->category, j->last, sizeof(data1->category)-1);
        data1->tatami = j->belt;
        data1->group = j->birthyear;
        data1->deleted = j->deleted;
        set_category_to_queue(data1);
    } else
        g_print("Error %s %d (cat %d)\n", __FUNCTION__, __LINE__, num);
}

void db_set_system(int num, struct compsys sys)
{
    db_exec_str(NULL, db_callback_categories,
                "UPDATE categories SET "
                "\"system\"=%d, \"numcomp\"=%d, \"table\"=%d, \"wishsys\"=%d WHERE \"index\"=%d",
                sys.system, sys.numcomp, sys.table, sys.wishsys, num);

    struct category_data data, *data1;
    data.index = num;
    if (avl_get_by_key(categories_tree, &data, (void *)&data1) == 0) {
        data1->system = sys;
        set_category_to_queue(data1);
    } else
        g_print("Error %s %d (num=%d sys=%d)\n", __FUNCTION__, __LINE__,
                num, sys.system);
}

struct compsys db_get_system(gint num)
{
    char buffer[1000];

    BZERO(category_system);
    sprintf(buffer, "SELECT * FROM categories WHERE \"index\"=%d", num);

    db_exec(db_name, buffer, (gpointer)DB_GET_SYSTEM, db_callback_categories);

    return category_system;
}

gint db_get_tatami(gint num)
{
    char buffer[128];

    sprintf(buffer, "SELECT * FROM categories WHERE \"index\"=%d", num);

    db_exec(db_name, buffer, (gpointer)DB_GET_SYSTEM, db_callback_categories);

    return j.belt;
}

void db_read_categories(void)
{
    char buffer[100];

    sprintf(buffer, "SELECT * FROM categories");
    db_exec(db_name, buffer, NULL, db_callback_categories);
}

void db_set_category_positions(gint category, gint competitor, gint position)
{
    if (automatic_web_page_update || create_statistics == FALSE)
        return;

    db_exec_str(NULL, NULL,
                "UPDATE categories SET "
                "\"pos%d\"=%d WHERE \"index\"=%d",
                position, competitor, category);
}

gint db_get_competitors_position(gint competitor, gint *catindex)
{
    gint i;

    BZERO(positions);

    db_exec_str((gpointer)DB_GET_SYSTEM, db_callback_categories,
                "SELECT * FROM categories WHERE \"pos1\"=%d OR \"pos2\"=%d OR \"pos3\"=%d OR \"pos4\"=%d OR "
                "\"pos5\"=%d OR \"pos6\"=%d OR \"pos7\"=%d OR \"pos8\"=%d",
                competitor, competitor, competitor, competitor, 
                competitor, competitor, competitor, competitor);

    for (i = 0; i < NUM_POSITIONS; i++)
        if (positions[i] == competitor) {
            if (catindex)
                *catindex = j.index;
            return i+1;
        }

    return 0;
}
