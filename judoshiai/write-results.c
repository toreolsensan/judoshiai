/* -*- mode: C; c-basic-offset: 4;  -*- */

/*
 * Copyright (C) 2006-2010 by Hannu Jokinen
 * Full copyright text is included in the software package.
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>

#include "sqlite3.h"
#include "judoshiai.h"

void write_results(FILE *f);

static gboolean create_statistics = FALSE;

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
                      const gchar *club, const gchar *category, const gint index)
{
    if (create_statistics)
        fprintf(f, 
                "<tr><td><a href=\"%d.html\">%s, %s</a></td><td>%s</td><td>%s</td>"
                "<td><a href=\"%s.html\">%s</a></td></tr>\n", 
                index, utf8_to_html(last), utf8_to_html(first), belt, 
                utf8_to_html(club), category, category);
    else
        fprintf(f, 
                "<tr><td>%s, %s</td><td>%s</td><td>%s</td>"
                "<td><a href=\"%s.html\">%s</a></td></tr>\n", 
                utf8_to_html(last), utf8_to_html(first), belt, utf8_to_html(club), 
                category, category);

    saved_competitors[saved_competitor_cnt] = index;
    if (saved_competitor_cnt < SAVED_COMP_SIZE)
        saved_competitor_cnt++;
}

static void make_top_frame(FILE *f)
{
    fprintf(f, "<html><head>"
            "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">"
            "<link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\">"
            "<title>%s  %s  %s</title></head>\n"
            "<body class=\"titleframe\"><table><tr>"
            "<td colspan=\"2\" align=\"center\"><h1>%s  %s  %s</h1></td></tr><tr>\n", 
            utf8_to_html(info_competition), utf8_to_html(info_date), utf8_to_html(info_place),
            utf8_to_html(info_competition), utf8_to_html(info_date), utf8_to_html(info_place));
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
                                
        if (n >= 1 && n <= 64) {
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
                            j->last, utf8_to_html(j->last), 
                            j->last);
                } else {
                    fprintf(f, "<tr><td class=\"categorylinksonly\"><a href=\"%s.html\">%s</a></td></tr>", 
                            j->last, utf8_to_html(j->last));
                }

                free_judoka(j);
            }
        }

        ok = gtk_tree_model_iter_next(current_model, &iter);
    }

    fprintf(f, "</table></td>\n");
    return num_cats;
}


static void pool_results(FILE *f, gint category, struct judoka *ctg, gint num_judokas)
{
    struct pool_matches pm;
    gint i;

    fill_pool_struct(category, num_judokas, &pm);

    if (pm.finished)
        get_pool_winner(num_judokas, pm.c, pm.yes, pm.wins, pm.pts, pm.mw, pm.j, pm.all_matched);

    for (i = 1; i <= num_judokas; i++) {
        if (pm.finished == FALSE || pm.j[pm.c[i]] == NULL)
            continue;

        if (i <= 3 && 
            (pm.j[pm.c[i]]->deleted & HANSOKUMAKE) == 0) {
            write_result(f, i, pm.j[pm.c[i]]->first, 
                         pm.j[pm.c[i]]->last, pm.j[pm.c[i]]->club, pm.j[pm.c[i]]->country);
        }
    }

    /* clean up */
    empty_pool_struct(&pm);
}

static void dpool_results(FILE *f, gint category, struct judoka *ctg, gint num_judokas)
{
    struct pool_matches pm;
    struct judoka *j1 = NULL;
    gint i;
    int gold, silver, bronze1, bronze2;

    fill_pool_struct(category, num_judokas, &pm);

    i = num_judokas <= 6 ? 7 : 10;

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
        free_judoka(j1);
    }
    if (gold && (j1 = get_data(silver))) {
        write_result(f, 2, j1->first, j1->last, j1->club, j1->country);
        free_judoka(j1);
    }
    if (gold && (j1 = get_data(bronze1))) {
        write_result(f, 3, j1->first, j1->last, j1->club, j1->country);
        free_judoka(j1);
    }
    if (gold && (j1 = get_data(bronze2))) {
        write_result(f, 3, j1->first, j1->last, j1->club, j1->country);
        free_judoka(j1);
    }

    /* clean up */
    empty_pool_struct(&pm);
}

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
                           gint num_judokas, gint systm, gint pagenum)
{
    struct match m[NUM_MATCHES];
    struct judoka *j1;
    gint gold = 0, silver = 0, bronze1 = 0, bronze2 = 0, fifth1 = 0, fifth2 = 0;
    gint sys = ((systm & SYSTEM_MASK) - SYSTEM_FRENCH_8)>>16;
    gint table = (systm & SYSTEM_TABLE_MASK) >> SYSTEM_TABLE_SHIFT;

    memset(m, 0, sizeof(m));
    db_read_category_matches(category, m);

    GET_GOLD(medal_matches[table][sys][2]);
    GET_BRONZE1(medal_matches[table][sys][0]);
    GET_BRONZE2(medal_matches[table][sys][1]);

    if (gold && (j1 = get_data(gold))) {
        write_result(f, 1, j1->first, j1->last, j1->club, j1->country);
        free_judoka(j1);
    }
    if (gold && (j1 = get_data(silver))) {
        write_result(f, 2, j1->first, j1->last, j1->club, j1->country);
        free_judoka(j1);
    }
    if (gold && (j1 = get_data(bronze1))) {
        write_result(f, 3, j1->first, j1->last, j1->club, j1->country);
        free_judoka(j1);
    }
    if (gold && (j1 = get_data(bronze2))) {
        write_result(f, 3, j1->first, j1->last, j1->club, j1->country);
        free_judoka(j1);
    }
    if (gold && (j1 = get_data(fifth1))) {
        write_result(f, 5, j1->first, j1->last, j1->club, j1->country);
        free_judoka(j1);
    }
    if (gold && (j1 = get_data(fifth2))) {
        write_result(f, 5, j1->first, j1->last, j1->club, j1->country);
        free_judoka(j1);
    }
}

static void write_cat_result(FILE *f, gint category)
{
    gint sys, num_judokas;
    struct judoka *ctg = NULL;
	
    ctg = get_data(category);

    if (ctg == NULL)
        return;

    fprintf(f, "<tr><td colspan=\"3\"><b><a href=\"%s.html\">%s</a></b></td></tr>\n", 
            ctg->last, utf8_to_html(ctg->last));

    /* find system */
    sys = db_get_system(category);
    num_judokas = (sys&COMPETITORS_MASK);

    switch (sys & SYSTEM_MASK) {
    case SYSTEM_POOL:
        pool_results(f, category, ctg, num_judokas);
        break;

    case SYSTEM_DPOOL:
        dpool_results(f, category, ctg, num_judokas);
        break;

    case SYSTEM_FRENCH_8:
    case SYSTEM_FRENCH_16:
    case SYSTEM_FRENCH_32:
    case SYSTEM_FRENCH_64:
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

        if (n >= 1 && n <= 64) {
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
    struct judoka *j = get_data(cat);
    if (!j)
        return;

    progress_show(-1.0, j->last);

    gint sys = db_get_system(cat);
        
    snprintf(buf, sizeof(buf), "%s.html", j->last);

    FILE *f = open_write(buf);
    if (!f)
        goto out;

    make_top_frame(f);
    make_left_frame(f);

    if ((sys & SYSTEM_MASK) == SYSTEM_FRENCH_64) {
        fprintf(f,
                "<td valign=\"top\"><img src=\"%s.png\"><br>"
                "<img src=\"%s-1.png\"><br>"
                "<img src=\"%s-2.png\">"
                "</td>\n",
                j->last, j->last, j->last);
    } else {
        fprintf(f,
                "<td valign=\"top\"><img src=\"%s.png\"></td>\n",
                j->last);
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

        gint blue_score = atoi(db_get_data(k, "blue_score"));
        gint white_score = atoi(db_get_data(k, "white_score"));
        gint blue_points = atoi(db_get_data(k, "blue_points"));
        gint white_points = atoi(db_get_data(k, "white_points"));
        gint time = atoi(db_get_data(k, "time"));

        if (blue_points == 0 && white_points == 0)
            continue;

        category_definitions[i].stat.total++;
        category_definitions[i].stat.time += time;

        if ((blue_score & 0xf0000) || (white_score & 0xf0000))
            category_definitions[i].stat.ippons++;
        else if ((blue_score & 0xf000) || (white_score & 0xf000))
            category_definitions[i].stat.wazaaris++;
        else if ((blue_score & 0xf00) || (white_score & 0xf00))
            category_definitions[i].stat.yukos++;
        else if ((blue_score & 0xf0) || (white_score & 0xf0))
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
                    "<tr><td>%s</td><td>%s %s</td><td class=\"bscore\">%05x</td>"
                    "<td align=\"center\">%d - %d</td>"
                    "<td class=\"wscore\">%05x</td><td>%s %s</td><td>%d:%02d</td></tr>\n",
                    utf8_to_html(c->last), utf8_to_html(j1->first), utf8_to_html(j1->last),
                    blue_score, blue_points,
                    white_points, white_score, 
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

        if (n >= 1 && n <= 64) {
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

    /* sheets */
    write_htmls(num_cats);

    /* statistics */
    if (create_statistics == FALSE)
        return;

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

void make_next_matches_html(void)
{
    FILE *f;
    gint i;

    if (strlen(current_directory) <= 1) {
        if (get_output_directory() < 0)
            return;
    }

    f = open_write("nextmatches.html");
    if (!f)
        return;

    make_top_frame(f);
    make_left_frame(f);

    fprintf(f, "<td><table class=\"nextmatches\" valign=\"top\"><tr>");
    for (i = 0; i < number_of_tatamis; i++) {
        fprintf(f, "<th>Tatami %d</th>\n", i+1);
    }
    fprintf(f, "</tr><tr>\n");

    for (i = 0; i < number_of_tatamis; i++) {
        gchar *xf, *xl, *xc;
        gint k;

        fprintf(f, "<td valign=\"top\"><table border=\"0\">");

        if (next_matches_info[i][0].won_catnum) {
            xf = next_matches_info[i][0].won_first;
            xl = next_matches_info[i][0].won_last;
            xc = next_matches_info[i][0].won_cat;
        } else {
            xf = "";
            xl = "";
            xc = "";
        }

        fprintf(f, "<tr><td>Edellinen voittaja:<br>");
        fprintf(f, "</td><td>%s<br>%s %s<br>&nbsp;</td>", 
                utf8_to_html(xc), utf8_to_html(xf), utf8_to_html(xl));

        g_static_mutex_lock(&next_match_mutex);
        struct match *m = get_cached_next_matches(i+1);

        for (k = 0; m[k].number != 1000 && k < NEXT_MATCH_NUM; k++) {
            gchar *bgcolor = (k & 1) ? "bgcolor=\"#a0a0a0\"" : "bgcolor=\"#f0f0f0\"";
            struct judoka *blue, *white, *cat;
            blue = get_data(m[k].blue);
            white = get_data(m[k].white);
            cat = get_data(m[k].category);

            fprintf(f, "<tr %s><td><b>Ottelu %d:</b></td><td><b>%s</b></td></tr>", 
                    bgcolor, k+1, cat ? cat->last : "?");

            fprintf(f, "<tr %s><td>%s %s<br>%s<br>&nbsp;</td><td>%s %s<br>%s<br>&nbsp;</td></tr>", 
                    bgcolor, 
                    blue ? utf8_to_html(blue->first) : "?", 
                    blue ? utf8_to_html(blue->last) : "?", 
                    blue ? utf8_to_html(blue->club) : "?", 
                    white ? utf8_to_html(white->first) : "?", 
                    white ? utf8_to_html(white->last) : "?", 
                    white ? utf8_to_html(white->club) : "?");
			
            free_judoka(blue);
            free_judoka(white);
            free_judoka(cat);
        }
        g_static_mutex_unlock(&next_match_mutex);
        fprintf(f, "</table></td>\r\n");
    }

    fprintf(f, "</tr></table></td>\n");
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

