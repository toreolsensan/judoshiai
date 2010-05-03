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

static void set_comment(GtkWidget *menuitem, gpointer userdata);

struct matchid {
    gint group;
    gint category;
    gint number;
};

GtkWidget *match_view[NUM_TATAMIS];
GtkWidget *next_match[NUM_TATAMIS][2];

struct next_match_info next_matches_info[NUM_TATAMIS][2];

#define COPY_MATCH_INFO(_what, _tatami, _which, _str) g_strlcpy(next_matches_info[_tatami-1][_which]._what, \
                                                                _str, sizeof(next_matches_info[0][0]._what))

#define NULL_MATCH_INFO(_what, _tatami, _which) next_matches_info[_tatami-1][_which]._what[0] = 0

static gint sort_iter_compare_func_1(GtkTreeModel *model,
                                     GtkTreeIter  *a,
                                     GtkTreeIter  *b,
                                     gpointer      userdata)
{
    gint sortcol = GPOINTER_TO_INT(userdata);
    guint data1, data2;

    gtk_tree_model_get(model, a, sortcol, &data1, -1);
    gtk_tree_model_get(model, b, sortcol, &data2, -1);

    if (data1 > data2)
        return 1;

    if (data1 < data2)
        return -1;

    return 0;
}

static void name_cell_data_func (GtkTreeViewColumn *col,
                                 GtkCellRenderer   *renderer,
                                 GtkTreeModel      *model,
                                 GtkTreeIter       *iter,
                                 gpointer           user_data)
{
    gchar  buf[128];
    guint  index;
    gboolean visible;
    gint category;
    struct judoka *j;

    gtk_tree_model_get(model, iter, 
                       (gint)user_data, &index, 
                       COL_MATCH_CAT, &category,
                       COL_MATCH_VIS, &visible, 
                       -1);
  
    if (visible == FALSE) {
        if ((gint)user_data != COL_MATCH_BLUE)
            return;

        gchar *sys;
        struct category_data *data = avl_get_category(category);
        if (!data) {
            g_object_set(renderer, "text", "", NULL);
            return;
        }

        switch (data->system & SYSTEM_MASK) {
        case SYSTEM_POOL: sys = _("Pool"); break;
        case SYSTEM_DPOOL: sys = _("Double pool"); break;
        default: sys = _("Double repechage");
        }

        g_object_set(renderer, 
                     "cell-background-set", FALSE, 
                     NULL);
        g_snprintf(buf, sizeof(buf), "%s [%d]", sys, data->system & COMPETITORS_MASK);
        g_object_set(renderer, "text", buf, NULL);
		
        return;
    }


    if (index == GHOST) {
        g_snprintf(buf, sizeof(buf), "-");
    } else {
        j = get_data(index);
        if (j && j->index)
            g_snprintf(buf, sizeof(buf), "%s %s", j->first, j->last);
        else
            g_snprintf(buf, sizeof(buf), "?");

        free_judoka(j);
    }
    g_object_set(renderer, "foreground-set", FALSE, NULL); /* print this normal */

    g_object_set(renderer, "text", buf, NULL);
}

static void category_cell_data_func (GtkTreeViewColumn *col,
                                     GtkCellRenderer   *renderer,
                                     GtkTreeModel      *model,
                                     GtkTreeIter       *iter,
                                     gpointer           user_data)
{
    gchar  buf[128];
    guint  index;
    gboolean visible;
    gint category;
    struct judoka *j;

    gtk_tree_model_get(model, iter, 
                       (gint)user_data, &index, 
                       COL_MATCH_CAT, &category,
                       COL_MATCH_VIS, &visible, 
                       -1);
  
    if (visible || category == 0) {
        g_object_set(renderer, "text", "", NULL);
        return;
    }

    j = get_data(index);
    if (j) {  
        g_snprintf(buf, sizeof(buf), "%s", j->last);
        g_object_set(renderer, "foreground-set", FALSE, NULL); /* print this normal */
        free_judoka(j);
    } else {
        g_snprintf(buf, sizeof(buf), "?");
        g_object_set(renderer, "foreground-set", FALSE, NULL); /* print this normal */
    }

    g_object_set(renderer, "text", buf, NULL);
}

static void group_cell_data_func (GtkTreeViewColumn *col,
                                  GtkCellRenderer   *renderer,
                                  GtkTreeModel      *model,
                                  GtkTreeIter       *iter,
                                  gpointer           user_data)
{
    gchar  buf[128];
    gboolean visible;
    gint category;
    gint group;

    gtk_tree_model_get(model, iter,
                       COL_MATCH_GROUP, &group,
                       COL_MATCH_CAT, &category,
                       COL_MATCH_VIS, &visible, 
                       -1);

    if (category == 0) {
        g_snprintf(buf, sizeof(buf), "%d", group);
        g_object_set(renderer, "foreground-set", FALSE, NULL); /* print this normal */
        g_object_set(renderer, "text", buf, NULL);
    } else {
        g_object_set(renderer, "text", "", NULL);
    }

}  

static void score_cell_data_func (GtkTreeViewColumn *col,
                                  GtkCellRenderer   *renderer,
                                  GtkTreeModel      *model,
                                  GtkTreeIter       *iter,
                                  gpointer           user_data)
{
    gchar  buf[128];
    gboolean visible;
    guint score;

    gtk_tree_model_get(model, iter,
                       (gint)user_data, &score,
                       COL_MATCH_VIS, &visible, 
                       -1);

    if (visible) {
        g_snprintf(buf, sizeof(buf), "%d%d%d%d%d", 
                   (score>>16)&15, (score>>12)&15, (score>>8)&15,
                   (score>>4)&15, score&15);
        g_object_set(renderer, "foreground-set", FALSE, NULL); /* print this normal */
        g_object_set(renderer, "text", buf, NULL);
    } else {
        g_object_set(renderer, "text", "", NULL);
    }

}  

static void time_cell_data_func (GtkTreeViewColumn *col,
                                 GtkCellRenderer   *renderer,
                                 GtkTreeModel      *model,
                                 GtkTreeIter       *iter,
                                 gpointer           user_data)
{
    gchar  buf[128];
    gboolean visible;
    guint tm;

    gtk_tree_model_get(model, iter,
                       (gint)user_data, &tm,
                       COL_MATCH_VIS, &visible, 
                       -1);

    if (visible) {
        g_snprintf(buf, sizeof(buf), "%d:%02d", 
                   tm / 60, tm % 60);
        g_object_set(renderer, "foreground-set", FALSE, NULL); /* print this normal */
        g_object_set(renderer, "text", buf, NULL);
    } else {
        g_object_set(renderer, "text", "", NULL);
    }
}  

static void comment_cell_data_func (GtkTreeViewColumn *col,
                                    GtkCellRenderer   *renderer,
                                    GtkTreeModel      *model,
                                    GtkTreeIter       *iter,
                                    gpointer           user_data)
{
    gboolean visible;
    guint comment;
    gint bpts, wpts, b, w, status, n;
    gchar buf[128];

    gtk_tree_model_get(model, iter,
                       COL_MATCH_VIS, &visible, 
                       COL_MATCH_COMMENT, &comment,
                       COL_MATCH_BLUE_POINTS, &bpts,
                       COL_MATCH_WHITE_POINTS, &wpts,
                       COL_MATCH_BLUE, &b,
                       COL_MATCH_WHITE, &w,
                       COL_MATCH_STATUS, &status,
                       -1);

    if (visible) {
        g_object_set(renderer, "foreground-set", FALSE, NULL); /* print this normal */

        if (bpts || wpts || b == GHOST || w == GHOST)
            g_object_set(renderer, 
                         "cell-background", "Green", 
                         "cell-background-set", TRUE, 
                         NULL);
        else if (status & MATCH_STATUS_TATAMI_MASK)
            g_object_set(renderer, 
                         "cell-background", "Yellow", 
                         "cell-background-set", TRUE, 
                         NULL);
        else
            g_object_set(renderer, 
                         "cell-background-set", FALSE, 
                         NULL);

        switch (comment) {
        case COMMENT_EMPTY:
            buf[0] = 0; n = 0;
            break;
        case COMMENT_MATCH_1:
            n = sprintf(buf, "%s", _("Next match"));
            break;
        case COMMENT_MATCH_2:
            n = sprintf(buf, "%s", _("Preparing"));
            break;
        case COMMENT_WAIT:
            n = sprintf(buf, "%s", _("Delay the match"));
            break;
        default:
            n = sprintf(buf, "?");
        }

        if (status & MATCH_STATUS_TATAMI_MASK)
            sprintf(buf+n, " (TATAMI %d)", status & MATCH_STATUS_TATAMI_MASK);

        g_object_set(renderer, "text", buf, NULL);
    } else {
        g_object_set(renderer, "text", "", NULL);
    }
}  

#define IGNORE_MATCHES(_i) ((ju[c[_i]]->deleted & HANSOKUMAKE) && all[c[_i]] == FALSE)
#define APPROVE_MATCHES(_i) (((ju[c[_i]]->deleted & HANSOKUMAKE) == 0) || all[c[_i]])

#define TIE3(_a,_b,_c) (wins[c[_a]] == wins[c[_b]] && wins[c[_b]] == wins[c[_c]] && \
                        pts[c[_a]] == pts[c[_b]] && pts[c[_b]] == pts[c[_c]] && \
                        mw[c[_a]][c[_b]] == mw[c[_b]][c[_c]] && mw[c[_b]][c[_c]] == mw[c[_c]][c[_a]] && \
                        APPROVE_MATCHES(_a) && APPROVE_MATCHES(_b) && APPROVE_MATCHES(_c))

#define SWAP(_a,_b) do { gint tmp = c[_a]; c[_a] = c[_b]; c[_b] = tmp; } while (0)

#define WEIGHT(_a) (ju[c[_a]]->weight)

#define MATCHED(_a) (m[_a].blue_points || m[_a].white_points)

#define MATCHED_FRENCH_1(_a) (m[_a].blue_points || m[_a].white_points || \
			      m[_a].blue == GHOST || m[_a].white == GHOST || \
			      db_has_hansokumake(m[_a].blue) ||         \
			      db_has_hansokumake(m[_a].white))

#define MATCHED_FRENCH(x) (x < 0 ? MATCHED_FRENCH_1(-x) : MATCHED_FRENCH_1(x))

#define WINNER_1(_a) (m[_a].blue_points ? m[_a].blue :			\
		      (m[_a].white_points ? m[_a].white :		\
		       (m[_a].blue == GHOST ? m[_a].white :		\
			(m[_a].white == GHOST ? m[_a].blue :            \
			 (db_has_hansokumake(m[_a].blue) ? m[_a].white : \
			  (db_has_hansokumake(m[_a].white) ? m[_a].blue : NO_COMPETITOR))))))

#define WINNER(x) (x < 0 ? WINNER_1(-x) : WINNER_1(x))

#define LOSER_1(_a) (m[_a].blue_points ? m[_a].white :			\
		     (m[_a].white_points ? m[_a].blue :			\
		      (m[_a].blue == GHOST ? m[_a].blue :		\
		       (m[_a].white == GHOST ? m[_a].white :		\
			(db_has_hansokumake(m[_a].blue) ? m[_a].blue :  \
			 (db_has_hansokumake(m[_a].white) ? m[_a].white : NO_COMPETITOR))))))

#define LOSER(x) (x < 0 ? LOSER_1(-x) : LOSER_1(x))

#define WINNER_OR_LOSER(x) (x < 0 ? LOSER_1(-x) : WINNER_1(x))

#define PREV_BLUE(_x) french_matches[table][sys][_x][0]
#define PREV_WHITE(_x) french_matches[table][sys][_x][1]

#define GET_PREV(_x) (_x < 0 ?                                          \
                      (m[-_x].blue_points ? PREV_WHITE(-_x) : PREV_BLUE(-_x)) : \
                      (m[_x].blue_points || m[_x].white == GHOST || db_has_hansokumake(m[_x].white) ? PREV_BLUE(_x) : PREV_WHITE(_x)))

#define CAN_WIN_H (yes[c[j]] && (ju[c[j]]->deleted & HANSOKUMAKE) == 0)
#define CAN_WIN (yes[c[j]])

/*
#define COPY_PLAYER(_to, _from)  do {                     \
        _to = _from;                                            \
        if (db_has_hansokumake(_to)) _to = GHOST; } while (0)
*/
#define COPY_PLAYER(_to, _which, _from)  do {				\
	if (!MATCHED(_to))						\
	    m[_to]._which = _from;					\
	else if (m[_to]._which != _from)				\
	    g_print("MISMATCH match=%d old=%d new=%d\n", _to, m[_to]._which, _from); \
    } while (0)

void get_pool_winner(gint num, gint c[11], gboolean yes[11], 
                     gint wins[11], gint pts[11], 
                     gboolean mw[11][11], struct judoka *ju[11], gboolean all[11])
{
    gint i, j;
        
    for (i = 0; i <= 10; i++) {
        c[i] = i;
    }
        
    for (i = 1; i < 10; i++) {
        for (j = i+1; j <= 10; j++) {
            if ((yes[c[i]] && yes[c[j]] && 
                 (ju[c[i]]->deleted & HANSOKUMAKE) &&
                 all[c[i]] == FALSE) ||                 /* hansoku-make */ 

                (yes[c[i]] == FALSE && CAN_WIN) ||      /* competitor not active */

                (wins[c[i]] < wins[c[j]] && CAN_WIN) || /* more wins  */

                (wins[c[i]] == wins[c[j]] &&            /* same wins, more points  */
                 pts[c[i]] < pts[c[j]]  && CAN_WIN) ||

                (wins[c[i]] == wins[c[j]] &&            /* wins same, points same */
                 pts[c[i]] == pts[c[j]] &&
                 mw[c[j]][c[i]] && CAN_WIN)) {
                gint tmp = c[i];
                c[i] = c[j];
                c[j] = tmp;
            }
        }
    }

    if (num >= 3 && TIE3(1,2,3)) {
        /* weight matters */
        if (WEIGHT(1) > WEIGHT(2)) SWAP(1,2);
        if (WEIGHT(1) > WEIGHT(3)) SWAP(1,3);
        if (WEIGHT(2) > WEIGHT(3)) SWAP(2,3);
        if (WEIGHT(1) == WEIGHT(2) && mw[c[2]][c[1]]) SWAP(1,2);
        if (WEIGHT(2) == WEIGHT(3) && mw[c[3]][c[2]]) SWAP(2,3);
    }
    if (num >= 4 && TIE3(2,3,4)) {
        /* weight matters */
        if (WEIGHT(2) > WEIGHT(3)) SWAP(2,3);
        if (WEIGHT(2) > WEIGHT(4)) SWAP(2,4);
        if (WEIGHT(3) > WEIGHT(4)) SWAP(3,4);
        if (WEIGHT(2) == WEIGHT(3) && mw[c[3]][c[2]]) SWAP(2,3);
        if (WEIGHT(3) == WEIGHT(4) && mw[c[4]][c[3]]) SWAP(3,4);
    }
    if (num >= 5 && TIE3(3,4,5)) {
        /* weight matters */
        if (WEIGHT(3) > WEIGHT(4)) SWAP(3,4);
        if (WEIGHT(3) > WEIGHT(5)) SWAP(3,5);
        if (WEIGHT(4) > WEIGHT(5)) SWAP(4,5);
        if (WEIGHT(3) == WEIGHT(4) && mw[c[4]][c[3]]) SWAP(3,4);
        if (WEIGHT(4) == WEIGHT(5) && mw[c[5]][c[4]]) SWAP(4,5);
    }

#if 0
    for (i = 1; i < 8; i++)
        if (ju[c[i]])
            g_print("Kilpailija %s: %d voittoa ja %d pistettä, paino %d, del %d\n", 
                    ju[c[i]]->last, wins[c[i]], pts[c[i]], ju[c[i]]->weight, ju[c[i]]->deleted);
    g_print("\n");
#endif
}

static gint num_matches_table[] = {0,0,1,3,6,10,6,9,12,16,20};

gint num_matches(gint num_judokas)
{
    if (num_judokas == 2 && three_matches_for_two)
        return 3;
    else
        return num_matches_table[num_judokas]; 
}

void fill_pool_struct(gint category, gint num, struct pool_matches *pm)
{
    gint i, k;
    gboolean all_done;

    memset(pm, 0, sizeof(*pm));
    pm->finished = TRUE;
    pm->num_matches = num_matches(num);

    db_read_category_matches(category, pm->m);

    if (num == 1) {
        GtkTreeIter iter, iter2;

        if (find_iter(&iter, category)) {
            if (gtk_tree_model_iter_children(current_model, &iter2, &iter)) {
                pm->j[1] = get_data_by_iter(&iter2);
            }
        }
        return;
    }

    /* initial information */
    for (i = 1; i <= num_matches(num); i++) {
        gint blue = pools[num][i-1][0];
        gint white = pools[num][i-1][1];
        pm->yes[blue] = pm->yes[white] = TRUE;
        if (pm->j[blue] == NULL)
            pm->j[blue] = get_data(pm->m[i].blue);
        if (pm->j[white] == NULL)
            pm->j[white] = get_data(pm->m[i].white);

        if (pm->j[blue] == NULL)
            pm->yes[blue] = FALSE;
        if (pm->j[white] == NULL)
            pm->yes[white] = FALSE;
    }

    /* check for hansoku-makes */
    for (k = 1; k <= num; k++) {
        if (pm->j[k] == NULL || (pm->j[k]->deleted & HANSOKUMAKE) == 0)
            continue; /* competitor ok */
        //pm->yes[k] = FALSE;
        all_done = TRUE;
        for (i = 1; i <= num_matches(num); i++) {
            gint blue = pools[num][i-1][0];
            gint white = pools[num][i-1][1];
            if ((blue == k || white == k) &&
                pm->m[i].blue_points == 0 && pm->m[i].white_points == 0) {
                all_done = FALSE; /* undone match */
                break;
            }
        }
        pm->all_matched[k] = all_done;
        if (all_done)
            continue; /* all matched, points are valid */

        for (i = 1; i <= num_matches(num); i++) {
            gint blue = pools[num][i-1][0];
            gint white = pools[num][i-1][1];
            if (blue == k || white == k)
                pm->m[i].ignore = TRUE; /* hansoku-make matches are void */
        }
    }

    for (i = 1; i <= num_matches(num); i++) {
        gint blue = pools[num][i-1][0];
        gint white = pools[num][i-1][1];

        if (pm->m[i].ignore)
            continue;

        if (pm->m[i].blue_points && pm->yes[blue] == TRUE && pm->yes[white] == TRUE) {
            pm->wins[blue]++;
            pm->pts[blue] += pm->m[i].blue_points;
            pm->mw[blue][white] = TRUE;
            pm->mw[white][blue] = FALSE;
        } else if (pm->m[i].white_points && pm->yes[blue] == TRUE && pm->yes[white] == TRUE) {
            pm->wins[white]++;
            pm->pts[white] += pm->m[i].white_points;
            pm->mw[blue][white] = FALSE;
            pm->mw[white][blue] = TRUE;
        } else if (pm->yes[blue] == TRUE && pm->yes[white] == TRUE)
            pm->finished = FALSE;
    }

    /* special case: two competitors and three matches */
    if (num == 2 && num_matches(num) == 3 &&
        ((pm->m[1].blue_points && pm->m[2].blue_points) ||
         (pm->m[1].white_points && pm->m[2].white_points))) {
        pm->finished = TRUE;

        db_set_comment(category, 3, COMMENT_WAIT);        
    }
}

void empty_pool_struct(struct pool_matches *pm)
{
    gint i;

    for (i = 0; i < 11; i++)
        free_judoka(pm->j[i]);
}

static void update_pool_matches(gint category, gint num)
{
    struct pool_matches pm;
    gint i;

    if (automatic_sheet_update || automatic_web_page_update)
        current_category = category;
    //g_print("current_category = %d\n", category);

    fill_pool_struct(category, num, &pm);

    if (pm.finished && num <= 5) {
        get_pool_winner(num, pm.c, pm.yes, pm.wins, pm.pts, pm.mw, pm.j, pm.all_matched);
        goto out;
    } 

    if (num <= 5 || num > 10) 
        goto out;

    gboolean yes_a[11], yes_b[11];
    gint c_a[11], c_b[11];
    gint last_match = num_matches(num);

    memset(yes_a, 0, sizeof(yes_a));
    memset(yes_b, 0, sizeof(yes_b));
    memset(c_a, 0, sizeof(c_a));
    memset(c_b, 0, sizeof(c_b));

    for (i = 1; i <= num; i++) {
        if (i <= (num - num/2))
            yes_a[i] = TRUE;
        else
            yes_b[i] = TRUE;
    }
    
    get_pool_winner(num - num/2, c_a, yes_a, pm.wins, pm.pts, pm.mw, pm.j, pm.all_matched);
    get_pool_winner(num/2, c_b, yes_b, pm.wins, pm.pts, pm.mw, pm.j, pm.all_matched);

    if (!pm.finished /*MATCHED_POOL(last_match)*/) {
        struct match ma;
                
        memset(&ma, 0, sizeof(ma));
        ma.category = category;

        for (i = 1; i <= 3; i++) {
            ma.number = i + last_match;
            set_match(&ma);
            db_set_match(&ma);
        }

        goto out;
    }
    
    if ((MATCHED_POOL(last_match+1) == FALSE) ||
        (MATCHED_POOL_P(last_match+1) == FALSE && 
         MATCHED_POOL_P(last_match+2) == FALSE &&
         MATCHED_POOL(last_match+3) == FALSE)) {
        struct match ma;
                
        memset(&ma, 0, sizeof(ma));
        ma.category = category;
        ma.number = last_match+1;
        ma.blue = (pm.j[c_a[1]]->deleted & HANSOKUMAKE) ? GHOST : pm.j[c_a[1]]->index;
        ma.white = (pm.j[c_b[2]]->deleted & HANSOKUMAKE) ? GHOST : pm.j[c_b[2]]->index;

        set_match(&ma);
        db_set_match(&ma);
    }

    if ((MATCHED_POOL(last_match+2) == FALSE) ||
        (MATCHED_POOL_P(last_match+2) == FALSE &&
         MATCHED_POOL(last_match+3) == FALSE)) {
        struct match ma;
                
        memset(&ma, 0, sizeof(ma));
        ma.category = category;
        ma.number = last_match+2;
        ma.blue = (pm.j[c_b[1]]->deleted & HANSOKUMAKE) ? GHOST : pm.j[c_b[1]]->index;
        ma.white = (pm.j[c_a[2]]->deleted & HANSOKUMAKE) ? GHOST : pm.j[c_a[2]]->index;

        set_match(&ma);
        db_set_match(&ma);
    }

    
    if (MATCHED_POOL(last_match+1) &&
        MATCHED_POOL(last_match+2) &&
        !MATCHED_POOL(last_match+3)) {
        struct match ma;
                
        memset(&ma, 0, sizeof(ma));
        ma.category = category;
        ma.number = last_match+3;
        if (pm.m[last_match+1].blue_points || pm.m[last_match+1].white == GHOST)
            ma.blue = pm.m[last_match+1].blue;
        else
            ma.blue = pm.m[last_match+1].white;

        if (pm.m[last_match+2].blue_points || pm.m[last_match+2].white == GHOST)
            ma.white = pm.m[last_match+2].blue;
        else
            ma.white = pm.m[last_match+2].white;

        set_match(&ma);
        db_set_match(&ma);
    }

 out:
    empty_pool_struct(&pm);
}


static void set_repechage_16(struct match m[], gint table, gint i)
{
    gint p;
    gint sys = 1;
    
    if (table == TABLE_SWE_ENKELT_AATERKVAL) {
	if (!MATCHED_FRENCH(13+i))
	    return;

	p = GET_PREV(13+i);
	COPY_PLAYER((15+i), white, LOSER(p));
	p = GET_PREV(p);
	COPY_PLAYER((15+i), blue, LOSER(p));
	set_match(&m[15+i]); db_set_match(&m[15+i]);
    } else {
	if (!MATCHED_FRENCH(9+i))
	    return;

	COPY_PLAYER((13+i), white, LOSER(9+i));
	p = GET_PREV(9+i);
	COPY_PLAYER((13+i), blue, LOSER(p));
	set_match(&m[13+i]); db_set_match(&m[13+i]);
    }
}

static void set_repechage_32(struct match m[], gint table, gint i)
{
    gint p;
    gint sys = 2;

    if (table == TABLE_SWE_ENKELT_AATERKVAL) {
	if (!MATCHED_FRENCH(29+i))
	    return;

	p = GET_PREV(29+i);
	COPY_PLAYER((33+i), white, LOSER(p));
	p = GET_PREV(p);
	COPY_PLAYER((31+i), white, LOSER(p));
	p = GET_PREV(p);
	COPY_PLAYER((31+i), blue, LOSER(p));
	set_match(&m[33+i]); db_set_match(&m[33+i]);
	set_match(&m[31+i]); db_set_match(&m[29+i]);
    } else {
	if (!MATCHED_FRENCH(25+i))
	    return;

	COPY_PLAYER((33+i), white, LOSER(25+i));
	p = GET_PREV(25+i);
	COPY_PLAYER((29+i), white, LOSER(p));
	p = GET_PREV(p);
	COPY_PLAYER((29+i), blue, LOSER(p));
	set_match(&m[33+i]); db_set_match(&m[33+i]);
	set_match(&m[29+i]); db_set_match(&m[29+i]);
    }
}

static void set_repechage_64(struct match m[], gint table, gint i)
{
    gint p;
    gint sys = 3;

    if (table == TABLE_SWE_ENKELT_AATERKVAL) {
	if (!MATCHED_FRENCH(61+i))
	    return;

	p = GET_PREV(61+i);
	COPY_PLAYER((67+i), white, LOSER(p));
	p = GET_PREV(p);
	COPY_PLAYER((65+i), white, LOSER(p));
	p = GET_PREV(p);
	COPY_PLAYER((63+i), white, LOSER(p));
	p = GET_PREV(p);
	COPY_PLAYER((63+i), blue, LOSER(p));
	set_match(&m[67+i]); db_set_match(&m[69+i]);
	set_match(&m[65+i]); db_set_match(&m[65+i]);
	set_match(&m[63+i]); db_set_match(&m[61+i]);
    } else {
	if (!MATCHED_FRENCH(57+i))
	    return;

	COPY_PLAYER((69+i), white, LOSER(57+i));
	p = GET_PREV(57+i);
	COPY_PLAYER((65+i), white, LOSER(p));
	p = GET_PREV(p);
	COPY_PLAYER((61+i), white, LOSER(p));
	p = GET_PREV(p);
	COPY_PLAYER((61+i), blue, LOSER(p));
	set_match(&m[69+i]); db_set_match(&m[69+i]);
	set_match(&m[65+i]); db_set_match(&m[65+i]);
	set_match(&m[61+i]); db_set_match(&m[61+i]);
    }
}

static void update_french_matches(gint category, gint systm)
{
    gint i;
    struct match m[NUM_MATCHES];
    gint sys = ((systm & SYSTEM_MASK) - SYSTEM_FRENCH_8) >> SYSTEM_MASK_SHIFT;
    gint table = (systm & SYSTEM_TABLE_MASK) >> SYSTEM_TABLE_SHIFT;

    if (automatic_sheet_update || automatic_web_page_update)
        current_category = category;
    //g_print("current_category = %d\n", category);

    memset(m, 0, sizeof(m));
    db_read_category_matches(category, m);
        
    if (table == TABLE_DOUBLE_REPECHAGE || 
	table == TABLE_SWE_DUBBELT_AATERKVAL ||
	table == TABLE_SWE_ENKELT_AATERKVAL) {
	/* Repechage */
	if (sys == FRENCH_16) {
	    set_repechage_16(m, table, 0);
	    set_repechage_16(m, table, 1);
	    if (table != TABLE_SWE_ENKELT_AATERKVAL) {
		set_repechage_16(m, table, 2);
		set_repechage_16(m, table, 3);
	    }
	} else if (sys == FRENCH_32) {
	    set_repechage_32(m, table, 0);
	    set_repechage_32(m, table, 1);
	    if (table != TABLE_SWE_ENKELT_AATERKVAL) {
		set_repechage_32(m, table, 2);
		set_repechage_32(m, table, 3);
	    }
	} else if (sys == FRENCH_64) {
	    set_repechage_64(m, table, 0);
	    set_repechage_64(m, table, 1);
	    if (table != TABLE_SWE_ENKELT_AATERKVAL) {
		set_repechage_64(m, table, 2);
		set_repechage_64(m, table, 3);
	    }
	}
    }

    for (i = 1; i <= french_num_matches[table][sys]; i++) {
        if (1 || m[i].blue == 0 || m[i].white == 0) {
            gint prev_match_blue = french_matches[table][sys][i][0];
            gint prev_match_white = french_matches[table][sys][i][1];

            m[i].category = category;
            m[i].number = i;
            m[i].visible = TRUE;

            if (prev_match_blue != 0) {
                struct judoka *g = get_data(category);

                if (MATCHED_FRENCH(prev_match_blue)) {
                    if (m[i].blue && 
                        m[i].blue != WINNER_OR_LOSER(prev_match_blue) &&
                        (m[i].blue_points || m[i].white_points)) {
                        SHOW_MESSAGE("%s %s:%d (%d-%d) %s!", _("Match"),
                                     g && g->last ? g->last : "?",
                                     i, m[i].blue_points, m[i].white_points,
                                     _("canceled"));
                        m[i].blue_points = m[i].white_points = 0;
                    }
                    m[i].blue = WINNER_OR_LOSER(prev_match_blue);

                    if (db_has_hansokumake(m[i].blue) && prev_match_blue < 0 &&
			!MATCHED(i))
                        m[i].blue = GHOST;

                } else if (m[i].blue) {
                    if (MATCHED(i))
                        SHOW_MESSAGE("%s %s:%d (%d-%d) %s!", _("Match"),
                                     g && g->last ? g->last : "?",
                                     i, m[i].blue_points, m[i].white_points,
                                     _("canceled"));
                    else
                        SHOW_MESSAGE("%s %s:%d %s!", _("Match"),
                                     g && g->last ? g->last : "?", i, _("changed"));

                    m[i].blue = 0;
                }

                free_judoka(g);
            } else {
		if (db_has_hansokumake(m[i].blue) &&
		    m[i].white >= COMPETITOR &&
		    m[i].blue_points == 0 && m[i].white_points == 0)
		    m[i].white_points = 10;
	    }
                                
            if (prev_match_white != 0) {
                struct judoka *g = get_data(category);

                if (MATCHED_FRENCH(prev_match_white)) {
                    if (m[i].white && 
                        m[i].white != WINNER_OR_LOSER(prev_match_white) &&
                        (m[i].blue_points || m[i].white_points)) {
                        SHOW_MESSAGE("%s %s:%d (%d-%d) %s!", _("Match"),
                                     g && g->last ? g->last : "?",
                                     i, m[i].blue_points, m[i].white_points,
                                     _("canceled"));
                        m[i].blue_points = m[i].white_points = 0;
                    }
                    m[i].white = WINNER_OR_LOSER(prev_match_white);

                    if (db_has_hansokumake(m[i].white) && prev_match_white < 0 &&
			!MATCHED(i))
                        m[i].white = GHOST;

                } else if (m[i].white) {
                    if (MATCHED(i))
                        SHOW_MESSAGE("%s %s:%d (%d-%d) %s!", _("Match"),
                                     g && g->last ? g->last : "?",
                                     i, m[i].blue_points, m[i].white_points,
                                     _("canceled"));
                    else
                        SHOW_MESSAGE("%s %s:%d %s!", _("Match"), 
                                     g && g->last ? g->last : "?", i,
                                     _("changed"));
                    m[i].white = 0;
                }

                free_judoka(g);
            } else {
		if (db_has_hansokumake(m[i].white) &&
		    m[i].blue >= COMPETITOR &&
		    m[i].blue_points == 0 && m[i].white_points == 0)
		    m[i].blue_points = 10;
	    }

            set_match(&m[i]);
            db_set_match(&m[i]);
        }
    }

}

gint find_match_time(const gchar *cat)
{
    if (strchr(cat, 'A') || strchr(cat, 'a'))
        return 4;
    else if (strchr(cat, 'B') || strchr(cat, 'b') ||
             strchr(cat, 'C') || strchr(cat, 'c'))
        return 3;
    else if (strchr(cat, 'D') || strchr(cat, 'd'))
        return 2;
    else if (strchr(cat, 'E') || strchr(cat, 'e'))
        return 1;
    else
        return 5;
}

gint get_match_number_flag(gint category, gint number)
{
    struct category_data *cat = avl_get_category(category);
    if (!cat)
        return 0;

    gint sys = ((cat->system & SYSTEM_MASK) - SYSTEM_FRENCH_8) >> SYSTEM_MASK_SHIFT;
    gint table = (cat->system & SYSTEM_TABLE_MASK) >> SYSTEM_TABLE_SHIFT;

    gint matchnum = 1000;

    switch (cat->system & SYSTEM_MASK) {
    case SYSTEM_POOL:
	return 0;

    case SYSTEM_DPOOL:
        matchnum = num_matches(cat->system & COMPETITORS_MASK) + 3;

	if (number == matchnum - 2)
	    return MATCH_FLAG_SEMIFINAL_A;
	else if (number == matchnum - 1)
	    return MATCH_FLAG_SEMIFINAL_B;
	else if (number == matchnum)
	    return MATCH_FLAG_GOLD;
	return 0;

    case SYSTEM_FRENCH_8:
    case SYSTEM_FRENCH_16:
    case SYSTEM_FRENCH_32:
    case SYSTEM_FRENCH_64:
	if (number == medal_matches[table][sys][0])
	    return MATCH_FLAG_BRONZE_A;
	else if (number == medal_matches[table][sys][1])
	    return MATCH_FLAG_BRONZE_B;
	else if (number == medal_matches[table][sys][2])
	    return MATCH_FLAG_GOLD;
        else if (number == french_matches[table][sys][medal_matches[table][sys][2]][0])
	    return MATCH_FLAG_SEMIFINAL_A;
        else if (number == french_matches[table][sys][medal_matches[table][sys][2]][1])
	    return MATCH_FLAG_SEMIFINAL_B;
    }

    return 0;
}

gchar *get_match_number_text(gint category, gint number)
{
    gint flags = get_match_number_flag(category, number);

    if (flags & MATCH_FLAG_GOLD)
        return _("Gold medal match");
    else if (flags & MATCH_FLAG_BRONZE_A)
        return _("Bronze match A");
    else if (flags & MATCH_FLAG_BRONZE_B)
        return _("Bronze match B");
    else if (flags & MATCH_FLAG_SEMIFINAL_A)
        return _("Semifinal A");
    else if (flags & MATCH_FLAG_SEMIFINAL_B)
        return _("Semifinal B");

    return NULL;
}

void send_matches(gint tatami)
{
    struct message msg;
    struct match *m = get_cached_next_matches(tatami);
    gint t = tatami - 1, k;

    if (t < 0 || t >= NUM_TATAMIS)
        return;

    memset(&msg, 0, sizeof(msg));
    msg.type = MSG_MATCH_INFO;
    msg.u.match_info.tatami   = tatami;
    msg.u.match_info.position = 0;
    msg.u.match_info.category = next_matches_info[t][0].won_catnum;
    msg.u.match_info.number   = next_matches_info[t][0].won_matchnum;
    msg.u.match_info.blue     = next_matches_info[t][0].won_ix;
    msg.u.match_info.white    = 0;
    msg.u.match_info.flags    = get_match_number_flag(next_matches_info[t][0].won_catnum,
                                                      next_matches_info[t][0].won_matchnum);
    send_packet(&msg);

    for (k = 0; k < NEXT_MATCH_NUM; k++) {
        memset(&msg, 0, sizeof(msg));
        msg.type = MSG_MATCH_INFO;
        msg.u.match_info.tatami   = tatami;
        msg.u.match_info.position = k + 1;
        msg.u.match_info.category = m[k].category;
        msg.u.match_info.number   = m[k].number;
        msg.u.match_info.blue     = m[k].blue;
        msg.u.match_info.white    = m[k].white;

        struct category_data *cat = avl_get_category(m[k].category);
        if (cat) {
            msg.u.match_info.flags = get_match_number_flag(m[k].category, m[k].number);

            time_t last_time1 = avl_get_competitor_last_match_time(m[k].blue);
            time_t last_time2 = avl_get_competitor_last_match_time(m[k].white);
            time_t last_time = last_time1 > last_time2 ? last_time1 : last_time2;
            gint rest_time = cat->rest_time;
            time_t now = time(NULL);
			
            if (now < last_time + rest_time) {
                msg.u.match_info.rest_time = last_time + rest_time - now;
                if (last_time1 > last_time2)
                    msg.u.match_info.flags |= MATCH_FLAG_BLUE_REST | MATCH_FLAG_BLUE_DELAYED;
                else
                    msg.u.match_info.flags |= MATCH_FLAG_WHITE_REST | MATCH_FLAG_WHITE_DELAYED;
            } else
                msg.u.match_info.rest_time = 0;
        }

        send_packet(&msg);
    }
}

void update_matches_small(guint category, gint sys_or_tatami)
{
    if (sys_or_tatami == 0)
        sys_or_tatami = get_cat_system(category);

    switch (sys_or_tatami & SYSTEM_MASK) {
    case SYSTEM_POOL:
    case SYSTEM_DPOOL:
        update_pool_matches(category, sys_or_tatami & COMPETITORS_MASK);
        break;
    case SYSTEM_FRENCH_8:
    case SYSTEM_FRENCH_16:
    case SYSTEM_FRENCH_32:
    case SYSTEM_FRENCH_64:
        update_french_matches(category, sys_or_tatami);
        break;
    }
}

void update_matches(guint category, gint sys, gint tatami)
{
    struct match *nm;
    struct judoka *j1 = NULL, *j2 = NULL, *g = NULL;
    struct message msg;

    if (category) {
        if (sys == 0)
            sys = db_get_system(category);

        switch (sys & SYSTEM_MASK) {
        case SYSTEM_POOL:
        case SYSTEM_DPOOL:
            update_pool_matches(category, sys & COMPETITORS_MASK);
            break;
        case SYSTEM_FRENCH_8:
        case SYSTEM_FRENCH_16:
        case SYSTEM_FRENCH_32:
        case SYSTEM_FRENCH_64:
            update_french_matches(category, sys);
            break;
        }
    }

    g_static_mutex_lock(&next_match_mutex);
    nm = db_next_match(tatami ? 0 : category, tatami);

    if (nm == NULL) {
        g_static_mutex_unlock(&next_match_mutex);
        return;
    }

    /* create a message for the judotimers */
    memset(&msg, 0, sizeof(msg));
    msg.type = MSG_NEXT_MATCH;
    msg.u.next_match.category = nm[0].category;
    msg.u.next_match.match = nm[0].number;

    if (nm[0].number < 1000) {
        if (!tatami)
            tatami = db_find_match_tatami(nm[0].category, nm[0].number);
        g = get_data(nm[0].category);
        j1 = get_data(nm[0].blue);
        j2 = get_data(nm[0].white);

        if (g && j1 && j2) {
            gchar buf[100];

            if (fill_in_next_match_message_data(g->last, &msg.u.next_match)) {
                msg.u.next_match.minutes = 1<<20; //find_match_time(g->last);
                time_t last_time1 = avl_get_competitor_last_match_time(nm[0].blue);
                time_t last_time2 = avl_get_competitor_last_match_time(nm[0].white);
                time_t last_time = last_time1 > last_time2 ? last_time1 : last_time2;
                gint rest_time = msg.u.next_match.rest_time;
                time_t now = time(NULL);

                if (now < last_time + rest_time) {
                    msg.u.next_match.rest_time = last_time + rest_time;
                    if (last_time1 > last_time2)
                        msg.u.next_match.minutes |= MATCH_FLAG_BLUE_REST;
                    else
                        msg.u.next_match.minutes |= MATCH_FLAG_WHITE_REST;
                } else
                    msg.u.next_match.rest_time = 0;
            }
            set_graph_rest_time(tatami, msg.u.next_match.rest_time, msg.u.next_match.minutes);

            msg.u.next_match.tatami = tatami;

            snprintf(msg.u.next_match.cat_1, 
                     sizeof(msg.u.next_match.cat_1),
                     "%s", g->last);
            snprintf(msg.u.next_match.blue_1, 
                     sizeof(msg.u.next_match.blue_1),
                     "%s %s, %s", j1->first, j1->last, get_club_text(j1, CLUB_TEXT_ABBREVIATION));
            snprintf(msg.u.next_match.white_1, sizeof(msg.u.next_match.white_1),
                     "%s %s, %s", j2->first, j2->last, get_club_text(j2, CLUB_TEXT_ABBREVIATION));

            snprintf(buf, sizeof(buf), "%s: %s: %s %s, %s - %s %s, %s", _("Match"), 
                     g->last, 
                     j1->first, j1->last, get_club_text(j1, 0),
                     j2->first, j2->last, get_club_text(j2, 0));

            if (tatami > 0 && tatami <= NUM_TATAMIS) {
                gtk_label_set_text(GTK_LABEL(next_match[tatami-1][0]), buf); 
                COPY_MATCH_INFO(category, tatami, 0, g->last); 
                COPY_MATCH_INFO(blue_first, tatami, 0, j1->first); 
                COPY_MATCH_INFO(blue_last, tatami, 0, j1->last); 
                COPY_MATCH_INFO(blue_club, tatami, 0, get_club_text(j1, 0)); 
                COPY_MATCH_INFO(white_first, tatami, 0, j2->first); 
                COPY_MATCH_INFO(white_last, tatami, 0, j2->last); 
                COPY_MATCH_INFO(white_club, tatami, 0, get_club_text(j2, 0)); 
                next_matches_info[tatami-1][0].catnum = nm[0].category;
                next_matches_info[tatami-1][0].matchnum = nm[0].number;
                next_matches_info[tatami-1][0].blue = nm[0].blue;
                next_matches_info[tatami-1][0].white = nm[0].white;
            }
        }
        free_judoka(g);
        free_judoka(j1);
        free_judoka(j2);
    } else {
        if (tatami == 0 && category) {
            g = get_data(category);
            if (g) {
                tatami = g->belt;
                free_judoka(g);
            }
        }

        if (tatami > 0 && tatami <= NUM_TATAMIS) {
            msg.u.next_match.tatami = tatami;
            gtk_label_set_text(GTK_LABEL(next_match[tatami-1][0]), _("Match:"));
            NULL_MATCH_INFO(category, tatami, 0); 
            NULL_MATCH_INFO(blue_first, tatami, 0); 
            NULL_MATCH_INFO(blue_last, tatami, 0); 
            NULL_MATCH_INFO(blue_club, tatami, 0); 
            NULL_MATCH_INFO(white_first, tatami, 0); 
            NULL_MATCH_INFO(white_last, tatami, 0); 
            NULL_MATCH_INFO(white_club, tatami, 0); 
            next_matches_info[tatami-1][0].catnum = 0;
        } 
    }

    if (nm[1].number < 1000) {
        if (!tatami)
            tatami = db_find_match_tatami(nm[1].category, nm[1].number);
        g = get_data(nm[1].category);
        j1 = get_data(nm[1].blue);
        j2 = get_data(nm[1].white);

        if (g) {
            gchar buf[100];

            snprintf(msg.u.next_match.cat_2, sizeof(msg.u.next_match.cat_2),
                     "%s", g->last);
            if (j1)
                snprintf(msg.u.next_match.blue_2, sizeof(msg.u.next_match.blue_2),
                         "%s %s, %s", j1->first, j1->last, 
			 get_club_text(j1, CLUB_TEXT_ABBREVIATION));
            else
                snprintf(msg.u.next_match.blue_2, sizeof(msg.u.next_match.blue_2), "?");

            if (j2)
                snprintf(msg.u.next_match.white_2, sizeof(msg.u.next_match.white_2),
                         "%s %s, %s", j2->first, j2->last, 
			 get_club_text(j2, CLUB_TEXT_ABBREVIATION));
            else
                snprintf(msg.u.next_match.white_2, sizeof(msg.u.next_match.white_2), "?");

            if (j1 && j2)
                snprintf(buf, sizeof(buf), "%s: %s: %s %s, %s - %s %s, %s", _("Preparing"), 
                         g->last, 
                         j1->first, j1->last, get_club_text(j1, 0), 
                         j2->first, j2->last, get_club_text(j2, 0));
            else
                snprintf(buf, sizeof(buf), "%s: %s: ? - ?", _("Preparing"), g->last); 

            if (tatami > 0 && tatami <= NUM_TATAMIS) {
                gtk_label_set_text(GTK_LABEL(next_match[tatami-1][1]), buf);
                COPY_MATCH_INFO(category, tatami, 1, g->last); 
                COPY_MATCH_INFO(blue_first, tatami, 1, j1 ? j1->first : "?"); 
                COPY_MATCH_INFO(blue_last, tatami, 1, j1 ? j1->last : "?"); 
                COPY_MATCH_INFO(blue_club, tatami, 1, j1 ? get_club_text(j1, 0) : "?"); 
                COPY_MATCH_INFO(white_first, tatami, 1, j2 ? j2->first : "?"); 
                COPY_MATCH_INFO(white_last, tatami, 1, j2 ? j2->last : "?"); 
                COPY_MATCH_INFO(white_club, tatami, 1, j2 ? get_club_text(j2, 0) : "?"); 
                next_matches_info[tatami-1][1].catnum = nm[1].category;
                next_matches_info[tatami-1][1].matchnum = nm[1].number;
                next_matches_info[tatami-1][1].blue = nm[1].blue;
                next_matches_info[tatami-1][1].white = nm[1].white;
            }
        }
        free_judoka(g);
        free_judoka(j1);
        free_judoka(j2);
    } else {
        if (tatami == 0 && category) {
            g = get_data(category);
            if (g) {
                tatami = g->belt;
                free_judoka(g);
            }
        }

        if (tatami > 0 && tatami <= NUM_TATAMIS) {
            gtk_label_set_text(GTK_LABEL(next_match[tatami-1][1]), 
                               _("Preparing:"));
            NULL_MATCH_INFO(category, tatami, 1); 
            NULL_MATCH_INFO(blue_first, tatami, 1); 
            NULL_MATCH_INFO(blue_last, tatami, 1); 
            NULL_MATCH_INFO(blue_club, tatami, 1); 
            NULL_MATCH_INFO(white_first, tatami, 1); 
            NULL_MATCH_INFO(white_last, tatami, 1); 
            NULL_MATCH_INFO(white_club, tatami, 1); 
            next_matches_info[tatami-1][1].catnum = 0;
        }
    }
	
    g_static_mutex_unlock(&next_match_mutex);

    send_packet(&msg);

    if (category)
        update_category_status_info(category);

    refresh_sheet_display(FALSE);
    refresh_window();

    send_matches(tatami);
}

static void log_match(gint category, gint number, gint bluepts, gint whitepts)
{
    gint blue_ix, white_ix, win_ix;
    struct judoka *blue = NULL, *white = NULL, *cat = get_data(category), *winner;
    gint tatami = db_find_match_tatami(category, number);

    if (!cat)
        return;

    gchar *cmd = g_strdup_printf("select * from matches where \"category\"=%d and \"number\"=%d",
                                 category, number);
    gint numrows = db_get_table(cmd);
    g_free(cmd);
        
    if (numrows < 0)
        goto error;

    blue_ix = atoi(db_get_data(0, "blue"));
    white_ix = atoi(db_get_data(0, "white"));
    blue = get_data(blue_ix);
    white = get_data(white_ix);

    if (blue == NULL || white == NULL)
        goto error;

    if (bluepts > whitepts) {
        winner = blue;
        win_ix = blue_ix;
    } else {
        winner = white;
        win_ix = white_ix;
    }

    COMP_LOG_INFO(tatami, "%s: %s (%d), %s-%s, %s: %d-%d", _("Match"),
                  cat->last, number, blue->last, white->last,
                  _("points"), bluepts, whitepts);

    if (tatami >= 1 && tatami <= NUM_TATAMIS) {
        g_strlcpy(next_matches_info[tatami-1][0].won_last, 
                  winner->last, sizeof(next_matches_info[0][0].won_last));
        g_strlcpy(next_matches_info[tatami-1][0].won_first, 
                  winner->first, sizeof(next_matches_info[0][0].won_first));
        g_strlcpy(next_matches_info[tatami-1][0].won_cat, 
                  cat->last, sizeof(next_matches_info[0][0].won_cat));
        next_matches_info[tatami-1][0].won_catnum = category;
        next_matches_info[tatami-1][0].won_matchnum = number;
        next_matches_info[tatami-1][0].won_ix = win_ix;
    }

error:
    free_judoka(blue);
    free_judoka(white);
    free_judoka(cat);
    db_close_table();
}

void set_points(GtkWidget *menuitem, gpointer userdata)
{
    guint data = (guint)userdata;
    guint category = data >> 18;
    guint number = (data >> 5) & 0x1fff;
    gboolean is_blue = (data & 16) != 0;
    guint points = data & 0xf;
    gint sys = db_get_system(category);

    db_set_points(category, number, 0, 
                  is_blue ? points : 0,
                  is_blue ? 0 : points, 0, 0);

    db_read_match(category, number);
        
    log_match(category, number, is_blue ? points : 0, is_blue ? 0 : points);

    update_matches(category, sys, db_find_match_tatami(category, number));

    /* send points to net */
    struct message msg;
    memset(&msg, 0, sizeof(msg));
    msg.type = MSG_SET_POINTS;
    msg.u.set_points.category     = category;
    msg.u.set_points.number       = number;
    msg.u.set_points.sys          = sys;
    msg.u.set_points.blue_points  = is_blue ? points : 0;
    msg.u.set_points.white_points = is_blue ? 0 : points;
    send_packet(&msg);

    make_backup();
}

void set_points_from_net(struct message *msg)
{
    gint sys = db_get_system(msg->u.set_points.category);

    db_set_points(msg->u.set_points.category, 
                  msg->u.set_points.number, 
                  0, 
                  msg->u.set_points.blue_points,
                  msg->u.set_points.white_points,
                  0, 0);

    db_read_match(msg->u.set_points.category, msg->u.set_points.number);
        
    log_match(msg->u.set_points.category, msg->u.set_points.number, 
              msg->u.set_points.blue_points, msg->u.set_points.white_points);

    update_matches(msg->u.set_points.category, sys, 
                   db_find_match_tatami(msg->u.set_points.category, msg->u.set_points.number));

    make_backup();
}

void set_points_and_score(struct message *msg)
{
    guint category = msg->u.result.category;
    guint number = msg->u.result.match;
    gint  minutes = msg->u.result.minutes;
    gint  sys = db_get_system(category);
    gint  winscore = 0, losescore = 0;
    gint  points = 2, blue_pts = 0, white_pts = 0;
    //gint  numcompetitors = sys & COMPETITORS_MASK;

    if (number >= 1000 || category < 10000)
        return;

    if (msg->u.result.blue_hansokumake) {
        msg->u.result.blue_score = 0;
        msg->u.result.white_score = 0x10000;
    } else if (msg->u.result.white_hansokumake) {
        msg->u.result.blue_score = 0x10000;
        msg->u.result.white_score = 0;
    }

    if ((msg->u.result.blue_score & 0xffff0) > (msg->u.result.white_score & 0xffff0)) {
        winscore = msg->u.result.blue_score & 0xffff0;
        losescore = msg->u.result.white_score & 0xffff0;
    } else if ((msg->u.result.blue_score & 0xffff0) < (msg->u.result.white_score & 0xffff0)) {
        winscore = msg->u.result.white_score & 0xffff0;
        losescore = msg->u.result.blue_score & 0xffff0;
    } else if (msg->u.result.blue_vote == msg->u.result.white_vote)
        return;

    if (winscore != losescore) {
        if ((winscore & 0xf0000) && (losescore & 0xf0000) == 0) points = 10;
        else if ((winscore & 0xf000) && (losescore & 0xf000) == 0) points = 7;
        else if ((winscore & 0xf00) > (losescore & 0xf00)) points = 5;
        else if ((winscore & 0xf0) > (losescore & 0xf0)) points = 3;
                
        if ((msg->u.result.blue_score & 0xffff0) > (msg->u.result.white_score & 0xffff0))
            blue_pts = points;
        else
            white_pts = points;
                        
    } else {
        if (msg->u.result.blue_vote > msg->u.result.white_vote)
            blue_pts = 1;
        else
            white_pts = 1;
    }

    if (msg->u.result.blue_hansokumake || msg->u.result.white_hansokumake) {
        db_set_match_hansokumake(category, number, 
                                 msg->u.result.blue_hansokumake, 
                                 msg->u.result.white_hansokumake);
    }

    db_set_points(category, number, minutes,
                  blue_pts, white_pts, msg->u.result.blue_score, msg->u.result.white_score); 

    db_read_match(category, number);
        
    log_match(category, number, blue_pts, white_pts);

    update_matches(category, sys, db_find_match_tatami(category, number));

    make_backup();
}

void update_competitors_categories(gint competitor)
{
    GtkTreeIter iter, parent;
    gint sys;
    gint category;

    if (find_iter(&iter, competitor) == FALSE)
        return;

    if (gtk_tree_model_iter_parent(current_model, &parent, &iter) == FALSE)
        return;

    gtk_tree_model_get(current_model, &parent,
                       COL_INDEX, &category,
                       -1);

    sys = db_get_system(category);

    update_matches(category, sys, 0);
}

static void expand_all(GtkWidget *widget, gpointer userdata)
{
    GtkWidget *treeview = userdata;
    gtk_tree_view_expand_all(GTK_TREE_VIEW(treeview));
}

static void collapse_all(GtkWidget *widget, gpointer userdata)
{
    GtkWidget *treeview = userdata;
    gtk_tree_view_collapse_all(GTK_TREE_VIEW(treeview));
}

static void view_match_popup_menu(GtkWidget *treeview, 
                                  GdkEventButton *event, 
                                  guint category,
                                  guint number,
                                  gboolean visible)
{
    GtkWidget *menu, *menuitem;

    menu = gtk_menu_new();

    menuitem = gtk_menu_item_new_with_label(_("Expand All"));
    g_signal_connect(menuitem, "activate",
                     (GCallback) expand_all, treeview);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	
    menuitem = gtk_menu_item_new_with_label(_("Collapse All"));
    g_signal_connect(menuitem, "activate",
                     (GCallback) collapse_all, treeview);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

    gtk_widget_show_all(menu);

    /* Note: event can be NULL here when called from view_onPopupMenu;
     *  gdk_event_get_time() accepts a NULL argument */
    gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
                   (event != NULL) ? event->button : 0,
                   gdk_event_get_time((GdkEvent*)event));
}

//void matches_refresh_1(gint tatami);

static void set_comment(GtkWidget *menuitem, gpointer userdata)
{
    guint data = (guint)userdata;
    guint category = data>>16;
    guint number = (data&0xfffc) >> 2;
    guint cmd = data & 0x3;
    gint sys = db_get_system(category);
    //gint tatami = db_get_tatami(category);

    db_set_comment(category, number, cmd);        
    //db_read_matches(); too heavy and unnecessary
    update_matches(category, sys, db_find_match_tatami(category, number));
    //matches_refresh_1(tatami);

    /* send comment to net */
    struct message msg;
    memset(&msg, 0, sizeof(msg));
    msg.type = MSG_SET_COMMENT;
    msg.u.set_comment.data = data;
    msg.u.set_comment.category = category;
    msg.u.set_comment.number = number;
    msg.u.set_comment.cmd = cmd;
    msg.u.set_comment.sys = sys;
    send_packet(&msg);
}

void set_comment_from_net(struct message *msg)
{
    gint sys = db_get_system(msg->u.set_comment.category);
    //gint tatami = db_get_tatami(category);

    db_set_comment(msg->u.set_comment.category, msg->u.set_comment.number, msg->u.set_comment.cmd);        
    //db_read_matches(); too heavy operation
    update_matches(msg->u.set_comment.category, sys, 
                   db_find_match_tatami(msg->u.set_comment.category, msg->u.set_comment.number));
    //matches_refresh_1(tatami);
}

static void view_next_match_popup_menu(GtkWidget *treeview, 
                                       GdkEventButton *event, 
                                       guint category,
                                       guint number,
                                       gboolean visible)
{
    GtkWidget *menu, *menuitem;

    menu = gtk_menu_new();

    menuitem = gtk_menu_item_new_with_label(_("Next match"));
    g_signal_connect(menuitem, "activate",
                     (GCallback) set_comment, (gpointer)((category<<16)|(number<<2)|COMMENT_MATCH_1));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

    menuitem = gtk_menu_item_new_with_label(_("Preparing"));
    g_signal_connect(menuitem, "activate",
                     (GCallback) set_comment, (gpointer)((category<<16)|(number<<2)|COMMENT_MATCH_2));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

    menuitem = gtk_menu_item_new_with_label(_("Delay the match"));
    g_signal_connect(menuitem, "activate",
                     (GCallback) set_comment, (gpointer)((category<<16)|(number<<2)|COMMENT_WAIT));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

    menuitem = gtk_menu_item_new_with_label(_("Remove comment"));
    g_signal_connect(menuitem, "activate",
                     (GCallback) set_comment, (gpointer)((category<<16)|(number<<2)));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

    gtk_widget_show_all(menu);

    /* Note: event can be NULL here when called from view_onPopupMenu;
     *  gdk_event_get_time() accepts a NULL argument */
    gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
                   (event != NULL) ? event->button : 0,
                   gdk_event_get_time((GdkEvent*)event));
}

static void view_match_points_popup_menu(GtkWidget *treeview, 
                                         GdkEventButton *event, 
                                         guint category,
                                         guint number,
                                         gboolean is_blue)
{
    GtkWidget *menu, *menuitem;
    guint data;

    data = (category << 18) | (number << 5) | (is_blue ? 16: 0);

    menu = gtk_menu_new();

    menuitem = gtk_menu_item_new_with_label("10");
    g_signal_connect(menuitem, "activate",
                     (GCallback) set_points, (gpointer)(data + 10));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
    menuitem = gtk_menu_item_new_with_label("7");
    g_signal_connect(menuitem, "activate",
                     (GCallback) set_points, (gpointer)(data + 7));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
    menuitem = gtk_menu_item_new_with_label("5");
    g_signal_connect(menuitem, "activate",
                     (GCallback) set_points, (gpointer)(data + 5));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
    menuitem = gtk_menu_item_new_with_label("3");
    g_signal_connect(menuitem, "activate",
                     (GCallback) set_points, (gpointer)(data + 3));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
    menuitem = gtk_menu_item_new_with_label("1");
    g_signal_connect(menuitem, "activate",
                     (GCallback) set_points, (gpointer)(data + 1));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
    menuitem = gtk_menu_item_new_with_label("0");
    g_signal_connect(menuitem, "activate",
                     (GCallback) set_points, (gpointer)(data));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
        
    gtk_widget_show_all(menu);

    gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
                   (event != NULL) ? event->button : 0,
                   gdk_event_get_time((GdkEvent*)event));
}


static gboolean view_onButtonPressed(GtkWidget *treeview, 
                                     GdkEventButton *event, 
                                     gpointer userdata)
{
    /* single click with the right mouse button? */
    if (event->type == GDK_BUTTON_PRESS) {
        GtkTreePath *path;
        GtkTreeModel *model;
        GtkTreeIter iter;
        GtkTreeViewColumn *col, *blue_pts_col, *white_pts_col;
        guint category, number, blue, white, w, b;
        gboolean visible, handled = FALSE;

        if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeview),
                                          (gint) event->x, 
                                          (gint) event->y,
                                          &path, &col, NULL, NULL)) {
            model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
            gtk_tree_model_get_iter(model, &iter, path);

            gtk_tree_model_get(model, &iter,
                               COL_MATCH_CAT, &category,
                               COL_MATCH_NUMBER, &number,
                               COL_MATCH_VIS, &visible,
                               COL_MATCH_BLUE_POINTS, &blue,
                               COL_MATCH_WHITE_POINTS, &white,
                               COL_MATCH_BLUE, &b,
                               COL_MATCH_WHITE, &w,
                               -1);

            if (category == 0) {
                if (event->button == 3) {
                    view_match_popup_menu(treeview, event, 
                                          category,
                                          number,
                                          visible);
                    return TRUE;
                }
                return FALSE;
            }

            current_category = category;

            blue_pts_col = gtk_tree_view_get_column (GTK_TREE_VIEW(treeview), COL_MATCH_BLUE_POINTS);
            white_pts_col = gtk_tree_view_get_column (GTK_TREE_VIEW(treeview), COL_MATCH_WHITE_POINTS);

            if (!visible && event->button == 3) {
                view_match_popup_menu(treeview, event, 
                                      category,
                                      number,
                                      visible);
                handled = TRUE;
            } else if (visible && event->button == 3 && col == blue_pts_col 
                       /*&&
                         b != GHOST && w != GHOST*/) {
                view_match_points_popup_menu(treeview, event, 
                                             category,
                                             number,
                                             TRUE);
                handled = TRUE;
            } else if (visible && event->button == 3 && col == white_pts_col 
                       /*&&
                         b != GHOST && w != GHOST*/) {
                view_match_points_popup_menu(treeview, event, 
                                             category,
                                             number,
                                             FALSE);
                handled = TRUE;
            } else if (visible && event->button == 3 && b != GHOST && w != GHOST) {
                view_next_match_popup_menu(treeview, event, 
                                           category,
                                           number,
                                           visible);
                handled = TRUE;
            }

            gtk_tree_path_free(path);
        }
        return handled; /* we handled this */
    }
    return FALSE; /* we did not handle this */
}

static void view_onRowActivated (GtkTreeView        *treeview,
				 GtkTreePath        *path,
				 GtkTreeViewColumn  *col,
				 gpointer            userdata)
{
    GtkTreeModel *model;
    GtkTreeIter   iter;
    gint cat;

    model = gtk_tree_view_get_model(treeview);
    if (gtk_tree_model_get_iter(model, &iter, path)) {
        gtk_tree_model_get(model, &iter,
                           COL_MATCH_CAT, &cat,
                           -1);

        if (cat < 10000)
            return;
		
        category_window(cat);
    }
}

static GtkTreeModel *create_and_fill_model (void)
{
    GtkTreeStore  *treestore;

    treestore = gtk_tree_store_new(NUM_MATCH_COLUMNS,
                                   G_TYPE_UINT, /* group */
                                   G_TYPE_UINT, /* category */
                                   G_TYPE_UINT, /* number */
                                   G_TYPE_UINT, /* blue */
                                   G_TYPE_UINT, /* blue score */
                                   G_TYPE_UINT, /* blue points */
                                   G_TYPE_UINT, /* white points */
                                   G_TYPE_UINT, /* white score */
                                   G_TYPE_UINT, /* white */
                                   G_TYPE_UINT, /* time */
                                   G_TYPE_UINT, /* comment */
                                   G_TYPE_BOOLEAN,/* visible */
                                   G_TYPE_UINT  /* status */
        );

    return GTK_TREE_MODEL(treestore);
}

static GtkWidget *create_view_and_model(void)
{
    GtkTreeViewColumn   *col;
    GtkCellRenderer     *renderer;
    GtkWidget           *view;
    GtkTreeModel        *model;
    GtkTreeSelection    *selection;
    gint col_offset;

    view = gtk_tree_view_new();

    /* --- Column group --- */

    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "xalign", 0.0, NULL);
    col_offset = gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view),
                                                             -1, _("Group"),
                                                             renderer, "text",
                                                             COL_MATCH_GROUP,
                                                             NULL);
    col = gtk_tree_view_get_column (GTK_TREE_VIEW (view), col_offset - 1);
    gtk_tree_view_column_set_cell_data_func(col, renderer, 
                                            group_cell_data_func, 
                                            (gpointer)COL_MATCH_GROUP, NULL);

    /* --- Column category --- */

    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer,
                 "weight", PANGO_WEIGHT_BOLD,
                 "weight-set", TRUE,
                 "xalign", 0.0,
                 NULL);
    col_offset = gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view),
                                                             -1, _("Category"),
                                                             renderer, "text",
                                                             COL_MATCH_CAT,
                                                             NULL);
    col = gtk_tree_view_get_column (GTK_TREE_VIEW (view), col_offset - 1);
    gtk_tree_view_column_set_cell_data_func(col, renderer, 
                                            category_cell_data_func, 
                                            (gpointer)COL_MATCH_CAT, NULL);
    gtk_tree_view_column_set_sort_column_id(GTK_TREE_VIEW_COLUMN(col), COL_MATCH_CAT);
 
    /* --- Column number --- */

    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "xalign", 0.0, NULL);
    col_offset = gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view),
                                                             -1, _("Match"),
                                                             renderer, "text",
                                                             COL_MATCH_NUMBER,
                                                             "visible",
                                                             COL_MATCH_VIS,
                                                             NULL);
    col = gtk_tree_view_get_column (GTK_TREE_VIEW (view), col_offset - 1);

    /* --- Column blue --- */

    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "xalign", 0.0, NULL);
    col_offset = gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view),
                                                             -1, _("Blue"),
                                                             renderer, "text",
                                                             COL_MATCH_BLUE,
                                                             /*"visible",
                                                               COL_MATCH_VIS,*/
                                                             NULL);
    col = gtk_tree_view_get_column (GTK_TREE_VIEW (view), col_offset - 1);
    gtk_tree_view_column_set_cell_data_func(col, renderer, 
                                            name_cell_data_func, 
                                            (gpointer)COL_MATCH_BLUE, NULL);

    /* --- Column blue score --- */

    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "xalign", 0.5, NULL);
    col_offset = gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view),
                                                             -1, "IWYKS",
                                                             renderer, "text",
                                                             COL_MATCH_BLUE_SCORE,
                                                             "visible",
                                                             COL_MATCH_VIS,
                                                             NULL);
    col = gtk_tree_view_get_column (GTK_TREE_VIEW (view), col_offset - 1);
    gtk_tree_view_column_set_cell_data_func(col, renderer, 
                                            score_cell_data_func, 
                                            (gpointer)COL_MATCH_BLUE_SCORE, NULL);

    /* --- Column blue points --- */

    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "xalign", 0.5, NULL);
    col_offset = gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view),
                                                             -1, _("Points"),
                                                             renderer, "text",
                                                             COL_MATCH_BLUE_POINTS,
                                                             "visible",
                                                             COL_MATCH_VIS,
                                                             NULL);
    col = gtk_tree_view_get_column (GTK_TREE_VIEW (view), col_offset - 1);

    /* --- Column white points --- */

    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "xalign", 0.5, NULL);
    col_offset = gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view),
                                                             -1, _("Points"),
                                                             renderer, "text",
                                                             COL_MATCH_WHITE_POINTS,
                                                             "visible",
                                                             COL_MATCH_VIS,
                                                             NULL);
    col = gtk_tree_view_get_column (GTK_TREE_VIEW (view), col_offset - 1);

    /* --- Column white score --- */

    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "xalign", 0.5, NULL);
    col_offset = gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view),
                                                             -1, "IWYKS",
                                                             renderer, "text",
                                                             COL_MATCH_WHITE_SCORE,
                                                             "visible",
                                                             COL_MATCH_VIS,
                                                             NULL);
    col = gtk_tree_view_get_column (GTK_TREE_VIEW (view), col_offset - 1);
    gtk_tree_view_column_set_cell_data_func(col, renderer, 
                                            score_cell_data_func, 
                                            (gpointer)COL_MATCH_WHITE_SCORE, NULL);

    /* --- Column white --- */

    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "xalign", 0.0, NULL);
    col_offset = gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view),
                                                             -1, _("White"),
                                                             renderer, "text",
                                                             COL_MATCH_WHITE,
                                                             "visible",
                                                             COL_MATCH_VIS,
                                                             NULL);
    col = gtk_tree_view_get_column (GTK_TREE_VIEW (view), col_offset - 1);
    gtk_tree_view_column_set_cell_data_func(col, renderer, 
                                            name_cell_data_func, 
                                            (gpointer)COL_MATCH_WHITE, NULL);

    /* --- Column time --- */

    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "xalign", 0.0, NULL);
    col_offset = gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view),
                                                             -1, _("Time"),
                                                             renderer, "text",
                                                             COL_MATCH_TIME,
                                                             "visible",
                                                             COL_MATCH_VIS,
                                                             NULL);
    col = gtk_tree_view_get_column (GTK_TREE_VIEW (view), col_offset - 1);
    gtk_tree_view_column_set_cell_data_func(col, renderer, 
                                            time_cell_data_func, 
                                            (gpointer)COL_MATCH_TIME, NULL);

    /* --- Column comment --- */

    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "xalign", 0.0, NULL);
    col_offset = gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view),
                                                             -1, _("Comment"),
                                                             renderer, "text",
                                                             COL_MATCH_COMMENT,
                                                             "visible",
                                                             COL_MATCH_VIS,
                                                             NULL);
    col = gtk_tree_view_get_column (GTK_TREE_VIEW (view), col_offset - 1);
    gtk_tree_view_column_set_cell_data_func(col, renderer, 
                                            comment_cell_data_func, 
                                            (gpointer)COL_MATCH_COMMENT, NULL);

    /*****/

    model = create_and_fill_model();

    gtk_tree_view_set_model(GTK_TREE_VIEW(view), model);
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (view), TRUE);
				    
    g_object_unref(model); /* destroy model automatically with view */

    g_signal_connect(view, "row-activated", (GCallback) view_onRowActivated, NULL);
    g_signal_connect(view, "button-press-event", (GCallback) view_onButtonPressed, NULL);
    //g_signal_connect(view, "popup-menu", (GCallback) view_onPopupMenu, NULL);

    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
    gtk_tree_selection_set_mode(selection, GTK_SELECTION_NONE);

    gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(model),
                                    COL_MATCH_CAT,
                                    sort_iter_compare_func_1,
                                    GINT_TO_POINTER(COL_MATCH_CAT),
                                    NULL);

    return view;
}

static GtkWidget *match_pages[NUM_TATAMIS];

void update_match_pages_visibility(void)
{
    gint i;
    for (i = 0; i < NUM_TATAMIS; i++) {
        if (match_pages[i] == NULL)
            continue;

        if (i < number_of_tatamis)
            gtk_widget_show(GTK_WIDGET(match_pages[i]));
        else
            gtk_widget_hide(GTK_WIDGET(match_pages[i]));
    }
}

void set_match_pages(GtkWidget *notebook)
{
    GtkWidget *scrolled_window;
    GtkWidget *label;
    GtkWidget *view;
    GtkWidget *match1;
    GtkWidget *match2;
    GtkWidget *vbox;

    int        i;
    char       buffer[20];

    for (i = 0; i < NUM_TATAMIS; i++) {
        match_pages[i] = vbox = gtk_vbox_new(FALSE, 1);

        gtk_container_set_border_width(GTK_CONTAINER(vbox), 1);

        scrolled_window = gtk_scrolled_window_new(NULL, NULL);
        gtk_container_set_border_width(GTK_CONTAINER(scrolled_window), 10);

        match_view[i] = view = create_view_and_model();

        gtk_scrolled_window_add_with_viewport(
            GTK_SCROLLED_WINDOW(scrolled_window), view);

        sprintf(buffer, "Tatami %d", i+1);
        label = gtk_label_new (buffer);

        next_match[i][0] = match1 = gtk_label_new(_("Match:"));
        next_match[i][1] = match2 = gtk_label_new(_("Preparing:"));

        gtk_misc_set_alignment(GTK_MISC(match1), 0.0, 0.0);
        gtk_misc_set_alignment(GTK_MISC(match2), 0.0, 0.0);

        gtk_box_pack_start(GTK_BOX(vbox), match1, FALSE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), match2, FALSE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);

        gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, label);

    }

    db_read_matches();

    for (i = 0; i < NUM_TATAMIS; i++) {
        update_matches(0, 0, i+1);
        /******
               g_static_mutex_lock(&next_match_mutex);
               nm = db_next_match(0, i+1);
               gint n = nm[0].number;
               g_static_mutex_unlock(&next_match_mutex);
               if (n < 1000) {
               gint sys = db_get_system(nm[0].category);
               update_matches(nm[0].category, sys);
               }
        ****/
    }
}

static GtkTreeIter found_iter;
static gboolean    iter_found;

static gboolean traverse_rows(GtkTreeModel *model,
			      GtkTreePath *path,
			      GtkTreeIter *iter,
			      gpointer data)
{
    struct matchid *id = data;
    gint category, number, group;

    gtk_tree_model_get(model, iter,
                       COL_MATCH_GROUP,  &group,
                       COL_MATCH_CAT,    &category,
                       COL_MATCH_NUMBER, &number,
                       -1);
    if (id->category == category && 
        id->number == number &&
        (id->group == group || id->group == 0)) {
        iter_found = TRUE;
        found_iter = *iter;
        return TRUE;
    }
    return FALSE;
}

static gboolean find_match_iter(GtkTreeModel *model, 
                                GtkTreeIter *iter, 
                                gint group,
                                gint category, 
                                gint number)
{
    struct matchid id;

    id.group = group;
    id.category = category;
    id.number = number;
    iter_found = FALSE;
    gtk_tree_model_foreach(model, traverse_rows, (gpointer)&id);
    if (iter_found) {
        *iter = found_iter;
        return TRUE;
    }
    return FALSE;
}

static gboolean find_group_iter(GtkTreeModel *model,
                                GtkTreeIter *iter, 
                                gint group)
{
    gboolean ok;
    gint grp;

    ok = gtk_tree_model_get_iter_first(model, iter);
    while (ok) {
        gtk_tree_model_get(model, iter,
                           COL_MATCH_GROUP, &grp,
                           -1);
        if (grp == group)
            return TRUE;

        ok = gtk_tree_model_iter_next(model, iter);
    }

    return FALSE;
}

#define T g_print("FILE %s LINE %d\n",__FILE__,  __LINE__)

static GStaticMutex set_match_mutex = G_STATIC_MUTEX_INIT;

void set_match(struct match *m)
{
    gint i, tatami, group;
    GtkTreeIter iter, cat, grp_iter, gi, tmp_iter, ic;
    GtkTreeModel *model;
    gboolean ok, iter_set = FALSE;
    gint grp = -1, num = -1;

    g_static_mutex_lock(&set_match_mutex);

    /* Find info about category */
    if (find_iter(&cat, m->category) == FALSE) {
        g_print("CANNOT FIND CAT %d (num=%d)\n", m->category, m->number);
        g_static_mutex_unlock(&set_match_mutex);
        //assert(0);
        return; /* error */
    }
        
    /* Match found. Look for tatami number. */
    gtk_tree_model_get(current_model, &cat,
                       COL_BELT, &tatami,
                       COL_BIRTHYEAR, &group,
                       -1);

    for (i = 0; i < NUM_TATAMIS; i++) {
        model = gtk_tree_view_get_model(GTK_TREE_VIEW(match_view[i]));

        if (find_match_iter(model, 
                            &iter,
                            0,
                            m->category, 
                            m->number)) {
            guint old_group;

            gtk_tree_model_get(model, &iter,
                               COL_MATCH_GROUP, &old_group,
                               -1);

            if (tatami != (i+1) || group != old_group) {
                GtkTreeIter parent, grandparent;
                gint num_categories, num_matches;

                /* Changed. */
                gtk_tree_model_iter_parent(model, &parent, &iter);
                gtk_tree_model_iter_parent(model, &grandparent, &parent);
                num_matches = gtk_tree_model_iter_n_children(model, &parent);
                num_categories = gtk_tree_model_iter_n_children(model, &grandparent);
                if (num_categories == 1 && num_matches == 1)
                    gtk_tree_store_remove(GTK_TREE_STORE(model), &grandparent);
                else if (num_matches == 1)
                    gtk_tree_store_remove(GTK_TREE_STORE(model), &parent);
                else
                    gtk_tree_store_remove(GTK_TREE_STORE(model), &iter);
            } else {
                /* Same tatami. Update information. */
                goto update;
            }

            break;
        }
    }

    if (tatami == 0) {
        //g_print("TATAMI NOT SET\n");
        g_static_mutex_unlock(&set_match_mutex);
        return; /* tatami not set */
    }

    model = gtk_tree_view_get_model(GTK_TREE_VIEW(match_view[tatami-1]));

    if (find_group_iter(model, &grp_iter, group) == FALSE) {
        iter_set = FALSE;
        ok = gtk_tree_model_get_iter_first(model, &gi);
        while (ok) {
            gtk_tree_model_get(model, &gi,
                               COL_MATCH_GROUP, &grp,
                               -1);
            if (grp > group) {
                gtk_tree_store_insert_before((GtkTreeStore *)model,
                                             &grp_iter,
                                             NULL,
                                             &gi);
                iter_set = TRUE;
                break;
            }
            ok = gtk_tree_model_iter_next(model, &gi);
        }
        if (iter_set == FALSE)
            gtk_tree_store_append((GtkTreeStore *)model, 
                                  &grp_iter, NULL);

        gtk_tree_store_set((GtkTreeStore *)model, &grp_iter,
                           COL_MATCH_GROUP,        group,
                           COL_MATCH_CAT,          0,
                           COL_MATCH_NUMBER,       0,
                           COL_MATCH_BLUE,         0,
                           COL_MATCH_BLUE_SCORE,   0,
                           COL_MATCH_BLUE_POINTS,  0,
                           COL_MATCH_WHITE_POINTS, 0,
                           COL_MATCH_WHITE_SCORE,  0,
                           COL_MATCH_WHITE,        0,
                           COL_MATCH_TIME,         0,
                           COL_MATCH_COMMENT,      0,
                           COL_MATCH_VIS,          FALSE,
                           COL_MATCH_STATUS,       0,
                           -1);
    }

    if (find_match_iter(model, &cat, group, m->category, 0) == FALSE) {
        iter_set = FALSE;
        ok = gtk_tree_model_iter_children(model, &ic, &grp_iter);
        while (ok) {
            gint colcat;

            gtk_tree_model_get(model, &ic,
                               COL_MATCH_CAT, &colcat,
                               -1);
            if (colcat > m->category) {
                gtk_tree_store_insert_before((GtkTreeStore *)model,
                                             &cat,
                                             NULL,
                                             &ic);
                iter_set = TRUE;
                break;
            }
            ok = gtk_tree_model_iter_next(model, &ic);
        }
        if (iter_set == FALSE)
            gtk_tree_store_append((GtkTreeStore *)model, 
                                  &cat, &grp_iter);

        gtk_tree_store_set((GtkTreeStore *)model, &cat,
                           COL_MATCH_GROUP,        group,
                           COL_MATCH_CAT,          m->category,
                           COL_MATCH_NUMBER,       0,
                           COL_MATCH_BLUE,         0,
                           COL_MATCH_BLUE_SCORE,   0,
                           COL_MATCH_BLUE_POINTS,  0,
                           COL_MATCH_WHITE_POINTS, 0,
                           COL_MATCH_WHITE_SCORE,  0,
                           COL_MATCH_WHITE,        0,
                           COL_MATCH_TIME,         0,
                           COL_MATCH_COMMENT,      0,
                           COL_MATCH_VIS,          FALSE,
                           COL_MATCH_STATUS,       0,
                           -1);
    }

    iter_set = FALSE;
    ok = gtk_tree_model_iter_children(model, &tmp_iter, &cat);
    while (ok) {
        gtk_tree_model_get(model, &tmp_iter,
                           COL_MATCH_NUMBER, &num,
                           -1);
        if (num > m->number) {
            gtk_tree_store_insert_before((GtkTreeStore *)model,
                                         &iter,
                                         NULL,
                                         &tmp_iter);
            iter_set = TRUE;
            break;
        }
        ok = gtk_tree_model_iter_next(model, &tmp_iter);
    }
    if (iter_set == FALSE)
        gtk_tree_store_append((GtkTreeStore *)model, 
                              &iter, &cat);
update:
    gtk_tree_store_set((GtkTreeStore *)model, &iter,
                       COL_MATCH_GROUP,        group,
                       COL_MATCH_CAT,          m->category,
                       COL_MATCH_NUMBER,       m->number,
                       COL_MATCH_BLUE,         m->blue,
                       COL_MATCH_BLUE_SCORE,   m->blue_score,
                       COL_MATCH_BLUE_POINTS,  m->blue_points,
                       COL_MATCH_WHITE_POINTS, m->white_points,
                       COL_MATCH_WHITE_SCORE,  m->white_score,
                       COL_MATCH_WHITE,        m->white,
                       COL_MATCH_TIME,         m->match_time,
                       COL_MATCH_COMMENT,      m->comment,
                       COL_MATCH_VIS,          TRUE,
                       COL_MATCH_STATUS,       m->forcedtatami,
                       -1);

    g_static_mutex_unlock(&set_match_mutex);
}

static gint remove_category_from_matches(gint category)
{
    GtkTreeIter grp_iter, cat_iter;
    GtkTreeModel *model;
    gboolean ok_grp, ok_cat;
    gint cat, i;

    /* find current placement */
    for (i = 0; i < NUM_TATAMIS; i++) {
        model = gtk_tree_view_get_model(GTK_TREE_VIEW(match_view[i]));

        /* go through groups */
        ok_grp = gtk_tree_model_get_iter_first(model, &grp_iter);
        while (ok_grp) {
            /* go through categories */
            ok_cat = gtk_tree_model_iter_children(model, &cat_iter, &grp_iter);
            while (ok_cat) {
                gtk_tree_model_get(model, &cat_iter,
                                   COL_MATCH_CAT, &cat,
                                   -1);
                if (cat == category) {
                    gtk_tree_store_remove(GTK_TREE_STORE(model), &cat_iter);
                    if (gtk_tree_model_iter_n_children(model, &grp_iter) == 0)
                        gtk_tree_store_remove(GTK_TREE_STORE(model), &grp_iter);
                    return i+1; /* return tatami number */
                }
					
                ok_cat = gtk_tree_model_iter_next(model, &cat_iter);
            }

            ok_grp = gtk_tree_model_iter_next(model, &grp_iter);
        }
		
    }
	
    //g_print("Error: %s %d (cannot remove %d)\n", __FUNCTION__, __LINE__, category);
    return 0;
}

void category_refresh(gint category)
{
    gint tatami, group, old_tatami, sys;
    GtkTreeIter cat_iter;

    g_static_mutex_lock(&set_match_mutex);
    old_tatami = remove_category_from_matches(category);
    g_static_mutex_unlock(&set_match_mutex);

    /* Find info about category */
    if (find_iter(&cat_iter, category) == FALSE) {
        g_print("Error: %s %d (cat %d)\n", __FUNCTION__, __LINE__, category);
        return; /* error */
    }
        
    /* Match found. Look for tatami number. */
    gtk_tree_model_get(current_model, &cat_iter,
                       COL_BELT, &tatami,
                       COL_BIRTHYEAR, &group,
                       -1);
#if 0
    struct match m;
    memset(&m, 0, sizeof(m));
    m.category = category;
    m.group = group;
    m.tatami = tatami;
    set_match(&m);
#endif
    db_read_matches_of_category(category);
    sys = db_get_system(category);
    update_matches(category, sys, 0);

    if (old_tatami && old_tatami != tatami) {
        update_matches(0, 0, old_tatami);
        /**********
                   g_static_mutex_lock(&next_match_mutex);
                   struct match *nm = db_next_match(0, old_tatami);
                   gint n = nm[0].number;
                   gint cat = nm[0].category;
                   g_static_mutex_unlock(&next_match_mutex);
                   if (n < 1000) {
                   gint sys = db_get_system(cat);
                   update_matches(cat, sys);
                   }
        *************/
    }
}

void gategories_refresh(void)
{
#if 0
    GtkTreeIter cat_iter;
    gboolean ok_cat;

    ok_cat = gtk_tree_model_get_iter_first(current_model, &cat_iter);
    while (ok_cat) {
        gboolean visible;
        gint     index;

        gtk_tree_model_get(current_model, &cat_iter, 
                           COL_MATCH_INDEX,   &index,
                           COL_MATCH_VIS, &visible, 
                           -1);
		
        if (!visible) {
            g_print("refresh cat %d\n", index);
            category_refresh(index);			
        }

        ok_cat = gtk_tree_model_iter_next(current_model, &cat_iter);
    }
#endif
}

void matches_refresh(void)
{
    gint i;

    for (i = 0; i < NUM_TATAMIS; i++)
        gtk_tree_store_clear(GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(match_view[i]))));

    db_read_matches();

    for (i = 0; i < NUM_TATAMIS; i++) {
        update_matches(0, 0, i+1);
        /********************
		g_static_mutex_lock(&next_match_mutex);
                struct match *nm = db_next_match(0, i+1);
		gint n = nm[0].number;
		g_static_mutex_unlock(&next_match_mutex);
                if (n < 1000) {
                        gint sys = db_get_system(nm[0].category);
                        update_matches(nm[0].category, sys);
                } else {
			struct message msg;
			gint tatami = i+1;

			NULL_MATCH_INFO(category, tatami, 0); 
			NULL_MATCH_INFO(blue_first, tatami, 0); 
			NULL_MATCH_INFO(blue_last, tatami, 0); 
			NULL_MATCH_INFO(blue_club, tatami, 0); 
			NULL_MATCH_INFO(white_first, tatami, 0); 
			NULL_MATCH_INFO(white_last, tatami, 0); 
			NULL_MATCH_INFO(white_club, tatami, 0); 
			next_matches_info[tatami-1][0].catnum = 0;
			NULL_MATCH_INFO(category, tatami, 1); 
			NULL_MATCH_INFO(blue_first, tatami, 1); 
			NULL_MATCH_INFO(blue_last, tatami, 1); 
			NULL_MATCH_INFO(blue_club, tatami, 1); 
			NULL_MATCH_INFO(white_first, tatami, 1); 
			NULL_MATCH_INFO(white_last, tatami, 1); 
			NULL_MATCH_INFO(white_club, tatami, 1); 
			next_matches_info[tatami-1][1].catnum = 0;

			memset(&msg, 0, sizeof(msg));
			msg.type = MSG_NEXT_MATCH;
			msg.u.next_match.tatami = tatami;
			send_packet(&msg);
		}
        ************/
    }

    refresh_sheet_display(TRUE);
}

void matches_refresh_1(gint category, gint sys)
{
    db_read_matches();
    update_matches(category, sys, 0);
    refresh_sheet_display(TRUE);
}

void matches_clear(void)
{
        
}

void get_match_data(GtkTreeModel *model, GtkTreeIter *iter, gint *group, gint *category, gint *number)
{
    gtk_tree_model_get(model, iter,
                       COL_MATCH_GROUP,  group,
                       COL_MATCH_CAT,    category,
                       COL_MATCH_NUMBER, number,
                       -1);
}

void set_match_col_titles(void)
{
    GtkTreeViewColumn *col;
    gint i;

    for (i = 0; i < NUM_TATAMIS; i++) {
        if (!match_view[i])
            continue;

        col = gtk_tree_view_get_column(GTK_TREE_VIEW(match_view[i]), COL_MATCH_GROUP);
        gtk_tree_view_column_set_title(col, _("Group"));

        col = gtk_tree_view_get_column(GTK_TREE_VIEW(match_view[i]), COL_MATCH_CAT);
        gtk_tree_view_column_set_title(col, _("Category"));

        col = gtk_tree_view_get_column(GTK_TREE_VIEW(match_view[i]), COL_MATCH_NUMBER);
        gtk_tree_view_column_set_title(col, _("Match"));

        col = gtk_tree_view_get_column(GTK_TREE_VIEW(match_view[i]), COL_MATCH_BLUE);
        gtk_tree_view_column_set_title(col, _("Blue"));

        col = gtk_tree_view_get_column(GTK_TREE_VIEW(match_view[i]), COL_MATCH_BLUE_POINTS);
        gtk_tree_view_column_set_title(col, _("Points"));

        col = gtk_tree_view_get_column(GTK_TREE_VIEW(match_view[i]), COL_MATCH_WHITE_POINTS);
        gtk_tree_view_column_set_title(col, _("Points"));

        col = gtk_tree_view_get_column(GTK_TREE_VIEW(match_view[i]), COL_MATCH_WHITE);
        gtk_tree_view_column_set_title(col, _("White"));

        col = gtk_tree_view_get_column(GTK_TREE_VIEW(match_view[i]), COL_MATCH_TIME);
        gtk_tree_view_column_set_title(col, _("Time"));

        col = gtk_tree_view_get_column(GTK_TREE_VIEW(match_view[i]), COL_MATCH_COMMENT);
        gtk_tree_view_column_set_title(col, _("Comment"));
    }
}
