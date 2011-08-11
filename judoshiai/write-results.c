/* -*- mode: C; c-basic-offset: 4;  -*- */

/*
 * Copyright (C) 2006-2011 by Hannu Jokinen
 * Full copyright text is included in the software package.
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>

#include "sqlite3.h"
#include "judoshiai.h"

void write_results(FILE *f);

gboolean create_statistics = FALSE;

#define SAVED_COMP_SIZE 2000
static gint saved_competitors[SAVED_COMP_SIZE];
static gint saved_competitor_cnt = 0;


static void write_result(FILE *f, gint num, const gchar *first, const gchar *last, 
			 const gchar *club, const gchar *country)
{
    if (club_text == (CLUB_TEXT_CLUB|CLUB_TEXT_COUNTRY))
	fprintf(f, "<tr><td>%d.</td><td>%s %s</td><td>%s/%s</td></tr>\n", 
		num, utf8_to_html(first), utf8_to_html(last), utf8_to_html(club), utf8_to_html(country));
    else {
	struct club_name_data *data = club_name_get(club);
	if (club_text == CLUB_TEXT_CLUB && 
	    data && data->address && data->address[0])
	    fprintf(f, "<tr><td>%d.</td><td>%s %s</td><td>%s, %s</td></tr>\n", 
		    num, utf8_to_html(first), utf8_to_html(last), 
		    utf8_to_html(club), utf8_to_html(data->address));
	else
	    fprintf(f, "<tr><td>%d.</td><td>%s %s</td><td>%s</td></tr>\n", 
		    num, utf8_to_html(first), utf8_to_html(last), utf8_to_html(club_text==CLUB_TEXT_CLUB ? club : country));
    }

    if (create_statistics)
        club_stat_add(club, country, num);
}

void write_competitor(FILE *f, const gchar *first, const gchar *last, const gchar *belt, 
                      const gchar *club, const gchar *category, const gint index, const gboolean by_club)
{
    // save last club as a hash
    static gint last_crc = 0;
    static gint member_count = 0;

    if (by_club) {
        gchar buf[16], buf2[16];
        gint crc = pwcrc32((guchar *)club, strlen(club));
        
        if (crc != last_crc)
            member_count = 0;
        member_count++;
        snprintf(buf2, sizeof(buf2), "&nbsp;#%d", member_count);

        if (create_statistics) {
            gint pos = avl_get_competitor_position(index);
        
            if (pos > 0)
                snprintf(buf, sizeof(buf), "%d.", pos);
            else
                snprintf(buf, sizeof(buf), "-");

            fprintf(f, 
                    "<tr><td>%d</td><td>%s</td><td><a href=\"%d.html\">%s, %s</a></td><td>%s</td>"
                    "<td><a href=\"%s.html\">%s</a></td><td align=\"center\">%s</td></tr>\n", 
                    member_count, member_count == 1 ? utf8_to_html(club) : "", 
                    index, utf8_to_html(last), utf8_to_html(first), grade_visible ? belt : "", 
                    txt2hex(category), category, buf);
        } else
            fprintf(f, 
                    "<tr><td>%d</td><td>%s</td><td>%s, %s</td><td>%s</td>"
                    "<td><a href=\"%s.html\">%s</a></td><td>&nbsp;</td></tr>\n", 
                    member_count, member_count == 1 ? utf8_to_html(club) : "", 
                    utf8_to_html(last), utf8_to_html(first), grade_visible ? belt : "",  
                    txt2hex(category), category);

        last_crc = crc;
    } else {
        if (create_statistics) {
            gchar buf[16];
            gint pos = avl_get_competitor_position(index);
        
            if (pos > 0)
                snprintf(buf, sizeof(buf), "%d.", pos);
            else
                snprintf(buf, sizeof(buf), "-");

            fprintf(f, 
                    "<tr><td><a href=\"%d.html\">%s, %s</a></td><td>%s</td><td>%s</td>"
                    "<td><a href=\"%s.html\">%s</a></td><td align=\"center\">%s</td></tr>\n", 
                    index, utf8_to_html(last), utf8_to_html(first), grade_visible ? belt : "", 
                    utf8_to_html(club), txt2hex(category), category, buf);
        } else
            fprintf(f, 
                    "<tr><td>%s, %s</td><td>%s</td><td>%s</td>"
                    "<td><a href=\"%s.html\">%s</a></td><td>&nbsp;</td></tr>\n", 
                    utf8_to_html(last), utf8_to_html(first), grade_visible ? belt : "", utf8_to_html(club), 
                    txt2hex(category), category);

        saved_competitors[saved_competitor_cnt] = index;
        if (saved_competitor_cnt < SAVED_COMP_SIZE)
            saved_competitor_cnt++;

        last_crc = 0; // this branch is called first and thus last_crc is initialised (many times)
    }
}

static void make_top_frame_1(FILE *f, gchar *meta)
{
    fprintf(f, "<html><head>"
            "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\r\n"
            "%s"
            "<link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\">\r\n"
            "<meta name=\"keywords\" content=\"JudoShiai-%s\" />\r\n"
            "<title>%s  %s  %s</title></head>\r\n"
            "<body class=\"titleframe\"><table><tr>\r\n"
            "<td colspan=\"2\" align=\"center\"><h1>%s  %s  %s</h1></td></tr><tr>\r\n", 
            meta,
            SHIAI_VERSION,
            utf8_to_html(info_competition), utf8_to_html(info_date), utf8_to_html(info_place),
            utf8_to_html(info_competition), utf8_to_html(info_date), utf8_to_html(info_place));
}

static void make_top_frame(FILE *f)
{
    make_top_frame_1(f, "");
}

static void make_top_frame_refresh(FILE *f)
{
    make_top_frame_1(f, "<meta http-equiv=\"refresh\" content=\"30\"/>\r\n");
}

static void make_bottom_frame(FILE *f)
{
    fprintf(f, "</tr></table></body></html>\n");
}

static gint make_left_frame(FILE *f)
{
    GtkTreeIter iter;
    gboolean ok;
    gint num_cats = 0;

    fprintf(f, 
            "<td valign=\"top\"><table class=\"resultslink\"><tr><td class=\"resultslink\">"
            "<a href=\"index.html\">%s</a></td></tr></table>\n", _T(results));
    fprintf(f,
            "<table class=\"competitorslink\"><tr><td class=\"competitorslink\">"
            "<a href=\"competitors.html\">%s</a></td></tr></table>\n", _T(competitor));
    if (create_statistics) {
        fprintf(f, 
                "<table class=\"medalslink\"><tr><td class=\"medalslink\">"
                "<a href=\"medals.html\">%s</a></td></tr></table>\n", _T(medals));
        fprintf(f, 
                "<table class=\"statisticslink\"><tr><td class=\"statisticslink\">"
                "<a href=\"statistics.html\">%s</a></td></tr></table>\n", _T(statistics));
    }

    if (automatic_web_page_update)
        fprintf(f, 
                "<table class=\"nextmatcheslink\"><tr><td class=\"nextmatcheslink\">"
                "<a href=\"nextmatches.html\">%s</a></td></tr></table>\n", _T(nextmatch2));

    fprintf(f,
            "<table class=\"categorieshdr\"><tr><td class=\"categorieshdr\">"
            "%s</td></tr></table>\n", _T(categories));

    fprintf(f, "<table class=\"categorylinks\">");

    ok = gtk_tree_model_get_iter_first(current_model, &iter);

    while (ok) {
        gint n = gtk_tree_model_iter_n_children(current_model, &iter);
        gint index;

        num_cats++;
                                
        if (n >= 1 && n <= NUM_COMPETITORS) {
            struct judoka *j;

            gtk_tree_model_get(current_model, &iter,
                               COL_INDEX, &index,
                               -1);
            j = get_data(index);
            if (j) {
                if (!automatic_web_page_update) {
                    fprintf(f, 
                            "<tr><td class=\"categorylinksleft\"><a href=\"%s.html\">%s</a></td>"
                            "<td class=\"categorylinksright\">"
                            "<a href=\"%s.pdf\" target=\"_blank\"> "
                            "(PDF)</a></td></tr>", 
                            txt2hex(j->last), utf8_to_html(j->last), 
                            txt2hex(j->last));
                } else {
                    fprintf(f, "<tr><td class=\"categorylinksonly\"><a href=\"%s.html\">%s</a></td></tr>", 
                            txt2hex(j->last), utf8_to_html(j->last));
                }

                free_judoka(j);
            }
        }

        ok = gtk_tree_model_iter_next(current_model, &iter);
    }

    fprintf(f, "</table></td>\n");
    return num_cats;
}


static void pool_results(FILE *f, gint category, struct judoka *ctg, gint num_judokas, gboolean dpool2)
{
    struct pool_matches pm;
    gint i;

    fill_pool_struct(category, num_judokas, &pm, dpool2);

    if (dpool2)
        num_judokas = 4;

    if (pm.finished)
        get_pool_winner(num_judokas, pm.c, pm.yes, pm.wins, pm.pts, pm.mw, pm.j, pm.all_matched);

    for (i = 1; i <= num_judokas; i++) {
        if (pm.finished == FALSE || pm.j[pm.c[i]] == NULL)
            continue;

        // Spanish have two bronzes in pool system
        if (i <= 4 && draw_system == DRAW_SPANISH &&
            (pm.j[pm.c[i]]->deleted & HANSOKUMAKE) == 0) {
            write_result(f, i == 4 ? 3 : i, pm.j[pm.c[i]]->first, 
                         pm.j[pm.c[i]]->last, pm.j[pm.c[i]]->club, pm.j[pm.c[i]]->country);
            avl_set_competitor_position(pm.j[pm.c[i]]->index, i == 4 ? 3 : i);
            db_set_category_positions(category, pm.j[pm.c[i]]->index, i == 4 ? 3 : i);
        } else if (i <= 3 && 
            (pm.j[pm.c[i]]->deleted & HANSOKUMAKE) == 0) {
            write_result(f, i, pm.j[pm.c[i]]->first, 
                         pm.j[pm.c[i]]->last, pm.j[pm.c[i]]->club, pm.j[pm.c[i]]->country);
            avl_set_competitor_position(pm.j[pm.c[i]]->index, i);
            db_set_category_positions(category, pm.j[pm.c[i]]->index, i);
        }
    }

    /* clean up */
    empty_pool_struct(&pm);
}

static void dqpool_results(FILE *f, gint category, struct judoka *ctg, gint num_judokas, struct compsys sys)
{
    struct pool_matches pm;
    struct judoka *j1 = NULL;
    gint i;
    int gold, silver, bronze1, bronze2;

    fill_pool_struct(category, num_judokas, &pm, FALSE);

    i = num_matches(sys.system, num_judokas) + 
        (sys.system == SYSTEM_DPOOL ? 1 : 5);

    /* first semifinal */
    if (pm.m[i].blue_points)
        bronze1 = pm.m[i].white;
    else
        bronze1 = pm.m[i].blue;

    i++;

    /* second semifinal */
    if (pm.m[i].blue_points)
        bronze2 = pm.m[i].white;
    else
        bronze2 = pm.m[i].blue;

    i++;

    /* final */
    if (pm.m[i].blue_points || pm.m[i].white == GHOST) {
        gold = pm.m[i].blue;
        silver = pm.m[i].white;
    } else {
        gold = pm.m[i].white;
        silver = pm.m[i].blue;
    }

    if (gold && (j1 = get_data(gold))) {
        write_result(f, 1, j1->first, j1->last, j1->club, j1->country);
        avl_set_competitor_position(gold, 1);
        db_set_category_positions(category, gold, 1);
        free_judoka(j1);
    }
    if (gold && (j1 = get_data(silver))) {
        write_result(f, 2, j1->first, j1->last, j1->club, j1->country);
        avl_set_competitor_position(silver, 2);
        db_set_category_positions(category, silver, 2);
        free_judoka(j1);
    }
    if (gold && (j1 = get_data(bronze1))) {
        write_result(f, 3, j1->first, j1->last, j1->club, j1->country);
        avl_set_competitor_position(bronze1, 3);
        db_set_category_positions(category, bronze1, 3);
        free_judoka(j1);
    }
    if (gold && (j1 = get_data(bronze2))) {
        write_result(f, 3, j1->first, j1->last, j1->club, j1->country);
        avl_set_competitor_position(bronze2, 3);
        db_set_category_positions(category, bronze2, 4);
        free_judoka(j1);
    }

    /* clean up */
    empty_pool_struct(&pm);
}

#define GET_WINNER_AND_LOSER(_w)                                        \
    winner = (m[_w].blue_points || m[_w].white==GHOST) ? m[_w].blue :   \
        ((m[_w].white_points || m[_w].blue==GHOST) ? m[_w].white : 0);  \
    loser = (m[_w].blue_points || m[_w].white==GHOST) ? m[_w].white :   \
        ((m[_w].white_points || m[_w].blue==GHOST) ? m[_w].blue : 0)

#define GET_GOLD(_w)                                                    \
    gold = (m[_w].blue_points || m[_w].white==GHOST) ? m[_w].blue :     \
        ((m[_w].white_points || m[_w].blue==GHOST) ? m[_w].white : 0);  \
    silver = (m[_w].blue_points || m[_w].white==GHOST) ? m[_w].white :  \
        ((m[_w].white_points || m[_w].blue==GHOST) ? m[_w].blue : 0)

#define GET_BRONZE1(_w)                                                 \
    bronze1 = (m[_w].blue_points || m[_w].white==GHOST) ? m[_w].blue :  \
        ((m[_w].white_points || m[_w].blue==GHOST) ? m[_w].white : 0);  \
    fifth1 = (m[_w].blue_points && m[_w].white > GHOST) ? m[_w].white : \
        ((m[_w].white_points && m[_w].blue > GHOST) ? m[_w].blue : 0)

#define GET_BRONZE2(_w)                                                 \
    bronze2 = (m[_w].blue_points || m[_w].white==GHOST) ? m[_w].blue :  \
        ((m[_w].white_points || m[_w].blue==GHOST) ? m[_w].white : 0);  \
    fifth2 = (m[_w].blue_points && m[_w].white > GHOST) ? m[_w].white : \
        ((m[_w].white_points && m[_w].blue > GHOST) ? m[_w].blue : 0)

static void french_results(FILE *f, gint category, struct judoka *ctg,
                           gint num_judokas, struct compsys systm, gint pagenum)
{
    struct match m[NUM_MATCHES];
    struct judoka *j1;
    gint gold = 0, silver = 0, bronze1 = 0, bronze2 = 0, fourth = 0, 
        fifth1 = 0, fifth2 = 0, seventh1 = 0, seventh2 = 0;
    gint winner= 0, loser = 0;
    gint sys = systm.system - SYSTEM_FRENCH_8;
    gint table = systm.table;

    memset(m, 0, sizeof(m));
    db_read_category_matches(category, m);

    GET_WINNER_AND_LOSER(get_abs_matchnum_by_pos(systm, 1, 1));
    gold = winner;
    silver = loser;
    if (table == TABLE_MODIFIED_DOUBLE_ELIMINATION) {
        GET_WINNER_AND_LOSER(get_abs_matchnum_by_pos(systm, 2, 1));
        silver = winner;
        bronze1 = loser;
    } else if (one_bronze(table, sys)) {
        GET_WINNER_AND_LOSER(get_abs_matchnum_by_pos(systm, 3, 1));
        bronze1 = winner;
        fourth = loser;
        GET_WINNER_AND_LOSER(get_abs_matchnum_by_pos(systm, 5, 1));
        fifth1 = loser;
        GET_WINNER_AND_LOSER(get_abs_matchnum_by_pos(systm, 5, 2));
        fifth2 = loser;
    } else {
        GET_WINNER_AND_LOSER(get_abs_matchnum_by_pos(systm, 3, 1));
        bronze1 = winner;
        fifth1 = loser;
        GET_WINNER_AND_LOSER(get_abs_matchnum_by_pos(systm, 3, 2));
        bronze2 = winner;
        fifth2 = loser;

        GET_WINNER_AND_LOSER(get_abs_matchnum_by_pos(systm, 7, 1));
        seventh1 = loser;
        GET_WINNER_AND_LOSER(get_abs_matchnum_by_pos(systm, 7, 2));
        seventh2 = loser;
    }

    if (table == TABLE_NO_REPECHAGE ||
        table == TABLE_ESP_DOBLE_PERDIDA) {
	bronze1 = fifth1;
	bronze2 = fifth2;
	fifth1 = fifth2 = 0;
        seventh1 = seventh2 = 0;
    }

    if (gold && (j1 = get_data(gold))) {
        write_result(f, 1, j1->first, j1->last, j1->club, j1->country);
        avl_set_competitor_position(gold, 1);
        db_set_category_positions(category, gold, 1);
        free_judoka(j1);
    }
    if (gold && (j1 = get_data(silver))) {
        write_result(f, 2, j1->first, j1->last, j1->club, j1->country);
        avl_set_competitor_position(silver, 2);
        db_set_category_positions(category, silver, 2);
        free_judoka(j1);
    }
    if (gold && (j1 = get_data(bronze1))) {
        write_result(f, 3, j1->first, j1->last, j1->club, j1->country);
        avl_set_competitor_position(bronze1, 3);
        db_set_category_positions(category, bronze1, 3);
        free_judoka(j1);
    }
    if (gold && (j1 = get_data(bronze2))) {
        write_result(f, 3, j1->first, j1->last, j1->club, j1->country);
        avl_set_competitor_position(bronze2, 3);
        db_set_category_positions(category, bronze2, 4);
        free_judoka(j1);
    }
    if (gold && (j1 = get_data(fourth))) {
        write_result(f, 4, j1->first, j1->last, j1->club, j1->country);
        avl_set_competitor_position(fourth, 4);
        db_set_category_positions(category, fourth, 4);
        free_judoka(j1);
    }
    if (gold && (j1 = get_data(fifth1))) {
        write_result(f, 5, j1->first, j1->last, j1->club, j1->country);
        avl_set_competitor_position(fifth1, 5);
        db_set_category_positions(category, fifth1, 5);
        free_judoka(j1);
    }
    if (gold && (j1 = get_data(fifth2))) {
        write_result(f, 5, j1->first, j1->last, j1->club, j1->country);
        avl_set_competitor_position(fifth2, 5);
        db_set_category_positions(category, fifth2, 6);
        free_judoka(j1);
    }
    if (gold && (j1 = get_data(seventh1))) {
        write_result(f, 7, j1->first, j1->last, j1->club, j1->country);
        avl_set_competitor_position(seventh1, 7);
        db_set_category_positions(category, seventh1, 7);
        free_judoka(j1);
    }
    if (gold && (j1 = get_data(seventh2))) {
        write_result(f, 7, j1->first, j1->last, j1->club, j1->country);
        avl_set_competitor_position(seventh2, 7);
        db_set_category_positions(category, seventh2, 8);
        free_judoka(j1);
    }
}

static void write_cat_result(FILE *f, gint category)
{
    gint num_judokas;
    struct judoka *ctg = NULL;
    struct compsys sys;

    ctg = get_data(category);

    if (ctg == NULL)
        return;

    fprintf(f, "<tr><td colspan=\"3\"><b><a href=\"%s.html\">%s</a></b></td></tr>\n", 
            txt2hex(ctg->last), utf8_to_html(ctg->last));

    /* find system */
    sys = db_get_system(category);
    num_judokas = sys.numcomp;

    switch (sys.system) {
    case SYSTEM_POOL:
    case SYSTEM_DPOOL2:
        pool_results(f, category, ctg, num_judokas, sys.system == SYSTEM_DPOOL2);
        break;

    case SYSTEM_DPOOL:
    case SYSTEM_QPOOL:
        dqpool_results(f, category, ctg, num_judokas, sys);
        break;

    case SYSTEM_FRENCH_8:
    case SYSTEM_FRENCH_16:
    case SYSTEM_FRENCH_32:
    case SYSTEM_FRENCH_64:
    case SYSTEM_FRENCH_128:
        french_results(f, category, ctg, num_judokas, 
                       sys, 0);
        break;
    }

    /* clean up */
    free_judoka(ctg);
}

void write_results(FILE *f)
{
    GtkTreeIter iter;
    gboolean ok;

    if (create_statistics)
        init_club_tree();

    fprintf(f, "<td valign=\"top\"><table class=\"resultlist\">\n"
            "<tr><td colspan=\"3\" align=\"center\"><h2>%s</h2></td></tr>",
            _T(results));

    ok = gtk_tree_model_get_iter_first(current_model, &iter);

    while (ok) {
        gint n = gtk_tree_model_iter_n_children(current_model, &iter);
        gint index;

        if (n >= 1 && n <= NUM_COMPETITORS) {
            gtk_tree_model_get(current_model, &iter,
                               COL_INDEX, &index,
                               -1);
            write_cat_result(f, index);
        }

        ok = gtk_tree_model_iter_next(current_model, &iter);
    }
    fprintf(f, "</table></td>\n");
}

static FILE *open_write(gchar *filename)
{
    gchar *file = g_build_filename(current_directory, filename, NULL);
    FILE *f = fopen(file, "w");
    g_free(file);
    return f;
}

void write_html(gint cat)
{
    gchar buf[64];
    gchar *hextext;

    struct judoka *j = get_data(cat);
    if (!j)
        return;

    progress_show(-1.0, j->last);

    struct compsys sys = db_get_system(cat);
        
    hextext = txt2hex(j->last);
    snprintf(buf, sizeof(buf), "%s.html", hextext);

    FILE *f = open_write(buf);
    if (!f)
        goto out;

    make_top_frame(f);
    make_left_frame(f);

    hextext = txt2hex(j->last);

    if (sys.system == SYSTEM_FRENCH_128) {
        fprintf(f,
                "<td valign=\"top\"><img src=\"%s.png\"><br>"
                "<img src=\"%s-1.png\"><br>"
                "<img src=\"%s-2.png\"><br>"
                "<img src=\"%s-3.png\"><br>"
                "<img src=\"%s-4.png\">"
                "</td>\n",
                hextext, hextext, hextext, hextext, hextext);
    } else if (sys.system == SYSTEM_FRENCH_64 ||
        sys.system == SYSTEM_QPOOL) {
        fprintf(f,
                "<td valign=\"top\"><img src=\"%s.png\"><br>"
                "<img src=\"%s-1.png\"><br>"
                "<img src=\"%s-2.png\">"
                "</td>\n",
                hextext, hextext, hextext);
    } else if (sys.system == SYSTEM_DPOOL2) {
        fprintf(f,
                "<td valign=\"top\"><img src=\"%s.png\"><br>"
                "<img src=\"%s-1.png\">"
                "</td>\n",
                hextext, hextext);
    } else {
        fprintf(f,
                "<td valign=\"top\"><img src=\"%s.png\"></td>\n",
                hextext);
    }

    make_bottom_frame(f);
    fclose(f);

    write_png(NULL, (gpointer)cat);
out:
    free_judoka(j);
}

void match_statistics(FILE *f)
{
    extern gint num_categories;
    extern struct cat_def category_definitions[];

    gint i, k;
    gint toti = 0, totw = 0, toty = 0, toth = 0, tott = 0, tots = 0, totc = 0;
    gchar *cmd = g_strdup_printf("select * from matches");
    gint numrows = db_get_table(cmd);
    g_free(cmd);

    if (numrows < 0)
        goto out;

    for (i = 0; i < num_categories; i++) {
        memset(&category_definitions[i].stat, 0, sizeof(category_definitions[0].stat));
    }

    for (k = 0; k < numrows; k++) {
        gint cat = atoi(db_get_data(k, "category"));
        struct judoka *j = get_data(cat);
        if (!j)
            continue;
        i = find_age_index(j->last);
        free_judoka(j);

        if (i < 0)
            continue;

        gint blue_points = atoi(db_get_data(k, "blue_points"));
        gint white_points = atoi(db_get_data(k, "white_points"));
        gint time = atoi(db_get_data(k, "time"));

        if (blue_points == 0 && white_points == 0)
            continue;

        category_definitions[i].stat.total++;
        category_definitions[i].stat.time += time;

        if (blue_points == 10 || white_points == 10)
            category_definitions[i].stat.ippons++;
        else if (blue_points == 7 || white_points == 7)
            category_definitions[i].stat.wazaaris++;
        else if (blue_points == 5 || white_points == 5)
            category_definitions[i].stat.yukos++;
        else if (blue_points == 3 || white_points == 3)
            category_definitions[i].stat.kokas++;
        else
            category_definitions[i].stat.hanteis++;
    }

    db_close_table();

    /* competitors */
    cmd = g_strdup_printf("select * from competitors where \"deleted\"&1=0");
    numrows = db_get_table(cmd);
    g_free(cmd);

    if (numrows < 0)
        goto out;

    for (k = 0; k < numrows; k++) {
        gchar *cat = db_get_data(k, "category");
        if (!cat)
            continue;
        i = find_age_index(cat);
        if (i < 0)
            continue;
        category_definitions[i].stat.comp++;
    }

    db_close_table();
out:
    for (i = 0; i < num_categories; i++) {
        totc += category_definitions[i].stat.comp;
        toti += category_definitions[i].stat.ippons;
        totw += category_definitions[i].stat.wazaaris;
        toty += category_definitions[i].stat.yukos;
        toth += category_definitions[i].stat.hanteis;
        tott += category_definitions[i].stat.total;
        tots += category_definitions[i].stat.time;
    }

    fprintf(f, "<td valign=\"top\"><table class=\"statistics\">\n"
            "<tr><td class=\"stat2\" colspan=\"%d\"><h2>%s</h2></td></tr>\n"
            "<tr><th class=\"stat1\">%s:<th>%s", num_categories + 2,
            _T(statistics), _T(category), _T(total));
    for (i = 0; i < num_categories; i++)
        if (category_definitions[i].stat.total)
            fprintf(f, "<th>%s", category_definitions[i].agetext);

    fprintf(f, "\n<tr><td class=\"stat1\">%s:<td>%d", _T(competitor), totc);
    for (i = 0; i < num_categories; i++)
        if (category_definitions[i].stat.total)
            fprintf(f, "<td>%d", category_definitions[i].stat.comp);

    fprintf(f, "\n<tr><td class=\"stat1\">%s:<td>%d", _T(matches), tott);
    for (i = 0; i < num_categories; i++)
        if (category_definitions[i].stat.total)
            fprintf(f, "<td>%d", category_definitions[i].stat.total);

    fprintf(f, "\n<tr><td class=\"stat1\">Ippon:<td>%d", toti);
    for (i = 0; i < num_categories; i++)
        if (category_definitions[i].stat.total)
            fprintf(f, "<td>%d", category_definitions[i].stat.ippons);

    fprintf(f, "\n<tr><td class=\"stat1\">Waza-ari:<td>%d", totw);
    for (i = 0; i < num_categories; i++)
        if (category_definitions[i].stat.total)
            fprintf(f, "<td>%d", category_definitions[i].stat.wazaaris);

    fprintf(f, "\n<tr><td class=\"stat1\">Yuko:<td>%d", toty);
    for (i = 0; i < num_categories; i++)
        if (category_definitions[i].stat.total)
            fprintf(f, "<td>%d", category_definitions[i].stat.yukos);

    fprintf(f, "\n<tr><td class=\"stat1\">Hantei:<td>%d", toth);
    for (i = 0; i < num_categories; i++)
        if (category_definitions[i].stat.total)
            fprintf(f, "<td>%d", category_definitions[i].stat.hanteis);

    fprintf(f, "\n<tr><td class=\"stat1\">%s:<td>%d:%02d", _T(time), 
            tott ? (tots/tott)/60 : 0, tott ? (tots/tott)%60 : 0);
    for (i = 0; i < num_categories; i++)
        if (category_definitions[i].stat.total) {
            gint tmo = category_definitions[i].stat.time / 
                category_definitions[i].stat.total;
            fprintf(f, "<td>%d:%02d", tmo/60, tmo%60);
        }

    fprintf(f, "</table></td>\n");
}

void write_comp_stat(gint index)
{
    gchar buf[32];
    gint i;
    struct judoka *j = get_data(index);
    if (!j)
        return;
        
    snprintf(buf, sizeof(buf), "%d.html", index);

    FILE *f = open_write(buf);
    if (!f)
        goto out;

    make_top_frame(f);
    make_left_frame(f);

    fprintf(f, "<td valign=\"top\"><table class=\"compstat\">"
            "<tr><th colspan=\"7\">%s %s, %s</th></tr>"
            "<tr><td class=\"cshdr\">%s<td class=\"cshdr\">%s<td class=\"cshdr\">IWYKS"
            "<td align=\"center\" class=\"cshdr\">%s<td class=\"cshdr\">IWYKS"
            "<td class=\"cshdr\">%s<td class=\"cshdr\">%s</tr>",
            utf8_to_html(j->first), utf8_to_html(j->last), utf8_to_html(j->club),
            _T(category), _T(name), _T(points), _T(name), _T(time));


    gchar *cmd = g_strdup_printf("select * from matches where \"blue\"=%d or \"white\"=%d "
                                 "order by \"category\"",
                                 index, index);
    gint numrows = db_get_table(cmd);
    g_free(cmd);
        
    if (numrows < 0)
        goto out;

    for (i = 0; i < numrows; i++) {
        gint blue = atoi(db_get_data(i, "blue"));
        gint white = atoi(db_get_data(i, "white"));
        gint cat = atoi(db_get_data(i, "category"));
        struct judoka *j1 = get_data(blue);
        struct judoka *j2 = get_data(white);
        struct judoka *c = get_data(cat);
        if (j1 == NULL || j2 == NULL || c == NULL)
            goto done;

        gint blue_score = atoi(db_get_data(i, "blue_score"));
        gint white_score = atoi(db_get_data(i, "white_score"));
        gint blue_points = atoi(db_get_data(i, "blue_points"));
        gint white_points = atoi(db_get_data(i, "white_points"));
        gint mtime = atoi(db_get_data(i, "time"));
        if (blue_points || white_points)
            fprintf(f, 
                    "<tr><td>%s</td><td>%s %s</td><td class=\"%s\">%05x</td>"
                    "<td align=\"center\">%d - %d</td>"
                    "<td class=\"%s\">%05x</td><td>%s %s</td><td>%d:%02d</td></tr>\n",
                    utf8_to_html(c->last), utf8_to_html(j1->first), utf8_to_html(j1->last),
                    info_white_first ? "wscore" : "bscore",
                    blue_score, blue_points,
                    white_points, 
                    info_white_first ? "bscore" : "wscore",
                    white_score, 
                    utf8_to_html(j2->first), utf8_to_html(j2->last), mtime/60, mtime%60);

    done:
        free_judoka(j1);
        free_judoka(j2);
        free_judoka(c);
    }

    db_close_table();

    fprintf(f, "</table></td>");

    make_bottom_frame(f);
out:
    if (f)
        fclose(f);
    free_judoka(j);
}

void write_htmls(gint num_cats)
{
    GtkTreeIter iter;
    gboolean ok;
    gint cnt = 0;

    ok = gtk_tree_model_get_iter_first(current_model, &iter);

    while (ok) {
        gint n = gtk_tree_model_iter_n_children(current_model, &iter);
        gint index;

        cnt++;

        progress_show(cnt < num_cats ? 1.0*cnt/num_cats : 1.0, NULL);

        if (n >= 1 && n <= NUM_COMPETITORS) {
            gtk_tree_model_get(current_model, &iter,
                               COL_INDEX, &index,
                               -1);
            write_html(index);
        }

        ok = gtk_tree_model_iter_next(current_model, &iter);
    }

    progress_show(0.0, "");
}

static void init_club_data(void)
{ 
    /* competitors */
    gchar *cmd = g_strdup_printf("select * from competitors where \"deleted\"&1=0");
    gint numrows = db_get_table(cmd), k;
    g_free(cmd);

    if (numrows < 0)
        return;

    for (k = 0; k < numrows; k++) {
        gchar *club = db_get_data(k, "club");
        gchar *country = db_get_data(k, "country");
        if (!club)
            continue;
        club_stat_add(club, country, 0);
    }

    db_close_table();
}

void make_png_all(GtkWidget *w, gpointer data)
{
    FILE *f;
    gchar fname[1024];
    gint num_cats = 0, i;

    if (get_output_directory() < 0)
        return;

    /* make competitor positions = 0. write_results() will recalculate them. */
    avl_init_competitor_position();

    /* copy style.css */
    f = open_write("style.css");
    if (f) {
        gint n;
        gchar *stylefile = g_build_filename(installation_dir, "etc", "style.css", NULL);
        FILE *sf = fopen(stylefile, "r");
        g_free(stylefile);

        if (sf) {
            while ((n = fread(fname, 1, sizeof(fname), sf)) > 0) {
                fwrite(fname, 1, n, f);
            }
            fclose(sf);
        }
        fclose(f);
    }

    /* index.html */
    f = open_write("index.html");
    if (f) {
        make_top_frame(f);
        num_cats = make_left_frame(f);
        write_results(f);
        make_bottom_frame(f);
        fclose(f);
    }

    init_club_data();

    /* netxmatches.html */
    if (automatic_web_page_update) {
        make_next_matches_html();
    }

    /* sheets */
    write_htmls(num_cats);

    /* competitors */
    f = open_write("competitors.html");
    if (f) {
        make_top_frame(f);
        make_left_frame(f);
        saved_competitor_cnt = 0;
        db_print_competitors(f);
        make_bottom_frame(f);
        fclose(f);
    }

    f = open_write("competitors2.html");
    if (f) {
        make_top_frame(f);
        make_left_frame(f);
        db_print_competitors_by_club(f);
        make_bottom_frame(f);
        fclose(f);
    }

    /* sheets */
    write_htmls(num_cats);

    /* statistics */
    if (create_statistics) {
        /* medal list */
        f = open_write("medals.html");
        if (f) {
            make_top_frame(f);
            make_left_frame(f);
            club_stat_print(f);
            make_bottom_frame(f);
            fclose(f);
        }

        /* competitor stat */
        for (i = 0; i < saved_competitor_cnt; i++) {
            gchar buf[32];
            snprintf(buf, sizeof(buf), "%s %d%%", _("Competitors:"), 100*i/saved_competitor_cnt);
            progress_show((gdouble)i/(gdouble)saved_competitor_cnt, buf);
            write_comp_stat(saved_competitors[i]);
        }

        /* statistics */
        f = open_write("statistics.html");
        if (f) {
            make_top_frame(f);
            make_left_frame(f);
            match_statistics(f);
            make_bottom_frame(f);
            fclose(f);
        }
    }
    progress_show(0.0, "");
}

void make_next_matches_html(void)
{
    FILE *f;
    gint i, k;

    if (strlen(current_directory) <= 1) {
        if (get_output_directory() < 0)
            return;
    }

    f = open_write("nextmatches.html");
    if (!f)
        return;

    make_top_frame_refresh(f);
    make_left_frame(f);

    // header row
    fprintf(f, "<td><table class=\"nextmatches\"><tr>");
    for (i = 0; i < number_of_tatamis; i++) {
        fprintf(f, "<th align=\"center\" colspan=\"2\">Tatami %d</th>", i+1);
    }
    fprintf(f, "</tr><tr>\r\n");

    // last winners
    for (i = 0; i < number_of_tatamis; i++) {
        gchar *xf, *xl, *xc;

        //fprintf(f, "<td><table class=\"tatamimatches\">");

        if (next_matches_info[i][0].won_catnum) {
            xf = next_matches_info[i][0].won_first;
            xl = next_matches_info[i][0].won_last;
            xc = next_matches_info[i][0].won_cat;
        } else {
            xf = "";
            xl = "";
            xc = "";
        }

        fprintf(f, "<td class=\"cpl\">%s:<br>", _T(prevwinner));
        fprintf(f, "</td>\r\n<td class=\"cpr\">%s<br>%s %s<br>&nbsp;</td>", 
                utf8_to_html(xc), utf8_to_html(xf), utf8_to_html(xl));
    }
    fprintf(f, "</tr>\r\n");

    // next matches
    g_static_mutex_lock(&next_match_mutex);
    for (k = 0; k < NEXT_MATCH_NUM; k++) {
        //gchar *bgcolor = (k & 1) ? "bgcolor=\"#a0a0a0\"" : "bgcolor=\"#f0f0f0\"";
        gchar *class_ul = (k & 1) ? "class=\"cul1\"" : "class=\"cul2\"";
        gchar *class_ur = (k & 1) ? "class=\"cur1\"" : "class=\"cur2\"";
        gchar *class_dl = (k & 1) ? "class=\"cdl1\"" : "class=\"cdl2\"";
        gchar *class_dr = (k & 1) ? "class=\"cdr1\"" : "class=\"cdr2\"";
        struct judoka *blue, *white, *cat;

        fprintf(f, "<tr>");

        // match number and category
        for (i = 0; i < number_of_tatamis; i++) {
            struct match *m = get_cached_next_matches(i+1);

            if (m[k].number >= 1000) {
                fprintf(f, "<td %s>&nbsp;</td><td %s>&nbsp;</td>", class_ul, class_ur);
                continue;
            }

            cat = get_data(m[k].category);

            fprintf(f, "<td %s><b>%s %d:</b></td><td %s><b>%s</b></td>",
                    class_ul, _T(match), k+1, class_ur, cat ? cat->last : "?");

            free_judoka(cat);
        }

        fprintf(f, "</tr>\r\n<tr>");

        for (i = 0; i < number_of_tatamis; i++) {
            struct match *m = get_cached_next_matches(i+1);

            if (m[k].number >= 1000) {
                fprintf(f, "<td %s>&nbsp;</td><td %s>&nbsp;</td>", class_dl, class_dr);
                continue;
            }

            blue = get_data(m[k].blue);
            white = get_data(m[k].white);

            fprintf(f, "<td %s>%s %s<br>%s<br>&nbsp;</td><td %s>%s %s<br>%s<br>&nbsp;</td>", 
                    class_dl,
                    blue ? utf8_to_html(blue->first) : "?", 
                    blue ? utf8_to_html(blue->last) : "?", 
                    blue ? utf8_to_html(blue->club) : "?", 
                    class_dr,
                    white ? utf8_to_html(white->first) : "?", 
                    white ? utf8_to_html(white->last) : "?", 
                    white ? utf8_to_html(white->club) : "?");
			
            free_judoka(blue);
            free_judoka(white);
        }

        fprintf(f, "</tr>\r\n");
    }
    g_static_mutex_unlock(&next_match_mutex);

    fprintf(f, "</table></td>\n");
    make_bottom_frame(f);
    fclose(f);
}

int get_output_directory(void)
{
    GtkWidget *dialog, *auto_update, *statistics, *vbox;
    gchar *name;

    dialog = gtk_file_chooser_dialog_new(_("Choose a directory"),
                                         NULL,
                                         GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                         GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                         NULL);

    vbox = gtk_vbox_new(FALSE, 1);
    gtk_widget_show(vbox);
    auto_update = gtk_check_button_new_with_label(_("Automatic Web Page Update"));
    gtk_widget_show(auto_update);
    statistics = gtk_check_button_new_with_label(_("Create Statistics"));
    gtk_widget_show(statistics);
    gtk_box_pack_start(GTK_BOX(vbox), statistics, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), auto_update, FALSE, TRUE, 0);

    gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(dialog), vbox);

    if (strlen(current_directory) > 1) {
        gchar *dirname = g_path_get_dirname(current_directory);
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), dirname);
        g_free(dirname);
    } else if (database_name[0] == 0) {
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), g_get_home_dir());
    } else {
        gchar *dirname = g_path_get_dirname(database_name);
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), dirname);
        g_free(dirname);
    }

    if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_ACCEPT) {
        gtk_widget_destroy(dialog);
        return -1;
    }
                
    name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
    strcpy(current_directory, name);
    g_free (name);

    valid_ascii_string(current_directory);

    automatic_web_page_update = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(auto_update));
    create_statistics = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(statistics));

    gtk_widget_destroy(dialog);

    return 0;
}

gchar *txt2hex(const gchar *txt)
{
    static gchar buffers[4][64];
    static gint sel = 0;
    guchar *p = (guchar *)txt;
    buffers[sel][0] = 0;
    gchar *ret;
    gint n = 0;

    while (*p && n < 62)
        n += snprintf(&buffers[sel][n], 64 - n, "%02x", *p++);

    ret = buffers[sel];
    sel++;
    if (sel >= 4)
        sel = 0;
    return ret;
}
