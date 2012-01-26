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

guint french_matches_blue[64] = {
    1, 33, 17, 49,  9, 41, 25, 57,  5, 37, 21, 53, 13, 45, 29, 61, 3, 35, 19, 51, 11, 43, 27, 59, 7, 39, 23, 55, 15, 47, 31, 63, 
    2, 34, 18, 50, 10, 42, 26, 58,  6, 38, 22, 54, 14, 46, 30, 62, 4, 36, 20, 52, 12, 44, 28, 60, 8, 40, 24, 56, 16, 48, 32, 64
};
guint french_matches_white_offset[NUM_FRENCH] = {4, 8, 16, 32, 64};
guint french_mul[NUM_FRENCH] = {16, 8, 4, 2, 1};

#define MAX_MATES 100000

struct mdata {
    struct mcomp {
        gint index;
        gint pos;
        gint seeded;
        gint clubseeded;
        gint num_mates;
        gchar *club;
        gchar *country;
        GtkWidget *eventbox, *label;
    } mcomp[NUM_COMPETITORS+1];

    struct mpos {
        gint judoka;
        GtkWidget *eventbox, *label;
    } mpos[NUM_COMPETITORS+1];

    gint mjudokas, mpositions, mcategory_ix;
    gint selected, mfrench_sys;
    gboolean hidden;
    gint drawn, seeded1, seeded2, seeded3;
    struct compsys sys;
};

GtkWidget *draw_one_category_manually_1(GtkTreeIter *parent, gint competitors, 
                                        struct mdata *mdata);
static void make_manual_matches_callback(GtkWidget *widget, 
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

    // club name is the same
    if (m1->club && m2->club &&
	m1->club[0] && m2->club[0] &&
	strcmp(m1->club, m2->club) == 0)
	result |= CLUB_TEXT_CLUB;
    
    // country name is the same
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
        make_manual_matches_callback(dialog, (gpointer)BUTTON_REST, mdata);
        make_manual_matches_callback(dialog, (gpointer)GTK_RESPONSE_OK, mdata);
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
    for (i = 1; i <= NUM_COMPETITORS; i++) {
        g_free(mdata->mcomp[i].club);
        g_free(mdata->mcomp[i].country);
    }
    free(mdata);
}

static gint last_selected = 0;

/* Competitors are drawed in the following order:
 * - seeded. 
 * - their clubmates.
 * - competitors from the biggest club.
 * - competitors from the same country.
 * - competitors from the 2nd biggest club.
 * - competitors from the same country.
 * - etc.
 */
static gint get_next_comp(struct mdata *mdata)
{
    gint i, seed, x = (rand()%mdata->mjudokas) + 1;
    gint highest_val = -1, highest_num = 0, same_club = 0, same_country = 0;
    gint club_seeding = 0, same_club_seeding = 0, same_country_seeding = 0;
    
    // find seeded in order
    for (seed = 1; seed <= NUM_SEEDED; seed++) {
        for (i = 0; i < mdata->mjudokas; i++) {
            if (mdata->mcomp[x].pos == 0 && 
                mdata->mcomp[x].seeded == seed)
                return x;

            if (++x > mdata->mjudokas)
                x = 1;
        }
    }

    // find ordinary competitors
    // find from the same club as previous
    if (last_selected) {
        for (i = 1; i <= mdata->mjudokas; i++) {
            if (mdata->mcomp[x].pos == 0) {
                    gint c = cmp_clubs(&mdata->mcomp[x], &mdata->mcomp[last_selected]);
                    if (c & CLUB_TEXT_CLUB)
                        if ((same_club_seeding && mdata->mcomp[x].clubseeded && 
                             same_club_seeding > mdata->mcomp[x].clubseeded) ||
                            (same_club_seeding == 0)) {
                            same_club_seeding = mdata->mcomp[x].clubseeded;
                            same_club = x;
                        }                        
                    if (c & CLUB_TEXT_COUNTRY)
                        if ((same_country_seeding && mdata->mcomp[x].clubseeded && 
                             same_country_seeding > mdata->mcomp[x].clubseeded) ||
                            (same_country_seeding == 0)) {
                            same_country_seeding = mdata->mcomp[x].clubseeded;
                            same_country = x;
                        }           
            }
            
            if (++x > mdata->mjudokas)
                x = 1;
        }
    }

    if (same_club) {
        last_selected = same_club;
        return same_club;
    }
    if (same_country) {
        last_selected = same_country;
        return same_country;
    }

    // find from the rest
    for (i = 0; i < mdata->mjudokas; i++) {
	if (mdata->mcomp[x].pos == 0) {
            // not yet drawed
	    if (mdata->mcomp[x].num_mates >= highest_val) {
                if (mdata->mcomp[x].num_mates > highest_val) {
                    // select from club who has the most competitors
                    highest_val = mdata->mcomp[x].num_mates;
                    highest_num = x;
                    club_seeding = mdata->mcomp[x].clubseeded;
                } 

                if ((club_seeding && mdata->mcomp[x].clubseeded && club_seeding > mdata->mcomp[x].clubseeded) ||
                    (club_seeding == 0 && mdata->mcomp[x].clubseeded)) {
                    club_seeding = mdata->mcomp[x].clubseeded;
                    highest_num = x;
                }
	    }
	}
	if (++x > mdata->mjudokas)
	    x = 1;
    }

    last_selected = highest_num;

    return last_selected;
}

static gint get_section(gint num, struct mdata *mdata)
{
    gint i, sec_siz = mdata->mpositions/NUM_SEEDED;
    gint sec_max = sec_siz;

    // for pool & double pool section is competitor number
    if (mdata->mfrench_sys < 0)
        return (num - 1);

    // for french system section is A1, A2, B1, B2, C1, C2, D1 or D2.
    for (i = 0; i < NUM_SEEDED; i++) {
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
    case FRENCH_8:   num = 8;  break;
    case FRENCH_16:  num = 16; break;
    case FRENCH_32:  num = 32; break;
    case FRENCH_64:  num = 64; break;
#if (NUM_COMPETITORS > 64)
    case FRENCH_128: num = 128; break;
#endif
    default: return;
    }        

    // add one to place value if fight has the other competitor
    // add also a seeding based number if the competitor is seeded
    for (x = 1; x <= mdata->mpositions-1; x += 2) {
        if (mdata->mpos[x].judoka) {
            place_values[x+1]++;
            if (mdata->mcomp[mdata->mpos[x].judoka].seeded)
                place_values[x+1] += NUM_SEEDED + 1 - mdata->mcomp[mdata->mpos[x].judoka].seeded;
        }
        if (mdata->mpos[x+1].judoka) {
            place_values[x]++;
            if (mdata->mcomp[mdata->mpos[x+1].judoka].seeded)
                place_values[x] += NUM_SEEDED + 1 - mdata->mcomp[mdata->mpos[x+1].judoka].seeded;
        }
    }

    // calculate number of competitors in each quarter
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

    // balance halfs 
    if (q1 + q2 > q3 + q4) {
        for (x = 1; x <= mdata->mpositions/2; x++)
            place_values[x] += 2;
    } else if (q1 + q2 < q3 + q4) {
        for (x = mdata->mpositions/2 + 1; x <= mdata->mpositions; x++)
            place_values[x] += 2;
    }

    // balance 1st and 2nd quarters
    if (q1 > q2) {
        for (x = 1; x <= mdata->mpositions/4; x++)
            place_values[x]++;
    } else if (q1 < q2) {
        for (x = mdata->mpositions/4 + 1; x <= mdata->mpositions/2; x++)
            place_values[x]++;
    }

    // balance 3rd and 4th quarters
    if (q3 > q4) {
        for (x = mdata->mpositions/2 + 1; x <= mdata->mpositions*3/4; x++)
            place_values[x]++;
    } else if (q3 < q4) {
        for (x = mdata->mpositions*3/4 + 1; x <= mdata->mpositions; x++)
            place_values[x]++;
    }

    // add value based on the closeness of the club mates
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
    gint place_values[NUM_COMPETITORS+1], selected[NUM_COMPETITORS];
    gint i, x, valid_place = 0, valid_place_seeded = 0;
    gint min = mdata->mpositions, max = 1;
    gint best_value = 10000;
    gint seeded = mdata->mcomp[mdata->selected].seeded;

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

    // find minimum valid position
    for (i = 0; i < NUM_SEEDED; i++) {
        if (mask & (1<<i)) {
            min = mdata->mpositions*i/NUM_SEEDED + 1;
            break;
        }
    }

    // find maximum valid position
    for (i = NUM_SEEDED - 1; i >= 0; i--) {
        if (mask & (1<<i)) {
            max = mdata->mpositions*(i+1)/NUM_SEEDED;
            break;
        }
    }

    // random number between min and max
    x = rand()%(max - min + 1) + min;

    // calculate avoiding points for the positions
    memset(place_values, 0, sizeof(place_values));
    memset(selected, 0, sizeof(selected));
    calc_place_values(place_values, mdata);

    for (i = 1; i <= mdata->mpositions; i++) {
        // is this position in a valid mask section
        gint currmask = 1<<get_section(x, mdata);
        if ((currmask & mask) && mdata->mpos[x].judoka == 0 
            /*XXX && get_competitor_number(x, mdata) <= mdata->mjudokas*/) {
            // update the lowest avoid value
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

    if (seeded) { // ensure seeded can wear white judogi
        for (i = 1; i <= mdata->mpositions; i++) {
            if ((place_values[i] == best_value && (i & 1))) {
                valid_place_seeded = i;
                break;
            }
        }
    }

    x = 0;
    for (i = 1; i <= mdata->mpositions; i++) {
        if ((place_values[i] == best_value && valid_place_seeded == 0) ||
            (place_values[i] == best_value && (i & 1)))
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

    for (seed = 0; seed < NUM_SEEDED; seed++) {
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

// return sections who have competitors from the same club or country
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

// return sections who have competitors from the same club
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

// return competitor's number based on position
gint get_drawed_number(gint pos, gint sys)
{
    if (sys >= 0 && (pos&1))
        return french_matches_blue[(pos-1)/2 * french_mul[sys]];
    else if (sys >= 0 && (pos&1) == 0)
        return french_matches_blue[(pos-1)/2 * french_mul[sys]] 
            + french_matches_white_offset[sys];
    return pos;
}

// return competitor's number based on position
static gint get_competitor_number(gint pos, struct mdata *mdata)
{
    return get_drawed_number(pos, mdata->mfrench_sys);
}

// manually select position for the competitor
static gboolean select_number(GtkWidget *eventbox, GdkEventButton *event, void *param)
{
    GdkColor bg;
    gchar buf[200];
    gint num = 0, i, nxt;
    struct mdata *mdata = param;

    for (i = 1; i <= NUM_COMPETITORS; i++)
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

// manually select next competitor to be drawn
static gboolean select_competitor(GtkWidget *eventbox, GdkEventButton *event, void *param)
{
    guint index = 0;
    GdkColor bg;
    gint i;
    struct mdata *mdata = param;

    for (i = 1; i <= NUM_COMPETITORS; i++)
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

// get mask for example from half length one quarter = section C
// = create_mask(1, 2, 4)
// half from beginning = create_mask(0, 1, 2)
static gint create_mask(gint start_dividend, gint start_divisor, gint length_divisor)
{
    gint mask = 0, i;
    gint start = start_dividend*NUM_SEEDED/start_divisor;
    gint end = start + NUM_SEEDED/length_divisor;
    for (i = start; i < end; i++)
        mask |= 1<<i;
    return mask;
}

static gboolean draw_one_comp(struct mdata *mdata)
{
    gint comp = mdata->selected;
    gint found = 0, mask;
    gint sys = mdata->sys.system;

    if (mdata->selected == 0)
        return 0;

    if (mdata->mfrench_sys >= 0) {
        if (draw_system == DRAW_SPANISH &&
            mdata->mcomp[comp].seeded > 0 && mdata->mcomp[comp].seeded <= NUM_SEEDED) {
            gint x;
            switch (mdata->mcomp[comp].seeded) {
            case 1: case 5: x = 0; break;
            case 2: case 6: x = 2; break;
            case 3: case 7: x = 1; break;
            case 4: case 8: x = 3; break;
            }
            found = mdata->mpositions*(x)/4 + 
                mdata->mcomp[comp].seeded > 4 ? mdata->mpositions/8 + 1 : 1;

            if (mdata->mpos[found].judoka)
                found = 0;
        }

        if (!found) {
            gint seeded = mdata->mcomp[comp].seeded;
            if (seeded > 1) {
                gint d = 0, k = 2;
                gint test_mask = create_mask(d,NUM_SEEDED,2); // AB

                do {
                    mask = get_seeded_mask(1<<(seeded-k), mdata); // look for the previous seeded
                    k++;
                } while (mask == 0 && (seeded-k >= 0));

                if (seeded & 1) {
                    // put to same half as the previous
                    if ((mask & test_mask) == 0)
                        d = NUM_SEEDED/2; // prev seeded in CD half
                    
                    test_mask = create_mask(d,NUM_SEEDED,4);
                    if (mask & test_mask)
                        d += NUM_SEEDED/4;

                    test_mask = create_mask(d,NUM_SEEDED,4);
                } else {
                    // put to different half as the previous
                    if (mask & test_mask)
                        d = NUM_SEEDED/2; // prev seeded in AB half
                    
                    test_mask = create_mask(d,NUM_SEEDED,2);
                }

                found = get_free_pos_by_mask((test_mask ^ mask) & test_mask, mdata);
            }
        }

        if (!found)
            found = get_free_pos_by_mask(create_mask(0,1,1), mdata); // look everywhere
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
            gint pool_start[4];
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

static void make_manual_matches_callback(GtkWidget *widget, 
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
        for (i = 0; i < num_matches(mdata->sys.system, mdata->mjudokas); i++) {
            struct match m;
                
            memset(&m, 0, sizeof(m));
            m.category = mdata->mcategory_ix;
            m.number = i+1;
            if (mdata->sys.system == SYSTEM_QPOOL) {
                m.blue = mdata->mcomp[ mdata->mpos[ poolsq[mdata->mjudokas][i][0] ].judoka ].index;
                m.white = mdata->mcomp[ mdata->mpos[ poolsq[mdata->mjudokas][i][1] ].judoka ].index;
            } else if (mdata->sys.system == SYSTEM_DPOOL ||
                       mdata->sys.system == SYSTEM_DPOOL2) {
                m.blue = mdata->mcomp[ mdata->mpos[ poolsd[mdata->mjudokas][i][0] ].judoka ].index;
                m.white = mdata->mcomp[ mdata->mpos[ poolsd[mdata->mjudokas][i][1] ].judoka ].index;
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
    update_matches(mdata->mcategory_ix, (struct compsys){0,0,0,0}, 0);

    if (mdata->hidden == FALSE && (gulong)event == GTK_RESPONSE_OK)
        category_window(mdata->mcategory_ix);

    free_mdata(mdata);

    if (widget)
        gtk_widget_destroy(widget);
}

struct compsys get_system_for_category(gint index, gint competitors)
{
    struct compsys systm = get_cat_system(index);
    gint wishsys = systm.wishsys;
    gint table = systm.table;
    struct category_data *cat = NULL;
    gint sys = 0;

    if (systm.system)
        return systm;

    if (competitors <= 5 && (wishsys == CAT_SYSTEM_POOL || wishsys == CAT_SYSTEM_DEFAULT)) {
        sys = SYSTEM_POOL;
    } else if (competitors <= 7 && wishsys == CAT_SYSTEM_POOL) {
        sys = SYSTEM_POOL;
    } else if (competitors > 5 && competitors <= 10 && wishsys == CAT_SYSTEM_DPOOL) {
        sys = SYSTEM_DPOOL;
    } else if (competitors > 5 && competitors <= 10 && wishsys == CAT_SYSTEM_DPOOL2) {
        sys = SYSTEM_DPOOL2;
    } else if (competitors >= 8 && competitors <= 20 && wishsys == CAT_SYSTEM_QPOOL) {
        sys = SYSTEM_QPOOL;
    } else if (competitors > 5 && competitors <= 10 && wishsys == CAT_SYSTEM_DEFAULT &&
               (draw_system == DRAW_SPANISH || draw_system == DRAW_NORWEGIAN)) {
            sys = SYSTEM_DPOOL;
    } else if (competitors > 5 && competitors <= 7 && wishsys == CAT_SYSTEM_DEFAULT) {
        if (draw_system == DRAW_INTERNATIONAL || draw_system == DRAW_ESTONIAN) {
            if (draw_system == DRAW_INTERNATIONAL)
                table = cat_system_to_table[CAT_IJF_DOUBLE_REPECHAGE];
            sys = SYSTEM_FRENCH_8;
        } else {
            sys = SYSTEM_DPOOL;
        }
    } else if (competitors > 16 && wishsys == CAT_ESP_DOBLE_PERDIDA) {
        table = cat_system_to_table[CAT_ESP_REPESCA_DOBLE];
        if (competitors <= 32)
            sys = SYSTEM_FRENCH_32;
        else
            sys = SYSTEM_FRENCH_64;
    } else if (competitors > 64) { // only 2 tables supported
        if (wishsys != DRAW_FINNISH &&
            wishsys != CAT_IJF_DOUBLE_REPECHAGE) {
            if (draw_system == DRAW_FINNISH)
                wishsys = CAT_SYSTEM_REPECHAGE;
            else
                wishsys = CAT_IJF_DOUBLE_REPECHAGE;
        }

        table = cat_system_to_table[wishsys];
        sys = SYSTEM_FRENCH_128;
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
                break;
            case DRAW_NORWEGIAN:
                wishsys = CAT_SYSTEM_DUBBELT_AATERKVAL;
                break;
            }
        }

        table = cat_system_to_table[wishsys];

        if (competitors <= 8) {
            sys = SYSTEM_FRENCH_8;
        } else if (competitors <= 16) {
            sys = SYSTEM_FRENCH_16;
        } else if (competitors <= 32) {
            sys = SYSTEM_FRENCH_32;
        } else if (competitors <= 64) {
            sys = SYSTEM_FRENCH_64;
        } else {
            sys = SYSTEM_FRENCH_128;
        }
    }
    
    systm.system  = sys;
    systm.numcomp = competitors;
    systm.table   = table;
    systm.wishsys = wishsys;

    return systm;
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

    switch (mdata->sys.system) {
    case SYSTEM_POOL:
    case SYSTEM_DPOOL:
    case SYSTEM_DPOOL2:
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
    case SYSTEM_FRENCH_128:
        mdata->mpositions = 128;
        mdata->mfrench_sys = FRENCH_128;
        break;
    }

    if (catname && catname[0] == '?') {
        //SHOW_MESSAGE("Cannot draw %s", catname);
        g_free(catname);
        g_free(mdata);
        return NULL;
    }

    g_free(catname);

    if (db_category_match_status(mdata->mcategory_ix) & REAL_MATCH_EXISTS) {
        // Cannot draw again.
#if 0
        struct judoka *j = get_data(mdata->mcategory_ix);
        SHOW_MESSAGE("%s %s. %s.",
                     _("Matches matched in category"),
                     j ? j->last : "?", _("Clear the results first"));
        free_judoka(j);
#endif
        g_free(mdata);
        return dialog;
    }

    // Clean up the database.
    struct compsys cs;
    cs = get_cat_system(mdata->mcategory_ix);
    cs.system = cs.numcomp = cs.table = 0; // leave wishsys as is

    db_set_system(mdata->mcategory_ix, cs);
    db_remove_matches(mdata->mcategory_ix);

    // Get data of all the competitors.
    for (i = 0; i < competitors; i++) {
        guint index;
        gchar *club, *country;
        gint deleted;
        gint seeding, clubseeding;
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
                           COL_SEEDING, &seeding,
                           COL_CLUBSEEDING, &clubseeding,
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
        mdata->mcomp[i+1].seeded = seeding;
        mdata->mcomp[i+1].clubseeded = clubseeding;
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
            (mdata->mfrench_sys == 3 && i > 48) ||
            (mdata->mfrench_sys == 3 && i > 32 && i <= 64) ||
            (mdata->mfrench_sys == 3 && i > 96))
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
                     G_CALLBACK(make_manual_matches_callback), mdata);

    last_selected = 0;
    gboolean retval = FALSE;
    g_signal_emit_by_name(mdata->mcomp[get_next_comp(mdata)].eventbox, "button_press_event", 
                          NULL, &retval);

    return dialog;
}

void draw_all(GtkWidget *w, gpointer data)
{
    GtkTreeIter iter;
    gint i;

    gint num_cats = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(current_model), NULL);
    gint cnt = 0;

    gdk_window_set_cursor(GTK_WIDGET(main_window)->window, wait_cursor);

    for (i = 0; i <= number_of_tatamis; i++) {
        struct category_data *catdata = category_queue[i].next;
        while (catdata) {
            if (find_iter(&iter, catdata->index)) {
                gint n = gtk_tree_model_iter_n_children(current_model, &iter);
                if (n >= 1 && n <= NUM_COMPETITORS)
                    draw_one_category(&iter, n);

                cnt++;
                if (num_cats > 4)
                    progress_show(1.0*cnt/num_cats, "");
            }
            catdata = catdata->next;
        }
    }

    gdk_window_set_cursor(GTK_WIDGET(main_window)->window, NULL);
    SYS_LOG_INFO(_("All the categories has been drawn"));

    for (i = 1; i <= NUM_TATAMIS; i++) {
        progress_show(1.0*i/NUM_TATAMIS, "");
        update_matches(0, (struct compsys){0,0,0,0}, i);
    }

    progress_show(0.0, "");
}

