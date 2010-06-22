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

guint french_size[NUM_FRENCH] = {8, 16, 32, 64};
guint estim_num_matches[65] = {
    0, 0, 1, 3, 6, 10, 9, 12,                                         /* 0 - 7 */
    11, 12, 14, 16, 18, 20, 21, 22,                                   /* 8 - 15 */
    23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 34, 36, 38, 40, 41, 42,   /* 16 - 31 */
    43, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59,   /* 32 - 47 */
    60, 62, 63, 65, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78,   /* 48 - 63 */
    79                                                                /* 64 */
};


static gboolean for_each_row(GtkTreeModel *model,
                             GtkTreePath  *path,
                             GtkTreeIter  *iter,
                             GList       **rowref_list)
{
    GtkTreeRowReference  *rowref;
    rowref = gtk_tree_row_reference_new(GTK_TREE_MODEL(current_model), path);
    *rowref_list = g_list_append(*rowref_list, rowref);

    return FALSE;
}

void create_categories(GtkWidget *w, gpointer data)
{
    GList *rr_list = NULL;    /* list of GtkTreeRowReferences to remove */
    GList *node;
#if 0
    if (db_matches_exists()) {
        g_print("Otteluita arvottu. Poista arvonnat ensin.\n");
        show_message("Otteluita arvottu. Poista arvonnat ensin.");
        return;
    }
#endif
    gint num_comp, num_weighted;
    db_read_competitor_statistics(&num_comp, &num_weighted);
    gint cnt = 0;

    gdk_window_set_cursor(GTK_WIDGET(main_window)->window, wait_cursor);

    gtk_tree_model_foreach(GTK_TREE_MODEL(current_model),
                           (GtkTreeModelForeachFunc) for_each_row,
                           &rr_list);

    for ( node = rr_list;  node != NULL;  node = node->next ) {
        GtkTreePath *path;

        path = gtk_tree_row_reference_get_path((GtkTreeRowReference*)node->data);

        if (path) {
            GtkTreeIter iter;

            cnt++;
            if (num_comp > 40 && (cnt % 10) == 0)
                progress_show(1.0*cnt/num_comp, "");

            if (gtk_tree_model_get_iter(GTK_TREE_MODEL(current_model), &iter, path)) {
                struct judoka *j = get_data_by_iter(&iter);
                struct judoka e;
                gint r;

                if (j && j->visible && j->category && j->category[0] == '?'&& j->category[1] == 0) {
                    g_free((void *)j->category);

                    if (j->regcategory == NULL || j->regcategory[0] == 0) {
                        j->category = find_correct_category(current_year - j->birthyear, 
                                                            j->weight, 
                                                            find_gender(j->first), 
                                                            NULL, TRUE);
                    } else {
                        j->category = find_correct_category(0, j->weight, 0, j->regcategory, FALSE);
                    }

                    if (j->category == NULL) {
                        if (j->regcategory)
                            j->category = g_strdup(j->regcategory);
                        else 
                            j->category = g_strdup("?");
                    }

                    if (avl_get_category_status_by_name(j->category) & MATCH_EXISTS) {
                        SHOW_MESSAGE("%s %s %s -> %s.",
                                     _("Cannot move"),
                                     j->first, j->last, j->category);
                        g_free((void *)j->category);
                        j->category = g_strdup("?");
                    }

                    db_update_judoka(j->index, j);

                    r = display_one_judoka(j);
                    if (r >= 0) {
                        /* new category */
                        e.index = r;
                        e.last = j->category;
                        e.belt = 0;
                        e.deleted = 0;
                        e.birthyear = 0;
                        db_add_category(e.index, &e);
                    }
                }
                free_judoka(j);
            }

            /* FIXME/CHECK: Do we need to free the path here? */
        }
    }

    g_list_foreach(rr_list, (GFunc) gtk_tree_row_reference_free, NULL);
    g_list_free(rr_list);        

    gdk_window_set_cursor(GTK_WIDGET(main_window)->window, NULL);

    progress_show(0.0, "");
}

void toggle_three_matches(GtkWidget *menu_item, gpointer data)
{
    three_matches_for_two = GTK_CHECK_MENU_ITEM(menu_item)->active;
    db_save_config();
    if (current_category)
        matches_refresh();
}


void view_popup_menu_draw_category(GtkWidget *menuitem, gpointer userdata)
{
    gint n, num_selected = 0;
    guint index = (guint)userdata;
    GtkTreeIter iter;
    gboolean ok;
    GtkTreeSelection *selection = 
        gtk_tree_view_get_selection(GTK_TREE_VIEW(current_view));

    ok = gtk_tree_model_get_iter_first(current_model, &iter);
    while (ok) {
        if (gtk_tree_selection_iter_is_selected(selection, &iter)) {
            gint index2;

            gtk_tree_model_get(current_model, &iter,
                               COL_INDEX, &index2,
                               -1);
			
            num_selected++;
            n = gtk_tree_model_iter_n_children(current_model, &iter);
            if (n >= 1 && n <= 64)
                draw_one_category(&iter, n);
            update_matches(index2, 0, 0);
        }
        ok = gtk_tree_model_iter_next(current_model, &iter);
    }

    if (num_selected)
        return;
        
    if (find_iter(&iter, index) == FALSE)
        return;

    n = gtk_tree_model_iter_n_children(current_model, &iter);
    if (n >= 1 && n <= 64)
        draw_one_category(&iter, n);

    update_matches(index, 0, 0);
    //matches_refresh();

    SYS_LOG_INFO(_("Category drawn"));
    //refresh_window();
}

void view_popup_menu_draw_category_manually(GtkWidget *menuitem, gpointer userdata)
{
    gint n;
    guint index = (guint)userdata;
    GtkTreeIter iter;
        
    if (find_iter(&iter, index) == FALSE)
        return;

    n = gtk_tree_model_iter_n_children(current_model, &iter);
    if (n >= 2 && n <= 64)
        draw_one_category_manually(&iter, n);
#if 0
    matches_refresh();

    SYS_LOG_INFO("Sarja arvottu");
#endif
}

void locate_to_tatamis(GtkWidget *w, gpointer data)
{
    gint n = (gint)data;
    GtkTreeIter iter;
    gboolean ok;
    gint t = 1, i = 0;
    gint match_cnt[NUM_TATAMIS] = {0};
    gint group_cnt[NUM_TATAMIS] = {0};
    gint group_num[NUM_TATAMIS] = {1};

    gint num_cats = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(current_model), NULL);
    gint cnt = 0;

    for (i = 0; i < NUM_TATAMIS; i++)
        group_num[i] = 1;

    gdk_window_set_cursor(GTK_WIDGET(main_window)->window, wait_cursor);

    ok = gtk_tree_model_get_iter_first(current_model, &iter);
    while (ok) {
        struct judoka *j;
        gint index;

        cnt++;
        if (num_cats > 4)
            progress_show(1.0*cnt/num_cats, "");

        gtk_tree_model_get(current_model, &iter,
                           COL_INDEX, &index,
                           -1);
		
        gint num_competitors = gtk_tree_model_iter_n_children(current_model, &iter);
        gint num_matches = estim_num_matches[num_competitors <= 64 ? num_competitors : 64];

        struct category_data *catdata = avl_get_category(index);
        if (catdata && catdata->match_count)
            num_matches = catdata->match_count - catdata->matched_matches_count;

        j = get_data(index);
        if (j == NULL)
            goto cont;

        if (j->belt == 0) {
            gint mincnt = 10000;
            t = 1;
            for (i = 0; i < n; i++) {
                if (match_cnt[i] < mincnt) {
                    t = i+1;
                    mincnt = match_cnt[i];
                }
            }
			
            j->belt = t;
            j->birthyear = group_num[t-1];

        } else 
            t = j->belt;

        match_cnt[t-1] += num_matches;

        if (++group_cnt[t-1] >= 3) {
            group_cnt[t-1] = 0;
            group_num[t-1]++;
        }

        db_update_category(j->index, j);
        display_one_judoka(j);
        free_judoka(j);

    cont:
        ok = gtk_tree_model_iter_next(current_model, &iter);
    }

    matches_refresh();

    progress_show(0.0, "");

    gdk_window_set_cursor(GTK_WIDGET(main_window)->window, NULL);

    SYS_LOG_INFO(_("All the categories located to the tatamis"));
}

void update_category_status_info(gint category)
{
    db_category_match_status(category);
#if 0
    GtkTreeIter iter;
    gint weight;

    weight = db_category_match_status(category);
    if (find_iter(&iter, category) == FALSE) 
        return;


    gtk_tree_store_set((GtkTreeStore *)current_model, 
                       &iter,
                       COL_WEIGHT, weight,
                       -1);
    if (!categories_tree)
        return;

    struct category_data data, *data1;
    data.index = category;
    if (avl_get_by_key(categories_tree, &data, (void *)&data1) == 0) {
        data1->match_status = weight;
    } /***else
          g_print("Error %s %d\n", __FUNCTION__, __LINE__);***/
#endif
}

void update_category_status_info_all(void)
{
    GtkTreeIter iter;
    gboolean ok;
    gint weight, index, visible;

    ok = gtk_tree_model_get_iter_first(current_model, &iter);
    while (ok) {
        gtk_tree_model_get(current_model, &iter,
                           COL_INDEX, &index,
                           COL_VISIBLE, &visible,
                           -1);

        if (visible == 0) {
            weight = db_category_match_status(index);
#if 0			
            gtk_tree_store_set(GTK_TREE_STORE(current_model), 
                               &iter,
                               COL_WEIGHT, weight,
                               -1);
#endif
            struct category_data data, *data1;
            data.index = index;
            if (avl_get_by_key(categories_tree, &data, (void *)&data1) == 0) {
                data1->match_status = weight;
            } else
                g_print("Error %s %d (%d)\n", __FUNCTION__, __LINE__, index);

            ok = gtk_tree_model_iter_next(current_model, &iter);
        } else
            g_print("ERROR: %s:%d\n", __FUNCTION__, __LINE__);

    }

    refresh_window();
}

gint get_cat_system(gint index)
{
    struct category_data *catdata = avl_get_category(index);
    if (catdata)
	return catdata->system;

    return db_get_system(index);
}
