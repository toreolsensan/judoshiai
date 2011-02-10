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

#define switch_comp(_a, _b) do {                \
        struct comp_s tmp = comp[_a];           \
        comp[_a] = comp[_b];                    \
        comp[_b] = tmp;                         \
    } while (0)

enum draw_dialog_buttons {
    BUTTON_NEXT = 1000,
    BUTTON_REST,
    BUTTON_SHOW
};

guint french_matches_blue[32] = {
    1, 17,  9, 25, 5, 21, 13, 29, 3, 19, 11, 27, 7, 23, 15, 31, 
    2, 18, 10, 26, 6, 22, 14, 30, 4, 20, 12, 28, 8, 24, 16, 32
};
guint french_matches_white_offset[NUM_FRENCH] = {4, 8, 16, 32};
guint french_mul[NUM_FRENCH] = {8, 4, 2, 1};

#define MAX_MATES 100000

struct mdata {
    struct mcomp {
        gint index;
        gint pos;
        gint seeded;
        gint num_mates;
        gchar *club;
        gchar *country;
        GtkWidget *eventbox, *label;
    } mcomp[65];

    struct mpos {
        gint judoka;
        GtkWidget *eventbox, *label;
    } mpos[65];

    gint mjudokas, mpositions, mcategory_ix;
    gint selected, mfrench_sys;
    gboolean hidden;
    gint drawn, seeded1, seeded2, seeded3;
    gint sys;
};

GtkWidget *draw_one_category_manually_1(GtkTreeIter *parent, gint competitors, 
                                        struct mdata *mdata);
static void make_manual_mathes_callback(GtkWidget *widget, 
                                        GdkEvent *event,
                                        void *data);
static gboolean select_competitor(GtkWidget *eventbox, GdkEventButton *event, void *param);
static gint get_competitor_number(gint pos, struct mdata *mdata);

#if 0
static gint get_free_pos(gint pos[], gint num_competitors, gint group)
{
    guint p = rand()%32;
    gint i, j, pos1 = group;
    while (p > 0) {
        pos1 += 4;
        if (pos1 >= num_competitors)
            pos1 = group;
        p--;
    }

    for (i = 0; i < 4; i++) {
        gint pos2 = pos1 + i;
        if (pos2 >= num_competitors)
            pos2 = (group+i)%4;
        for (j = 0; j < 16; j++) {
            if (pos[pos2] == -1)
                return pos2;
            pos2 += 4;
            if (pos2 >= num_competitors)
                pos2 = (group+i)%4;
        }
    }
    return -1;
}
#endif

static gint cmp_clubs(struct mcomp *m1, struct mcomp *m2) 
{
    gint result = 0;

    if (m1->club && m2->club &&
	m1->club[0] && m2->club[0] &&
	strcmp(m1->club, m2->club) == 0)
	result |= CLUB_TEXT_CLUB;
    
    if (m1->country && m2->country && 
	m1->country[0] && m2->country[0] && 
	strcmp(m1->country, m2->country) == 0)
	result |= CLUB_TEXT_COUNTRY;

    return result;
}

void draw_one_category(GtkTreeIter *parent, gint competitors)
{
    struct mdata *mdata;
    GtkWidget *dialog;

    mdata = malloc(sizeof(*mdata));
    memset(mdata, 0, sizeof(*mdata));
    mdata->hidden = TRUE;

    dialog = draw_one_category_manually_1(parent, competitors, mdata);

    if (dialog) {
        make_manual_mathes_callback(dialog, (gpointer)BUTTON_REST, mdata);
        make_manual_mathes_callback(dialog, (gpointer)GTK_RESPONSE_OK, mdata);
    }
}

void draw_one_category_manually(GtkTreeIter *parent, gint competitors)
{
    struct mdata *mdata;

    mdata = malloc(sizeof(*mdata));
    memset(mdata, 0, sizeof(*mdata));

    draw_one_category_manually_1(parent, competitors, mdata);
}

static void free_mdata(struct mdata *mdata)
{
    gint i;
    for (i = 1; i <= 64; i++) {
        g_free(mdata->mcomp[i].club);
        g_free(mdata->mcomp[i].country);
    }
    free(mdata);
}

static gint last_selected = 0;

static gint get_next_comp(struct mdata *mdata)
{
    gint i, seed, x = (rand()%mdata->mjudokas) + 1;
    gint highest_val = -1, highest_num = 0, same_club = 0, same_country = 0;
    
    // find seeded in order
    for (seed = 1; seed <= 4; seed++) {
        for (i = 0; i < mdata->mjudokas; i++) {
            if (mdata->mcomp[x].pos == 0 && 
                mdata->mcomp[x].seeded == seed)
                return x;

            if (++x > mdata->mjudokas)
                x = 1;
        }
    }

    // find ordinary competitors
    for (i = 0; i < mdata->mjudokas; i++) {
	if (mdata->mcomp[x].pos == 0) {
	    if (mdata->mcomp[x].num_mates > highest_val) {
		highest_val = mdata->mcomp[x].num_mates;
		highest_num = x;
		same_club = 0;
		same_country = 0;
	    } else if (mdata->mcomp[x].num_mates == highest_val &&
		       last_selected) {
		gint c = cmp_clubs(&mdata->mcomp[x], &mdata->mcomp[last_selected]);
		if (c & CLUB_TEXT_CLUB)
		    same_club = x;
		if (c & CLUB_TEXT_COUNTRY)
		    same_country = x;
	    }
	}
	if (++x > mdata->mjudokas)
	    x = 1;
    }

    if (same_club)
	last_selected = same_club;
    else if (same_country)
	last_selected = same_country;
    else
	last_selected = highest_num;

    return last_selected;
}

static gint get_section(gint num, struct mdata *mdata)
{
    gint i, sec_siz = mdata->mpositions/4;
    gint sec_max = sec_siz;

    // for pool & double pool section is competitor number
    if (mdata->mfrench_sys < 0)
        return (num - 1);

    // for french system section is A, B, C, or D.
    for (i = 0; i < 4; i++) {
        if (num <= sec_max)
            return i;
        sec_max += sec_siz;
    }
    //g_print("%s:%d: num=%d\n", __FUNCTION__, __LINE__, num);
    return 0;
}

static gint club_mate_in_range(gint start, gint len, struct mdata *mdata)
{
    gint i, result = 0, cmp;

    for (i = start; i < start+len; i++) {
        gint other = mdata->mpos[i].judoka;
        if (other == 0)
            continue;

        if ((cmp = cmp_clubs(&mdata->mcomp[mdata->selected], &mdata->mcomp[other]))) {
            gint val = 0;

	    if (cmp & CLUB_TEXT_CLUB)
		val++;
	    if (cmp & CLUB_TEXT_COUNTRY)
		val++;
            if (mdata->mcomp[other].seeded)
                val++;

            if (val > result)
                result = val;
        }
    }
    return result;
}

static void calc_place_values(gint *place_values, struct mdata *mdata)
{
    gint num, cnt, x, j, s;
    gint q1 = 0, q2 = 0, q3 = 0, q4 = 0;

    switch (mdata->mfrench_sys) {
    case FRENCH_8:  num = 8;  break;
    case FRENCH_16: num = 16; break;
    case FRENCH_32: num = 32; break;
    case FRENCH_64: num = 64; break;
    default: return;
    }        

    for (x = 1; x <= mdata->mpositions-1; x += 2) {
        if (mdata->mpos[x].judoka) {
            place_values[x+1]++;
            if (mdata->mcomp[mdata->mpos[x].judoka].seeded)
                place_values[x+1] += 5 - mdata->mcomp[mdata->mpos[x].judoka].seeded;
        }
        if (mdata->mpos[x+1].judoka) {
            place_values[x]++;
            if (mdata->mcomp[mdata->mpos[x+1].judoka].seeded)
                place_values[x] += 5 - mdata->mcomp[mdata->mpos[x+1].judoka].seeded;
        }
    }

    for (x = 1; x <= mdata->mpositions; x++) {
        if (mdata->mpos[x].judoka == 0)
            continue;

        if (x <= mdata->mpositions/4)
            q1++;
        else if (x <= mdata->mpositions/2)
            q2++;
        else if (x <= mdata->mpositions*3/4)
            q3++;
        else
            q4++;
    }

    if (q1 + q2 > q3 + q4) {
        for (x = 1; x <= mdata->mpositions/2; x++)
            place_values[x] += 2;
    } else if (q1 + q2 < q3 + q4) {
        for (x = mdata->mpositions/2 + 1; x <= mdata->mpositions; x++)
            place_values[x] += 2;
    }

    if (q1 > q2) {
        for (x = 1; x <= mdata->mpositions/4; x++)
            place_values[x]++;
    } else if (q1 < q2) {
        for (x = mdata->mpositions/4 + 1; x <= mdata->mpositions/2; x++)
            place_values[x]++;
    }

    if (q3 > q4) {
        for (x = mdata->mpositions/2 + 1; x <= mdata->mpositions*3/4; x++)
            place_values[x]++;
    } else if (q3 < q4) {
        for (x = mdata->mpositions*3/4 + 1; x <= mdata->mpositions; x++)
            place_values[x]++;
    }

    cnt = 2;
    while (cnt < num) {
        for (x = 1; x <= mdata->mpositions; x += cnt) {
            if ((s = club_mate_in_range(x, cnt, mdata))) {
                for (j = x; j < x+cnt; j++)
                    place_values[j] += 4*s;
            }
        }
        cnt *= 2;
    }

    /***
    g_print("\nplace values: ");
    for (x = 1; x <= mdata->mpositions; x++)
        g_print("%d=%d ", x, place_values[x]);
    g_print("\n");
    ***/
}

static gint get_free_pos_by_mask(gint mask, struct mdata *mdata)
{
    gint place_values[65], selected[64];
    gint i, x, valid_place = 0;
    gint min = mdata->mpositions, max = 1;
    gint best_value = 10000;

    if (mask == 0)
        return 0;

    if (mdata->mfrench_sys < 0) {
        // pool or double pool
        x = rand()%mdata->mpositions + 1;
        for (i = 1; i <= mdata->mpositions; i++) {
            gint currmask = 1<<(x-1);
            if ((currmask & mask) && mdata->mpos[x].judoka == 0)
                return x;

            if (++x > mdata->mpositions)
                x = 1;
        }

        //g_print("%s:%d: mask=%x\n", __FUNCTION__, __LINE__, mask);
        return 0;
    }

    for (i = 0; i < 4; i++) {
        if (mask & (1<<i)) {
            min = mdata->mpositions*i/4 + 1;
            break;
        }
    }

    for (i = 3; i >= 0; i--) {
        if (mask & (1<<i)) {
            max = mdata->mpositions*(i+1)/4;
            break;
        }
    }

    x = rand()%(max - min + 1) + min;

    memset(place_values, 0, sizeof(place_values));
    memset(selected, 0, sizeof(selected));
    calc_place_values(place_values, mdata);

    for (i = 1; i <= mdata->mpositions; i++) {
        gint currmask = 1<<get_section(x, mdata);
        if ((currmask & mask) && mdata->mpos[x].judoka == 0 
            /*XXX && get_competitor_number(x, mdata) <= mdata->mjudokas*/) {
            if (place_values[x] < best_value) {
                best_value = place_values[x];
                valid_place = x;
            }
        } else
            place_values[x] = 10001;

        if (++x > mdata->mpositions)
            x = 1;
    }

    if (valid_place == 0) {
        g_print("%s:%d: no valid position for competitor!\n",
                __FUNCTION__, __LINE__);
        return 0;
    }

    x = 0;
    for (i = 1; i <= mdata->mpositions; i++) {
        if (place_values[i] == best_value)
            selected[x++] = i;
    }

    i = rand()%x;
    valid_place = selected[i];

    return valid_place;
}

// return sections with seedings
static gint get_seeded_mask(gint mask, struct mdata *mdata)
{
    gint i, res = 0, seed = 0;

    for (seed = 0; seed < 4; seed++) {
        if ((mask & (1<<seed)) == 0)
            continue;

        for (i = 1; i <= mdata->mpositions; i++) {
            if (mdata->mpos[i].judoka &&
                mdata->mcomp[mdata->mpos[i].judoka].seeded == seed + 1) {
                res |= 1<<get_section(i, mdata);
            }
        }
    }

    return res;
}

static gint get_club_mask(struct mdata *mdata)
{
    gint i, res = 0;
	
    if (mdata->selected == 0)
        return 0;

    for (i = 1; i <= mdata->mpositions; i++) {
        gint comp = mdata->mpos[i].judoka;
        if (comp && 
            cmp_clubs(&mdata->mcomp[comp], &mdata->mcomp[mdata->selected])) {
            res |= 1<<get_section(i, mdata);
        }
    }

    return res;
}

static gint get_club_only_mask(struct mdata *mdata)
{
    gint i, res = 0;
	
    if (mdata->selected == 0)
        return 0;

    for (i = 1; i <= mdata->mpositions; i++) {
        gint comp = mdata->mpos[i].judoka;
        if (comp && 
            (cmp_clubs(&mdata->mcomp[comp], &mdata->mcomp[mdata->selected]) & CLUB_TEXT_CLUB)) {
            res |= 1<<get_section(i, mdata);
        }
    }

    return res;
}

#if 0
static gint get_competitor_group(gint comp, struct mdata *mdata)
{
    gint pos = mdata->mcomp[comp].pos;
    return ((pos - 1)*4)/mdata->mpositions;
}
#endif

gint get_drawed_number(gint pos, gint sys)
{
    if (sys >= 0 && (pos&1))
        return french_matches_blue[(pos-1)/2 * french_mul[sys]];
    else if (sys >= 0 && (pos&1) == 0)
        return french_matches_blue[(pos-1)/2 * french_mul[sys]] 
            + french_matches_white_offset[sys];
    return pos;
}

static gint get_competitor_number(gint pos, struct mdata *mdata)
{
    return get_drawed_number(pos, mdata->mfrench_sys);
}

static gboolean select_number(GtkWidget *eventbox, GdkEventButton *event, void *param)
{
    GdkColor bg;
    gchar buf[200];
    gint num = 0, i, nxt;
    struct mdata *mdata = param;

    for (i = 1; i <= 64; i++)
        if (eventbox == mdata->mpos[i].eventbox) {
            num = i;
            break;
        }

    if (num && mdata->mpos[num].judoka) {
        gdk_color_parse("#000000", &bg); 
        gtk_widget_modify_fg(mdata->mcomp[mdata->mpos[num].judoka].label, 
                             GTK_STATE_NORMAL, &bg);
        sprintf(buf, "%2d.", get_competitor_number(num, mdata));
        gtk_label_set_text(GTK_LABEL(mdata->mpos[num].label), buf);
        mdata->mcomp[mdata->mpos[num].judoka].pos = 0;
        mdata->mpos[num].judoka = 0;

        if (mdata->drawn > 0)
            mdata->drawn--;

        return TRUE;
    }

    if (mdata->selected == 0 || num == 0) {
        g_print("error %d\n", __LINE__);
        return FALSE;
    }

    for (i = 1; i <= mdata->mpositions; i++) {
        if (mdata->mpos[i].judoka == mdata->selected) {
            mdata->mpos[i].judoka = 0;
            sprintf(buf, "%2d.", get_competitor_number(i, mdata));
            gtk_label_set_text(GTK_LABEL(mdata->mpos[i].label), buf);
        }
    }

    mdata->mpos[num].judoka = mdata->selected;
    mdata->mcomp[mdata->selected].pos = num;
        
    snprintf(buf, sizeof(buf), "%2d. %s", get_competitor_number(num, mdata), 
             gtk_label_get_text(GTK_LABEL(mdata->mcomp[mdata->selected].label)));
    gtk_label_set_text(GTK_LABEL(mdata->mpos[num].label), buf);

    gdk_color_parse("#7F7F7F", &bg); 
    gtk_widget_modify_fg(mdata->mcomp[mdata->selected].label, GTK_STATE_NORMAL, &bg);
    gdk_color_parse("#FFFFFF", &bg); 
    gtk_widget_modify_bg(mdata->mcomp[mdata->selected].eventbox, GTK_STATE_NORMAL, &bg);

    nxt = get_next_comp(mdata);
    if (nxt)
        select_competitor(mdata->mcomp[nxt].eventbox, NULL, mdata);
    //g_signal_emit_by_name(mdata->mcomp[nxt].eventbox, "button_press_event", NULL, &retval);
    else
        mdata->selected = 0;

    return TRUE;
}

static gboolean select_competitor(GtkWidget *eventbox, GdkEventButton *event, void *param)
{
    guint index = 0;
    GdkColor bg;
    gint i;
    struct mdata *mdata = param;

    for (i = 1; i <= 64; i++)
        if (eventbox == mdata->mcomp[i].eventbox) {
            index = i;
            break;
        }

    if (index == 0) {
        g_print("error %d\n", __LINE__);
        return FALSE;
    }

    gdk_color_parse("#FFFFFF", &bg); 
    for (i = 1; i <= mdata->mjudokas; i++)
        gtk_widget_modify_bg(mdata->mcomp[i].eventbox, GTK_STATE_NORMAL, &bg);

    gdk_color_parse("#E0E0FF", &bg); 
    gtk_widget_modify_bg(mdata->mcomp[index].eventbox, GTK_STATE_NORMAL, &bg);
    mdata->selected = index;

    return TRUE;
}

static gint seedposq[21] = {0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0x1fe, 0x3f6, // 0 - 10
                            0x7b6, 0xdb6, 0x1b66, 0x3666, 0x6666, 0x6666, 0xccd2, 0x19a52, 0x34a52, 0x94a52}; // 11 - 20

static gboolean draw_one_comp(struct mdata *mdata)
{
    gint comp = mdata->selected;
    gint found = 0, mask;
    gint sys = mdata->sys & SYSTEM_MASK;

    if (mdata->selected == 0)
        return 0;

    if (mdata->mfrench_sys >= 0) {
        if (draw_system == DRAW_SPANISH &&
            mdata->mcomp[comp].seeded > 0 && mdata->mcomp[comp].seeded <= 4) {
            gint x;
            switch (mdata->mcomp[comp].seeded) {
            case 1: x = 0; break;
            case 2: x = 2; break;
            case 3: x = 1; break;
            case 4: x = 3; break;
            }
            found = mdata->mpositions*(x)/4 + 1;

            if (mdata->mpos[found].judoka)
                found = 0;
        }

        if (!found) {
            if (mdata->mcomp[comp].seeded == 2) {
                mask = get_seeded_mask(1, mdata); // look for 1. seeded
                if (mask & 0x3)
                    found = get_free_pos_by_mask((mask ^ 0xc) & 0xc, mdata);
                else
                    found = get_free_pos_by_mask(0x3, mdata);
            } else if (mdata->mcomp[comp].seeded == 3) {
                mask = get_seeded_mask(2, mdata); // look for 2. seeded
                if (mask & 0x3)
                    found = get_free_pos_by_mask(mask ^ 0x3, mdata);
                else 
                    found = get_free_pos_by_mask(mask ^ 0xc, mdata);
            } else if (mdata->mcomp[comp].seeded == 4) {
                mask = get_seeded_mask(1, mdata); // look for 3. seeded
                if (mask & 0x3)
                    found = get_free_pos_by_mask(mask ^ 0x3, mdata);
                else 
                    found = get_free_pos_by_mask(mask ^ 0xc, mdata);
            }
        }
        if (!found)
            found = get_free_pos_by_mask(0xf, mdata);
    } else if (sys != SYSTEM_QPOOL) {
        gint getmask;

        if (mdata->mcomp[comp].seeded) {
            mask = get_seeded_mask(0xf, mdata); // look for any seeded

            switch (mdata->mpositions) {
            case 2:
                getmask = 0x3; // 1 and 2
                break;
            case 3:
                getmask = 0x6; // 2 and 3
                break;
            case 4:
                getmask = 0x6; // 2 and 3
                break;
            case 5:
                getmask = 0x12; // 2 and 5
                break;
            case 6:
                if (mdata->drawn == 1 || mdata->drawn == 2 ) {
                    if (mdata->seeded1 <= 3)
                        getmask = 0x30;
                    else 
                        getmask = 0x06;
                    break;
                } else
                    getmask = 0x36;
                break;
            case 7:
            case 8:
                if (mdata->drawn == 1 || mdata->drawn == 2 ) {
                    if (mdata->seeded1 <= 4)
                        getmask = 0x60;
                    else 
                        getmask = 0x06;
                    break;
                } else
                    getmask = 0x66;
                break;
            case 9:
                if (mdata->drawn == 1 || mdata->drawn == 2 ) {
                    if (mdata->seeded1 <= 5)
                        getmask = 0xc0;
                    else 
                        getmask = 0x12;
                    break;
                } else
                    getmask = 0xd2;
                break;
            case 10:
                if (mdata->drawn == 1 || mdata->drawn == 2 ) {
                    if (mdata->seeded1 <= 5)
                        getmask = 0x240;
                    else 
                        getmask = 0x12;
                    break;
                } else
                    getmask = 0x252;
                break;
            }
            found = get_free_pos_by_mask((mask ^ getmask) & getmask, mdata);
        } else {
            gint a_pool = 0, b_pool = 0, a_pool_club = 0, b_pool_club = 0, 
                a_mask = 0, b_mask = 0;
            mask = get_club_mask(mdata);
            gint clubmask = get_club_only_mask(mdata);

            if (mdata->mpositions >= 6) { // double pool
                gint i;
                for (i = 0; i < mdata->mpositions; i++) { // count mates in a and b pool
                    if (mask & (1<<i)) {
                        if (i < (mdata->mpositions + 1)/2)
                            a_pool++;
                        else
                            b_pool++;
                    }

                    if (clubmask & (1<<i)) {
                        if (i < (mdata->mpositions + 1)/2)
                            a_pool_club++;
                        else
                            b_pool_club++;
                    }

                    if (i < (mdata->mpositions + 1)/2)
                        a_mask |= 1<<i;
                    else
                        b_mask |= 1<<i;
                }
            }

            switch (mdata->mpositions) {
            case 2:
                getmask = 0x3;
                break;
            case 3:
                if (mask & 0x6)
                    getmask = 0x1;
                else
                    getmask = 0x7;
                break;
            case 4:
                if (mask & 0x6)
                    getmask = 0x9;
                else
                    getmask = 0xf;
                break;
            case 5:
                if (mask & 0x12)
                    getmask = 0x0d;
                else
                    getmask = 0x1f;
                break;
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
                if (a_pool_club < b_pool_club)
                    getmask = a_mask;
                else if (a_pool_club > b_pool_club)
                    getmask = b_mask;
                else if (a_pool < b_pool)
                    getmask = a_mask;
                else if (a_pool > b_pool)
                    getmask = b_mask;
                else
                    getmask = a_mask | b_mask;
                break;
            }
            found = get_free_pos_by_mask(getmask, mdata);
        }

        if (!found)
            found = get_free_pos_by_mask(0x3ff, mdata);
    } else { // QPOOL
        gint getmask;

        if (mdata->mcomp[comp].seeded) {
            mask = get_seeded_mask(0xfffff, mdata); // look for any seeded
            getmask = seedposq[mdata->mpositions];
            found = get_free_pos_by_mask((mask ^ getmask) & getmask, mdata);
        } else {
            gint i, j;
            gint mates[4], clubmates[4];
            gint clubmask = get_club_only_mask(mdata);
            gint pool_start[0];
            gint pool_size[4];
            gint pool_mask[4];
            gint size = mdata->mpositions/4;
            gint rem = mdata->mpositions - size*4;

            mask = get_club_mask(mdata);
            
            for (i = 0; i < 4; i++) {
                pool_size[i] = size;
                mates[i] = clubmates[i] = pool_mask[i] = 0;
            }

            for (i = 0; i < rem; i++)
                pool_size[i]++;

            gint start = 0;
            for (i = 0; i < 4; i++) {
                pool_start[i] = start;
                start += pool_size[i];
            }

            for (i = 0; i < mdata->mpositions; i++) { // count mates in pools
                if (mask & (1<<i)) {
                    for (j = 3; j >= 0; j--) {
                        if (i >= pool_start[j]) {
                            mates[j]++;
                            break;
                        }
                    }
                }

                if (clubmask & (1<<i)) {
                    for (j = 3; j >= 0; j--) {
                        if (i >= pool_start[j]) {
                            clubmates[j]++;
                            break;
                        }
                    }
                }

                for (j = 3; j >= 0; j--) {
                    if (i >= pool_start[j]) {
                        pool_mask[j] |= 1<<i;
                        break;
                    }
                }
            }

            gint min_mates = 10000, min_clubmates = 10000;
            gint min_mates_ix = 0, min_clubmates_ix = 0;
            for (i = 0; i < 4; i++) {
                if (mates[i] < min_mates) {
                    min_mates = mates[i];
                    min_mates_ix = i;
                }
                if (clubmates[i] < min_clubmates) {
                    min_clubmates = clubmates[i];
                    min_clubmates_ix = i;
                }
            }

            getmask = pool_mask[min_mates_ix];

            found = get_free_pos_by_mask(getmask, mdata);
        } // !seeded

        if (!found)
            found = get_free_pos_by_mask(0xfffff, mdata);
    } // QPOOL
    
    if (found)
        select_number(mdata->mpos[found].eventbox, NULL, mdata);

    mdata->drawn++;
    if (mdata->mcomp[comp].seeded && mdata->drawn == 1) 
        mdata->seeded1 = found;

    return (found > 0);
}

static void make_manual_mathes_callback(GtkWidget *widget, 
                                        GdkEvent *event,
                                        void *data)
{
    gint i;
    struct mdata *mdata = data;

    if ((gulong)event == BUTTON_NEXT) {
        draw_one_comp(mdata);
                
        return;
    }

    if ((gulong)event == BUTTON_REST) {
        while (draw_one_comp(mdata))
            ;
                
        return;
    }

    if ((gulong)event != GTK_RESPONSE_OK)
        goto out;

    /* check drawing is complete */
    for (i = 1; i <= mdata->mjudokas; i++) {
        if (mdata->mcomp[i].pos == 0) {
            show_message(_("Drawing not finished!"));
            return;
        }
    }

    if (mdata->mfrench_sys < 0) {
        for (i = 0; i < num_matches(mdata->sys, mdata->mjudokas); i++) {
            struct match m;
                
            memset(&m, 0, sizeof(m));
            m.category = mdata->mcategory_ix;
            m.number = i+1;
            if ((mdata->sys & SYSTEM_MASK) == SYSTEM_QPOOL) {
                m.blue = mdata->mcomp[ mdata->mpos[ poolsq[mdata->mjudokas][i][0] ].judoka ].index;
                m.white = mdata->mcomp[ mdata->mpos[ poolsq[mdata->mjudokas][i][1] ].judoka ].index;
            } else {
                m.blue = mdata->mcomp[ mdata->mpos[ pools[mdata->mjudokas][i][0] ].judoka ].index;
                m.white = mdata->mcomp[ mdata->mpos[ pools[mdata->mjudokas][i][1] ].judoka ].index;
            }

            set_match(&m);
            db_set_match(&m);
        }
    } else {
        for (i = 1; i < mdata->mpositions; i += 2) {
            struct match m;
            gint b, w;
                
            memset(&m, 0, sizeof(m));
            m.category = mdata->mcategory_ix;
            m.number = ((i - 1)/2) + 1;
            b = mdata->mpos[i].judoka;
            w = mdata->mpos[i+1].judoka;
            m.blue = b ? mdata->mcomp[b].index : GHOST;
            m.white = w ? mdata->mcomp[w].index : GHOST;

            set_match(&m);
            db_set_match(&m);
        }
    }

    db_set_system(mdata->mcategory_ix, mdata->sys);
out:

    update_category_status_info(mdata->mcategory_ix);
    update_matches(mdata->mcategory_ix, 0, 0);

    if (mdata->hidden == FALSE && (gulong)event == GTK_RESPONSE_OK)
        category_window(mdata->mcategory_ix);

    free_mdata(mdata);

    if (widget)
        gtk_widget_destroy(widget);
}

gint get_system_for_category(gint index, gint competitors)
{
    gint systm = get_cat_system(index);
    gint wish = systm & SYSTEM_WISH_MASK;
    gint wishsys = wish >> SYSTEM_WISH_SHIFT;
    struct category_data *cat = NULL;
    gint sys = 0;

    if (systm & SYSTEM_MASK)
        return systm;

    if (competitors <= 5 && (wishsys == CAT_SYSTEM_POOL || wishsys == CAT_SYSTEM_DEFAULT)) {
        sys = wish | SYSTEM_POOL | competitors;
    } else if (competitors > 5 && competitors <= 10 && wishsys == CAT_SYSTEM_DPOOL) {
        sys = wish | SYSTEM_DPOOL | competitors;
    } else if (competitors >= 8 && competitors <= 20 && wishsys == CAT_SYSTEM_QPOOL) {
        sys = wish | SYSTEM_QPOOL | competitors;
    } else if (competitors > 5 && competitors <= 10 && wishsys == CAT_SYSTEM_DEFAULT &&
               draw_system == DRAW_SPANISH) {
            sys = wish | SYSTEM_DPOOL | competitors;
    } else if (competitors > 5 && competitors <= 7 && wishsys == CAT_SYSTEM_DEFAULT) {
	if (draw_system == DRAW_INTERNATIONAL || draw_system == DRAW_ESTONIAN) {
            if (draw_system == DRAW_INTERNATIONAL)
                wish |= (cat_system_to_table[CAT_IJF_DOUBLE_REPECHAGE]) << SYSTEM_TABLE_SHIFT;
            sys = wish | SYSTEM_FRENCH_8 | competitors;
	} else {
            sys = wish | SYSTEM_DPOOL | competitors;
	}
    } else if (competitors > 16 && wishsys == CAT_ESP_DOBLE_PERDIDA) {
        wish |= (cat_system_to_table[CAT_ESP_REPESCA_DOBLE]) << SYSTEM_TABLE_SHIFT;
        if (competitors <= 32)
            sys = wish | SYSTEM_FRENCH_32 | competitors;
        else
            sys = wish | SYSTEM_FRENCH_64 | competitors;
    } else {
        if (wishsys == CAT_SYSTEM_DEFAULT) {
            switch (draw_system) {
            case DRAW_INTERNATIONAL:
                wishsys = CAT_IJF_DOUBLE_REPECHAGE;
                break;
            case DRAW_FINNISH:
                break;
            case DRAW_SWEDISH:
                wishsys = CAT_SYSTEM_DIREKT_AATERKVAL;
                break;
            case DRAW_ESTONIAN:
                /* double elimination is used for under 11 years old */
                cat = avl_get_category(index);
                if (cat) {
                    gint aix = find_age_index(cat->category);
                    if (aix >= 0) {
                        if (category_definitions[aix].age <= 10)
                            wishsys = CAT_SYSTEM_EST_D_KLASS;
                    }
                }
                break;
            case DRAW_SPANISH:
                wishsys = CAT_ESP_REPESCA_DOBLE;
                cat = avl_get_category(index);
                if (cat) {
                    gint aix = find_age_index(cat->category);
                    if (aix >= 0 && competitors <= 16) {
                        if (category_definitions[aix].age <= 10)
                            wishsys = CAT_ESP_DOBLE_PERDIDA;
                    }
                }
            }
        }

        wish |= (cat_system_to_table[wishsys]) << SYSTEM_TABLE_SHIFT;

        if (competitors <= 8) {
            sys = wish | SYSTEM_FRENCH_8 | competitors;
        } else if (competitors <= 16) {
            sys = wish | SYSTEM_FRENCH_16 | competitors;
        } else if (competitors <= 32) {
            sys = wish | SYSTEM_FRENCH_32 | competitors;
        } else {
            sys = wish | SYSTEM_FRENCH_64 | competitors;
        }
    }
    return sys;
}

GtkWidget *draw_one_category_manually_1(GtkTreeIter *parent, gint competitors, 
					struct mdata *mdata)
{
    gint i;
    GtkWidget *dialog = NULL, *hbox, *vbox1, *vbox2, *label, *eventbox;
    GdkColor bg, bg1, bg2;
    gchar *catname = NULL;
    gchar buf[200];

    mdata->drawn = 0;
    mdata->mjudokas = competitors;

    // colours for the tables
    gdk_color_parse("#FFFFFF", &bg); 
    gdk_color_parse("#AFAF9F", &bg1); 
    gdk_color_parse("#AFAFBF", &bg2); 

    // find the index of the category
    gtk_tree_model_get(current_model, parent,
                       COL_INDEX, &(mdata->mcategory_ix),
                       COL_LAST_NAME, &catname,
                       -1);

    mdata->sys = get_system_for_category(mdata->mcategory_ix, competitors);

    switch (mdata->sys & SYSTEM_MASK) {
    case SYSTEM_POOL:
    case SYSTEM_DPOOL:
    case SYSTEM_QPOOL:
        mdata->mpositions = competitors;
        mdata->mfrench_sys = -1;
        break;
    case SYSTEM_FRENCH_8:
        mdata->mpositions = 8;
        mdata->mfrench_sys = FRENCH_8;
        break;
    case SYSTEM_FRENCH_16:
        mdata->mpositions = 16;
        mdata->mfrench_sys = FRENCH_16;
        break;
    case SYSTEM_FRENCH_32:
        mdata->mpositions = 32;
        mdata->mfrench_sys = FRENCH_32;
        break;
    case SYSTEM_FRENCH_64:
        mdata->mpositions = 64;
        mdata->mfrench_sys = FRENCH_64;
        break;
    }

    if (catname && catname[0] == '?') {
        //SHOW_MESSAGE("Cannot draw %s", catname);
        g_free(catname);
        free(mdata);
        return NULL;
    }

    g_free(catname);

    if (db_category_match_status(mdata->mcategory_ix) & MATCH_EXISTS) {
        // Cannot draw again.
#if 0
        struct judoka *j = get_data(mdata->mcategory_ix);
        SHOW_MESSAGE("%s %s. %s.",
                     _("Matches matched in category"),
                     j ? j->last : "?", _("Clear the results first"));
        free_judoka(j);
#endif
        free(mdata);
        return dialog;
    }

    // Clean up the database.
    db_set_system(mdata->mcategory_ix, get_cat_system(mdata->mcategory_ix)&SYSTEM_WISH_MASK);
    db_remove_matches(mdata->mcategory_ix);

    // Get data of all the competitors.
    for (i = 0; i < competitors; i++) {
        guint index;
        gchar *club, *country;
        gint deleted;
        GtkTreeIter iter;
        struct judoka *j;

        gtk_tree_model_iter_nth_child(current_model,
                                      &iter,
                                      parent,
                                      i);                
        gtk_tree_model_get(current_model, &iter,
                           COL_INDEX, &index,
                           COL_CLUB, &club,
                           COL_COUNTRY, &country,
                           COL_DELETED, &deleted,
                           -1);

        j = get_data(index);
	if (j && j->club && j->club[0] && j->country && j->country[0])
	    snprintf(buf, sizeof(buf), "%s %s, %s/%s", 
		     j->first, j->last, j->club, j->country);
	else if (j && j->club && j->club[0])
	    snprintf(buf, sizeof(buf), "%s %s, %s", 
		     j->first, j->last, j->club);
	else if (j && j->country && j->country[0])
	    snprintf(buf, sizeof(buf), "%s %s, %s", 
		     j->first, j->last, j->country);
	else
	    snprintf(buf, sizeof(buf), "???");

        mdata->mcomp[i+1].label = gtk_label_new(buf);
        mdata->mcomp[i+1].index = index;
        mdata->mcomp[i+1].club = club;
        mdata->mcomp[i+1].country = country;
        mdata->mcomp[i+1].seeded = deleted>>2;
        free_judoka(j);
    }
        
    // Create the dialog.
    dialog = gtk_dialog_new_with_buttons (_("Move the competitors"), //XXXX
                                          NULL,
                                          GTK_DIALOG_DESTROY_WITH_PARENT,
                                          _("Next"), BUTTON_NEXT,
                                          _("Rest"), BUTTON_REST,
                                          GTK_STOCK_OK, GTK_RESPONSE_OK,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          NULL);
    gtk_widget_set_size_request(GTK_WIDGET(dialog), FRAME_WIDTH, FRAME_HEIGHT);
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);

    hbox = gtk_hbox_new(FALSE, 5);
    vbox1 = gtk_vbox_new(FALSE, 5);
    vbox2 = gtk_vbox_new(FALSE, 5);

    gtk_container_set_border_width(GTK_CONTAINER(vbox1), 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox2), 10);

    gtk_box_pack_start_defaults(GTK_BOX(hbox), vbox2);
    gtk_box_pack_start_defaults(GTK_BOX(hbox), vbox1);

    gtk_box_pack_start(GTK_BOX(vbox1), gtk_label_new(_("Number")), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox2), gtk_label_new(_("Competitor")), FALSE, FALSE, 0);

    // Create the positions table.
    for (i = 1; i <= mdata->mpositions; i++) {
        sprintf(buf, "%2d.                    ", get_competitor_number(i, mdata));
        mdata->mpos[i].eventbox = eventbox = gtk_event_box_new();
        mdata->mpos[i].label = label = gtk_label_new(buf);
        gtk_container_add(GTK_CONTAINER(eventbox), label);
        gtk_box_pack_start(GTK_BOX(vbox1), eventbox, FALSE, FALSE, 0);
        g_signal_connect(G_OBJECT(eventbox), "button_press_event",
                         G_CALLBACK(select_number), mdata);
        g_object_set(label, "xalign", 0.0, NULL);

        if ((mdata->mfrench_sys < 0 && i > (mdata->mjudokas - mdata->mjudokas/2)) ||
            (mdata->mfrench_sys == 0 && i > 2 && i <= 4) ||
            (mdata->mfrench_sys == 0 && i > 6) ||
            (mdata->mfrench_sys == 1 && i > 4 && i <= 8) ||
            (mdata->mfrench_sys == 1 && i > 12) ||
            (mdata->mfrench_sys == 2 && i > 8 && i <= 16) ||
            (mdata->mfrench_sys == 2 && i > 24) ||
            (mdata->mfrench_sys == 3 && i > 16 && i <= 32) ||
            (mdata->mfrench_sys == 3 && i > 48))
            gtk_widget_modify_bg(eventbox, GTK_STATE_NORMAL, &bg1);
        else
            gtk_widget_modify_bg(eventbox, GTK_STATE_NORMAL, &bg2);

        if (mdata->mfrench_sys >= 0 && i < mdata->mpositions) {
            if ((i & 1) == 0)
                gtk_box_pack_start(GTK_BOX(vbox1), 
                                   gtk_hseparator_new(), FALSE, FALSE, 0);
            if ((i & 3) == 0)
                gtk_box_pack_start(GTK_BOX(vbox1), 
                                   gtk_hseparator_new(), FALSE, FALSE, 0);
            if ((i & 7) == 0)
                gtk_box_pack_start(GTK_BOX(vbox1), 
                                   gtk_hseparator_new(), FALSE, FALSE, 0);
        }
    }        

    // find club mates of seeded
    for (i = 1; i <= mdata->mjudokas; i++)
        if (mdata->mcomp[i].seeded) {
            gint j;

            mdata->mcomp[i].num_mates = MAX_MATES - mdata->mcomp[i].seeded;

            for (j = 1; j <= mdata->mjudokas; j++) {
                if (i == j)
                    continue;

                if (cmp_clubs(&mdata->mcomp[i], &mdata->mcomp[j]))
                    mdata->mcomp[j].num_mates = 
                        MAX_MATES - mdata->mcomp[i].seeded;
            }
        }


    // find club mates
    for (i = 1; i <= mdata->mjudokas; i++) {
        gint j, r;

        if (mdata->mcomp[i].num_mates)
            continue;

        for (j = 1; j <= mdata->mjudokas; j++) {
            if (i == j)
                continue;

            if ((r = cmp_clubs(&mdata->mcomp[i], &mdata->mcomp[j]))) {
                if (r & CLUB_TEXT_COUNTRY)
                    mdata->mcomp[i].num_mates += 8;
                if (r & CLUB_TEXT_CLUB)
                    mdata->mcomp[i].num_mates++;
            }
        }
    }

    // Create competitor table.
    for (i = 1; i <= mdata->mjudokas; i++) {
        mdata->mcomp[i].eventbox = eventbox = gtk_event_box_new();
        label = mdata->mcomp[i].label;
        gtk_container_add(GTK_CONTAINER(eventbox), label);
        gtk_box_pack_start(GTK_BOX(vbox2), eventbox, FALSE, FALSE, 0);
        g_signal_connect(G_OBJECT(eventbox), "button_press_event",
                         G_CALLBACK(select_competitor), mdata);
                
        gtk_widget_modify_bg(eventbox, GTK_STATE_NORMAL, &bg);
        g_object_set(label, "xalign", 0.0, NULL);
    }        

    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), 
                       gtk_label_new(_("Select first a competitor and then a number.")),
                       FALSE, FALSE, 0);

    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    //gtk_container_set_border_width(GTK_CONTAINER(scrolled_window), 10);
    gtk_scrolled_window_add_with_viewport(
        GTK_SCROLLED_WINDOW(scrolled_window), hbox);

    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), scrolled_window,
                       TRUE, TRUE, 0);

    if (mdata->hidden == FALSE)
        gtk_widget_show_all(dialog);

    g_signal_connect(G_OBJECT(dialog), "response",
                     G_CALLBACK(make_manual_mathes_callback), mdata);

    last_selected = 0;
    gboolean retval = FALSE;
    g_signal_emit_by_name(mdata->mcomp[get_next_comp(mdata)].eventbox, "button_press_event", 
                          NULL, &retval);

    return dialog;
}

void draw_all(GtkWidget *w, gpointer data)
{
    GtkTreeViewColumn *col;
    GtkTreeIter iter;
    gboolean ok;
    gint i;

    gint num_cats = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(current_model), NULL);
    gint cnt = 0;
    gboolean retval = FALSE;

    gdk_window_set_cursor(GTK_WIDGET(main_window)->window, wait_cursor);

    col = gtk_tree_view_get_column(GTK_TREE_VIEW(current_view), 0);
    g_signal_emit_by_name(col, "clicked", NULL, &retval);

    ok = gtk_tree_model_get_iter_first(current_model, &iter);
    while (ok) {
        gint n = gtk_tree_model_iter_n_children(current_model, &iter);
        if (n >= 1 && n <= 64)
            draw_one_category(&iter, n);

        cnt++;
        if (num_cats > 4)
            progress_show(1.0*cnt/num_cats, "");

        ok = gtk_tree_model_iter_next(current_model, &iter);
    }

    gdk_window_set_cursor(GTK_WIDGET(main_window)->window, NULL);
    SYS_LOG_INFO(_("All the categories has been drawn"));

    for (i = 1; i <= NUM_TATAMIS; i++) {
        progress_show(1.0*i/NUM_TATAMIS, "");
        update_matches(0, 0, i);
    }

    progress_show(0.0, "");
}

