/* -*- mode: C; c-basic-offset: 4;  -*- */

/*
 * Copyright (C) 2006-2016 by Hannu Jokinen
 * Full copyright text is included in the software package.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "judoshiai.h"

extern gboolean mirror_display;

static gboolean mouse_click(GtkWidget *sheet_page,
			    GdkEventButton *event,
			    gpointer userdata);
static gboolean motion_notify(GtkWidget *sheet_page,
                              GdkEventMotion *event,
			      gpointer userdata);
static gboolean release_notify(GtkWidget *sheet_page,
			       GdkEventButton *event,
			       gpointer userdata);
static gboolean cannot_move_category(gint cat);
static cairo_surface_t *get_image(const gchar *name);


#define THIN_LINE     (paper_width < 700.0 ? 1.0 : paper_width/700.0)
#define THICK_LINE    (2*THIN_LINE)

#define W(_w) ((_w)*paper_width)
#define H(_h) ((_h)*paper_height)

#define BOX_HEIGHT (1.4*extents.height > n*mt/30.0 ? 1.4*extents.height : n*mt/30.0)

#define NUM_RECTANGLES 1000

static struct {
    double x1, y1, x2, y2;
    gint   index;
    gint   tatami;
    gint   orig_tatami;
    gint   group;
} point_click_areas[NUM_RECTANGLES];

static gint num_rectangles;
static GdkCursor *hand_cursor = NULL;

static gboolean button_drag = FALSE;
static gint     dragged_cat = 0;
static gchar    dragged_text[32];
static gint     start_box = 0;
static gdouble  dragged_x, dragged_y;

#define NO_SCROLL   0
#define SCROLL_DOWN 1
#define SCROLL_UP   2
static gint     scroll_up_down = 0;
static cairo_surface_t *anchor = NULL;
static GtkWidget *cat_graph_label = NULL;

static struct win_collection {
    GtkWidget *scrolled_window;
    GtkWidget *darea;
} wincoll;

static void refresh_darea(void)
{
    gtk_widget_queue_draw(wincoll.darea);
    //gtk_widget_queue_draw_area(w.darea, 0, 0, 600, 2000);
}
#define refresh_window refresh_darea

void draw_gategory_graph(void)
{
    refresh_darea();
}

#define set_left_right(_t) do {						\
    left = (mirror_display && _t) ?					\
	(number_of_tatamis - _t + 1)*colwidth :				\
	(_t)*colwidth;						\
    right = (mirror_display && _t) ?					\
	(number_of_tatamis - _t + 2)*colwidth :				\
	(_t+1)*colwidth;						\
    } while (0)


static void paint(cairo_t *c, gdouble paper_width, gdouble paper_height, gpointer userdata)
{
    gint i, matches_left[NUM_TATAMIS+1], matches_time[NUM_TATAMIS+1], tatami;
    cairo_text_extents_t extents;
    gdouble       y_pos[NUM_TATAMIS+1], colwidth = W(1.0/(number_of_tatamis + 1));
    gint          max_group[NUM_TATAMIS+1];
    struct tm    *tm;
    time_t        secs;
    cairo_surface_t *cs = userdata;
    struct category_data *catdatas[NUM_TATAMIS+1];
    gint old_group = 1000000;
    gdouble left, right;
    gchar buf[32];

    if (!anchor)
        anchor = get_image("anchor.png");

    cairo_set_font_size(c, 12);
    cairo_text_extents(c, "Hj:000", &extents);

    if (cs) {
        cairo_set_source_surface(c, cs, 0, 0);
        cairo_paint(c);
        goto drag;
    }

    num_rectangles = 0;

    cairo_set_source_rgb(c, 1.0, 1.0, 1.0);
    cairo_rectangle(c, 0.0, 0.0, paper_width, paper_height);
    cairo_fill(c);

    cairo_set_line_width(c, THIN_LINE);
    cairo_set_source_rgb(c, 0.0, 0.0, 0.0);

    for (i = 0; i <= number_of_tatamis; i++) {
	catdatas[i] = category_queue[i].next;
	y_pos[i] = extents.height*1.4;
        matches_left[i] = 0;
        matches_time[i] = 0;
        max_group[i] = 0;
    }

    while (1) {
	gint grp_found = 0;
	old_group = 1000000;

	/* find min group */
	for (i = 0; i <= number_of_tatamis; i++) {
	    if (catdatas[i] && catdatas[i]->group < old_group)
		old_group = catdatas[i]->group;
	}
	for (tatami = 0; tatami <= number_of_tatamis; tatami++) {
	    struct category_data *catdata = catdatas[tatami];
	    gboolean group_on_tatami = FALSE;
	    while (catdata && catdata->group == old_group) {
		gint n = 1;
		gdouble x;
		gint mul = 1;
		gint mt = get_category_match_time(catdata->category);
		if (mt < 180) mt = 180;
		grp_found++;
		group_on_tatami = TRUE;
		if (old_group > max_group[tatami])
		    max_group[tatami] = old_group;

		cairo_set_line_width(c, THIN_LINE);
		cairo_set_source_rgb(c, 0.0, 0.0, 0.0);

		set_left_right(tatami);

		x = n = 0;
		if (catdata->by_tatami[0].match_count > 0)
		    x = catdata->by_tatami[0].matched_matches_count*colwidth /
			catdata->by_tatami[0].match_count;

		/* find total number of matches, even w/o draw */
		if (catdata->match_status & REAL_MATCH_EXISTS) {
		    if ((catdata->match_status & MATCH_UNMATCHED))
			n = catdata->by_tatami[0].match_count -
			    catdata->by_tatami[0].matched_matches_count;
		} else {
		    GtkTreeIter tmp_iter;
		    if (find_iter(&tmp_iter, catdata->index)) {
			gint k = gtk_tree_model_iter_n_children(current_model, &tmp_iter);
			n = num_matches_left(catdata->index, k);
		    }
		}

		if (n == 0)
		    goto loop;

		cairo_save(c);
		cairo_set_source_rgb(c, 0.0, 1.0, 0.0);
		cairo_rectangle(c, left, y_pos[tatami], x, BOX_HEIGHT);
		cairo_fill(c);

		if (catdata->match_count == 0)
		    cairo_set_source_rgb(c, 1.0, 1.0, 1.0);
		else if (catdata->matched_matches_count == 0)
		    cairo_set_source_rgb(c, 1.0, 1.0, 0.0);
		else
		    cairo_set_source_rgb(c, 1.0, 0.65, 0.0);

		cairo_rectangle(c, left+x, y_pos[tatami], colwidth - x, BOX_HEIGHT);
		cairo_fill(c);

		if (cannot_move_category(catdata->index)) {
		    cairo_set_source_surface(c, anchor, left + colwidth/2, y_pos[tatami]);
		    cairo_paint(c);
		}

		cairo_restore(c);

		/* category name and time */
		if (catdata->deleted & TEAM_EVENT) {
		    mul = find_num_weight_classes(catdata->category);
		    if (mul == 0) mul = 1;
		} else
		    mul = 1;

		matches_left[tatami] += n*mul;//catdata->match_count - catdata->matched_matches_count;
		matches_time[tatami] += n*mt*mul;

		cairo_move_to(c, left+5, y_pos[tatami]+extents.height);
		cairo_show_text(c, catdata->category);
#if 0
		struct judoka *j = get_data(catdata->index);
		if (j) {

		    cairo_move_to(c, left+5, y_pos[tatami]+extents.height);
		    cairo_show_text(c, j->last);
		    free_judoka(j);
		}
#endif

		/* point click areas */
		point_click_areas[num_rectangles].index = catdata->index;
		point_click_areas[num_rectangles].group = catdata->group;
		point_click_areas[num_rectangles].tatami = tatami;
		point_click_areas[num_rectangles].orig_tatami = 0;
		point_click_areas[num_rectangles].x1 = left;
		point_click_areas[num_rectangles].y1 = y_pos[tatami];
		point_click_areas[num_rectangles].x2 = right;
		y_pos[tatami] += BOX_HEIGHT;
		point_click_areas[num_rectangles].y2 = y_pos[tatami];
		if (num_rectangles < NUM_RECTANGLES-1)
		    num_rectangles++;

		cairo_move_to(c, left, y_pos[tatami]);
		cairo_line_to(c, right, y_pos[tatami]);
		cairo_stroke(c);

	    loop:
		/* Matches moved to other tatamis. */
		for (i = 1; i <= number_of_tatamis; i++) {
		    n = catdata->by_tatami[i].match_count -
			catdata->by_tatami[i].matched_matches_count;
		    if (n <= 0)
			continue;

		    set_left_right(i);
		    matches_left[i] += n*mul;
		    matches_time[i] += n*mt*mul;
		    x = catdata->by_tatami[i].matched_matches_count*colwidth /
			catdata->by_tatami[i].match_count;

		    cairo_save(c);
		    cairo_set_source_rgb(c, 0.5, 1.0, 0.5);
		    cairo_rectangle(c, left, y_pos[i], x, BOX_HEIGHT);
		    cairo_fill(c);

		    if (catdata->by_tatami[i].matched_matches_count == 0)
			cairo_set_source_rgb(c, 1.0, 1.0, 0.5);
		    else
			cairo_set_source_rgb(c, 1.0, 0.75, 0.5);

		    cairo_rectangle(c, left+x, y_pos[i], colwidth - x, BOX_HEIGHT);
		    cairo_fill(c);

		    cairo_restore(c);

		    snprintf(buf, sizeof(buf), "%s (T%d)", catdata->category, tatami);
		    cairo_move_to(c, left+5, y_pos[i]+extents.height);
		    cairo_show_text(c, buf);

		    /* point click areas */
		    point_click_areas[num_rectangles].index = catdata->index;
		    point_click_areas[num_rectangles].group = catdata->group;
		    point_click_areas[num_rectangles].tatami = i;
		    point_click_areas[num_rectangles].orig_tatami = tatami;
		    point_click_areas[num_rectangles].x1 = left;
		    point_click_areas[num_rectangles].y1 = y_pos[i];
		    point_click_areas[num_rectangles].x2 = right;
		    y_pos[i] += BOX_HEIGHT;
		    point_click_areas[num_rectangles].y2 = y_pos[i];
		    if (num_rectangles < NUM_RECTANGLES-1)
			num_rectangles++;

		    cairo_move_to(c, left, y_pos[i]);
		    cairo_line_to(c, right, y_pos[i]);
		    cairo_stroke(c);
		} /* for i */

		catdatas[tatami] = catdata = catdata->next;
	    } /* while group */

	    set_left_right(tatami);

	    /* draw horizontal line after group */
	    if (group_on_tatami) {
		cairo_save(c);

		cairo_set_line_width(c, THICK_LINE);
		cairo_set_source_rgb(c, 0.0, 0.0, 1.0);
		cairo_move_to(c, left, y_pos[tatami]);
		cairo_line_to(c, right, y_pos[tatami]);
		cairo_stroke(c);

		if (matches_left[tatami]) {
		    /* write time */
		    cairo_rectangle(c, right - extents.width,
				    y_pos[tatami] - extents.height,
				    extents.width, extents.height);
		    cairo_fill(c);

		    secs = time(NULL) + matches_time[tatami];
		    tm = localtime(&secs);

		    cairo_set_source_rgb(c, 1.0, 1.0, 1.0);
		    sprintf(buf, "%02d:%02d", tm->tm_hour, tm->tm_min);
		    cairo_move_to(c, right - extents.width, y_pos[tatami]);
		    cairo_show_text(c, buf);
		}

		cairo_restore(c);
	    }

	    y_pos[tatami] += 1.0;
	} /* for tatami */

	if (!grp_found)
	    break;
    } /* while 1 */

    for (tatami = 0; tatami <= number_of_tatamis; tatami++) {
	set_left_right(tatami);

        cairo_move_to(c, 10 + left, extents.height);
        if (i == 0)
            sprintf(buf, "%s  [%d]", _("Unlocated"), matches_left[tatami]);
        else
            sprintf(buf, "Tatami %d  [%d]", tatami, matches_left[tatami]);
        cairo_show_text(c, buf);

        point_click_areas[num_rectangles].index = 0;
        point_click_areas[num_rectangles].group = max_group[tatami]+1;
        point_click_areas[num_rectangles].tatami = tatami;
        point_click_areas[num_rectangles].orig_tatami = 0;
        point_click_areas[num_rectangles].x1 = left;
        point_click_areas[num_rectangles].y1 = y_pos[tatami];
        point_click_areas[num_rectangles].x2 = right;
        point_click_areas[num_rectangles].y2 = H(1.0);
        if (num_rectangles < NUM_RECTANGLES-1)
            num_rectangles++;

        if (tatami) {
            cairo_save(c);
            cairo_set_line_width(c, THICK_LINE);
            cairo_set_source_rgb(c, 0.0, 0.0, 0.0);
            cairo_move_to(c, left, 0);
            cairo_line_to(c, left, H(1.0));
            cairo_stroke(c);
            cairo_restore(c);
        }
    } // for tatamis

    cairo_save(c);
    cairo_set_line_width(c, THICK_LINE);
    cairo_set_source_rgb(c, 0.0, 0.0, 0.0);
    cairo_move_to(c, 0, extents.height*1.4);
    cairo_line_to(c, W(1.0), extents.height*1.4);
    cairo_stroke(c);
    cairo_restore(c);

 drag:
    if (button_drag) {
        cairo_set_line_width(c, THIN_LINE);
        cairo_text_extents(c, dragged_text, &extents);
        cairo_set_source_rgb(c, 1.0, 1.0, 1.0);
        cairo_rectangle(c, dragged_x - extents.width/2.0, dragged_y - extents.height,
                        extents.width + 4, extents.height + 4);
        cairo_fill(c);
        cairo_set_source_rgb(c, 0, 0, 0);
        cairo_rectangle(c, dragged_x - extents.width/2.0 - 1, dragged_y - extents.height - 1,
                        extents.width + 4, extents.height + 4);
        cairo_stroke(c);
        cairo_move_to(c, dragged_x - extents.width/2.0, dragged_y);
        cairo_show_text(c, dragged_text);
    }
}

static gboolean expose_scrolled(GtkWidget *widget, GdkEventExpose *event, gpointer userdata)
{
    static time_t last = 0;
    time_t now = time(NULL);
    if (now > last)
        gtk_widget_queue_draw(wincoll.darea);
    last = now;
    return FALSE;
}

/* This is called when we need to draw the windows contents */
static gboolean expose(GtkWidget *widget, GdkEventExpose *event, gpointer userdata)
{
#if (GTKVER == 3)
    cairo_t *c = (cairo_t *)event;
    paint(c, gtk_widget_get_allocated_width(widget), gtk_widget_get_allocated_height(widget), NULL);
    return FALSE;
#else
    cairo_t *c = gdk_cairo_create(widget->window);
#endif
    static cairo_surface_t *cs = NULL;
    static gint oldw = 0, oldh = 0;

#if (GTKVER == 3)
    if (cs && (oldw != gtk_widget_get_allocated_width(widget) ||
               oldh != gtk_widget_get_allocated_height(widget))) {
#else
    if (cs && (oldw != widget->allocation.width || oldh != widget->allocation.height)) {
#endif
        cairo_surface_destroy(cs);
        cs = NULL;
    }

    if (!cs) {
#if (GTKVER == 3)
        cs = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                                        gtk_widget_get_allocated_width(widget),
                                        gtk_widget_get_allocated_height(widget));
        oldw = gtk_widget_get_allocated_width(widget);
        oldh = gtk_widget_get_allocated_height(widget);
#else
        cs = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                                        widget->allocation.width, widget->allocation.height);
        oldw = widget->allocation.width;
        oldh = widget->allocation.height;
#endif
    }

    if (button_drag) {
#if (GTKVER == 3)
        paint(c, gtk_widget_get_allocated_width(widget), gtk_widget_get_allocated_height(widget), cs);
#else
        paint(c, widget->allocation.width, widget->allocation.height, cs);
#endif
    } else {
        cairo_t *c1 = cairo_create(cs);
#if (GTKVER == 3)
        paint(c1, gtk_widget_get_allocated_width(widget), gtk_widget_get_allocated_height(widget), NULL);
#else
        paint(c1, widget->allocation.width, widget->allocation.height, NULL);
#endif
        cairo_set_source_surface(c, cs, 0, 0);
        cairo_paint(c);
        cairo_destroy(c1);
    }

    cairo_show_page(c);
    cairo_destroy(c);
    //cairo_surface_destroy(cs);

    return FALSE;
}

static gint scroll_callback(gpointer userdata)
{
    struct win_collection *wc = userdata;

    if (button_drag == FALSE || scroll_up_down == NO_SCROLL)
        return TRUE;

    GtkAdjustment *adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(wc->scrolled_window));
    gdouble adjnow = gtk_adjustment_get_value(adj);

    if (scroll_up_down == SCROLL_DOWN)
        gtk_adjustment_set_value(adj, adjnow + 50.0);
    else if (scroll_up_down == SCROLL_UP)
        gtk_adjustment_set_value(adj, adjnow - 50.0);

    return TRUE;
}


void set_category_graph_page(GtkWidget *nb)
{
    wait_cursor = gdk_cursor_new(GDK_HAND1);

    wincoll.scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_set_border_width(GTK_CONTAINER(wincoll.scrolled_window), 10);

    wincoll.darea = gtk_drawing_area_new();
    gtk_widget_set_size_request(wincoll.darea, 600, 6000);

#if (GTKVER != 3)
    GTK_WIDGET_SET_FLAGS(wincoll.darea, GTK_CAN_FOCUS);
#endif
    gtk_widget_add_events(wincoll.darea,
                          GDK_BUTTON_PRESS_MASK |
                          GDK_BUTTON_RELEASE_MASK |
                          /*GDK_POINTER_MOTION_MASK |*/
                          GDK_POINTER_MOTION_HINT_MASK |
                          GDK_BUTTON_MOTION_MASK);


    /* pack the table into the scrolled window */
#if (GTKVER == 3) && GTK_CHECK_VERSION(3,8,0)
    gtk_container_add(GTK_CONTAINER(wincoll.scrolled_window), wincoll.darea);
#else
    gtk_scrolled_window_add_with_viewport (
        GTK_SCROLLED_WINDOW(wincoll.scrolled_window), wincoll.darea);
#endif

    cat_graph_label = gtk_label_new (_("Categories"));
    gtk_notebook_append_page(GTK_NOTEBOOK(nb), wincoll.scrolled_window, cat_graph_label);

    gtk_widget_show(wincoll.darea);

#if (GTKVER == 3)
    g_signal_connect(G_OBJECT(wincoll.scrolled_window),
                     "draw", G_CALLBACK(expose_scrolled), NULL);
    g_signal_connect(G_OBJECT(wincoll.darea),
                     "draw", G_CALLBACK(expose), wincoll.darea);
#else
    g_signal_connect(G_OBJECT(wincoll.darea),
                     "expose-event", G_CALLBACK(expose), wincoll.darea);
#endif
    g_signal_connect(G_OBJECT(wincoll.darea),
                     "button-press-event", G_CALLBACK(mouse_click), NULL);
#if 0
    g_signal_connect(G_OBJECT(wincoll.darea),
                     "screen-changed", G_CALLBACK(screen_changed), NULL)
#endif

        g_signal_connect(G_OBJECT(wincoll.darea),
			 "button-release-event", G_CALLBACK(release_notify), NULL);
    g_signal_connect(G_OBJECT(wincoll.darea),
                     "motion-notify-event", G_CALLBACK(motion_notify), &wincoll);

    g_timeout_add(500, scroll_callback, &wincoll);
}

static gint find_box(gdouble x, gdouble y)
{
    gint t;

    for (t = 0; t < num_rectangles; t++) {
        if (x >= point_click_areas[t].x1 &&
            x <= point_click_areas[t].x2 &&
            y >= point_click_areas[t].y1 &&
            y <= point_click_areas[t].y2) {
            return t;
        }
    }
    return -1;
}

#define AREA_SHIFT        4
#define ARG_MASK        0xf

static void handle_menu(GtkWidget *menuitem, gpointer userdata)
{
    gint t = (ptr_to_gint(userdata)) >> AREA_SHIFT;
    gint arg = (ptr_to_gint(userdata)) & ARG_MASK;

    if (arg == UNFREEZE_EXPORTED ||
	arg == UNFREEZE_IMPORTED) {
	db_freeze_matches(point_click_areas[t].tatami, 0, 0, arg);
    } else if (arg == UNFREEZE_THESE) {
	db_freeze_matches(point_click_areas[t].tatami, point_click_areas[t].index, 0, arg);
    } else if (arg == MOVE_MATCHES) {
	view_popup_menu_move_matches(NULL, gint_to_ptr(point_click_areas[t].index));
    }
}

static gboolean mouse_click(GtkWidget *sheet_page,
			    GdkEventButton *event,
			    gpointer userdata)
{
#if (GTKVER == 3)
    gdk_window_set_cursor(gtk_widget_get_window(GTK_WIDGET(main_window)), hand_cursor);
#else
    gdk_window_set_cursor(GTK_WIDGET(main_window)->window, hand_cursor);
#endif
    button_drag = FALSE;

    if (event->type == GDK_BUTTON_PRESS) {
        gdouble x = event->x, y = event->y;
        gint t;

        t = find_box(x, y);
        if (t < 0)
            return FALSE;

        if (event->button == 1) {
	    if (cannot_move_category(point_click_areas[t].index))
		return FALSE;

	    dragged_x = x;
	    dragged_y = y;
	    button_drag = TRUE;
	    dragged_cat = point_click_areas[t].index;
	    if (dragged_cat < 10000)
		button_drag = FALSE;
	    start_box = t;

	    struct category_data *catdata = avl_get_category(dragged_cat);
	    if (catdata)
		snprintf(dragged_text, sizeof(dragged_text), "%s -> ?",
			 catdata->category);

	    refresh_window();
	    return TRUE;
        } else if (event->button == 3) {
	    GtkWidget *menu, *menuitem;
	    gpointer p;

	    menu = gtk_menu_new();

	    menuitem = gtk_menu_item_new_with_label(_("Unfreeze exported"));
	    p = gint_to_ptr(UNFREEZE_EXPORTED | (t << AREA_SHIFT));
	    g_signal_connect(menuitem, "activate",
			     (GCallback) handle_menu, p);
	    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

	    menuitem = gtk_menu_item_new_with_label(_("Unfreeze imported"));
	    p = gint_to_ptr(UNFREEZE_IMPORTED | (t << AREA_SHIFT));
	    g_signal_connect(menuitem, "activate",
			     (GCallback) handle_menu, p);
	    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

	    if (point_click_areas[t].orig_tatami) {
		menuitem = gtk_menu_item_new_with_label(_("Unfreeze this"));
		p = gint_to_ptr(UNFREEZE_THESE | (t << AREA_SHIFT));
		g_signal_connect(menuitem, "activate",
				 (GCallback) handle_menu, p);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	    }

	    menuitem = gtk_menu_item_new_with_label(_("Move Matches"));
	    p = gint_to_ptr(MOVE_MATCHES | (t << AREA_SHIFT));
	    g_signal_connect(menuitem, "activate",
			     (GCallback) handle_menu, p);
	    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

	    if (point_click_areas[t].index >= 10000) {
		menuitem = gtk_menu_item_new_with_label(_("Show Sheet"));
		g_signal_connect(menuitem, "activate",
				 (GCallback) show_category_window,
				 gint_to_ptr(point_click_areas[t].index));
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	    }

	    gtk_widget_show_all(menu);

	    gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
			   (event != NULL) ? event->button : 0,
			   gdk_event_get_time((GdkEvent*)event));

	    refresh_window();
	    return TRUE;
	}
    }
    return FALSE;
}

static gboolean motion_notify(GtkWidget *sheet_page,
                              GdkEventMotion *event,
                              gpointer userdata)
{
    struct win_collection *wc = userdata;

    if (button_drag == FALSE)
        return FALSE;

    gdouble x = event->x, y = event->y;
    gint t;
    GtkAdjustment *adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(wc->scrolled_window));
    gdouble adjnow = gtk_adjustment_get_value(adj);

#if (GTKVER == 3)
    if (y - adjnow > gtk_widget_get_allocated_height(wc->scrolled_window) - 50) {
#else
    if (y - adjnow > wc->scrolled_window->allocation.height - 50) {
#endif
        scroll_up_down = SCROLL_DOWN;
    } else if (y - adjnow < 20.0) {
        scroll_up_down = SCROLL_UP;
    } else
        scroll_up_down = NO_SCROLL;

    dragged_x = x;
    dragged_y = y;

    t = find_box(x, y);
    if (t < 0)
        return FALSE;

    struct category_data *catdata = avl_get_category(dragged_cat);
    if (catdata && t >= 0)
        snprintf(dragged_text, sizeof(dragged_text), "%s -> T%d:%d",
                 catdata->category, point_click_areas[t].tatami,
                 point_click_areas[t].group);
    refresh_window();

    return FALSE;
}

static gboolean release_notify(GtkWidget *sheet_page,
			       GdkEventButton *event,
			       gpointer userdata)
{
#if (GTKVER == 3)
    gdk_window_set_cursor(gtk_widget_get_window(GTK_WIDGET(main_window)), NULL);
#else
    gdk_window_set_cursor(GTK_WIDGET(main_window)->window, NULL);
#endif
    if (!button_drag)
        return FALSE;

    button_drag = FALSE;

    /* single click with the right mouse button? */
    if (event->type == GDK_BUTTON_RELEASE &&
        (event->button == 1 || event->button == 3)) {
        gdouble x = event->x, y = event->y;
        gint t;

        t = find_box(x, y);
        if (t < 0)
            return FALSE;

        struct judoka *j = get_data(dragged_cat);
        if (!j)
            return FALSE;

        j->index = dragged_cat;
        j->belt = point_click_areas[t].tatami;
        j->birthyear = point_click_areas[t].group;
        db_update_category(j->index, j);
        if (display_one_judoka(j) >= 0)
            g_print("Error: %s %d\n", __FUNCTION__, __LINE__);

        category_refresh(j->index);
        update_category_status_info_all();

        free_judoka(j);

        refresh_window();

        draw_match_graph();

	if(point_click_areas[t].tatami != point_click_areas[start_box].tatami) {
	    update_matches(0, (struct compsys){0,0,0,0}, point_click_areas[t].tatami);
	    update_matches(0, (struct compsys){0,0,0,0}, point_click_areas[start_box].tatami);
	}

        return TRUE;
    }

    return FALSE;
}

static gboolean cannot_move_category(gint cat) {
    gint i;

    for (i = 1; i <= NUM_TATAMIS; i++) {
        struct match *m = get_cached_next_matches(i);
        if ((m[0].category & MATCH_CATEGORY_MASK) == cat && m[0].number < 1000 &&
            (m[0].blue >= COMPETITOR || m[0].white >= COMPETITOR))
            return TRUE;
    }
    return FALSE;
}

static cairo_surface_t *get_image(const gchar *name)
{
    cairo_surface_t *pic;
    gchar *file = g_build_filename(installation_dir, "etc", name, NULL);
    pic = cairo_image_surface_create_from_png(file);
    g_free(file);
    return pic;
}

void set_cat_graph_titles(void)
{
    if (cat_graph_label)
        gtk_label_set_text(GTK_LABEL(cat_graph_label), _("Categories"));
}
