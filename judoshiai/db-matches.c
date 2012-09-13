/* -*- mode: C; c-basic-offset: 4;  -*- */

/*
 * Copyright (C) 2006-2012 by Hannu Jokinen
 * Full copyright text is included in the software package.
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <assert.h>

#include "sqlite3.h"
#include "judoshiai.h"

//#define TATAMI_DEBUG 1

gboolean auto_arrange = FALSE;
//gboolean use_weights = FALSE;

static gboolean match_row_found;
static gboolean match_found;
static gboolean matched_match_found;
static gboolean unmatched_match_found;
static gboolean real_match_found;
static struct match *category_matches_p;
static struct match m;

#define NUM_SAVED_MATCHES 16
static gint num_saved_matches;
static struct match saved_matches[NUM_SAVED_MATCHES];

static struct match *next_match, next_matches[NUM_TATAMIS+1][NEXT_MATCH_NUM];
static struct match matches_waiting[WAITING_MATCH_NUM];
static gint num_matches_waiting = 0;
static gint next_match_num;
static gint next_match_tatami;

static gint match_count, matched_matches_count;
static gint bluecomp, whitecomp, bluepts, whitepts;

/* Use mutex when calling db_next_match(). */
GStaticMutex next_match_mutex = G_STATIC_MUTEX_INIT;

// data for coach info
static struct {
    gint tatami;
    gint waittime;
    gint matchnum;
    gint round;
} coach_info[10000];
static gint current_round = 0;

struct match *get_cached_next_matches(gint tatami)
{
    return next_matches[tatami];
}

#define INSERT_SPACE(x)							\
    do {								\
        gint k; next_match_num++;                                       \
        if (next_match_num > NEXT_MATCH_NUM) next_match_num = NEXT_MATCH_NUM; \
        for (k = next_match_num-1; k > x; k--)                          \
            next_match[k] = next_match[k-1];                            \
    } while (0)

void db_matches_init(void)
{
    gint i, j;

    memset(&next_matches, 0, sizeof(next_matches));

    for (i = 0; i <= NUM_TATAMIS; i++)
        for (j = 0; j < NEXT_MATCH_NUM; j++)
            next_matches[i][j].number = INVALID_MATCH;
}

static int db_callback_matches(void *data, int argc, char **argv, char **azColName)
{
    guint flags = (int)data;
    gint i, val;

    for (i = 0; i < argc; i++) {
        val =  argv[i] ? atoi(argv[i]) : 0;

        if (IS(category))
            m.category = val;
        else if (IS(number))
            m.number = val;
        else if (IS(blue))
            m.blue = val;
        else if (IS(white))
            m.white = val;
        else if (IS(blue_score))
            m.blue_score = val;
        else if (IS(white_score))
            m.white_score = val;
        else if (IS(blue_points))
            m.blue_points = val;
        else if (IS(white_points))
            m.white_points = val;
        else if (IS(time))
            m.match_time = val;
        else if (IS(comment))
            m.comment = val;
        else if (IS(forcedtatami))
            m.forcedtatami = val;
        else if (IS(forcednumber))
            m.forcednumber = val;
        else if (IS(date))
            m.date = val;
        else if (IS(legend))
            m.legend = val;
        else if (IS(deleted)) {
            m.deleted = val;
            match_row_found = TRUE;

            if ((val & DELETED) == 0)
                match_found = TRUE;

            if ((val & DELETED) == 0 &&
                m.blue >= COMPETITOR && m.white >= COMPETITOR)
                real_match_found = TRUE;

            if ((val & DELETED) == 0 &&
                (m.blue_points || m.white_points) &&
                m.blue >= COMPETITOR && m.white >= COMPETITOR) {
                matched_match_found = TRUE;
                matched_matches_count++;
            } else if ((val & DELETED) == 0 &&
                       m.blue >= COMPETITOR && m.white >= COMPETITOR &&
                       m.blue_points == 0 && m.white_points == 0 &&
                       ((avl_get_competitor_status(m.blue) & HANSOKUMAKE) ||
                        (avl_get_competitor_status(m.white) & HANSOKUMAKE))) {
                matched_matches_count++;
            }

            if ((val & DELETED) == 0 &&
                m.blue >= COMPETITOR && m.white >= COMPETITOR &&
                m.blue_points == 0 && m.white_points == 0 &&
                (avl_get_competitor_status(m.blue) & HANSOKUMAKE) == 0 &&
                (avl_get_competitor_status(m.white) & HANSOKUMAKE) == 0) {
                unmatched_match_found = TRUE;
            }

            if ((val & DELETED) == 0 &&
                m.blue != GHOST && m.white != GHOST) {
                match_count++;
            }

            if (bluecomp == 0)
                bluecomp = m.blue;
            else if (bluecomp != m.blue)
                bluecomp = -1;
            if (whitecomp == 0)
                whitecomp = m.white;
            else if (whitecomp != m.white)
                whitecomp = -1;
            bluepts += m.blue_points;
            whitepts += m.white_points;
        }
    }

    if (flags & DB_NEXT_MATCH) {
        struct category_data *catdata = avl_get_category(m.category);

        if (catdata == NULL)
            return 0;

        m.tatami = m.forcedtatami ? m.forcedtatami : catdata->tatami;
        m.group = catdata->group;

        if (m.blue == GHOST || m.white == GHOST)
            return 0;

        if (m.blue_points || m.white_points)
            return 0;

        if (m.tatami != next_match_tatami)
            return 0;
#ifdef TATAMI_DEBUG
        if (m.tatami == TATAMI_DEBUG) {
            struct judoka *cat = get_data(m.category);
            g_print("%s: %s:%d comment %d\n", __FUNCTION__,
                    cat ? cat->last : "?", m.number, m.comment);
            free_judoka(cat);
        }
#endif
        // coach info
        if (m.blue > 0 && m.blue < 10000) {
            if (current_round != coach_info[m.blue].round) {
                coach_info[m.blue].matchnum = 0;
                coach_info[m.blue].round = current_round;
                coach_info[m.blue].waittime = -1;
            }
            if ((m.number < coach_info[m.blue].matchnum) || (coach_info[m.blue].matchnum == 0)) {
                coach_info[m.blue].matchnum = m.number;
                coach_info[m.blue].tatami = m.tatami;
            }
        }
        if (m.white > 0 && m.white < 10000) {
            if (current_round != coach_info[m.white].round) {
                coach_info[m.white].matchnum = 0;
                coach_info[m.white].round = current_round;
                coach_info[m.white].waittime = -1;
            }
            if ((m.number < coach_info[m.white].matchnum) || (coach_info[m.white].matchnum == 0)) {
                coach_info[m.white].matchnum = m.number;
                coach_info[m.white].tatami = m.tatami;
            }
        }

        if (m.comment == COMMENT_MATCH_1) {
#ifdef TATAMI_DEBUG
            if (m.tatami == TATAMI_DEBUG) {
                g_print("COMMENT 1\n");
            }
#endif
            INSERT_SPACE(0);
            next_match[0] = m;
            return 0;
        } else if (m.comment == COMMENT_MATCH_2) {
#ifdef TATAMI_DEBUG
            if (m.tatami == TATAMI_DEBUG) {
                g_print("COMMENT 2\n");
            }
#endif
            if (next_match_num > 0 && next_match[0].comment == COMMENT_MATCH_1) {
                INSERT_SPACE(1);
                next_match[1] = m;
            } else {
                INSERT_SPACE(0);
                next_match[0] = m;
            }
            return 0;
        } else if (m.comment == COMMENT_WAIT) {
            return 0;
        }

        for (i = 0; i < next_match_num; i++) {
            gboolean insert = FALSE;

            if (next_match[i].comment == COMMENT_MATCH_1 ||
                next_match[i].comment == COMMENT_MATCH_2)
                continue;

            if (m.forcednumber) {
                if ((next_match[i].forcednumber == 0) ||
                    (next_match[i].forcednumber > m.forcednumber))
                    insert = TRUE;
            } else if (next_match[i].forcednumber == 0) {
                if (next_match[i].group > m.group) insert = TRUE;
                else if (next_match[i].group == m.group) {
                    gint a = (100000*m.number)/num_matches_estimate(m.category);
                    gint b = (100000*next_match[i].number)/num_matches_estimate(next_match[i].category);
                    if (b > a) insert = TRUE;
                    else if (b == a) {
                        if (next_match[i].category > m.category)
                            insert = TRUE;
                    }
                }
            }
            if (insert) {
                INSERT_SPACE(i);
                next_match[i] = m;
                return 0;
            }
        }

        if (next_match_num < NEXT_MATCH_NUM) {
            next_match[next_match_num++] = m;
        }

        return 0;
    } else if ((flags & ADD_MATCH) && ((m.deleted & DELETED) == 0)) {
        set_match(&m);
    } else if ((flags & SAVE_MATCH) && ((m.deleted & DELETED) == 0)) {
        if (num_saved_matches < NUM_SAVED_MATCHES) {
            saved_matches[num_saved_matches++] = m;
        }
    } else if ((flags & DB_REMOVE_COMMENT) && ((m.deleted & DELETED) == 0)) {
        m.comment = COMMENT_EMPTY;
        set_match(&m);
    } else if ((flags & DB_READ_CATEGORY_MATCHES) && ((m.deleted & DELETED) == 0)) {
        category_matches_p[m.number] = m;
    } else if (flags & DB_MATCH_WAITING) {
        if (num_matches_waiting < WAITING_MATCH_NUM)
            matches_waiting[num_matches_waiting++] = m;
    } else if (flags & DB_UPDATE_LAST_MATCH_TIME) {
        if (m.blue > COMPETITOR)
            avl_set_competitor_last_match_time(m.blue);
        if (m.white > COMPETITOR)
            avl_set_competitor_last_match_time(m.white);
    } else if ((flags & DB_RESET_LAST_MATCH_TIME_B) || (flags & DB_RESET_LAST_MATCH_TIME_W)) {
        if (m.blue > COMPETITOR && (flags & DB_RESET_LAST_MATCH_TIME_B))
            avl_reset_competitor_last_match_time(m.blue);
        if (m.white > COMPETITOR && (flags & DB_RESET_LAST_MATCH_TIME_W))
            avl_reset_competitor_last_match_time(m.white);
    }

    return 0;
}

void db_freeze_matches(gint tatami, gint category, gint number, gint arg)
{
    gint i, num = 1;

    switch (arg) {
    case FREEZE_MATCHES:
        for (i = 0; i < NEXT_MATCH_NUM; i++) {
            if (next_matches[tatami][i].number == INVALID_MATCH)
                continue;

            db_exec_str(NULL, NULL,
                        "UPDATE matches SET \"forcedtatami\"=%d, "
                        "\"forcednumber\"=%d "
                        "WHERE \"category\"=%d AND \"number\"=%d",
                        tatami, num++, 
                        next_matches[tatami][i].category, 
                        next_matches[tatami][i].number);
        }
        db_read_matches();
        update_matches(0, (struct compsys){0,0,0,0}, tatami);
        break;                
    case UNFREEZE_EXPORTED:
        db_exec_str(NULL, NULL,
                    "UPDATE matches SET \"forcedtatami\"=0, \"forcednumber\"=0 "
                    "WHERE EXISTS(SELECT * FROM categories "
                    "WHERE categories.\"tatami\"=%d "
                    "AND categories.\"index\"=matches.\"category\")",
                    tatami);
        db_read_matches();
        for (i = 1; i <= number_of_tatamis; i++)
            update_matches(0, (struct compsys){0,0,0,0}, i);
        break;
    case UNFREEZE_IMPORTED:
        db_exec_str(NULL, NULL,
                    "UPDATE matches SET \"forcedtatami\"=0, \"forcednumber\"=0, "
                    "\"comment\"=%d WHERE \"forcedtatami\"=%d",
                    COMMENT_EMPTY, tatami);
        db_read_matches();
        for (i = 1; i <= number_of_tatamis; i++)
            update_matches(0, (struct compsys){0,0,0,0}, i);
        break;
    case UNFREEZE_THIS:
        db_exec_str(NULL, NULL,
                    "UPDATE matches SET \"forcedtatami\"=0, \"forcednumber\"=0, "
                    "\"comment\"=%d WHERE \"category\"=%d AND \"number\"=%d",
                    COMMENT_EMPTY, category, number);
        db_read_match(category, number);
        update_matches(category, (struct compsys){0,0,0,0}, 0);
        update_matches(0, (struct compsys){0,0,0,0}, tatami);
        break;
    }
}

static gboolean db_match_ready(gint cat, gint num)
{
    match_found = FALSE;

    db_exec_str(NULL, db_callback_matches, 
                "SELECT * from matches WHERE \"category\"=%d AND \"number\"=%d "
                "AND \"blue\">%d AND \"white\">%d",
                cat, num, COMPETITOR, COMPETITOR);

    return match_found;
}

static void db_set_comm(gint comm, gint cat, gint num)
{
    if (comm == COMMENT_MATCH_1 &&
        db_match_ready(cat, num) == FALSE)
        comm = COMMENT_EMPTY;

    db_exec_str(NULL, NULL, 
                "UPDATE matches SET \"comment\"=%d "
                "WHERE \"category\"=%d AND \"number\"=%d",
                comm, cat, num);

    db_read_match(cat, num);
}

void db_change_freezed(gint category, gint number, 
		       gint tatami, gint position, gboolean after)
{
    gint comment = COMMENT_EMPTY, i;
    //gint default_tatami = db_get_tatami(category);
    gint current_tatami = db_find_match_tatami(category, number);
    gint forcedmax = 0;

    for (i = 0; i < NEXT_MATCH_NUM && i < position - (after ? 0 : 1); i++) {
        if (next_matches[tatami][i].forcednumber > forcedmax)
            forcedmax = next_matches[tatami][i].forcednumber;
    }
    forcedmax++;

    if (tatami == 0) {
        /* delay match */
        comment = COMMENT_WAIT;
    } else if (position == 0 || (position == 1 && after == FALSE)) {
        /* first match */
        comment = COMMENT_MATCH_1;
    } else if ((position == 1 && after == TRUE) || (position == 2 && after == FALSE)) {
        /* second match */
        comment = COMMENT_MATCH_2;
    }
#if 0
    if (tatami && comment == COMMENT_MATCH_1 &&
        db_match_ready(category, number) == FALSE)
        return;
#endif
    if (tatami) {
        db_exec_str(NULL, NULL, "UPDATE matches SET \"forcednumber\"=\"forcednumber\"+1 "
                    "WHERE \"forcednumber\">=%d AND \"forcedtatami\"=%d",
                    forcedmax, tatami);
    }

    db_exec_str(NULL, NULL, "UPDATE matches SET \"forcedtatami\"=%d, \"forcednumber\"=%d "
                "WHERE \"category\"=%d AND \"number\"=%d",
                tatami, tatami ? forcedmax : 0, category, number);

#ifdef TATAMI_DEBUG
    g_print("%s: %d:%d, move to T%d P%d (after=%d), comment to %d\n", 
            __FUNCTION__, category, number, 
            tatami, position, after, comment);
#endif

    if (tatami && current_tatami == tatami) {
        gint current_pos;

        for (current_pos = 0; 
             current_pos < NEXT_MATCH_NUM;
             current_pos++)
            if (next_matches[tatami][current_pos].category == category &&
                next_matches[tatami][current_pos].number == number)
                break;

        if (current_pos == 0) {
            db_set_comm(COMMENT_MATCH_1,
                        next_matches[tatami][1].category, 
                        next_matches[tatami][1].number);

            if (comment != COMMENT_MATCH_2) {
                db_set_comm(COMMENT_MATCH_2,
                            next_matches[tatami][2].category, 
                            next_matches[tatami][2].number);
            }
        } else if (current_pos == 1) {
            if (comment == COMMENT_MATCH_1) {
                db_set_comm(COMMENT_MATCH_2,
                            next_matches[tatami][0].category, 
                            next_matches[tatami][0].number);
            } else {
                db_set_comm(COMMENT_MATCH_2,
                            next_matches[tatami][2].category, 
                            next_matches[tatami][2].number);
            }
        } else {
            if (comment == COMMENT_MATCH_1 || comment == COMMENT_MATCH_2) {
                db_set_comm(COMMENT_EMPTY,
                            next_matches[tatami][1].category, 
                            next_matches[tatami][1].number);
            }
            if (comment == COMMENT_MATCH_1) {
                db_set_comm(COMMENT_MATCH_2,
                            next_matches[tatami][0].category, 
                            next_matches[tatami][0].number);
            }
        }
    } else if (tatami) {
        if (comment == COMMENT_MATCH_1 || comment == COMMENT_MATCH_2) {
            db_set_comm(COMMENT_EMPTY,
                        next_matches[tatami][1].category, 
                        next_matches[tatami][1].number);
        }
        if (comment == COMMENT_MATCH_1) {
            db_set_comm(COMMENT_MATCH_2,
                        next_matches[tatami][0].category, 
                        next_matches[tatami][0].number);
        }
    }


    db_set_comment(category, number, comment);
    //db_set_comm(comment, category, number);

    update_matches(0, (struct compsys){0,0,0,0}, current_tatami);
    if (tatami && tatami != current_tatami)
        update_matches(0, (struct compsys){0,0,0,0}, tatami);
}

void db_add_match(struct match *m)
{
    gchar buffer[400];

    assert(m->blue < 10000);
    assert(m->white < 10000);

    sprintf(buffer, "INSERT INTO matches VALUES ("
            VALINT ", " VALINT ", " VALINT ", " VALINT ", " 
            VALINT ", " VALINT ", " 
            VALINT ", " VALINT ", " VALINT ", " VALINT ", " VALINT ", " 
            VALINT ", " VALINT ", " VALINT ", " VALINT " )",
            m->category, m->number,
            m->blue, m->white, m->blue_score, m->white_score,
            m->blue_points, m->white_points,
            m->match_time, m->comment, m->deleted, 0, 0, m->date, m->legend);
        
    //g_print("%s\n", buffer);
    db_exec(db_name, buffer, NULL, db_callback_matches);
}

void db_update_match(struct match *m)
{
    gchar buffer[400];

    assert(m->blue < 10000);
    assert(m->white < 10000);

    sprintf(buffer, "UPDATE matches SET "
            VARVAL(blue, %d) ", "
            VARVAL(white, %d) ", "
            VARVAL(blue_score, %d) ", "
            VARVAL(white_score, %d) ", "
            VARVAL(blue_points, %d) ", "
            VARVAL(white_points, %d) ", "
            VARVAL(deleted, %d) ", "
            VARVAL(time, %d) " WHERE "
            VARVAL(category, %d) " AND "
            VARVAL(number, %d),
            m->blue, m->white, m->blue_score, m->white_score,
            m->blue_points, m->white_points, m->deleted,
            m->match_time, m->category, m->number);

    //g_print("%s\n", buffer);
    db_exec(db_name, buffer, NULL, db_callback_matches);
}

gboolean db_match_exists(gint category, gint number, gint flags)
{
    gchar buffer[100];

    sprintf(buffer, "SELECT * FROM matches WHERE "
            VARVAL(category, %d) " AND "
            VARVAL(number, %d),
            category, number);
                
    match_found = match_row_found = FALSE;
    db_exec(db_name, buffer, NULL, db_callback_matches);
        
    return (flags & DB_MATCH_ROW) ? match_row_found : match_found;
}

struct match *db_get_match_data(gint category, gint number)
{
    if (db_match_exists(category, number, 0))
        return &m;
    return NULL;
}

gboolean db_matched_matches_exist(gint category)
{
    gchar buffer[100];

    sprintf(buffer, "SELECT * FROM matches WHERE "
            VARVAL(category, %d), category);
                
    match_found = match_row_found = matched_match_found = FALSE;
    db_exec(db_name, buffer, NULL, db_callback_matches);
        
    return matched_match_found;
}


gint db_category_match_status(gint category)
{
    gchar buffer[100];
    gint res = 0;

    match_count = matched_matches_count = bluecomp = whitecomp = bluepts = whitepts = 0;

    sprintf(buffer, "SELECT * FROM matches WHERE "
            VARVAL(category, %d), category);
                
    match_found = match_row_found = matched_match_found = unmatched_match_found = real_match_found = FALSE;
    db_exec(db_name, buffer, NULL, db_callback_matches);
        
    if (matched_matches_count > match_count) {
        g_print("%s[%d]: ERROR %d %d %d\n", __FUNCTION__, __LINE__,
                category, matched_matches_count, match_count);
        matched_matches_count = match_count;
    }

    if (match_found)
        res |= MATCH_EXISTS;
    if (matched_match_found)
        res |= MATCH_MATCHED;
    if (real_match_found)
        res |= REAL_MATCH_EXISTS;

    if (match_count == 3 && matched_matches_count == 2 &&
        bluecomp > 0 && whitecomp > 0 && 
        ((bluepts && whitepts == 0) || (bluepts == 0 && whitepts))) {
        /* two's pool with three matches completed after two matches */
    } else if (unmatched_match_found)
        res |= MATCH_UNMATCHED;

    struct category_data *catdata = avl_get_category(category);
    if (catdata) {
        catdata->match_status = res;
        catdata->match_count = match_count;
        catdata->matched_matches_count = matched_matches_count;

        if (find_age_index(catdata->category) < 0)
            catdata->defined = FALSE;
        else
            catdata->defined = TRUE;

        // data for coach
        if (automatic_web_page_update) {
            gint i, n = 0;
            gchar buf[80];

            n = sprintf(buf, "c-");
            for (i = 0; i < (sizeof(catdata->category)-1) && catdata->category[i]; i++)
                n += sprintf(buf + n, "%02x", (guchar)catdata->category[i]);
            n += sprintf(buf + n, ".txt");

            gchar *file = g_build_filename(current_directory, buf, NULL);
            FILE *f = fopen(file, "wb");
            g_free(file);
            if (f) {
                fprintf(f, "%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n", 
                        catdata->index, catdata->tatami, catdata->group,
                        catdata->system.system, catdata->system.numcomp, catdata->system.table, catdata->system.wishsys, 
                        catdata->match_status, catdata->match_count, catdata->matched_matches_count);
                fclose(f);
            }
        }
    }

    return res;
}

gint db_competitor_match_status(gint competitor)
{
    gchar buffer[100];
    gint res = 0;

    sprintf(buffer, "SELECT * FROM matches WHERE \"blue\"=%d OR \"white\"=%d",
            competitor, competitor);
                
    match_found = match_row_found = matched_match_found = unmatched_match_found = FALSE;
    db_exec(db_name, buffer, NULL, db_callback_matches);
        
    if (match_found)
        res |= MATCH_EXISTS;
    if (matched_match_found)
        res |= MATCH_MATCHED;
    if (unmatched_match_found)
        res |= MATCH_UNMATCHED;

    return res;
}


void db_set_match(struct match *m1)
{
    memset(&m, 0, sizeof(m));

    if (db_match_exists(m1->category, m1->number, DB_MATCH_ROW)) {
        if (m.blue != m1->blue ||
            m.white != m1->white ||
            m.blue_score != m1->blue_score ||
            m.white_score != m1->white_score ||
            m.blue_points != m1->blue_points ||
            m.white_points != m1->white_points ||
            m.deleted != m1->deleted ||
            m.category != m1->category ||
            m.number != m1->number ||
            m.match_time != m1->match_time ||
            m.comment != m1->comment ||
            m.forcedtatami != m1->forcedtatami ||
            m.forcednumber != m1->forcednumber ||
            m.date != m1->date ||
            m.legend != m1->legend)
            db_update_match(m1);
    } else
        db_add_match(m1);


    // clear coach info
    if (m1->blue > 0 && m1->blue < 10000) {
        coach_info[m1->blue].tatami = 0;
        coach_info[m1->blue].waittime = -1;
        coach_info[m1->blue].matchnum = 0;
    }
    if (m1->white > 0 && m1->white < 10000) {
        coach_info[m1->white].tatami = 0;
        coach_info[m1->white].waittime = -1;
        coach_info[m1->white].matchnum = 0;
    }
}

void db_read_category_matches(gint category, struct match *m)
{
    gchar buffer[100];

    sprintf(buffer, "SELECT * FROM matches WHERE \"category\"=%d", category);
                
    category_matches_p = m;
    db_exec(db_name, buffer, (gpointer)DB_READ_CATEGORY_MATCHES, db_callback_matches);
}

void db_read_matches(void)
{
    db_exec(db_name, "SELECT * FROM matches", 
            (gpointer)ADD_MATCH, db_callback_matches);
}

void db_remove_matches(guint category)
{
    gchar buffer[100];

    sprintf(buffer, "DELETE FROM matches WHERE \"category\"=%d", category);
                
    db_exec(db_name, buffer, NULL, db_callback_matches);
}

void db_set_points(gint category, gint number, gint minutes, 
                   gint blue, gint white, gint blue_score, gint white_score, gint legend)
{
    gchar buffer[256];

    if (blue_score || white_score || minutes) // full information available
        sprintf(buffer, 
                "UPDATE matches SET \"blue_points\"=%d, \"white_points\"=%d, "
                "\"blue_score\"=%d, \"white_score\"=%d, \"time\"=%d, \"date\"=%ld, \"legend\"=%d "
                "WHERE \"category\"=%d AND \"number\"=%d", 
                blue, white, blue_score, white_score, minutes, time(NULL), legend, category, number);
    else // correct points but leave other info as is
        sprintf(buffer, 
                "UPDATE matches SET \"blue_points\"=%d, \"white_points\"=%d "
                "WHERE \"category\"=%d AND \"number\"=%d", 
                blue, white, category, number);
                
    db_exec(db_name, buffer, NULL, db_callback_matches);

    sprintf(buffer, 
            "SELECT * FROM matches "
            "WHERE \"category\"=%d AND \"number\"=%d", 
            category, number);
                
    db_exec(db_name, buffer, (gpointer)DB_UPDATE_LAST_MATCH_TIME, db_callback_matches);

    if (blue == 0 && white == 0) {
        sprintf(buffer, 
                "UPDATE matches SET \"blue\"=0 "
                "WHERE \"category\"=%d AND \"number\"=%d AND \"blue\"=%d", 
                category, number, GHOST);
        db_exec(db_name, buffer, NULL, db_callback_matches);
        sprintf(buffer, 
                "UPDATE matches SET \"white\"=0 "
                "WHERE \"category\"=%d AND \"number\"=%d AND \"white\"=%d", 
                category, number, GHOST);
        db_exec(db_name, buffer, NULL, db_callback_matches);
    }
}

void db_reset_last_match_times(gint category, gint number, gboolean blue, gboolean white)
{
    gint x = 0;
    if (blue)
        x = DB_RESET_LAST_MATCH_TIME_B;
    if (white)
        x |= DB_RESET_LAST_MATCH_TIME_W;

    db_exec_str((gpointer)x, db_callback_matches,
                "SELECT * FROM matches "
                "WHERE \"category\"=%d AND \"number\"=%d", 
                category, number);
}

void db_read_match(gint category, gint number)
{
    gchar buffer[200];

    sprintf(buffer, "SELECT * FROM matches WHERE \"category\"=%d AND \"number\"=%d",
            category, number);

    db_exec(db_name, buffer, (gpointer)ADD_MATCH, db_callback_matches);
}

void db_read_matches_of_category(gint category)
{
    gchar buffer[200];

    sprintf(buffer, "SELECT * FROM matches WHERE \"category\"=%d",
            category);

    db_exec(db_name, buffer, (gpointer)ADD_MATCH, db_callback_matches);
}

static gint competitor_cannot_match(gint index, gint tatami, gint num)
{
    if (index == 0)
        return 0;

    gint i, j, result = 0;
    gint hash = avl_get_competitor_hash(index);
    time_t last_time = avl_get_competitor_last_match_time(index);
    gint rest_time = 180;
    struct category_data *catdata = 
        avl_get_category(next_matches[tatami][num].category);

    if (catdata)
        rest_time = catdata->rest_time;

    if (time(NULL) < last_time + rest_time) {
        //g_print("T%d:%d rested %ld s\n", tatami, num, time(NULL) - last_time);
        return MATCH_FLAG_REST_TIME;
    }

    for (i = 1; i <= NUM_TATAMIS; i++) {
        for (j = 0; j <= 1; j++) {
            if (i == tatami && j == num)
                continue;
            if (next_matches[i][j].number >= INVALID_MATCH)
                continue;
			
            result = 0;
            if (next_matches[i][j].blue && 
                (hash == avl_get_competitor_hash(next_matches[i][j].blue)))
                result |= MATCH_FLAG_BLUE_DELAYED;
            if (next_matches[i][j].white && 
                (hash == avl_get_competitor_hash(next_matches[i][j].white)))
                result |= MATCH_FLAG_WHITE_DELAYED;

            if (result) {
                //g_print("T%d:%d matching in T%d:%d\n", tatami, num, i, j);
                return result;
            }
        }
    }

    return 0;
}

/* Use next_match_mutex while calling and handling data (next_match[] is static). */
struct match *db_next_matchXXX(gint category, gint tatami)
{
    gchar buffer[1000];
    gint i;

    if (category) {
        tatami = db_get_tatami(category);
        /*** BUG
             sprintf(buffer, "SELECT * FROM categories WHERE \"category\"=%d", category);
             db_exec(db_name, buffer, (gpointer)DB_GET_SYSTEM, db_callback_categories);
             tatami = j.belt;
        ***/
    }

    if (tatami == 0)
        return NULL;

    next_match = next_matches[tatami];

    for (i = 0; i < NEXT_MATCH_NUM; i++) {
        memset(&next_match[i], 0, sizeof(struct match));
        next_match[i].number = INVALID_MATCH;
    }		

    next_match_num = 0;
    next_match_tatami = tatami;

    /* save current match data */
    num_saved_matches = 0;
    sprintf(buffer, 
            "SELECT matches.* "
            "FROM matches, categories "
            "WHERE categories.\"tatami\"=%d AND categories.\"index\"=matches.\"category\" "
            "AND categories.\"deleted\"=0 "
            "AND (matches.\"comment\"=%d OR matches.\"comment\"=%d)"
            "AND matches.\"forcedtatami\"=0",
            tatami, COMMENT_MATCH_1, COMMENT_MATCH_2);
    db_exec(db_name, buffer, (gpointer)SAVE_MATCH, db_callback_matches);
    sprintf(buffer, 
            "SELECT * FROM matches WHERE \"forcedtatami\"=%d "
            "AND (\"comment\"=%d OR \"comment\"=%d)",
            tatami, COMMENT_MATCH_1, COMMENT_MATCH_2);
    db_exec(db_name, buffer, (gpointer)SAVE_MATCH, db_callback_matches);

    /* remove comments from the display */
    sprintf(buffer, 
            "SELECT matches.* "
            "FROM matches, categories "
            "WHERE categories.\"tatami\"=%d AND categories.\"index\"=matches.\"category\" "
            "AND categories.\"deleted\"=0 AND matches.\"comment\">%d "
            "AND matches.\"forcedtatami\"=0",
            tatami, COMMENT_EMPTY);
    db_exec(db_name, buffer, (gpointer)DB_REMOVE_COMMENT, db_callback_matches);
    sprintf(buffer, 
            "SELECT * FROM matches WHERE \"forcedtatami\"=%d "
            "AND \"comment\">%d",
            tatami, COMMENT_EMPTY);
    db_exec(db_name, buffer, (gpointer)DB_REMOVE_COMMENT, db_callback_matches);

    /* remove comments from old matches */
    sprintf(buffer, 
            "UPDATE matches SET \"comment\"=%d "
            "WHERE EXISTS (SELECT * FROM categories WHERE "
            "categories.\"tatami\"=%d AND (matches.\"blue_points\">0 OR "
            "matches.\"white_points\">0) AND categories.\"index\"=matches.\"category\" "
            "AND matches.\"comment\">%d AND categories.\"deleted\"=0 "
            "AND matches.\"forcedtatami\"=0)",
            COMMENT_EMPTY, tatami, COMMENT_EMPTY);
    db_exec(db_name, buffer, 0, 0);
    sprintf(buffer, 
            "UPDATE matches SET \"comment\"=%d "
            "WHERE \"forcedtatami\"=%d AND \"comment\">%d "
            "AND (\"blue_points\">0 OR \"white_points\">0)",
            COMMENT_EMPTY, tatami, COMMENT_EMPTY);
    db_exec(db_name, buffer, 0, 0);

    /* find all matches */
    sprintf(buffer, 
            "SELECT * FROM matches "
            "WHERE matches.\"blue_points\"=0 "
            "AND matches.\"deleted\"&1=0 "
            "AND matches.\"white_points\"=0 "
            "AND matches.\"blue\"<>%d AND matches.\"white\"<>%d "
            "AND (matches.\"blue\"=0 OR EXISTS (SELECT * FROM competitors WHERE "
            "competitors.\"index\"=matches.\"blue\" AND competitors.\"deleted\"&3=0))"
            "AND (matches.\"white\"=0 OR EXISTS (SELECT * FROM competitors WHERE "
            "competitors.\"index\"=matches.\"white\" AND competitors.\"deleted\"&3=0))",
            GHOST, GHOST /*COMPETITOR, COMPETITOR*/);
    db_exec(db_name, buffer, (gpointer)DB_NEXT_MATCH, db_callback_matches);

    /* check for rest times */
    if (auto_arrange &&
        next_match[1].number != INVALID_MATCH &&
        next_match[1].comment != COMMENT_MATCH_1 &&
        next_match[1].comment != COMMENT_MATCH_2) {
        gint result = 0, rtb = 0, rtw = 0;

        if ((rtb = competitor_cannot_match(next_match[1].blue, tatami, 1)))
            result |= MATCH_FLAG_BLUE_DELAYED | ((rtb & MATCH_FLAG_REST_TIME) ? MATCH_FLAG_BLUE_REST : 0);
        if ((rtw = competitor_cannot_match(next_match[1].white, tatami, 1)))
            result |= MATCH_FLAG_WHITE_DELAYED | ((rtw & MATCH_FLAG_REST_TIME) ? MATCH_FLAG_WHITE_REST : 0);

        next_match[1].flags = result;

        if (result && next_match[1].forcedtatami == 0) { // don't change fixed order
            for (i = 2; i < NEXT_MATCH_NUM; i++) {
                gint r = 0;
                rtb = rtw = 0;

                if (next_match[i].number == INVALID_MATCH)
                    break;

                if ((rtb = competitor_cannot_match(next_match[i].blue, tatami, i)))
                    r |= MATCH_FLAG_BLUE_DELAYED | ((rtb & MATCH_FLAG_REST_TIME) ? MATCH_FLAG_BLUE_REST : 0);
                if ((rtw = competitor_cannot_match(next_match[i].white, tatami, i)))
                    r |= MATCH_FLAG_WHITE_DELAYED | ((rtw & MATCH_FLAG_REST_TIME) ? MATCH_FLAG_WHITE_REST : 0);
                if (r == 0 && 
                    next_match[i].blue >= COMPETITOR &&
                    next_match[i].white >= COMPETITOR &&
                    (next_match[1].category != next_match[i].category ||
                     get_match_number_flag(next_match[i].category, 
                                           next_match[i].number) == 0)) {
                    struct match tmp = next_match[1];
                    next_match[1] = next_match[i];
                    next_match[i] = tmp;
                    break;
                } else {
                    next_match[i].flags = r;
                }
            }
        }
    }

    /* remove all match comments */
    sprintf(buffer, 
            "UPDATE matches SET \"comment\"=%d "
            "WHERE EXISTS (SELECT * FROM categories WHERE categories.\"tatami\"=%d AND "
            "categories.\"index\"=matches.\"category\" "
            "AND categories.\"deleted\"=0 "
            "AND (matches.\"comment\"=%d OR matches.\"comment\"=%d) "
            "AND matches.\"forcedtatami\"=0)",
            COMMENT_EMPTY, tatami, COMMENT_MATCH_1, COMMENT_MATCH_2);
    db_exec(db_name, buffer, 0, 0);
    sprintf(buffer, 
            "UPDATE matches SET \"comment\"=%d "
            "WHERE (\"comment\"=%d OR \"comment\"=%d) "
            "AND \"forcedtatami\"=%d",
            COMMENT_EMPTY, COMMENT_MATCH_1, COMMENT_MATCH_2, tatami);
    db_exec(db_name, buffer, 0, 0);

    /* set new next match */
    if (next_match[0].number != INVALID_MATCH) {
        sprintf(buffer, 
                "UPDATE matches SET \"comment\"=%d WHERE "
                "\"category\"=%d AND \"number\"=%d "
                "AND \"deleted\"&1=0",
                COMMENT_MATCH_1, next_match[0].category, next_match[0].number);
        db_exec(db_name, buffer, 0, 0);
    }

    /* set new second match */
    if (next_match[1].number != INVALID_MATCH) {
        sprintf(buffer, 
                "UPDATE matches SET \"comment\"=%d WHERE "
                "\"category\"=%d AND \"number\"=%d "
                "AND \"deleted\"&1=0",
                COMMENT_MATCH_2, next_match[1].category, next_match[1].number);
        db_exec(db_name, buffer, 0, 0);
    }

    /* update the display */
    sprintf(buffer, 
            "SELECT matches.* "
            "FROM matches, categories "
            "WHERE categories.\"tatami\"=%d AND categories.\"index\"=matches.\"category\" "
            "AND categories.\"deleted\"=0 AND matches.\"comment\">%d "
            "AND matches.\"forcedtatami\"=0",
            tatami, COMMENT_EMPTY);
    db_exec(db_name, buffer, (gpointer)ADD_MATCH, db_callback_matches);
    sprintf(buffer, 
            "SELECT * FROM matches "
            "WHERE \"forcedtatami\"=%d "
            "AND \"comment\">%d",
            tatami, COMMENT_EMPTY);
    db_exec(db_name, buffer, (gpointer)ADD_MATCH, db_callback_matches);

    for (i = 0; i < num_saved_matches; i++)
        db_read_match(saved_matches[i].category, saved_matches[i].number);

    num_saved_matches = 0;
#ifdef TATAMI_DEBUG
    if (tatami == TATAMI_DEBUG) {
        g_print("\nNEXT MATCHES ON TATAMI %d\n", tatami);
        for (i = 0; i < next_match_num; i++)
            g_print("%d: %d:%d = %d - %d\n", i, 
                    next_match[i].category, next_match[i].number, next_match[i].blue, next_match[i].white);
        g_print("\n");
    }
#endif
    return next_match;
}

struct match *db_next_match(gint category, gint tatami)
{
    gint i;

    current_round++;

    if (category) {
        tatami = db_get_tatami(category);
        /*** BUG
             sprintf(buffer, "SELECT * FROM categories WHERE \"category\"=%d", category);
             db_exec(db_name, buffer, (gpointer)DB_GET_SYSTEM, db_callback_categories);
             tatami = j.belt;
        ***/
    }

    if (tatami == 0)
        return NULL;

    next_match = next_matches[tatami];

    for (i = 0; i < NEXT_MATCH_NUM; i++) {
        memset(&next_match[i], 0, sizeof(struct match));
        next_match[i].number = INVALID_MATCH;
    }		

    next_match_num = 0;
    next_match_tatami = tatami;

    db_open();

    /* save current match data */
    num_saved_matches = 0;
    db_cmd((gpointer)SAVE_MATCH, db_callback_matches,
	   "SELECT matches.* "
	   "FROM matches, categories "
	   "WHERE categories.\"tatami\"=%d AND categories.\"index\"=matches.\"category\" "
	   "AND categories.\"deleted\"=0 "
	   "AND (matches.\"comment\"=%d OR matches.\"comment\"=%d)"
	   "AND matches.\"forcedtatami\"=0",
	   tatami, COMMENT_MATCH_1, COMMENT_MATCH_2);

    db_cmd((gpointer)SAVE_MATCH, db_callback_matches,
	   "SELECT * FROM matches WHERE \"forcedtatami\"=%d "
	   "AND (\"comment\"=%d OR \"comment\"=%d)",
	   tatami, COMMENT_MATCH_1, COMMENT_MATCH_2);

    /* remove comments from the display */
    db_cmd((gpointer)DB_REMOVE_COMMENT, db_callback_matches,
	   "SELECT matches.* "
	   "FROM matches, categories "
	   "WHERE categories.\"tatami\"=%d AND categories.\"index\"=matches.\"category\" "
	   "AND categories.\"deleted\"=0 AND matches.\"comment\">%d "
	   "AND matches.\"forcedtatami\"=0",
	   tatami, COMMENT_EMPTY);

    db_cmd((gpointer)DB_REMOVE_COMMENT, db_callback_matches,
	   "SELECT * FROM matches WHERE \"forcedtatami\"=%d "
	   "AND \"comment\">%d",
	   tatami, COMMENT_EMPTY);

    /* remove comments from old matches */
    db_cmd(NULL, NULL, 
	   "UPDATE matches SET \"comment\"=%d "
	   "WHERE EXISTS (SELECT * FROM categories WHERE "
	   "categories.\"tatami\"=%d AND (matches.\"blue_points\">0 OR "
	   "matches.\"white_points\">0) AND categories.\"index\"=matches.\"category\" "
	   "AND matches.\"comment\">%d AND categories.\"deleted\"=0 "
	   "AND matches.\"forcedtatami\"=0)",
	   COMMENT_EMPTY, tatami, COMMENT_EMPTY);

    db_cmd(NULL, NULL,
	   "UPDATE matches SET \"comment\"=%d "
	   "WHERE \"forcedtatami\"=%d AND \"comment\">%d "
	   "AND (\"blue_points\">0 OR \"white_points\">0)",
	   COMMENT_EMPTY, tatami, COMMENT_EMPTY);

    /* find all matches */
    db_cmd((gpointer)DB_NEXT_MATCH, db_callback_matches,
	   "SELECT * FROM matches "
	   "WHERE matches.\"blue_points\"=0 "
	   "AND matches.\"deleted\"&1=0 "
	   "AND matches.\"white_points\"=0 "
	   "AND matches.\"blue\"<>%d AND matches.\"white\"<>%d "
	   "AND (matches.\"blue\"=0 OR EXISTS (SELECT * FROM competitors WHERE "
	   "competitors.\"index\"=matches.\"blue\" AND competitors.\"deleted\"&3=0))"
	   "AND (matches.\"white\"=0 OR EXISTS (SELECT * FROM competitors WHERE "
	   "competitors.\"index\"=matches.\"white\" AND competitors.\"deleted\"&3=0))",
	   GHOST, GHOST /*COMPETITOR, COMPETITOR*/);

    db_close();

    // Distribute matches from the same category far away
#define CANNOT_MOVE(_i)  (next_match[_i].number == INVALID_MATCH ||     \
                          next_match[_i].comment == COMMENT_MATCH_1 ||  \
                          next_match[_i].comment == COMMENT_MATCH_2 ||  \
                          next_match[_i].forcednumber ||                \
                          next_match[_i].forcedtatami)

#define SAME_CAT_OR_PLAYERS(_m1, _m2)                             \
    (next_match[_m1].category == next_match[_m2].category ||      \
     next_match[_m1].blue == next_match[_m2].blue ||              \
     next_match[_m1].white == next_match[_m2].white ||            \
     next_match[_m1].blue == next_match[_m2].white ||             \
     next_match[_m1].white == next_match[_m2].blue)
    
#if 0  // can change order of bronze and gold fights   
    for (i = 0; i < next_match_num-2; i++) {
        gint j;

        if (CANNOT_MOVE(i+1))
            continue;
        
        if (!SAME_CAT_OR_PLAYERS(i, i+1))
            continue;

        for (j = i+2; j < next_match_num; j++) {
            if (next_match[i+1].group == next_match[j].group &&
                !SAME_CAT_OR_PLAYERS(i, j)) {
                gint k;
                struct match m = next_match[j];
                for (k = j; k > i+1; k--) {
                    next_match[k] = next_match[k-1];
                }
                next_match[i+1] = m;
                break;
            }
        }
    }
#endif

    /* check for rest times */
    if (auto_arrange &&
        next_match[1].number != INVALID_MATCH &&
        next_match[1].comment != COMMENT_MATCH_1 &&
        next_match[1].comment != COMMENT_MATCH_2) {
        gint result = 0, rtb = 0, rtw = 0;

        if ((rtb = competitor_cannot_match(next_match[1].blue, tatami, 1)))
            result |= MATCH_FLAG_BLUE_DELAYED | ((rtb & MATCH_FLAG_REST_TIME) ? MATCH_FLAG_BLUE_REST : 0);
        if ((rtw = competitor_cannot_match(next_match[1].white, tatami, 1)))
            result |= MATCH_FLAG_WHITE_DELAYED | ((rtw & MATCH_FLAG_REST_TIME) ? MATCH_FLAG_WHITE_REST : 0);

        next_match[1].flags = result;

        if (result && next_match[1].forcedtatami == 0) { // don't change fixed order
            for (i = 2; i < NEXT_MATCH_NUM; i++) {
                gint r = 0;
                rtb = rtw = 0;

                if (next_match[i].number == INVALID_MATCH)
                    break;

                if ((rtb = competitor_cannot_match(next_match[i].blue, tatami, i)))
                    r |= MATCH_FLAG_BLUE_DELAYED | ((rtb & MATCH_FLAG_REST_TIME) ? MATCH_FLAG_BLUE_REST : 0);
                if ((rtw = competitor_cannot_match(next_match[i].white, tatami, i)))
                    r |= MATCH_FLAG_WHITE_DELAYED | ((rtw & MATCH_FLAG_REST_TIME) ? MATCH_FLAG_WHITE_REST : 0);
                if (r == 0 && 
                    next_match[i].blue >= COMPETITOR &&
                    next_match[i].white >= COMPETITOR &&
                    (next_match[1].category != next_match[i].category ||
                     get_match_number_flag(next_match[i].category, 
                                           next_match[i].number) == 0)) {
                    struct match tmp = next_match[1];
                    next_match[1] = next_match[i];
                    next_match[i] = tmp;
                    break;
                } else {
                    next_match[i].flags = r;
                }
            }
        }
    }

    db_open();

    /* remove all match comments */
    db_cmd(NULL, NULL,
	   "UPDATE matches SET \"comment\"=%d "
	   "WHERE EXISTS (SELECT * FROM categories WHERE categories.\"tatami\"=%d AND "
	   "categories.\"index\"=matches.\"category\" "
	   "AND categories.\"deleted\"=0 "
	   "AND (matches.\"comment\"=%d OR matches.\"comment\"=%d) "
	   "AND matches.\"forcedtatami\"=0)",
	   COMMENT_EMPTY, tatami, COMMENT_MATCH_1, COMMENT_MATCH_2);

    db_cmd(NULL, NULL,
	   "UPDATE matches SET \"comment\"=%d "
	   "WHERE (\"comment\"=%d OR \"comment\"=%d) "
	   "AND \"forcedtatami\"=%d",
	   COMMENT_EMPTY, COMMENT_MATCH_1, COMMENT_MATCH_2, tatami);

    /* set new next match */
    if (next_match[0].number != INVALID_MATCH) {
	db_cmd(NULL, NULL,
	       "UPDATE matches SET \"comment\"=%d WHERE "
	       "\"category\"=%d AND \"number\"=%d "
	       "AND \"deleted\"&1=0",
	       COMMENT_MATCH_1, next_match[0].category, next_match[0].number);
    }

    /* set new second match */
    if (next_match[1].number != INVALID_MATCH) {
	db_cmd(NULL, NULL,
	       "UPDATE matches SET \"comment\"=%d WHERE "
	       "\"category\"=%d AND \"number\"=%d "
	       "AND \"deleted\"&1=0",
	       COMMENT_MATCH_2, next_match[1].category, next_match[1].number);
    }

    /* update the display */
    db_cmd((gpointer)ADD_MATCH, db_callback_matches,
	   "SELECT matches.* "
	   "FROM matches, categories "
	   "WHERE categories.\"tatami\"=%d AND categories.\"index\"=matches.\"category\" "
	   "AND categories.\"deleted\"=0 AND matches.\"comment\">%d "
	   "AND matches.\"forcedtatami\"=0",
	   tatami, COMMENT_EMPTY);

    db_cmd((gpointer)ADD_MATCH, db_callback_matches,
	   "SELECT * FROM matches "
	   "WHERE \"forcedtatami\"=%d "
	   "AND \"comment\">%d",
	   tatami, COMMENT_EMPTY);

    for (i = 0; i < num_saved_matches; i++) {
	db_cmd((gpointer)ADD_MATCH, db_callback_matches,
	       "SELECT * FROM matches WHERE \"category\"=%d AND \"number\"=%d",
	       saved_matches[i].category, saved_matches[i].number);
    }
    num_saved_matches = 0;

    db_close();

#ifdef TATAMI_DEBUG
    if (tatami == TATAMI_DEBUG) {
        g_print("\nNEXT MATCHES ON TATAMI %d\n", tatami);
        for (i = 0; i < next_match_num; i++)
            g_print("%d: %d:%d = %d - %d\n", i, 
                    next_match[i].category, next_match[i].number, next_match[i].blue, next_match[i].white);
        g_print("\n");
    }
#endif
    // coach info
    for (i = 0; i < NEXT_MATCH_NUM; i++) {
        if (next_match[i].number == INVALID_MATCH)
            continue;

        gint blue = next_match[i].blue;
        gint white = next_match[i].white;

        if (blue > 0 && blue < 10000 && coach_info[blue].waittime < 0)
            coach_info[blue].waittime = i;
        if (white > 0 && white < 10000 && coach_info[white].waittime < 0)
            coach_info[white].waittime = i;
    }		

    update_next_matches_coach_info();

    return next_match;
}

struct match *db_matches_waiting(void)
{
    gchar buffer[1000];
    gint i;

    for (i = 0; i < WAITING_MATCH_NUM; i++)
        matches_waiting[i].number = INVALID_MATCH;

    num_matches_waiting = 0;

    sprintf(buffer, 
            "SELECT * FROM matches "
            "WHERE matches.'blue_points'=0 "
            "AND matches.'deleted'&1=0 "
            "AND matches.'white_points'=0 "
            "AND matches.'blue'<>%d AND matches.'white'<>%d "
            "AND matches.'comment'=%d "
            "AND (matches.'blue'=0 OR EXISTS (SELECT * FROM competitors WHERE "
            "competitors.'index'=matches.'blue' AND competitors.'deleted'&3=0)) "
            "AND (matches.'white'=0 OR EXISTS (SELECT * FROM competitors WHERE "
            "competitors.'index'=matches.'white' AND competitors.'deleted'&3=0)) ",
            GHOST, GHOST, COMMENT_WAIT);
    db_exec(db_name, buffer, (gpointer)DB_MATCH_WAITING, db_callback_matches);

    return matches_waiting;
}
 
gint db_find_match_tatami(gint category, gint number)
{
    gchar buffer[1000];

    m.forcedtatami = 0;
    sprintf(buffer, "SELECT * FROM matches WHERE \"category\"=%d "
            "AND \"number\"=%d", category, number);
    db_exec(db_name, buffer, NULL, db_callback_matches);

    if (m.forcedtatami)
        return m.forcedtatami;

    return db_get_tatami(category);
}

void db_set_comment(gint category, gint number, gint comment)
{
    gchar buffer[1000];

    num_saved_matches = 0;
#ifdef TATAMI_DEBUG
    g_print("set comment cat=%d num=%d comm=%d\n", category, number, comment);
#endif
    /* remove existing */
    if (comment == COMMENT_MATCH_1 || comment == COMMENT_MATCH_2) {
        gint tatami = db_find_match_tatami(category, number);
#ifdef TATAMI_DEBUG
        g_print("%s: tatami=%d\n", __FUNCTION__, tatami);
#endif
#if 0
        sprintf(buffer, "SELECT * FROM categories WHERE \"category\"=%d", category);
        db_exec(db_name, buffer, (gpointer)DB_GET_SYSTEM, db_callback_categories);
        tatami = j.belt;
#endif
        /* save current match data */
        num_saved_matches = 0;
        sprintf(buffer, 
                "SELECT matches.* "
                "FROM matches, categories "
                "WHERE categories.\"tatami\"=%d AND categories.\"index\"=matches.\"category\" "
                "AND categories.\"deleted\"=0 "
                "AND (matches.\"comment\"=%d OR matches.\"comment\"=%d) "
                "AND matches.\"forcedtatami\"=0",
                tatami, COMMENT_MATCH_1, COMMENT_MATCH_2);
        db_exec(db_name, buffer, (gpointer)SAVE_MATCH, db_callback_matches);
        sprintf(buffer, 
                "SELECT * FROM matches WHERE \"forcedtatami\"=%d "
                "AND (\"comment\"=%d OR \"comment\"=%d)",
                tatami, COMMENT_MATCH_1, COMMENT_MATCH_2);
        db_exec(db_name, buffer, (gpointer)SAVE_MATCH, db_callback_matches);

        // remove existing old comment
        sprintf(buffer, 
                "UPDATE matches SET \"comment\"=%d "
                "WHERE EXISTS (SELECT * FROM categories WHERE categories.\"tatami\"=%d AND "
                "categories.\"index\"=matches.\"category\" "
                "AND categories.\"deleted\"=0 AND matches.\"comment\"=%d "
                "AND matches.\"forcedtatami\"=0)",
                COMMENT_EMPTY, tatami, comment);
        db_exec(db_name, buffer, 0, 0);

        sprintf(buffer, 
                "UPDATE matches SET \"comment\"=%d "
                "WHERE \"comment\"=%d "
                "AND \"forcedtatami\"=%d",
                COMMENT_EMPTY, comment, tatami);
        db_exec(db_name, buffer, 0, 0);
    }

    // set new comment
    sprintf(buffer, "UPDATE matches SET \"comment\"=%d WHERE \"category\"=%d AND \"number\"=%d",
            comment, category, number);
    db_exec(db_name, buffer, 0, 0);

    /* update the display */
    sprintf(buffer, 
            "SELECT * FROM matches WHERE \"category\"=%d AND \"number\"=%d AND \"deleted\"&1=0",
            category, number);
    db_exec(db_name, buffer, (gpointer)ADD_MATCH, db_callback_matches);

    /* update saved macthes display */
    gint i;
    for (i = 0; i < num_saved_matches; i++)
        db_read_match(saved_matches[i].category, saved_matches[i].number);

    num_saved_matches = 0;
}

void db_set_forced_tatami_number_delay(gint category, gint matchnumber, gint tatami, gint num, gboolean delay)
{
        db_exec_str(NULL, NULL,
                    "UPDATE matches SET \"forcedtatami\"=%d, \"forcednumber\"=%d, "
                    "\"comment\"=%d WHERE \"category\"=%d AND \"number\"=%d AND "
                    "\"forcedtatami\"=0 AND \"forcednumber\"=0",
                    tatami, num,
                    delay ? COMMENT_WAIT : COMMENT_EMPTY, category, matchnumber);
}


void set_judogi_status(gint index, gint flags)
{
    gint t, n;

    // only the three first matches are affected
    for (n = 0; n < 3; n++) {
        for (t = 1; t <= NUM_TATAMIS; t++) {
            if (next_matches[t][n].number == INVALID_MATCH)
                continue;
            if (next_matches[t][n].blue == index || next_matches[t][n].white == index) {
                if (next_matches[t][n].blue == index) {
                    next_matches[t][n].deleted &= ~(MATCH_FLAG_JUDOGI1_OK | MATCH_FLAG_JUDOGI1_NOK);
                    if (flags & JUDOGI_OK)
                        next_matches[t][n].deleted |= MATCH_FLAG_JUDOGI1_OK;
                    else if (flags & JUDOGI_NOK)
                        next_matches[t][n].deleted |= MATCH_FLAG_JUDOGI1_NOK;
                } else {
                    next_matches[t][n].deleted &= ~(MATCH_FLAG_JUDOGI2_OK | MATCH_FLAG_JUDOGI2_NOK);
                    if (flags & JUDOGI_OK)
                        next_matches[t][n].deleted |= MATCH_FLAG_JUDOGI2_OK;
                    else if (flags & JUDOGI_NOK)
                        next_matches[t][n].deleted |= MATCH_FLAG_JUDOGI2_NOK;
                }

                db_exec_str(NULL, NULL,
                            "UPDATE matches SET \"deleted\"=%d "
                            "WHERE \"category\"=%d AND \"number\"=%d",
                            next_matches[t][n].deleted, next_matches[t][n].category, next_matches[t][n].number);
                send_match(t, n+1, &(next_matches[t][n]));
                send_next_matches(0, t, &(next_matches[t][0]));
            }
        }
    }
}

gint get_judogi_status(gint index)
{
    gint t, n;

    for (n = 0; n < NEXT_MATCH_NUM; n++) {
        for (t = 1; t <= NUM_TATAMIS; t++) {
            if (next_matches[t][n].number == INVALID_MATCH)
                continue;

            if (next_matches[t][n].blue == index) {
                if (next_matches[t][n].deleted & MATCH_FLAG_JUDOGI1_OK)
                    return JUDOGI_OK;
                else if (next_matches[t][n].deleted & MATCH_FLAG_JUDOGI1_NOK)
                    return JUDOGI_NOK;
                return 0;
            } else if (next_matches[t][n].white == index) {
                if (next_matches[t][n].deleted & MATCH_FLAG_JUDOGI2_OK)
                    return JUDOGI_OK;
                else if (next_matches[t][n].deleted & MATCH_FLAG_JUDOGI2_NOK)
                    return JUDOGI_NOK;
                return 0;
            }
        }
    }

    return 0;
}

static gint maxmatch;
static gint nextforcednum = 1;
static gint medalmatchexists;

static int db_callback_match_num(void *data, int argc, char **argv, char **azColName)
{
    gint i;

    for (i = 0; i < argc; i++) {
        if (!strcmp(azColName[i], "MAX(number)"))
            maxmatch = argv[i] ? atoi(argv[i]) : 0;
        else if (!strcmp(azColName[i], "MAX(forcednumber)"))
            nextforcednum = argv[i] ? atoi(argv[i]) : 0;
    }

    return 0;
}

static int db_callback_medal_match_exists(void *data, int argc, char **argv, char **azColName)
{
    gint i;
    for (i = 0; i < argc; i++) {
        if (!strcmp(azColName[i], "forcedtatami"))
            medalmatchexists |= argv[i] ? 1 << atoi(argv[i]) : 0;
    }
    return 0;
}

gint db_force_match_number(gint category)
{
    gint i;

    maxmatch = 0;
    medalmatchexists = 0;

    db_exec_str(NULL, db_callback_match_num,
                "SELECT MAX(number) FROM  matches WHERE \"category\"=%d AND"
                "(\"blue_points\">0 OR \"white_points\">0)",
                category);

    db_exec_str(NULL, db_callback_medal_match_exists,
                "SELECT forcedtatami FROM matches WHERE \"category\"=%d AND "
                "\"number\"=%d AND \"forcedtatami\">0 AND \"forcednumber\"=0",
                category, maxmatch+1);

    if (!medalmatchexists)
        return 0;

    db_exec_str(NULL, db_callback_match_num,
                "SELECT MAX(forcednumber) FROM matches");

    db_exec_str(NULL, db_callback_medal_match_exists,
                "SELECT forcedtatami FROM matches WHERE \"category\"=%d AND "
                "\"number\">%d AND \"forcedtatami\">0 AND \"forcednumber\"=0",
                category, maxmatch);

    db_exec_str(NULL, NULL,
                "UPDATE matches SET \"forcednumber\"=%d "
                "WHERE \"category\"=%d AND \"number\"=%d",
                ++nextforcednum, category, maxmatch+1);
    db_exec_str(NULL, NULL,
                "UPDATE matches SET \"forcednumber\"=%d "
                "WHERE \"category\"=%d AND \"number\"=%d",
                ++nextforcednum, category, maxmatch+2);
    db_exec_str(NULL, NULL,
                "UPDATE matches SET \"forcednumber\"=%d "
                "WHERE \"category\"=%d AND \"number\"=%d",
                ++nextforcednum, category, maxmatch+3);

    for (i = 1; i <= number_of_tatamis; i++)
        if (medalmatchexists & (1 << i))
            update_matches(0, (struct compsys){0,0,0,0}, i);

    return medalmatchexists;
}

void update_next_matches_coach_info(void)
{
    gint i;

    if (automatic_web_page_update == FALSE)
        return;

    gchar *file = g_build_filename(current_directory, "c-matches.txt", NULL);
    FILE *f = fopen(file, "wb");
    g_free(file);

    if (f) {
        for (i = 0; i < 10000; i++)
            fprintf(f, "%d\t%d\n", coach_info[i].tatami, coach_info[i].waittime);

        fclose(f);
    }
}

