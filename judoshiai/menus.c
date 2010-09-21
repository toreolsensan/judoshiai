/* -*- mode: C; c-basic-offset: 4;  -*- */

/*
 * Copyright (C) 2006-2010 by Hannu Jokinen
 * Full copyright text is included in the software package.
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "judoshiai.h"

extern void about_shiai(GtkWidget *w, gpointer data);
extern void new_shiai(GtkWidget *w, gpointer data);
extern void open_shiai_from_net(GtkWidget *w, gpointer data);
extern void open_shiai(GtkWidget *w, gpointer data);
extern void font_dialog(GtkWidget *w, gpointer data);
extern void set_lang(gpointer data, guint action, GtkWidget *w);
extern void set_club_text(gpointer data, guint action, GtkWidget *w);
extern void set_club_abbr(GtkWidget *menu_item, gpointer data);
extern void set_draw_system(gpointer data, guint action, GtkWidget *w);
extern void toggle_automatic_sheet_update(gpointer callback_data, 
					  guint callback_action, GtkWidget *menu_item);
extern void toggle_automatic_web_page_update(gpointer callback_data, 
                                             guint callback_action, GtkWidget *menu_item);
extern void toggle_weights_in_sheets(gpointer callback_data, 
                                     guint callback_action, GtkWidget *menu_item);
extern void toggle_grade_visible(gpointer callback_data, 
                                 guint callback_action, GtkWidget *menu_item);
extern void properties(GtkWidget *w, gpointer data);
extern void get_from_old_competition(GtkWidget *w, gpointer data);
extern void get_from_old_competition_with_weight(GtkWidget *w, gpointer data);
extern void start_help(GtkWidget *w, gpointer data);
extern void locate_to_tatamis(GtkWidget *w, gpointer data);
extern void set_tatami_state(GtkWidget *menu_item, gpointer data);
extern void backup_shiai(GtkWidget *w, gpointer data);
extern void toggle_mirror(GtkWidget *menu_item, gpointer data);
extern void toggle_auto_arrange(GtkWidget *menu_item, gpointer data);
extern void select_use_logo(GtkWidget *w, gpointer data);


static GtkWidget *menubar, 
    *tournament_menu_item, *competitors_menu_item, 
    *categories_menu_item, *drawing_menu_item, *results_menu_item, 
    *judotimer_menu_item, *preferences_menu_item, *help_menu_item,
    *tournament_menu, *competitors_menu, 
    *categories_menu, *drawing_menu, *results_menu, 
    *judotimer_menu, *preferences_menu, *help_menu, *sql_dialog,
    *flag_fi, *flag_se, *flag_uk, *flag_es, 
    *menu_flag_fi, *menu_flag_se, *menu_flag_uk, *menu_flag_es,
    *tournament_new, *tournament_choose, *tournament_choose_net, *tournament_properties, 
    *tournament_quit, *tournament_backup,
    *competitor_new, *competitor_search, *competitor_select_from_tournament, *competitor_add_from_text_file,
    *competitor_add_all_from_shiai, *competitor_add_all_with_weights, *competitor_remove_unweighted,
    *competitor_restore_removed, *competitor_bar_code_search, *competitor_print_weigh_notes,
    *category_new, *category_remove_empty, *category_create_official, 
    *category_print_all, *category_print_all_pdf, *category_print_matches,
    *category_properties, *category_best_of_three, *category_to_tatamis[NUM_TATAMIS],
    *draw_all_categories, *draw_international, *draw_finnish, 
    *draw_swedish, *draw_estonian, *draw_spanish,
    *results_print_all, *results_print_schedule_printer, *results_print_schedule_pdf,
    *preference_comm, *preference_comm_node, *preference_own_ip_addr, *preference_show_connections,
    *preference_auto_sheet_update/*, *preference_auto_web_update*/, *preference_results_in_finnish, 
    *preference_langsel, *preference_results_in_swedish, *preference_results_in_english, 
    *preference_results_in_spanish, *preference_weights_to_pool_sheets, 
    *preference_grade_visible, *preference_layout,
    *preference_sheet_font, *preference_password, *judotimer_control[NUM_TATAMIS],
    *preference_mirror, *preference_auto_arrange, *preference_club_text,
    *preference_club_text_club, *preference_club_text_country, *preference_club_text_both,
    *preference_club_text_abbr, *preference_use_logo,
    *help_manual, *help_about;

static GSList *lang_group = NULL, *club_group = NULL, *draw_group = NULL;

static GtkTooltips *menu_tips;

static GtkWidget *get_picture(const gchar *name)
{
    gchar *file = g_build_filename(installation_dir, "etc", name, NULL);
    GtkWidget *pic = gtk_image_new_from_file(file);
    g_free(file);
    return pic;
}

GtkWidget *get_menubar_menu(GtkWidget  *window)
{
    GtkAccelGroup *group;
    gint i;
    gchar buf[64];

    menu_tips = gtk_tooltips_new ();
    group = gtk_accel_group_new ();
    menubar = gtk_menu_bar_new ();

    tournament_menu_item     = gtk_menu_item_new_with_label (_("Tournament"));
    competitors_menu_item = gtk_menu_item_new_with_label (_("Competitors"));
    categories_menu_item  = gtk_menu_item_new_with_label (_("Categories"));
    drawing_menu_item     = gtk_menu_item_new_with_label (_("Drawing"));
    results_menu_item     = gtk_menu_item_new_with_label (_("Results"));
    judotimer_menu_item   = gtk_menu_item_new_with_label (_("Judotimer"));
    preferences_menu_item = gtk_menu_item_new_with_label (_("Preferences"));
    help_menu_item        = gtk_menu_item_new_with_label (_("Help"));

    flag_fi     = get_picture("finland.png");
    flag_se     = get_picture("sweden.png");
    flag_uk     = get_picture("uk.png");
    flag_es     = get_picture("spain.png");
    menu_flag_fi = gtk_image_menu_item_new();
    menu_flag_se = gtk_image_menu_item_new();
    menu_flag_uk = gtk_image_menu_item_new();
    menu_flag_es = gtk_image_menu_item_new();
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_flag_fi), flag_fi);        
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_flag_se), flag_se);        
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_flag_uk), flag_uk);      
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_flag_es), flag_es);        
    gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM(menu_flag_fi), TRUE);
    gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM(menu_flag_se), TRUE);
    gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM(menu_flag_uk), TRUE);
    gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM(menu_flag_es), TRUE);

    tournament_menu  = gtk_menu_new();
    competitors_menu = gtk_menu_new();
    categories_menu  = gtk_menu_new();
    drawing_menu     = gtk_menu_new();
    results_menu     = gtk_menu_new();
    judotimer_menu   = gtk_menu_new();
    preferences_menu = gtk_menu_new();
    help_menu        = gtk_menu_new();

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(tournament_menu_item), tournament_menu); 
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(competitors_menu_item), competitors_menu); 
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(categories_menu_item), categories_menu); 
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(drawing_menu_item), drawing_menu); 
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(results_menu_item), results_menu); 
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(judotimer_menu_item), judotimer_menu); 
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(preferences_menu_item), preferences_menu); 
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(help_menu_item), help_menu); 

    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), tournament_menu_item); 
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), competitors_menu_item); 
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), categories_menu_item); 
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), drawing_menu_item); 
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), results_menu_item); 
    //gtk_menu_shell_append(GTK_MENU_SHELL(menubar), judotimer_menu_item); 
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), preferences_menu_item); 
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), help_menu_item); 
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), menu_flag_fi); 
    //gtk_menu_shell_append(GTK_MENU_SHELL(menubar), menu_flag_se); 
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), menu_flag_uk); 
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), menu_flag_es); 

    /* Language selection */
    g_signal_connect(G_OBJECT(menu_flag_fi), "button_press_event",
                     G_CALLBACK(change_language), (gpointer)LANG_FI);
    g_signal_connect(G_OBJECT(menu_flag_se), "button_press_event",
                     G_CALLBACK(change_language), (gpointer)LANG_SW);
    g_signal_connect(G_OBJECT(menu_flag_uk), "button_press_event",
                     G_CALLBACK(change_language), (gpointer)LANG_EN);
    g_signal_connect(G_OBJECT(menu_flag_es), "button_press_event",
                     G_CALLBACK(change_language), (gpointer)LANG_ES);

    /* Create the Tournament menu content. */
    tournament_new        = gtk_menu_item_new_with_label(_("New Tournament"));
    tournament_choose     = gtk_menu_item_new_with_label(_("Open Tournament"));
    tournament_choose_net = gtk_menu_item_new_with_label(_("Open Tournament from Net"));
    tournament_properties = gtk_menu_item_new_with_label(_("Properties"));
    tournament_backup     = gtk_menu_item_new_with_label(_("Tournament Backup"));
    sql_dialog            = gtk_menu_item_new_with_label(_("SQL Dialog"));
    tournament_quit       = gtk_menu_item_new_with_label(_("Quit"));

    gtk_menu_shell_append(GTK_MENU_SHELL(tournament_menu), tournament_new);
    gtk_menu_shell_append(GTK_MENU_SHELL(tournament_menu), tournament_choose);
    //gtk_menu_shell_append(GTK_MENU_SHELL(tournament_menu), tournament_choose_net);
    gtk_menu_shell_append(GTK_MENU_SHELL(tournament_menu), tournament_properties);
    gtk_menu_shell_append(GTK_MENU_SHELL(tournament_menu), tournament_backup);
    gtk_menu_shell_append(GTK_MENU_SHELL(tournament_menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(tournament_menu), sql_dialog);
    gtk_menu_shell_append(GTK_MENU_SHELL(tournament_menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(tournament_menu), tournament_quit);

    g_signal_connect(G_OBJECT(tournament_new),        "activate", G_CALLBACK(new_shiai), 0);
    g_signal_connect(G_OBJECT(tournament_choose),     "activate", G_CALLBACK(open_shiai), 0);
    g_signal_connect(G_OBJECT(tournament_choose_net), "activate", G_CALLBACK(open_shiai_from_net), 0);
    g_signal_connect(G_OBJECT(tournament_properties), "activate", G_CALLBACK(properties), 0);
    g_signal_connect(G_OBJECT(tournament_backup),     "activate", G_CALLBACK(backup_shiai), 0);
    g_signal_connect(G_OBJECT(sql_dialog),            "activate", G_CALLBACK(sql_window), 0);
    g_signal_connect(G_OBJECT(tournament_quit),       "activate", G_CALLBACK(destroy), 0);

    gtk_widget_add_accelerator(tournament_quit, "activate", group, GDK_Q, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    /* Create the Competitors menu content. */
    competitor_new                  = gtk_menu_item_new_with_label(_("New Competitor"));
    competitor_search               = gtk_menu_item_new_with_label(_("Search Competitor"));
    competitor_select_from_tournament  = gtk_menu_item_new_with_label(_("Select From Another Shiai"));
    competitor_add_from_text_file   = gtk_menu_item_new_with_label(_("Add From a Text File"));
    competitor_add_all_from_shiai   = gtk_menu_item_new_with_label(_("Add All From Another Shiai"));
    competitor_add_all_with_weights = gtk_menu_item_new_with_label(_("Add All with Weights"));
    competitor_remove_unweighted    = gtk_menu_item_new_with_label(_("Remove Unweighted"));
    competitor_restore_removed      = gtk_menu_item_new_with_label(_("Restore Removed"));
    competitor_bar_code_search      = gtk_menu_item_new_with_label(_("Bar Code Search"));
    competitor_print_weigh_notes    = gtk_menu_item_new_with_label(_("Print Weight Notes"));

    gtk_menu_shell_append(GTK_MENU_SHELL(competitors_menu), competitor_new);
    gtk_menu_shell_append(GTK_MENU_SHELL(competitors_menu), competitor_search);
    gtk_menu_shell_append(GTK_MENU_SHELL(competitors_menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(competitors_menu), competitor_select_from_tournament);
    gtk_menu_shell_append(GTK_MENU_SHELL(competitors_menu), competitor_add_from_text_file);
    gtk_menu_shell_append(GTK_MENU_SHELL(competitors_menu), competitor_add_all_from_shiai);
    gtk_menu_shell_append(GTK_MENU_SHELL(competitors_menu), competitor_add_all_with_weights);
    gtk_menu_shell_append(GTK_MENU_SHELL(competitors_menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(competitors_menu), competitor_remove_unweighted);
    gtk_menu_shell_append(GTK_MENU_SHELL(competitors_menu), competitor_restore_removed);
    gtk_menu_shell_append(GTK_MENU_SHELL(competitors_menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(competitors_menu), competitor_bar_code_search);
    gtk_menu_shell_append(GTK_MENU_SHELL(competitors_menu), competitor_print_weigh_notes);

    g_signal_connect(G_OBJECT(competitor_new),                 "activate", G_CALLBACK(new_judoka), 0);
    g_signal_connect(G_OBJECT(competitor_search),              "activate", G_CALLBACK(search_competitor), 0);
    g_signal_connect(G_OBJECT(competitor_select_from_tournament), "activate", G_CALLBACK(set_old_shiai_display), 0);
    g_signal_connect(G_OBJECT(competitor_add_from_text_file),  "activate", G_CALLBACK(import_txt_dialog), 0);
    g_signal_connect(G_OBJECT(competitor_add_all_from_shiai),  "activate", G_CALLBACK(get_from_old_competition), 0);
    g_signal_connect(G_OBJECT(competitor_add_all_with_weights),"activate", G_CALLBACK(get_from_old_competition_with_weight), 0);
    g_signal_connect(G_OBJECT(competitor_remove_unweighted),   "activate", G_CALLBACK(remove_unweighed_competitors), 0);
    g_signal_connect(G_OBJECT(competitor_restore_removed),     "activate", G_CALLBACK(db_restore_removed_competitors), 0);
    g_signal_connect(G_OBJECT(competitor_bar_code_search),     "activate", G_CALLBACK(barcode_search), 0);
    g_signal_connect(G_OBJECT(competitor_print_weigh_notes),   "activate", G_CALLBACK(print_doc), 
                     (gpointer)(PRINT_WEIGHING_NOTES | PRINT_TO_PDF));

    gtk_widget_add_accelerator(competitor_new, "activate", group, GDK_N, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(competitor_search, "activate", group, GDK_F, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    /* Create the Category menu content. */
    category_new             = gtk_menu_item_new_with_label(_("New Category"));
    category_remove_empty    = gtk_menu_item_new_with_label(_("Remove Empty"));
    category_create_official = gtk_menu_item_new_with_label(_("Create Categories"));
    category_print_all       = gtk_menu_item_new_with_label(_("Print All"));
    category_print_all_pdf   = gtk_menu_item_new_with_label(_("Print All (PDF)"));
    category_print_matches   = gtk_menu_item_new_with_label(_("Print Matches (CSV)"));
    category_properties      = gtk_menu_item_new_with_label(_("Properties"));
    category_best_of_three   = gtk_check_menu_item_new_with_label(_("Best of Three"));
    for (i = 0; i < NUM_TATAMIS; i++) {
        sprintf(buf, "%s %d %s", _("Place To"), i+1, _("Tatamis"));
        category_to_tatamis[i] = gtk_menu_item_new_with_label(buf);
    }

    gtk_menu_shell_append(GTK_MENU_SHELL(categories_menu), category_new);
    gtk_menu_shell_append(GTK_MENU_SHELL(categories_menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(categories_menu), category_remove_empty);
    gtk_menu_shell_append(GTK_MENU_SHELL(categories_menu), category_create_official);
    gtk_menu_shell_append(GTK_MENU_SHELL(categories_menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(categories_menu), category_print_all);
    gtk_menu_shell_append(GTK_MENU_SHELL(categories_menu), category_print_all_pdf);
    gtk_menu_shell_append(GTK_MENU_SHELL(categories_menu), category_print_matches);
    gtk_menu_shell_append(GTK_MENU_SHELL(categories_menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(categories_menu), category_properties);
    gtk_menu_shell_append(GTK_MENU_SHELL(categories_menu), category_best_of_three);
    gtk_menu_shell_append(GTK_MENU_SHELL(categories_menu), gtk_separator_menu_item_new());
    for (i = 0; i < NUM_TATAMIS; i++)
        gtk_menu_shell_append(GTK_MENU_SHELL(categories_menu), category_to_tatamis[i]);

    g_signal_connect(G_OBJECT(category_new),             "activate", G_CALLBACK(new_regcategory), 0);
    g_signal_connect(G_OBJECT(category_remove_empty),    "activate", G_CALLBACK(remove_empty_regcategories), 0);
    g_signal_connect(G_OBJECT(category_create_official), "activate", G_CALLBACK(create_categories), 0);
    g_signal_connect(G_OBJECT(category_print_all),       "activate", G_CALLBACK(print_doc), 
                     (gpointer)(PRINT_ALL_CATEGORIES | PRINT_TO_PRINTER));
    g_signal_connect(G_OBJECT(category_print_all_pdf),   "activate", G_CALLBACK(print_doc), 
                     (gpointer)(PRINT_ALL_CATEGORIES | PRINT_TO_PDF));
    g_signal_connect(G_OBJECT(category_print_matches),   "activate", G_CALLBACK(print_matches), 
                     (gpointer)(PRINT_ALL_CATEGORIES | PRINT_TO_PDF));
    g_signal_connect(G_OBJECT(category_properties),      "activate", G_CALLBACK(set_categories_dialog), 0);
    g_signal_connect(G_OBJECT(category_best_of_three),   "activate", G_CALLBACK(toggle_three_matches), (gpointer)1);
    for (i = 0; i < NUM_TATAMIS; i++)
        g_signal_connect(G_OBJECT(category_to_tatamis[i]), "activate", G_CALLBACK(locate_to_tatamis), 
                         (gpointer)(i+1));

    /* Create the Drawing menu content. */
    draw_all_categories = gtk_menu_item_new_with_label(_("Draw All Categories"));
    gtk_menu_shell_append(GTK_MENU_SHELL(drawing_menu), draw_all_categories);
    g_signal_connect(G_OBJECT(draw_all_categories), "activate", G_CALLBACK(draw_all), 0);

    draw_international = gtk_radio_menu_item_new_with_label(draw_group, "");
    draw_group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(draw_international));
    draw_finnish = gtk_radio_menu_item_new_with_label(draw_group, "");
    draw_group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(draw_finnish));
    draw_swedish = gtk_radio_menu_item_new_with_label(draw_group, "");
    draw_group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(draw_swedish));
    draw_estonian = gtk_radio_menu_item_new_with_label(draw_group, "");
    draw_group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(draw_estonian));
    draw_spanish = gtk_radio_menu_item_new_with_label(draw_group, "");

    gtk_menu_shell_append(GTK_MENU_SHELL(drawing_menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(drawing_menu), draw_international);
    gtk_menu_shell_append(GTK_MENU_SHELL(drawing_menu), draw_finnish);
    gtk_menu_shell_append(GTK_MENU_SHELL(drawing_menu), draw_swedish);
    gtk_menu_shell_append(GTK_MENU_SHELL(drawing_menu), draw_estonian);
    gtk_menu_shell_append(GTK_MENU_SHELL(drawing_menu), draw_spanish);
    g_signal_connect(G_OBJECT(draw_international), "activate", G_CALLBACK(set_draw_system), 
		     (gpointer)DRAW_INTERNATIONAL);
    g_signal_connect(G_OBJECT(draw_finnish),       "activate", G_CALLBACK(set_draw_system), 
		     (gpointer)DRAW_FINNISH);
    g_signal_connect(G_OBJECT(draw_swedish),       "activate", G_CALLBACK(set_draw_system), 
		     (gpointer)DRAW_SWEDISH);
    g_signal_connect(G_OBJECT(draw_estonian),      "activate", G_CALLBACK(set_draw_system), 
		     (gpointer)DRAW_ESTONIAN);
    g_signal_connect(G_OBJECT(draw_spanish),      "activate", G_CALLBACK(set_draw_system), 
		     (gpointer)DRAW_SPANISH);


    /* Create the Results menu content. */
    results_print_all              = gtk_menu_item_new_with_label(_("Print All (Web And PDF)"));
    results_print_schedule_printer = gtk_menu_item_new_with_label(_("Print Schedule to Printer"));
    results_print_schedule_pdf     = gtk_menu_item_new_with_label(_("Print Schedule to PDF"));

    gtk_menu_shell_append(GTK_MENU_SHELL(results_menu), results_print_all);
    gtk_menu_shell_append(GTK_MENU_SHELL(results_menu), results_print_schedule_printer);
    gtk_menu_shell_append(GTK_MENU_SHELL(results_menu), results_print_schedule_pdf);

    g_signal_connect(G_OBJECT(results_print_all),              "activate", G_CALLBACK(make_png_all), 0);
    g_signal_connect(G_OBJECT(results_print_schedule_printer), "activate", G_CALLBACK(print_doc),
                     (gpointer)(PRINT_SCHEDULE | PRINT_TO_PRINTER));
    g_signal_connect(G_OBJECT(results_print_schedule_pdf),      "activate", G_CALLBACK(print_doc),
                     (gpointer)(PRINT_SCHEDULE | PRINT_TO_PDF));


    /* Create the Judotimer menu content. */
    for (i = 0; i < NUM_TATAMIS; i++) {
        sprintf(buf, "%s %d", _("Control Tatami"), i+1);
        judotimer_control[i] = gtk_check_menu_item_new_with_label(buf);
        gtk_menu_shell_append(GTK_MENU_SHELL(judotimer_menu), judotimer_control[i]);
        g_signal_connect(G_OBJECT(judotimer_control[i]), "activate", G_CALLBACK(set_tatami_state), 
                         (gpointer)(i+1));
    }


    /* Create the Preferences menu content. */
    preference_comm_node              = gtk_menu_item_new_with_label(_("Communication Node"));
    preference_own_ip_addr            = gtk_menu_item_new_with_label(_("Own IP Addresses"));
    preference_show_connections       = gtk_menu_item_new_with_label(_("Show Attached Connections"));
    preference_auto_sheet_update      = gtk_check_menu_item_new_with_label(_("Automatic Sheet Update"));
    //preference_auto_web_update        = gtk_check_menu_item_new_with_label(_("Automatic Web Page Update"));

    preference_results_in_finnish     = gtk_radio_menu_item_new_with_label(lang_group, _("Results in Finnish"));
    lang_group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(preference_results_in_finnish));
    preference_results_in_swedish     = gtk_radio_menu_item_new_with_label(lang_group, _("Results in Swedish"));
    lang_group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(preference_results_in_swedish));
    preference_results_in_english     = gtk_radio_menu_item_new_with_label(lang_group, _("Results in English"));
    lang_group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(preference_results_in_english));
    preference_results_in_spanish     = gtk_radio_menu_item_new_with_label(lang_group, _("Results in Spanish"));

    preference_club_text_club         = gtk_radio_menu_item_new_with_label(club_group, _("Club Name Only"));
    club_group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(preference_club_text_club));
    preference_club_text_country      = gtk_radio_menu_item_new_with_label(club_group, _("Country Name Only"));
    club_group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(preference_club_text_country));
    preference_club_text_both         = gtk_radio_menu_item_new_with_label(club_group, _("Both Club and Country"));
    preference_club_text_abbr         = gtk_check_menu_item_new_with_label(_("Abbreviations in Sheets"));

    preference_weights_to_pool_sheets = gtk_check_menu_item_new_with_label(_("Weights Visible in Pool Sheets"));
    preference_grade_visible          = gtk_check_menu_item_new_with_label("");
    preference_sheet_font             = gtk_menu_item_new_with_label(_("Sheet Font"));
    preference_password               = gtk_menu_item_new_with_label(_("Password"));
    preference_mirror                 = gtk_check_menu_item_new_with_label("");
    preference_auto_arrange           = gtk_check_menu_item_new_with_label("");
    preference_use_logo               = gtk_menu_item_new_with_label("");

    //gtk_menu_shell_append(GTK_MENU_SHELL(preferences_menu), preference_comm_node);
    preference_comm = gtk_menu_item_new_with_label("");
    GtkWidget *submenu = gtk_menu_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(preferences_menu), preference_comm);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(preference_comm), submenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), preference_own_ip_addr);
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), preference_show_connections);

    gtk_menu_shell_append(GTK_MENU_SHELL(preferences_menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(preferences_menu), preference_auto_sheet_update);
    //gtk_menu_shell_append(GTK_MENU_SHELL(preferences_menu), preference_auto_web_update);
    gtk_menu_shell_append(GTK_MENU_SHELL(preferences_menu), gtk_separator_menu_item_new());

    preference_langsel = gtk_menu_item_new_with_label(_("Language Selection"));
    submenu = gtk_menu_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(preferences_menu), preference_langsel);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(preference_langsel), submenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), preference_results_in_finnish);
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), preference_results_in_swedish);
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), preference_results_in_english);
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), preference_results_in_spanish);

    gtk_menu_shell_append(GTK_MENU_SHELL(preferences_menu), gtk_separator_menu_item_new());

    preference_club_text = gtk_menu_item_new_with_label("");
    submenu = gtk_menu_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(preferences_menu), preference_club_text);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(preference_club_text), submenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), preference_club_text_club);
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), preference_club_text_country);
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), preference_club_text_both);
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), preference_club_text_abbr);

    gtk_menu_shell_append(GTK_MENU_SHELL(preferences_menu), gtk_separator_menu_item_new());

    preference_layout = gtk_menu_item_new_with_label("");
    submenu = gtk_menu_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(preferences_menu), preference_layout);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(preference_layout), submenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), preference_weights_to_pool_sheets);
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), preference_grade_visible);
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), preference_use_logo);
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), preference_sheet_font);

    gtk_menu_shell_append(GTK_MENU_SHELL(preferences_menu), preference_mirror);
    gtk_menu_shell_append(GTK_MENU_SHELL(preferences_menu), preference_auto_arrange);
    gtk_menu_shell_append(GTK_MENU_SHELL(preferences_menu), preference_password);

    g_signal_connect(G_OBJECT(preference_comm_node),              "activate", G_CALLBACK(ask_node_ip_address), 0);
    g_signal_connect(G_OBJECT(preference_own_ip_addr),            "activate", G_CALLBACK(show_my_ip_addresses), 0);
    g_signal_connect(G_OBJECT(preference_show_connections),       "activate", G_CALLBACK(show_node_connections), 0);
    g_signal_connect(G_OBJECT(preference_auto_sheet_update),      "activate", G_CALLBACK(toggle_automatic_sheet_update), 0);
    //g_signal_connect(G_OBJECT(preference_auto_web_update),        "activate", G_CALLBACK(toggle_automatic_web_page_update), 0);

    g_signal_connect(G_OBJECT(preference_results_in_finnish),     "activate", G_CALLBACK(set_lang), (gpointer)LANG_FI);
    g_signal_connect(G_OBJECT(preference_results_in_swedish),     "activate", G_CALLBACK(set_lang), (gpointer)LANG_SW);
    g_signal_connect(G_OBJECT(preference_results_in_english),     "activate", G_CALLBACK(set_lang), (gpointer)LANG_EN);
    g_signal_connect(G_OBJECT(preference_results_in_spanish),     "activate", G_CALLBACK(set_lang), (gpointer)LANG_ES);

    g_signal_connect(G_OBJECT(preference_club_text_club),    "activate", 
		     G_CALLBACK(set_club_text), (gpointer)CLUB_TEXT_CLUB);
    g_signal_connect(G_OBJECT(preference_club_text_country), "activate", 
		     G_CALLBACK(set_club_text), (gpointer)CLUB_TEXT_COUNTRY);
    g_signal_connect(G_OBJECT(preference_club_text_both),    "activate", 
		     G_CALLBACK(set_club_text), (gpointer)(CLUB_TEXT_CLUB|CLUB_TEXT_COUNTRY));
    g_signal_connect(G_OBJECT(preference_club_text_abbr),    "activate", 
		     G_CALLBACK(set_club_abbr), (gpointer)NULL);

    g_signal_connect(G_OBJECT(preference_weights_to_pool_sheets), "activate", G_CALLBACK(toggle_weights_in_sheets), 0);
    g_signal_connect(G_OBJECT(preference_grade_visible),          "activate", G_CALLBACK(toggle_grade_visible), 0);
    g_signal_connect(G_OBJECT(preference_sheet_font),             "activate", G_CALLBACK(font_dialog), 0);
    g_signal_connect(G_OBJECT(preference_password),               "activate", G_CALLBACK(set_webpassword_dialog), 0);
    g_signal_connect(G_OBJECT(preference_mirror),                 "activate", G_CALLBACK(toggle_mirror), 0);
    g_signal_connect(G_OBJECT(preference_auto_arrange),           "activate", G_CALLBACK(toggle_auto_arrange), 0);
    g_signal_connect(G_OBJECT(preference_use_logo),               "activate", G_CALLBACK(select_use_logo), 0);


    /* Create the Drawing menu content. */
    help_manual = gtk_menu_item_new_with_label(_("Manual"));
    help_about = gtk_menu_item_new_with_label(_("About"));

    gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), help_manual);
    gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), help_about);

    g_signal_connect(G_OBJECT(help_manual), "activate", G_CALLBACK(start_help), 0);
    g_signal_connect(G_OBJECT(help_about), "activate", G_CALLBACK(about_shiai), 0);

    /* Attach the new accelerator group to the window. */
    gtk_window_add_accel_group(GTK_WINDOW(window), group);

    set_menu_active();

    return menubar;
}

void set_configuration(void)
{
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(category_best_of_three),
                                   three_matches_for_two);
}

void set_preferences(void)
{
    GError  *error = NULL;
    gchar   *str;
    gint     i, x1, x2;
    gboolean b;

    error = NULL;
    str = g_key_file_get_string(keyfile, "preferences", "nodeipaddress", &error);
    if (!error) {
        gulong a,b,c,d;
        sscanf(str, "%ld.%ld.%ld.%ld", &a, &b, &c, &d);
        node_ip_addr = host2net((a << 24) | (b << 16) | (c << 8) | d);
        g_free(str);
    }

    for (i = 1; i <= NUM_TATAMIS; i++) {
        gchar t[10];
        sprintf(t, "tatami%d", i);
        error = NULL;
        b = g_key_file_get_boolean(keyfile, "preferences", t, &error);
        if (b && !error) {
            gtk_menu_item_activate(GTK_MENU_ITEM(judotimer_control[i-1]));
        }
    }

    error = NULL;
    if (g_key_file_get_boolean(keyfile, "preferences", "automatic_sheet_update", &error)) {
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(preference_auto_sheet_update), TRUE);
    }

    error = NULL;
    if (g_key_file_get_boolean(keyfile, "preferences", "weightvisible", &error)) {
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(preference_weights_to_pool_sheets), TRUE);
    }

    error = NULL;
    if (g_key_file_get_boolean(keyfile, "preferences", "gradevisible", &error) || error) {
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(preference_grade_visible), TRUE);
    }

    error = NULL;
    x1 = g_key_file_get_integer(keyfile, "preferences", "win_x_pos", &error);
    if (!error) {
        x2 = g_key_file_get_integer(keyfile, "preferences", "win_y_pos", &error);
        if (!error)
            gtk_window_move(GTK_WINDOW(main_window), x1, x2);
    }

    error = NULL;
    x1 = g_key_file_get_integer(keyfile, "preferences", "win_width", &error);
    if (!error) {
        x2 = g_key_file_get_integer(keyfile, "preferences", "win_height", &error);
        if (!error)
            gtk_window_resize(GTK_WINDOW(main_window), x1, x2);
    }

    error = NULL;
    str = g_key_file_get_string(keyfile, "preferences", "sheetfont", &error);
    if (!error) {
        set_font(str);
        g_free(str);
    }

    error = NULL;
    x1 = g_key_file_get_integer(keyfile, "preferences", "webpassword", &error);
    if (!error)
        webpwcrc32 = x1;
    else
        webpwcrc32 = 0;

    error = NULL;
    x1 = g_key_file_get_integer(keyfile, "preferences", "language", &error);
    if (!error)
        language = x1;
    else
        language = LANG_FI;

    error = NULL;
    if (g_key_file_get_boolean(keyfile, "preferences", "mirror", &error)) {
        gtk_menu_item_activate(GTK_MENU_ITEM(preference_mirror));
    }

    error = NULL;
    if (g_key_file_get_boolean(keyfile, "preferences", "autoarrange", &error) || error) {
        gtk_menu_item_activate(GTK_MENU_ITEM(preference_auto_arrange));
    }

    error = NULL;
    x1 = g_key_file_get_integer(keyfile, "preferences", "printlanguage", &error);
    if (!error)
        print_lang = x1;
    else
        print_lang = LANG_FI;

    switch(print_lang) {
    case LANG_SW:
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(preference_results_in_swedish), TRUE);
        break;	
    case LANG_EN:
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(preference_results_in_english), TRUE);
        break;	
    case LANG_ES:
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(preference_results_in_spanish), TRUE);
        break;	
    default:
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(preference_results_in_finnish), TRUE);
    }

    error = NULL;
    x1 = g_key_file_get_integer(keyfile, "preferences", "clubtext", &error);
    if (!error)
        club_text = x1;
    else
        club_text = CLUB_TEXT_CLUB;

    if (club_text == CLUB_TEXT_CLUB)
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(preference_club_text_club), TRUE);
    else if (club_text == CLUB_TEXT_COUNTRY)
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(preference_club_text_country), TRUE);
    else if (club_text == (CLUB_TEXT_CLUB|CLUB_TEXT_COUNTRY))
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(preference_club_text_both), TRUE);

    error = NULL;
    if (g_key_file_get_boolean(keyfile, "preferences", "clubabbr", &error) || error) {
        gtk_menu_item_activate(GTK_MENU_ITEM(preference_club_text_abbr));
    }

    error = NULL;
    x1 = g_key_file_get_integer(keyfile, "preferences", "drawsystem", &error);
    if (!error)
        draw_system = x1;
    else
        draw_system = DRAW_INTERNATIONAL;

    if (draw_system == DRAW_INTERNATIONAL)
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(draw_international), TRUE);
    else if (draw_system == DRAW_FINNISH)
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(draw_finnish), TRUE);
    else if (draw_system == DRAW_SWEDISH)
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(draw_swedish), TRUE);
    else if (draw_system == DRAW_ESTONIAN)
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(draw_estonian), TRUE);
    else if (draw_system == DRAW_SPANISH)
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(draw_spanish), TRUE);

    error = NULL;
    str = g_key_file_get_string(keyfile, "preferences", "logofile", &error);
    if (!error) {
        if (str[0])
            use_logo = str;
        else
            g_free(str);
    }

    error = NULL;
    if (g_key_file_get_boolean(keyfile, "preferences", "printheaders", &error) && !error) {
        print_headers = TRUE;
    }
}

static void change_menu_label(GtkWidget *item, const gchar *new_text)
{
    GtkWidget *menu_label = gtk_bin_get_child(GTK_BIN(item));
    gtk_label_set_text(GTK_LABEL(menu_label), new_text); 
}

#define DB_OK (db_name != NULL)

#define SET_SENSITIVE(_menu, _yes) gtk_widget_set_sensitive(GTK_WIDGET(_menu), _yes)

void set_menu_active(void)
{
    gint i;

    SET_SENSITIVE(tournament_properties, DB_OK);
    SET_SENSITIVE(tournament_backup    , DB_OK);
    SET_SENSITIVE(sql_dialog           , DB_OK);

    SET_SENSITIVE(competitor_new                 , DB_OK);
    SET_SENSITIVE(competitor_search              , DB_OK);
    SET_SENSITIVE(competitor_select_from_tournament, DB_OK);
    SET_SENSITIVE(competitor_add_from_text_file  , DB_OK);
    SET_SENSITIVE(competitor_add_all_from_shiai  , DB_OK);
    SET_SENSITIVE(competitor_add_all_with_weights, DB_OK);
    SET_SENSITIVE(competitor_remove_unweighted   , DB_OK);
    SET_SENSITIVE(competitor_restore_removed     , DB_OK);
    SET_SENSITIVE(competitor_bar_code_search     , DB_OK);
    SET_SENSITIVE(competitor_print_weigh_notes   , DB_OK);

    SET_SENSITIVE(category_new            , DB_OK);
    SET_SENSITIVE(category_remove_empty   , DB_OK);
    SET_SENSITIVE(category_create_official, DB_OK);
    SET_SENSITIVE(category_print_all      , DB_OK);
    SET_SENSITIVE(category_print_all_pdf  , DB_OK);
    SET_SENSITIVE(category_print_matches  , DB_OK);
    SET_SENSITIVE(category_properties     , DB_OK);
    SET_SENSITIVE(category_best_of_three  , DB_OK);

    for (i = 0; i < NUM_TATAMIS; i++)
        SET_SENSITIVE(category_to_tatamis[i], DB_OK);

    SET_SENSITIVE(draw_all_categories, DB_OK);

    SET_SENSITIVE(results_print_all             , DB_OK);
    SET_SENSITIVE(results_print_schedule_printer, DB_OK);
    SET_SENSITIVE(results_print_schedule_pdf    , DB_OK);
}

gboolean change_language(GtkWidget *eventbox, GdkEventButton *event, void *param)
{
    gint i;
    gchar *r = NULL;
    gchar buf[32];

    language = (gint)param;

    switch (language) {
    case LANG_FI:
        putenv("LANGUAGE=fi");
        r = setlocale(LC_ALL, "fi");
        break;        
    case LANG_SW:
        putenv("LANGUAGE=sv");
        r = setlocale(LC_ALL, "sv");
        break;        
    case LANG_EN:
        putenv("LANGUAGE=en");
        r = setlocale(LC_ALL, "en");
        break;        
    case LANG_ES:
        putenv("LANGUAGE=es");
        r = setlocale(LC_ALL, "es");
        break;        
    }

    gchar *dirname = g_build_filename(installation_dir, "share", "locale", NULL);
    bindtextdomain ("judoshiai", dirname);
    g_free(dirname);
    bind_textdomain_codeset ("judoshiai", "UTF-8");
    textdomain ("judoshiai");

    change_menu_label(tournament_menu_item , _("Tournament"));
    change_menu_label(competitors_menu_item, _("Competitors"));
    change_menu_label(categories_menu_item , _("Categories"));

    change_menu_label(drawing_menu_item    , _("Drawing"));
    change_menu_label(results_menu_item    , _("Results"));
    change_menu_label(judotimer_menu_item  , _("Judotimer"));
    change_menu_label(preferences_menu_item, _("Preferences"));
    change_menu_label(help_menu_item       , _("Help"));

    change_menu_label(tournament_new       , _("New Tournament"));
    change_menu_label(tournament_choose    , _("Open Tournament"));
    change_menu_label(tournament_choose_net, _("Open Tournament from Net"));
    change_menu_label(tournament_properties, _("Properties"));
    change_menu_label(tournament_backup    , _("Tournament Backup"));
    change_menu_label(sql_dialog,            _("SQL Dialog"));
    change_menu_label(tournament_quit      , _("Quit"));

    change_menu_label(competitor_new                 , _("New Competitor"));
    change_menu_label(competitor_search              , _("Search Competitor"));
    change_menu_label(competitor_select_from_tournament , _("Select From Another Shiai"));
    change_menu_label(competitor_add_from_text_file  , _("Add From a Text File"));
    change_menu_label(competitor_add_all_from_shiai  , _("Add All From Another Shiai"));
    change_menu_label(competitor_add_all_with_weights, _("Add All with Weights"));
    change_menu_label(competitor_remove_unweighted   , _("Remove Unweighted"));
    change_menu_label(competitor_restore_removed     , _("Restore Removed"));
    change_menu_label(competitor_bar_code_search     , _("Bar Code Search"));
    change_menu_label(competitor_print_weigh_notes   , _("Print Weight Notes"));

    change_menu_label(category_new            , _("New Category"));
    change_menu_label(category_remove_empty   , _("Remove Empty"));
    change_menu_label(category_create_official, _("Create Categories"));
    change_menu_label(category_print_all      , _("Print All"));
    change_menu_label(category_print_all_pdf  , _("Print All (PDF)"));
    change_menu_label(category_print_matches  , _("Print Matches (CSV)"));
    change_menu_label(category_properties     , _("Properties"));
    change_menu_label(category_best_of_three  , _("Best of Three"));

    for (i = 0; i < NUM_TATAMIS; i++) {
        sprintf(buf, "%s %d %s", _("Place To"), i+1, _("Tatamis"));
        change_menu_label(category_to_tatamis[i], buf);
    }

    change_menu_label(draw_all_categories, _("Draw All Categories"));
    change_menu_label(draw_international,  _("International System"));
    change_menu_label(draw_finnish,        _("Finnish System"));
    change_menu_label(draw_swedish,        _("Swedish System"));
    change_menu_label(draw_estonian,       _("Estonian System"));
    change_menu_label(draw_spanish,        _("Spanish System"));

    change_menu_label(results_print_all             , _("Print All (Web And PDF)"));
    change_menu_label(results_print_schedule_printer, _("Print Schedule to Printer"));
    change_menu_label(results_print_schedule_pdf    , _("Print Schedule to PDF"));

    for (i = 0; i < NUM_TATAMIS; i++) {
        sprintf(buf, "%s %d", _("Control Tatami"), i+1);
        change_menu_label(judotimer_control[i], buf);
    }

    change_menu_label(preference_comm                  , _("Communication"));
    change_menu_label(preference_comm_node             , _("Communication Node"));
    change_menu_label(preference_own_ip_addr           , _("Own IP Addresses"));
    change_menu_label(preference_show_connections      , _("Show Attached Connections"));
    change_menu_label(preference_auto_sheet_update     , _("Automatic Sheet Update"));
    //change_menu_label(preference_auto_web_update       , _("Automatic Web Page Update"));

    change_menu_label(preference_langsel               , _("Language Selection"));
    change_menu_label(preference_results_in_finnish    , _("Results in Finnish"));
    change_menu_label(preference_results_in_swedish    , _("Results in Swedish"));
    change_menu_label(preference_results_in_english    , _("Results in English"));
    change_menu_label(preference_results_in_spanish    , _("Results in Spanish"));

    change_menu_label(preference_club_text             , _("Club Text Selection"));
    change_menu_label(preference_club_text_club        , _("Club Name Only"));
    change_menu_label(preference_club_text_country     , _("Country Name Only"));
    change_menu_label(preference_club_text_both        , _("Both Club and Country"));
    change_menu_label(preference_club_text_abbr        , _("Abbreviations in Sheets"));

    change_menu_label(preference_layout                , _("Sheet Layout"));
    change_menu_label(preference_weights_to_pool_sheets, _("Weights Visible"));
    change_menu_label(preference_grade_visible         , _("Grade Visible"));
    change_menu_label(preference_sheet_font            , _("Sheet Font"));
    change_menu_label(preference_password              , _("Password"));
    change_menu_label(preference_mirror                , _("Mirror Tatami Order"));
    change_menu_label(preference_auto_arrange          , _("Automatic Match Delay"));
    change_menu_label(preference_use_logo              , _("Print Logo"));

    change_menu_label(help_manual, _("Manual"));
    change_menu_label(help_about , _("About"));

    set_competitors_col_titles();
    set_sheet_titles();
    set_match_col_titles();
    set_log_col_titles();
    set_cat_graph_titles();
    set_match_graph_titles();

    g_key_file_set_integer(keyfile, "preferences", "language", language);

    return TRUE;
}
