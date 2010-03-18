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


#define NUM_MOVABLE 1000
//static guint destination;
static guint which[NUM_MOVABLE];
static guint num_movable;
static gchar *dest_category = NULL;
static gint   dest_category_ix = 0;

static gboolean for_each_row_selected(GtkTreeModel *model,
                                      GtkTreePath *path,
                                      GtkTreeIter *iter,
                                      gpointer data)
{
    GtkTreeSelection *selection = 
        gtk_tree_view_get_selection(GTK_TREE_VIEW(current_view));

    if (gtk_tree_selection_iter_is_selected(selection, iter)) {
        guint index;

        gtk_tree_model_get(model, iter,
                           COL_INDEX, &index,
                           -1);

        if (num_movable < NUM_MOVABLE - 1) {
            which[num_movable++] = index;
        }
    }
        
    return FALSE;
}

static void view_popup_menu_move_judoka(GtkWidget *menuitem, gpointer userdata)
{
    int i;

    GtkTreeView *treeview = GTK_TREE_VIEW(current_view);
    GtkTreeModel *model = current_model;
    GtkTreeSelection *selection = 
        gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));

    if (!dest_category)
        return;

    if (db_category_match_status(dest_category_ix) & MATCH_EXISTS) {
        SHOW_MESSAGE("Sarja %s arvottu. Poista arvonta ensin.", dest_category);
        return;
    }

    //destination = (guint)userdata;
    num_movable = 0;
    gtk_tree_model_foreach(model, for_each_row_selected, userdata);
    gtk_tree_selection_unselect_all(selection);

    for (i = 0; i < num_movable; i++) {
        struct judoka *j = get_data(which[i]);
        if (!j)
            continue;

        if (db_competitor_match_status(j->index) & MATCH_EXISTS) {
            SHOW_MESSAGE("%s %s: %s.", 
                         j->first, j->last, _("Remove drawing first"));
        } else {
            if (j->category)
                g_free((void *)j->category);
            j->category = strdup(dest_category);
            display_one_judoka(j);
            db_update_judoka(j->index, j);
        }
        free_judoka(j);
    }
}

static void view_popup_menu_copy_judoka(GtkWidget *menuitem, gpointer userdata)
{
    int i;

    GtkTreeView *treeview = GTK_TREE_VIEW(current_view);
    GtkTreeModel *model = current_model;
    GtkTreeSelection *selection = 
        gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));

    num_movable = 0;
    gtk_tree_model_foreach(model, for_each_row_selected, userdata);
    gtk_tree_selection_unselect_all(selection);

    for (i = 0; i < num_movable; i++) {
        struct judoka *j = get_data(which[i]);
        if (!j)
            continue;

        if (j->category)
            g_free((void *)j->category);

        j->index = current_index++;
        j->category = g_strdup("?");
        db_add_judoka(j->index, j);
        display_one_judoka(j);

        free_judoka(j);
    }
}

static void view_popup_menu_change_category(GtkWidget *menuitem, gpointer userdata)
{
    struct judoka *j = get_data((gint)userdata);
    if (!j)
        return;

    if (dest_category)
        g_free(dest_category);

    dest_category = strdup(j->last);
    dest_category_ix = j->index;
    free_judoka(j);

    view_popup_menu_move_judoka(NULL, userdata);
}

static void view_popup_menu_remove_draw(GtkWidget *menuitem, gpointer userdata)
{
    if (db_category_match_status((gint)userdata) & MATCH_MATCHED) {
        SHOW_MESSAGE(_("Matches matched. Clear the results first."));
        return;
    }

    db_set_system((gint)userdata, get_cat_system((gint)userdata)&SYSTEM_WISH_MASK);
    db_remove_matches((gint)userdata);
    update_category_status_info((gint)userdata);
    matches_refresh();
    refresh_window();
}

static void change_display(GtkWidget *menuitem, gpointer userdata)
{
    if (userdata)
        gtk_tree_view_collapse_all(GTK_TREE_VIEW(current_view));
    else
        gtk_tree_view_expand_all(GTK_TREE_VIEW(current_view));
}

#if 0
static void do_nothing(GtkWidget *menuitem, gpointer userdata)
{

}
#endif

static void create_new_category(GtkWidget *menuitem, gpointer userdata)
{
    int i;
    gint age = 0, weight = 0;
    gboolean male = FALSE, female = FALSE;
    gchar cbuf[20];

    GtkTreeView *treeview = GTK_TREE_VIEW(current_view);
    GtkTreeModel *model = current_model;
    GtkTreeSelection *selection = 
        gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));

    num_movable = 0;
    gtk_tree_model_foreach(model, for_each_row_selected, userdata);
    gtk_tree_selection_unselect_all(selection);

    for (i = 0; i < num_movable; i++) {
        struct judoka *j = get_data(which[i]);
        if (!j)
            continue;

        if (j->birthyear) {
            if ((current_year - j->birthyear) > age)
                age = current_year - j->birthyear;
        }

        if (j->regcategory) {
            const gchar *p;
            gint age1 = 0;
            for (p = j->regcategory; *p; p++) {
                switch (*p) {
                case 'E': if (age1 < 10) age1 = 10; break;
                case 'D': if (age1 < 12) age1 = 12; break;
                case 'C': if (age1 < 14) age1 = 14; break;
                case 'B': if (age1 < 16) age1 = 16; break;
                case 'A': if (age1 < 18) age1 = 18; break;
                case 'M': if (age1 < 20) age1 = 20; male = TRUE; break;
                case 'N': if (age1 < 20) age1 = 20; female = TRUE; break;
                case 'T': female = TRUE; break;
                case 'P': male = TRUE; break;
                }
            }
            if (j->birthyear == 0 && age1 > age)
                age = age1;
        }

        if (j->weight > weight)
            weight = j->weight;

        free_judoka(j);
    }

    gint k = 0;
    if (age > 18 && male)
        k += sprintf(cbuf+k, "M");
    if (age > 18 && female)
        k += sprintf(cbuf+k, "N");
    if (age >= 0 && age <= 18) {
        k += sprintf(cbuf+k, "%c", 
                     "xxxxxxxxxEEDDCCBBAA"[age]);
        if (male)
            k += sprintf(cbuf+k, "P"); 
        if (female)
            k += sprintf(cbuf+k, "T");
    }
    k += sprintf(cbuf+k, "-%d,%d", 
                 weight/1000, 
                 (weight%1000)/100);


    GtkTreeIter tmp_iter;
    if (find_iter_category(&tmp_iter, cbuf)) {
        SHOW_MESSAGE("%s %s!", cbuf, _("already exists"));
        return;
    }
	

    for (i = 0; i < num_movable; i++) {
        struct judoka *j = get_data(which[i]);
        
        if (!j)
            continue;

        if (db_competitor_match_status(j->index) & MATCH_EXISTS) {
            SHOW_MESSAGE("%s %s: %s.", 
                         j->first, j->last, _("Remove drawing first"));
        } else {
            if (j->category)
                g_free((void *)j->category);
            j->category = strdup(cbuf);
            gint ret = display_one_judoka(j);
            if (ret >= 0) {
                /* new category */
                struct judoka e;
                e.index = ret;
                e.last = cbuf;
                e.belt = 0;
                e.deleted = 0;
                e.birthyear = 0;
                db_add_category(e.index, &e);
            }

            db_update_judoka(j->index, j);
        }
        free_judoka(j);
    }
}

static void view_popup_menu_draw_and_print_category(GtkWidget *menuitem, gpointer userdata)
{
    view_popup_menu_draw_category(menuitem, userdata);
    print_doc(menuitem, (gpointer)((gint)userdata | PRINT_SHEET | PRINT_TO_PRINTER));
}

static void show_category_window(GtkWidget *menuitem, gpointer userdata)
{
    category_window((gint)userdata);
}

void view_popup_menu(GtkWidget *treeview, 
                     GdkEventButton *event, 
                     gpointer userdata,
                     gchar *regcategory,
                     gboolean visible)
{
    GtkWidget *menu, *menuitem;
    //gboolean matches_exist = db_match_exists((gint)userdata, 1, 0);
    gboolean matched = db_matched_matches_exist((gint)userdata);

    if (dest_category)
        g_free(dest_category);

    dest_category = strdup(regcategory);
    dest_category_ix = (gint)userdata;
    menu = gtk_menu_new();

    menuitem = gtk_menu_item_new_with_label(_("Expand All"));
    g_signal_connect(menuitem, "activate",
                     (GCallback) change_display, (gpointer)0);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	
    menuitem = gtk_menu_item_new_with_label(_("Collapse All"));
    g_signal_connect(menuitem, "activate",
                     (GCallback) change_display, (gpointer)1);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
	
    if (matched == FALSE) {
        GtkWidget *submenu;
        GtkTreeIter iter;
        gboolean ok;

        menuitem = gtk_menu_item_new_with_label(_("Move Competitors to Category..."));
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

        submenu = gtk_menu_new();
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem), submenu);

        ok = gtk_tree_model_get_iter_first(current_model, &iter);
        while (ok) {
            gint index;
            gchar *cat = NULL;
            gtk_tree_model_get(current_model, &iter,
                               COL_INDEX, &index,
                               COL_LAST_NAME, &cat,
                               -1);
            menuitem = gtk_menu_item_new_with_label(cat);
            g_signal_connect(menuitem, "activate",
                             (GCallback) view_popup_menu_change_category, 
                             (gpointer)index);
            gtk_menu_shell_append(GTK_MENU_SHELL(submenu), menuitem);
            g_free(cat);

            ok = gtk_tree_model_iter_next(current_model, &iter);
        }

        menuitem = gtk_menu_item_new_with_label(_("Move Competitors Here"));
        g_signal_connect(menuitem, "activate",
                         (GCallback) view_popup_menu_move_judoka, userdata);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

        menuitem = gtk_menu_item_new_with_label(_("Compose Unoffial Category"));
        g_signal_connect(menuitem, "activate",
                         (GCallback) create_new_category, userdata);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

        gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
        menuitem = gtk_menu_item_new_with_label(_("Draw Selected"));
        g_signal_connect(menuitem, "activate",
                         (GCallback) view_popup_menu_draw_category, userdata);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

        menuitem = gtk_menu_item_new_with_label(_("Draw and Print Selected"));
        g_signal_connect(menuitem, "activate",
                         (GCallback) view_popup_menu_draw_and_print_category, userdata);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

        menuitem = gtk_menu_item_new_with_label(_("Draw Manually"));
        g_signal_connect(menuitem, "activate",
                         (GCallback) view_popup_menu_draw_category_manually, userdata);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

        menuitem = gtk_menu_item_new_with_label(_("Remove Drawing"));
        g_signal_connect(menuitem, "activate",
                         (GCallback) view_popup_menu_remove_draw, userdata);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

        gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());

        menuitem = gtk_menu_item_new_with_label(_("Copy Competitors"));
        g_signal_connect(menuitem, "activate",
                         (GCallback) view_popup_menu_copy_judoka, userdata);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

        menuitem = gtk_menu_item_new_with_label(_("Remove Competitors"));
        g_signal_connect(menuitem, "activate",
                         (GCallback) remove_competitors, userdata);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

        gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
    }

    menuitem = gtk_menu_item_new_with_label(_("Show Sheet"));
    g_signal_connect(menuitem, "activate",
                     (GCallback) show_category_window, userdata);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

    menuitem = gtk_menu_item_new_with_label(_("Print Selected Sheets (Printer)"));
    g_signal_connect(menuitem, "activate",
                     (GCallback) print_doc, (gpointer)((gint)userdata | PRINT_SHEET | PRINT_TO_PRINTER));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

    menuitem = gtk_menu_item_new_with_label(_("Print Selected Sheets (PDF)"));
    g_signal_connect(menuitem, "activate",
                     (GCallback) print_doc, (gpointer)((gint)userdata | PRINT_SHEET | PRINT_TO_PDF));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

    gtk_widget_show_all(menu);

    /* Note: event can be NULL here when called from view_onPopupMenu;
     *  gdk_event_get_time() accepts a NULL argument */
    gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
                   (event != NULL) ? event->button : 0,
                   gdk_event_get_time((GdkEvent*)event));
}

