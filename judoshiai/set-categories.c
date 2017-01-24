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

struct catdefwidgets;
static void teams_dialog(struct catdefwidgets *fields);

struct offial_category {
    gchar *name;
    gint maxage;
    gint match_time;
    gint gs_time;
    gint rest_time;
    struct {
        gint weight;
        gchar *weighttext;
    } weights[NUM_CAT_DEF_WEIGHTS];
};

struct offial_category official_categories[NUM_DRAWS][2][10] = {
    { // international
        { // men
            {
                "Men", 1000, 240, 0, 600, // name, max age, match gs rest times
                {{60000, "60"}, {66000, "66"}, {73000, "73"},
                 {81000, "81"}, {90000, "90"}, {100000, "100"}, {0, NULL}}
            }, {
                "MenU21", 20, 240, 0, 600, // name, max age, match gs rest times
                {{55000, "55"}, {60000, "60"}, {66000, "66"}, {73000, "73"},
                 {81000, "81"}, {90000, "90"}, {100000, "100"}, {0, NULL}}
            }, {
                "MenU18", 17, 180, 0, 600, // name, max age, match gs rest times
                {{45000, "45"}, {50000, "50"}, {55000, "55"}, {60000, "60"},
                 {66000, "66"}, {73000, "73"}, {81000, "81"}, {90000, "90"}, {0, NULL}}
            }, { NULL, 0, 0, 0, 0, {{0, NULL}}}

        }, { // women
            {
                "Women", 1000, 240, 0, 600, // name, max age, match gs rest times
                {{48000, "48"}, {52000, "52"}, {57000, "57"},
                 {63000, "63"}, {70000, "70"}, {78000, "78"}, {0, NULL}}
            }, {
                "WomenU21", 20, 240, 0, 600, // name, max age, match gs rest times
                {{44000, "44"}, {48000, "48"}, {52000, "52"}, {57000, "57"},
                 {63000, "63"}, {70000, "70"}, {78000, "78"}, {0, NULL}}
            }, {
                "WomenU18", 17, 180, 0, 600, // name, max age, match gs rest times
                {{44000, "44"}, {48000, "48"}, {52000, "52"}, {57000, "57"},
                 {63000, "63"}, {70000, "70"}, {0, NULL}}
            }, { NULL, 0, 0, 0, 0, {{0, NULL}}}
        }
    }, { // finnish
        { // men
            {
                "M", 1000, 240, 0, 600, // name, max age, match gs rest times
                {{60000, "60"}, {66000, "66"}, {73000, "73"},
                 {81000, "81"}, {90000, "90"}, {100000, "100"}, {0, NULL}}
            }, {
                "M21", 20, 240, 0, 600, // name, max age, match gs rest times
                {{55000, "55"}, {60000, "60"}, {66000, "66"}, {73000, "73"},
                 {81000, "81"}, {90000, "90"}, {0, NULL}}
            }, {
                "P18", 17, 240, 0, 600, // name, max age, match gs rest times
                {{45000, "45"}, {50000, "50"}, {55000, "55"}, {60000, "60"},
                 {66000, "66"}, {73000, "73"}, {81000, "81"}, {0, NULL}}
            }, {
                "P15", 14, 180, 0, 180, // name, max age, match gs rest times
                {{34000, "34"}, {38000, "38"}, {42000, "42"}, {46000, "46"},
                 {50000, "50"}, {55000, "55"}, {60000, "60"}, {66000, "66"}, {0, NULL}}
            }, {
                "P13", 12, 120, 0, 180, // name, max age, match gs rest times
                {{30000, "30"}, {34000, "34"}, {38000, "38"}, {42000, "42"},
                 {46000, "46"}, {50000, "50"}, {55000, "55"}, {0, NULL}}
            }, {
                "P11", 10, 120, 0, 180, // name, max age, match gs rest times
                {{27000, "27"}, {30000, "30"}, {34000, "34"}, {38000, "38"}, {42000, "42"},
                 {46000, "46"}, {50000, "50"}, {55000, "55"}, {0, NULL}}
            }, { NULL, 0, 0, 0, 0, {{0, NULL}}}
        }, { // women
            {
                "N", 1000, 240, 0, 600, // name, max age, match gs rest times
                {{48000, "48"}, {52000, "52"}, {57000, "57"},
                 {63000, "63"}, {70000, "70"}, {78000, "78"}, {0, NULL}}
            }, {
                "N21", 20, 240, 0, 600, // name, max age, match gs rest times
                {{48000, "48"}, {52000, "52"}, {57000, "57"},
                 {63000, "63"}, {70000, "70"}, {0, NULL}}
            }, {
                "T18", 17, 240, 0, 600, // name, max age, match gs rest times
                {{44000, "44"}, {48000, "48"}, {52000, "52"}, {57000, "57"},
                 {63000, "63"}, {0, NULL}}
            }, {
                "T15", 14, 180, 0, 180, // name, max age, match gs rest times
                {{32000, "32"}, {36000, "36"}, {40000, "40"}, {44000, "44"},
                 {48000, "48"}, {52000, "52"}, {57000, "57"}, {63000, "63"}, {0, NULL}}
            }, {
                "T13", 12, 120, 0, 180, // name, max age, match gs rest times
                {{28000, "28"}, {32000, "32"}, {36000, "36"}, {40000, "40"},
                 {44000, "44"}, {48000, "48"}, {52000, "52"}, {0, NULL}}
            }, {
                "T11", 10, 120, 0, 180, // name, max age, match gs rest times
                {{25000, "25"}, {28000, "28"}, {32000, "32"}, {36000, "36"},
                 {40000, "40"}, {44000, "44"}, {48000, "48"}, {52000, "52"}, {0, NULL}}
            }, { NULL, 0, 0, 0, 0, {{0, NULL}}}
        }
    }, { // swedish
        { // men
            {
                "Herrar", 1000, 240, 0, 600, // name, max age, match gs rest times
                {{60000, "60"}, {66000, "66"}, {73000, "73"},
                 {81000, "81"}, {90000, "90"}, {100000, "100"}, {0, NULL}}
            }, {
                "Herrar U21", 20, 240, 0, 600, // name, max age, match gs rest times
                {{55000, "55"}, {60000, "60"}, {66000, "66"}, {73000, "73"},
                 {81000, "81"}, {90000, "90"}, {100000, "100"}, {0, NULL}}
            }, {
                "Pojkar U18", 17, 180, 0, 600, // name, max age, match gs rest times
                {{46000, "46"}, {50000, "50"}, {55000, "55"}, {60000, "60"},
                 {66000, "66"}, {73000, "73"}, {81000, "81"}, {0, NULL}}
            }, {
                "Pojkar U15", 14, 180, 0, 180, // name, max age, match gs rest times
                {{34000, "34"}, {38000, "38"}, {42000, "42"}, {46000, "46"},
                 {50000, "50"}, {55000, "55"}, {60000, "60"}, {66000, "66"}, {0, NULL}}
            }, {
                "Pojkar U13", 12, 120, 0, 180, // name, max age, match gs rest times
                {{24000, "24"}, {27000, "27"}, {30000, "30"}, {34000, "34"},
                 {38000, "38"}, {42000, "42"}, {46000, "46"}, {50000, "50"}, {0, NULL}}
            }, {
                "Pojkar U11", 10, 120, 0, 180, // name, max age, match gs rest times
                {{24000, "24"}, {27000, "27"}, {30000, "30"}, {34000, "34"},
                 {38000, "38"}, {42000, "42"}, {46000, "46"}, {50000, "50"}, {0, NULL}}
            }, { NULL, 0, 0, 0, 0, {{0, NULL}}}
        }, { // women
            {
                "Damer", 1000, 240, 0, 600, // name, max age, match gs rest times
                {{48000, "48"}, {52000, "52"}, {57000, "57"},
                 {63000, "63"}, {70000, "70"}, {78000, "78"}, {0, NULL}}
            }, {
                "Damer U21", 20, 240, 0, 600, // name, max age, match gs rest times
                {{44000, "44"}, {48000, "48"}, {52000, "52"}, {57000, "57"},
                 {63000, "63"}, {70000, "70"}, {78000, "78"}, {0, NULL}}
            }, {
                "Flickor U18", 17, 180, 0, 600, // name, max age, match gs rest times
                {{44000, "44"}, {48000, "48"}, {52000, "52"}, {57000, "57"},
                 {63000, "63"}, {70000, "70"}, {0, NULL}}
            }, {
                "Flickor U15", 14, 180, 0, 180, // name, max age, match gs rest times
                {{32000, "32"}, {36000, "36"}, {40000, "40"}, {44000, "44"},
                 {48000, "48"}, {52000, "52"}, {57000, "57"}, {63000, "63"}, {0, NULL}}
            }, {
                "Flickor U13", 12, 120, 0, 180, // name, max age, match gs rest times
                {{22000, "22"}, {25000, "25"}, {28000, "28"}, {32000, "32"},
                 {36000, "36"}, {40000, "40"}, {44000, "44"}, {48000, "48"}, {0, NULL}}
            }, {
                "Flickor U11", 10, 120, 0, 180, // name, max age, match gs rest times
                {{22000, "22"}, {25000, "25"}, {28000, "28"}, {32000, "32"},
                 {36000, "36"}, {40000, "40"}, {44000, "44"}, {48000, "48"}, {0, NULL}}
            }, { NULL, 0, 0, 0, 0, {{0, NULL}}}
        }
    }, { // estonian
        { // men
            {
                "M", 1000, 240, 0, 600, // name, max age, match gs rest times
                {{60000, "60"}, {66000, "66"}, {73000, "73"},
                 {81000, "81"}, {90000, "90"}, {100000, "100"}, {0, NULL}}
            }, {
                "M", 20, 240, 0, 600, // name, max age, match gs rest times
                {{55000, "55"}, {60000, "60"}, {66000, "66"}, {73000, "73"},
                 {81000, "81"}, {90000, "90"}, {100000, "100"}, {0, NULL}}
            }, {
                "AP", 17, 180, 0, 600, // name, max age, match gs rest times
                {{50000, "50"}, {55000, "55"}, {60000, "60"},
                 {66000, "66"}, {73000, "73"}, {81000, "81"}, {90000, "90"}, {0, NULL}}
            }, {
                "BP", 14, 180, 0, 180, // name, max age, match gs rest times
                {{34000, "34"}, {38000, "38"}, {42000, "42"}, {45000, "45"},
                 {50000, "50"}, {55000, "55"}, {60000, "60"}, {66000, "66"},
                 {73000, "73"}, {0, NULL}}
            }, {
                "CP", 12, 120, 0, 180, // name, max age, match gs rest times
                {{27000, "27"}, {30000, "30"}, {34000, "34"}, {38000, "38"}, {42000, "42"},
                 {46000, "46"}, {50000, "50"}, {55000, "55"}, {60000, "60"}, {0, NULL}}
            }, {
                "DP", 10, 120, 0, 180, // name, max age, match gs rest times
                {{27000, "27"}, {30000, "30"}, {34000, "34"}, {38000, "38"}, {42000, "42"},
                 {46000, "46"}, {50000, "50"}, {55000, "55"}, {0, NULL}}
            }, { NULL, 0, 0, 0, 0, {{0, NULL}}}
        }, { // women
            {
                "N", 1000, 240, 0, 600, // name, max age, match gs rest times
                {{48000, "48"}, {52000, "52"}, {57000, "57"},
                 {63000, "63"}, {70000, "70"}, {78000, "78"}, {0, NULL}}
            }, {
                "N", 20, 240, 0, 600, // name, max age, match gs rest times
                {{44000, "44"}, {48000, "48"}, {52000, "52"}, {57000, "57"},
                 {63000, "63"}, {70000, "70"}, {78000, "78"}, {0, NULL}}
            }, {
                "AT", 17, 180, 0, 600, // name, max age, match gs rest times
                {{40000, "40"}, {44000, "44"}, {48000, "48"}, {52000, "52"},
                 {57000, "57"}, {63000, "63"}, {70000, "70"}, {0, NULL}}
            }, {
                "BT", 14, 180, 0, 180, // name, max age, match gs rest times
                {{36000, "36"}, {40000, "40"}, {44000, "44"},
                 {48000, "48"}, {52000, "52"}, {57000, "57"}, {0, NULL}}
            }, {
                "CT", 12, 120, 0, 180, // name, max age, match gs rest times
                {{28000, "28"}, {32000, "32"}, {36000, "36"}, {40000, "40"},
                 {44000, "44"}, {48000, "48"}, {0, NULL}}
            }, {
                "DT", 10, 120, 0, 180, // name, max age, match gs rest times
                {{25000, "25"}, {28000, "28"}, {32000, "32"}, {36000, "36"},
                 {40000, "40"}, {44000, "44"}, {48000, "48"}, {52000, "52"}, {0, NULL}}
            }, { NULL, 0, 0, 0, 0, {{0, NULL}}}
        }
    }, { // spanish
        { // men
            {
                "Sen M ", 1000, 240, 180, 600, // name, max age, match gs rest times
                {{60000, "60"}, {66000, "66"}, {73000, "73"},
                 {81000, "81"}, {90000, "90"}, {100000, "100"}, {0, NULL}}
            }, {
                "Jun M ", 18, 240, 120, 600, // name, max age, match gs rest times
                {{55000, "55"}, {60000, "60"}, {66000, "66"}, {73000, "73"},
                 {81000, "81"}, {90000, "90"}, {100000, "100"}, {0, NULL}}
            }, {
                "Cad M ", 16, 240, 120, 600, // name, max age, match gs rest times
                {{50000, "50"}, {55000, "55"}, {60000, "60"},
                 {66000, "66"}, {73000, "73"}, {81000, "81"}, {90000, "90"}, {0, NULL}}
            }, {
                "Inf M ", 14, 180, 120, 180, // name, max age, match gs rest times
                {{38000, "38"}, {42000, "42"}, {46000, "46"},
                 {50000, "50"}, {55000, "55"}, {60000, "60"}, {66000, "66"},
                 {0, NULL}}
            }, {
                "Ale M ", 12, 120, 60, 180, // name, max age, match gs rest times
                {{30000, "30"}, {34000, "34"}, {38000, "38"}, {42000, "42"},
                 {47000, "47"}, {52000, "52"}, {0, NULL}}
            }, {
                "Ben M ", 10, 120, 60, 180, // name, max age, match gs rest times
                {{26000, "26"}, {30000, "30"}, {34000, "34"}, {38000, "38"}, {42000, "42"},
                 {47000, "47"}, {0, NULL}}
            }, {
                "Mini ", 8, 120, 60, 180, // name, max age, match gs rest times
                {{100000, " "}, {0, NULL}}
            }, {
                "Micro ", 6, 120, 60, 180, // name, max age, match gs rest times
                {{100000, " "}, {0, NULL}}
            }, { NULL, 0, 0, 0, 0, {{0, NULL}}}
        }, { // women
            {
                "Sen F ", 1000, 240, 180, 600, // name, max age, match gs rest times
                {{48000, "48"}, {52000, "52"}, {57000, "57"},
                 {63000, "63"}, {70000, "70"}, {78000, "78"}, {0, NULL}}
            }, {
                "Jun F ", 18, 240, 120, 600, // name, max age, match gs rest times
                {{44000, "44"}, {48000, "48"}, {52000, "52"}, {57000, "57"},
                 {63000, "63"}, {70000, "70"}, {78000, "78"}, {0, NULL}}
            }, {
                "Cad F ", 16, 240, 120, 600, // name, max age, match gs rest times
                {{40000, "40"}, {44000, "44"}, {48000, "48"}, {52000, "52"},
                 {57000, "57"}, {63000, "63"}, {70000, "70"}, {0, NULL}}
            }, {
                "Inf F ", 14, 180, 120, 180, // name, max age, match gs rest times
                {{36000, "36"}, {40000, "40"}, {44000, "44"},
                 {48000, "48"}, {52000, "52"}, {57000, "57"}, {63000, "63"}, {0, NULL}}
            }, {
                "Ale F ", 12, 120, 60, 180, // name, max age, match gs rest times
                {{30000, "30"}, {34000, "34"}, {38000, "38"}, {42000, "42"},
                 {47000, "47"}, {52000, "52"}, {0, NULL}}
            }, {
                "Ben F ", 10, 120, 60, 180, // name, max age, match gs rest times
                {{26000, "26"}, {30000, "30"}, {34000, "34"}, {38000, "38"}, {42000, "42"},
                 {47000, "47"}, {0, NULL}}
            }, {
                "Mini ", 8, 120, 60, 180, // name, max age, match gs rest times
                {{100000, " "}, {0, NULL}}
            }, {
                "Micro ", 6, 120, 60, 180, // name, max age, match gs rest times
                {{100000, " "}, {0, NULL}}
            }, { NULL, 0, 0, 0, 0, {{0, NULL}}}
        }
    }, { // norwegian
        { // men
            {
                "SenH", 1000, 240, 0, 300, // name, max age, match gs rest times
                {{60000, "60"}, {66000, "66"}, {73000, "73"},
                 {81000, "81"}, {90000, "90"}, {100000, "100"}, {0, NULL}}
            }, {
                "U21H", 20, 240, 0, 240, // name, max age, match gs rest times
                {{55000, "55"}, {60000, "60"}, {66000, "66"}, {73000, "73"},
                 {81000, "81"}, {90000, "90"}, {100000, "100"}, {0, NULL}}
            }, {
                "U18G", 17, 180, 0, 180, // name, max age, match gs rest times
                {{46000, "46"}, {50000, "50"}, {55000, "55"}, {60000, "60"},
                 {66000, "66"}, {73000, "73"}, {81000, "81"}, {90000, "90"}, {0, NULL}}
            }, {
                "U15G", 14, 180, 0, 180, // name, max age, match gs rest times
                {{34000, "34"}, {38000, "38"}, {42000, "42"}, {46000, "46"},
                 {50000, "50"}, {55000, "55"}, {60000, "60"}, {66000, "66"}, {0, NULL}}
            },{
                "Barn", 12, 120, 0, 120, // name, max age, match gs rest times
                {{24000, "24"}, {27000, "27"}, {30000, "30"}, {34000, "34"},
                 {38000, "38"}, {42000, "42"}, {46000, "46"}, {50000, "50"}, {0, NULL}}
            }, {
                "Barn10", 10, 120, 0, 120, // name, max age, match gs rest times
                {{24000, "24"}, {27000, "27"}, {30000, "30"}, {34000, "34"},
                 {38000, "38"}, {42000, "42"}, {46000, "46"}, {50000, "50"}, {0, NULL}}
            }, {
                "Mini ", 8, 120, 0, 120, // name, max age, match gs rest times
                {{100000, " "}, {0, NULL}}
            }, {
                "Micro ", 6, 120, 0, 120, // name, max age, match gs rest times
                {{100000, " "}, {0, NULL}}
            }, { NULL, 0, 0, 0, 0, {{0, NULL}}}
        }, { // women
            {
                "SenD", 1000, 240, 0, 300, // name, max age, match gs rest times
                {{48000, "48"}, {52000, "52"}, {57000, "57"},
                 {63000, "63"}, {70000, "70"}, {78000, "78"}, {0, NULL}}
            }, {
                "U21D", 20, 240, 0, 240, // name, max age, match gs rest times
                {{48000, "48"}, {52000, "52"}, {57000, "57"},
                 {63000, "63"}, {70000, "70"}, {78000, "78"}, {0, NULL}}
            }, {
                "U18J", 17, 180, 0, 180, // name, max age, match gs rest times
                {{40000, "40"}, {44000, "44"}, {48000, "48"}, {52000, "52"}, {57000, "57"},
                 {63000, "63"}, {70000, "70"}, {0, NULL}}
            }, {
                "U15J", 14, 180, 0, 180, // name, max age, match gs rest times
                {{32000, "32"}, {36000, "36"}, {40000, "40"}, {44000, "44"},
                 {48000, "48"}, {52000, "52"}, {57000, "57"}, {63000, "63"}, {0, NULL}}
            },{
                "BarnJ", 12, 120, 0, 120, // name, max age, match gs rest times
                {{24000, "24"}, {27000, "27"}, {30000, "30"}, {34000, "34"},
                 {38000, "38"}, {42000, "42"}, {46000, "46"}, {50000, "50"}, {0, NULL}}
            }, {
                "Barn10", 10, 120, 0, 120, // name, max age, match gs rest times
                {{24000, "24"}, {27000, "27"}, {30000, "30"}, {34000, "34"},
                 {38000, "38"}, {42000, "42"}, {46000, "46"}, {50000, "50"}, {0, NULL}}
            }, {
                "Mini ", 8, 120, 0, 120, // name, max age, match gs rest times
                {{100000, " "}, {0, NULL}}
            }, {
                "Micro ", 6, 120, 0, 120, // name, max age, match gs rest times
                {{100000, " "}, {0, NULL}}
            }, { NULL, 0, 0, 0, 0, {{0, NULL}}}
        }
    }, { // british
        { // men
            {
                "Men", 1000, 240, 180, 600, // name, max age, match gs rest times
                {{60000, "60"}, {66000, "66"}, {73000, "73"},
                 {81000, "81"}, {90000, "90"}, {100000, "100"}, {0, NULL}}
            }, {
                "BoysU19", 18, 240, 120, 600, // name, max age, match gs rest times
                {{55000, "55"}, {60000, "60"}, {66000, "66"}, {73000, "73"},
                 {81000, "81"}, {90000, "90"}, {100000, "100"}, {0, NULL}}
            }, {
                "BoysU16", 15, 180, 120, 600, // name, max age, match gs rest times
                {{38000, "38"}, {42000, "42"}, {46000, "46"}, {50000, "50"}, {55000, "55"}, {60000, "60"},
                 {66000, "66"}, {73000, "73"}, {81000, "81"}, {90000, "90"}, {0, NULL}}
            }, {
                "BoysU14", 13, 180, 120, 600, // name, max age, match gs rest times
                {{30000, "30"}, {34000, "34"}, {38000, "38"}, {42000, "42"}, {46000, "46"},
                 {50000, "50"}, {55000, "55"}, {60000, "60"}, {66000, "66"}, {0, NULL}}
            },{
                "BoysU12", 11, 120, 60, 180, // name, max age, match gs rest times
                {{27000, "27"}, {30000, "30"}, {34000, "34"},
                 {38000, "38"}, {42000, "42"}, {46000, "46"}, {0, NULL}}
            }, { NULL, 0, 0, 0, 0, {{0, NULL}}}
        }, { // women
            {
                "Women", 1000, 240, 180, 600, // name, max age, match gs rest times
                {{48000, "48"}, {52000, "52"}, {57000, "57"},
                 {63000, "63"}, {70000, "70"}, {78000, "78"}, {0, NULL}}
            }, {
                "GirlsU19", 18, 240, 120, 600, // name, max age, match gs rest times
                {{44000, "44"}, {48000, "48"}, {52000, "52"}, {57000, "57"},
                 {63000, "63"}, {70000, "70"}, {78000, "78"}, {0, NULL}}
            }, {
                "GirlsU16", 15, 180, 120, 600, // name, max age, match gs rest times
                {{40000, "40"}, {44000, "44"}, {48000, "48"}, {52000, "52"}, {57000, "57"},
                 {63000, "63"}, {70000, "70"}, {0, NULL}}
            }, {
                "GirlsU14", 13, 180, 120, 600, // name, max age, match gs rest times
                {{32000, "32"}, {36000, "36"}, {40000, "40"}, {44000, "44"},
                 {48000, "48"}, {52000, "52"}, {57000, "57"}, {63000, "63"}, {0, NULL}}
            },{
                "GirlsU12", 11, 120, 60, 180, // name, max age, match gs rest times
                {{28000, "28"}, {32000, "32"}, {36000, "36"}, {40000, "40"},
                 {44000, "44"}, {48000, "48"}, {0, NULL}}
            }, { NULL, 0, 0, 0, 0, {{0, NULL}}}
        }
    }, { // australian
        { // men
            {
                "Men", 1000, 240, 180, 300, // name, max age, match gs rest times
                {{60000, "60"}, {66000, "66"}, {73000, "73"},
                 {81000, "81"}, {90000, "90"}, {100000, "100"}, {0, NULL}}
            }, {
                "Junior Men", 20, 240, 120, 300, // name, max age, match gs rest times
                {{55000, "55"}, {60000, "60"}, {66000, "66"}, {73000, "73"},
                 {81000, "81"}, {90000, "90"}, {100000, "100"}, {0, NULL}}
            }, {
                "Cadet Men", 17, 240, 120, 300, // name, max age, match gs rest times
                {{50000, "50"}, {55000, "55"}, {60000, "60"},
                 {66000, "66"}, {73000, "73"}, {81000, "81"}, {90000, "90"}, {0, NULL}}
            }, {
                "Senior Boys", 14, 180, 60, 180, // name, max age, match gs rest times
                {{36000, "36"}, {40000, "40"}, {45000, "45"},
                 {50000, "50"}, {55000, "55"}, {60000, "60"}, {66000, "66"}, {0, NULL}}
            },{
                "Junior Boys", 11, 180, 60, 180, // name, max age, match gs rest times
                {{27000, "27"}, {30000, "30"}, {34000, "34"},
                 {38000, "38"}, {42000, "42"}, {46000, "46"}, {50000, "50"}, {0, NULL}}
            },{
                "Mons Boys", 8, 90, 60, 180, // name, max age, match gs rest times
                {{21000, "21"}, {24000, "24"}, {27000, "27"}, {30000, "30"}, {34000, "34"},
                 {38000, "38"}, {42000, "42"}, {0, NULL}}
            }, { NULL, 0, 0, 0, 0, {{0, NULL}}}
        }, { // women
            {
                "Women", 1000, 240, 180, 300, // name, max age, match gs rest times
                {{48000, "48"}, {52000, "52"}, {57000, "57"},
                 {63000, "63"}, {70000, "70"}, {78000, "78"}, {0, NULL}}
            }, {
                "Junior Women", 20, 240, 120, 240, // name, max age, match gs rest times
                {{44000, "44"}, {48000, "48"}, {52000, "52"}, {57000, "57"},
                 {63000, "63"}, {70000, "70"}, {78000, "78"}, {0, NULL}}
            }, {
                "Cadet Women", 17, 240, 120, 240, // name, max age, match gs rest times
                {{40000, "40"}, {44000, "44"}, {48000, "48"}, {52000, "52"}, {57000, "57"},
                 {63000, "63"}, {70000, "70"}, {0, NULL}}
            }, {
                "Senior Girls", 14, 180, 60, 180, // name, max age, match gs rest times
                {{36000, "36"}, {40000, "40"}, {44000, "44"},
                 {48000, "48"}, {52000, "52"}, {57000, "57"}, {63000, "63"}, {0, NULL}}
            },{
                "Junior Girls", 11, 180, 60, 180, // name, max age, match gs rest times
                {{29000, "29"}, {32000, "32"}, {36000, "36"}, {40000, "40"},
                 {44000, "44"}, {48000, "48"}, {52000, "52"}, {0, NULL}}
            },{
                "Mons Girls", 8, 90, 60, 180, // name, max age, match gs rest times
                {{20000, "20"}, {23000, "23"}, {26000, "26"}, {29000, "29"},
                 {32000, "32"}, {36000, "36"}, {40000, "40"}, {0, NULL}}
            }, { NULL, 0, 0, 0, 0, {{0, NULL}}}
        }
    }, { // danish
        { // men
            {
                "Herrer", 1000, 240, 180, 600, // name, max age, match gs rest times
                {{60000, "60"}, {66000, "66"}, {73000, "73"},
                 {81000, "81"}, {90000, "90"}, {100000, "100"}, {0, NULL}}
            }, {
                "Herrer U21", 20, 240, 180, 600, // name, max age, match gs rest times
                {{50000, "50"}, {55000, "55"}, {60000, "60"}, {66000, "66"}, {73000, "73"},
                 {81000, "81"}, {90000, "90"}, {100000, "100"}, {0, NULL}}
            }, {
                "Drenge U18", 17, 240, 180, 600, // name, max age, match gs rest times
                {{35000, "35"}, {40000, "40"}, {45000, "45"}, {50000, "50"}, {55000, "55"}, {60000, "60"},
                 {66000, "66"}, {73000, "73"}, {81000, "81"}, {90000, "90"}, {0, NULL}}
            }, {
                "Drenge U13", 12, 180, 120, 600, // name, max age, match gs rest times
                {{25000, "25"}, {30000, "30"}, {35000, "35"},
                 {40000, "40"}, {45000, "45"}, {50000, "50"}, {0, NULL}}
            }, { NULL, 0, 0, 0, 0, {{0, NULL}}}
        }, { // women
            {
                "Damer", 1000, 240, 180, 600, // name, max age, match gs rest times
                {{48000, "48"}, {52000, "52"}, {57000, "57"},
                 {63000, "63"}, {70000, "70"}, {78000, "78"}, {0, NULL}}
            }, {
                "Damer U21", 20, 240, 180, 600, // name, max age, match gs rest times
                {{44000, "44"}, {48000, "48"}, {52000, "52"}, {57000, "57"},
                 {63000, "63"}, {70000, "70"}, {78000, "78"}, {0, NULL}}
            }, {
                "Piger U18", 17, 240, 180, 600, // name, max age, match gs rest times
                {{35000, "35"}, {40000, "40"}, {44000, "44"}, {48000, "48"}, {52000, "52"}, {57000, "57"},
                 {63000, "63"}, {70000, "70"}, {0, NULL}}
            }, {
                "Piger U13", 12, 180, 120, 600, // name, max age, match gs rest times
                {{25000, "25"}, {30000, "30"}, {35000, "35"},
                 {40000, "40"}, {44000, "44"}, {0, NULL}}
            }, { NULL, 0, 0, 0, 0, {{0, NULL}}}
        }
    }, { // polish
        { // men
            {
                "Men", 1000, 240, 0, 600, // name, max age, match gs rest times
                {{60000, "60"}, {66000, "66"}, {73000, "73"},
                 {81000, "81"}, {90000, "90"}, {100000, "100"}, {0, NULL}}
            }, {
                "MenU21", 20, 240, 0, 600, // name, max age, match gs rest times
                {{55000, "55"}, {60000, "60"}, {66000, "66"}, {73000, "73"},
                 {81000, "81"}, {90000, "90"}, {100000, "100"}, {0, NULL}}
            }, {
                "MenU18", 17, 180, 0, 600, // name, max age, match gs rest times
                {{45000, "45"}, {50000, "50"}, {55000, "55"}, {60000, "60"},
                 {66000, "66"}, {73000, "73"}, {81000, "81"}, {90000, "90"}, {0, NULL}}
            }, { NULL, 0, 0, 0, 0, {{0, NULL}}}

        }, { // women
            {
                "Women", 1000, 240, 0, 600, // name, max age, match gs rest times
                {{48000, "48"}, {52000, "52"}, {57000, "57"},
                 {63000, "63"}, {70000, "70"}, {78000, "78"}, {0, NULL}}
            }, {
                "WomenU21", 20, 240, 0, 600, // name, max age, match gs rest times
                {{44000, "44"}, {48000, "48"}, {52000, "52"}, {57000, "57"},
                 {63000, "63"}, {70000, "70"}, {78000, "78"}, {0, NULL}}
            }, {
                "WomenU18", 17, 180, 0, 600, // name, max age, match gs rest times
                {{44000, "44"}, {48000, "48"}, {52000, "52"}, {57000, "57"},
                 {63000, "63"}, {70000, "70"}, {0, NULL}}
            }, { NULL, 0, 0, 0, 0, {{0, NULL}}}
        }
    }, { // slovakian
        { // men
            {
                "Men", 1000, 240, 0, 600, // name, max age, match gs rest times
                {{60000, "60"}, {66000, "66"}, {73000, "73"},
                 {81000, "81"}, {90000, "90"}, {100000, "100"}, {0, NULL}}
            }, {
                "MenU21", 20, 240, 0, 600, // name, max age, match gs rest times
                {{55000, "55"}, {60000, "60"}, {66000, "66"}, {73000, "73"},
                 {81000, "81"}, {90000, "90"}, {100000, "100"}, {0, NULL}}
            }, {
                "MenU18", 17, 180, 0, 600, // name, max age, match gs rest times
                {{45000, "45"}, {50000, "50"}, {55000, "55"}, {60000, "60"},
                 {66000, "66"}, {73000, "73"}, {81000, "81"}, {90000, "90"}, {0, NULL}}
            }, { NULL, 0, 0, 0, 0, {{0, NULL}}}

        }, { // women
            {
                "Women", 1000, 240, 0, 600, // name, max age, match gs rest times
                {{48000, "48"}, {52000, "52"}, {57000, "57"},
                 {63000, "63"}, {70000, "70"}, {78000, "78"}, {0, NULL}}
            }, {
                "WomenU21", 20, 240, 0, 600, // name, max age, match gs rest times
                {{44000, "44"}, {48000, "48"}, {52000, "52"}, {57000, "57"},
                 {63000, "63"}, {70000, "70"}, {78000, "78"}, {0, NULL}}
            }, {
                "WomenU18", 17, 180, 0, 600, // name, max age, match gs rest times
                {{44000, "44"}, {48000, "48"}, {52000, "52"}, {57000, "57"},
                 {63000, "63"}, {70000, "70"}, {0, NULL}}
            }, { NULL, 0, 0, 0, 0, {{0, NULL}}}
        }
    }, { // ukrainian
        { // men
            {
                "Men", 1000, 240, 0, 600, // name, max age, match gs rest times
                {{60000, "60"}, {66000, "66"}, {73000, "73"},
                 {81000, "81"}, {90000, "90"}, {100000, "100"}, {0, NULL}}
            }, {
                "MenU21", 20, 240, 0, 600, // name, max age, match gs rest times
                {{55000, "55"}, {60000, "60"}, {66000, "66"}, {73000, "73"},
                 {81000, "81"}, {90000, "90"}, {100000, "100"}, {0, NULL}}
            }, {
                "MenU18", 17, 180, 0, 600, // name, max age, match gs rest times
                {{45000, "45"}, {50000, "50"}, {55000, "55"}, {60000, "60"},
                 {66000, "66"}, {73000, "73"}, {81000, "81"}, {90000, "90"}, {0, NULL}}
            }, { NULL, 0, 0, 0, 0, {{0, NULL}}}

        }, { // women
            {
                "Women", 1000, 240, 0, 600, // name, max age, match gs rest times
                {{48000, "48"}, {52000, "52"}, {57000, "57"},
                 {63000, "63"}, {70000, "70"}, {78000, "78"}, {0, NULL}}
            }, {
                "WomenU21", 20, 240, 0, 600, // name, max age, match gs rest times
                {{44000, "44"}, {48000, "48"}, {52000, "52"}, {57000, "57"},
                 {63000, "63"}, {70000, "70"}, {78000, "78"}, {0, NULL}}
            }, {
                "WomenU18", 17, 180, 0, 600, // name, max age, match gs rest times
                {{44000, "44"}, {48000, "48"}, {52000, "52"}, {57000, "57"},
                 {63000, "63"}, {70000, "70"}, {0, NULL}}
            }, { NULL, 0, 0, 0, 0, {{0, NULL}}}
        }
    }
};

struct cat_def category_definitions[NUM_CATEGORIES];
gint num_categories = 0;

static gint find_cat_data_index(const gchar *category)
{
    gint i;
    for (i = 0; i < num_categories; i++)
        if (strcmp(category_definitions[i].agetext, category) == 0)
            return i;
    return -1;
}

static gint find_cat_data_index_by_index(gint index)
{
    gint r = -1;
    struct judoka *j = get_data(index);
    if (j) r = find_cat_data_index(j->last);
    free_judoka(j);
    return r;
}

gint find_num_weight_classes(const gchar *category)
{
    gint j, i = find_cat_data_index(category);
    if (i < 0) return 0;
    for (j = 0; j < NUM_CAT_DEF_WEIGHTS && category_definitions[i].weights[j].weighttext[0]; j++) ;
    return j;
}

gchar *get_weight_class_name(const gchar *category, gint num)
{
    gint i = find_cat_data_index(category);
    if (i < 0) return NULL;
    if (num < 0 || num >= NUM_CAT_DEF_WEIGHTS) return NULL;
    gchar *r = category_definitions[i].weights[num].weighttext;
    return r[0] == 0 ? NULL : r;
}

gchar *get_weight_class_name_by_index(gint index, gint num)
{
    gint i = find_cat_data_index_by_index(index);
    if (i < 0) return NULL;
    if (num < 0 || num >= NUM_CAT_DEF_WEIGHTS) return NULL;
    gchar *r = category_definitions[i].weights[num].weighttext;
    return r[0] == 0 ? NULL : r;
}

gint find_age_index(const gchar *category)
{
    gint i, male = -1, female = -1;
    gchar catbuf[32], ch = 0;

    if (category == NULL || category[0] == 0)
        return -1;

    g_strlcpy(catbuf, category, sizeof(catbuf));
    gchar *p = strrchr(catbuf, '-');
    if (!p)
        p = strrchr(catbuf, '+');
    if (p) {
        ch = *p;
        *p = 0;
    }

    for (i = 0; i < num_categories; i++) {
        if (category_definitions[i].agetext[0] == 0 && category_definitions[i].age == 0)
            continue;

        if (strcmp(category_definitions[i].agetext, catbuf) == 0) {
            if (category_definitions[i].gender & IS_MALE)
                male = i;
            else if (category_definitions[i].gender & IS_FEMALE)
                female = i;
            if (male >= 0 && female >= 0)
                break;
        }
    }

    if (male >= 0 && female >= 0) {
        if (!p)
            return -1;

        *p = ch;
        for (i = 0; i < NUM_CAT_DEF_WEIGHTS && category_definitions[male].weights[i].weight; i++) {
            if (strcmp(p, category_definitions[male].weights[i].weighttext) == 0)
                return male;
        }
        return female;
    } else if (male >= 0)
        return male;
    else if (female >= 0)
        return female;

    return -1;
}

gint find_age_index_by_age(gint age, gint gender)
{
    gint i, age_ix = -1, best_age = 100000000;

    for (i = 0; i < num_categories; i++) {
        if (age <= category_definitions[i].age &&
            best_age > category_definitions[i].age &&
            gender == category_definitions[i].gender) {
            age_ix = i;
            best_age = category_definitions[i].age;
        }
    }

    return age_ix;
}

static gint find_weight_index(gint ageix, const gchar *category)
{
    gint i;

    for (i = 0; i < NUM_CAT_DEF_WEIGHTS && category_definitions[ageix].weights[i].weight; i++) {
        if (category == NULL || category_definitions[ageix].weights[i].weighttext[0] == 0)
            continue;
        if (strstr(category, category_definitions[ageix].weights[i].weighttext))
            return i;
    }

    return -1;
}

static gint find_weight_index_by_weight(gint ageix, gint weight)
{
    gint i, w_ix = -1, best_weight = 100000000;

    for (i = 0; i < NUM_CAT_DEF_WEIGHTS && category_definitions[ageix].weights[i].weight; i++) {
        if (weight <= category_definitions[ageix].weights[i].weight &&
            best_weight > category_definitions[ageix].weights[i].weight) {
            w_ix = i;
            best_weight = category_definitions[ageix].weights[i].weight;
        }
    }

    return w_ix;
}

gboolean fill_in_next_match_message_data(const gchar *cat, struct msg_next_match *msg)
{
    gint i = find_age_index(cat);
    if (i < 0)
        return FALSE;

    msg->match_time = category_definitions[i].match_time;
    msg->gs_time = category_definitions[i].gs_time;
    msg->rep_time = category_definitions[i].rep_time;
    msg->rest_time = category_definitions[i].rest_time;
    msg->pin_time_ippon = category_definitions[i].pin_time_ippon;
    msg->pin_time_wazaari = category_definitions[i].pin_time_wazaari;
    msg->pin_time_yuko = category_definitions[i].pin_time_yuko;
    msg->pin_time_koka = category_definitions[i].pin_time_koka;

    return TRUE;
}

gint get_category_rest_time(const gchar *cat)
{
    gint i = find_age_index(cat);
    if (i < 0)
        return 0;
    return category_definitions[i].rest_time;
}

gint get_category_match_time(const gchar *cat)
{
    gint i = find_age_index(cat);
    if (i < 0)
        return 0;
    return category_definitions[i].match_time;
}

gchar *find_correct_category(gint age, gint weight, gint gender, const gchar *category_now, gboolean best_match)
{
    gint i = -1, age_ix = 0, w_ix, weight1 = 0;

    if ((age == 0 || gender == 0) && category_now) {
        i = find_age_index(category_now);
        if (i >= 0) {
            if (age == 0)
                age = category_definitions[i].age;
            if (gender == 0)
                gender = category_definitions[i].gender;
        }
    }

    if (age == 0 || gender == 0)
        return NULL;

    if (i >= 0)
        age_ix = i;
    else
        age_ix = find_age_index_by_age(age, gender);

    if (age_ix < 0)
        return NULL;

    /* What is the weight category indicates? */
    if (category_now) {
        w_ix = find_weight_index(age_ix, category_now);
        if (w_ix >= 0)
            weight1 = category_definitions[age_ix].weights[w_ix].weight;
    }

    /* If we have no real weight or we want to keep the reg. category use weight1 */
    if (weight == 0 || (weight1 > weight && best_match == FALSE)) {
        weight = weight1;
    }

    if (weight == 0)
        return g_strdup_printf("%s", category_definitions[age_ix].agetext);

    w_ix = find_weight_index_by_weight(age_ix, weight);
    if (w_ix < 0)
        return NULL;

    return g_strdup_printf("%s%s",
                           category_definitions[age_ix].agetext,
                           category_definitions[age_ix].weights[w_ix].weighttext);
}

gint compare_categories(gchar *cat1, gchar *cat2)
{
    gint a1, a2, w1, w2;

    if (cat1 == NULL)
        return -1;
    if (cat2 == NULL)
        return 1;

    /* age is the most significant */
    a1 = find_age_index(cat1);
    a2 = find_age_index(cat2);
    if (a1 < 0 && a2 >= 0)
        return -1;
    if (a2 < 0 && a1 >= 0)
        return 1;
    if (a1 < 0 || a2 < 0)
        goto out;

    if (category_definitions[a1].age > category_definitions[a2].age)
        return 1;
    if (category_definitions[a1].age < category_definitions[a2].age)
        return -1;

    /* gender is the next significant criteria */
    if ((category_definitions[a1].gender & IS_MALE) &&
        (category_definitions[a2].gender & IS_FEMALE))
        return 1;
    if ((category_definitions[a1].gender & IS_FEMALE) &&
        (category_definitions[a2].gender & IS_MALE))
        return -1;

    /* weight is the least significant criteria */
    w1 = find_weight_index(a1, cat1);
    w2 = find_weight_index(a2, cat2);
    if (w1 < 0 && w2 >= 0)
        return -1;
    if (w2 < 0 && w1 >= 0)
        return 1;
    if (w1 < 0 || w2 < 0)
        goto out;

    if (category_definitions[a1].weights[w1].weight > category_definitions[a2].weights[w2].weight)
        return 1;
    if (category_definitions[a1].weights[w1].weight < category_definitions[a2].weights[w2].weight)
        return -1;

out:
    return strcmp(cat1, cat2);
}

extern void db_insert_cat_def_table_data_begin(void);
extern void db_insert_cat_def_table_data_end(void);

static void init_cat_data(void)
{
    gint i, j, k, n;
    struct cat_def def;

    i = draw_system;
    if (i >= NUM_DRAWS)
        i = 0;

    db_insert_cat_def_table_data_begin();

    for (j = 0; j < 2; j++) {
        for (k = 0; official_categories[i][j][k].name; k++) {
            def.age = official_categories[i][j][k].maxage;
            strcpy(def.agetext, official_categories[i][j][k].name);
            def.match_time = official_categories[i][j][k].match_time;
            def.rest_time = official_categories[i][j][k].rest_time;
            def.gs_time = official_categories[i][j][k].gs_time;
            def.rep_time = 0;
            if (i == DRAW_FINNISH && def.age <= 10) { // E-juniors
                def.pin_time_koka = 0;
                def.pin_time_yuko = 0;
                def.pin_time_wazaari = 5;
                def.pin_time_ippon = 15;
	    } else if (prop_get_int_val(PROP_RULES_2017)) {
                def.pin_time_koka = 0;
                def.pin_time_yuko = 0;
                def.pin_time_wazaari = 10;
                def.pin_time_ippon = 20;
            } else {
                def.pin_time_koka = 0;
                def.pin_time_yuko = 10;
                def.pin_time_wazaari = 15;
                def.pin_time_ippon = 20;
            }

            def.gender = (j == 0) ? IS_MALE : IS_FEMALE;

            for (n = 0; official_categories[i][j][k].weights[n].weight; n++) {
                def.weights[0].weight = official_categories[i][j][k].weights[n].weight;
                sprintf(def.weights[0].weighttext, "-%d", def.weights[0].weight/1000);
                db_insert_cat_def_table_data(&def);
            }

            sprintf(def.weights[0].weighttext, "+%d", def.weights[0].weight/1000);
            def.weights[0].weight = 1000000;
            db_insert_cat_def_table_data(&def);
        } // k
    } // j

    db_insert_cat_def_table_data_end();
}

void read_cat_definitions(void)
{
    gint row;
    gint numrows;

    if (catdef_needs_init())
        init_cat_data();

    memset(&category_definitions, 0, sizeof(category_definitions));
    num_categories = 0;

    numrows = db_get_table("select * from catdef order by \"age\", \"weight\", \"flags\" asc");
    if (numrows < 0) {
        g_print("FATAL ERROR: CANNOT OPEN CAT DEF TABLE!\n");
        goto out;
    }

    for (row = 0; row < numrows; row++) {
        gchar *agetext = db_get_data(row, "agetext");
        gchar *agestr = db_get_data(row, "age");
        gchar *weighttxt = db_get_data(row, "weighttext");
        gchar *weightstr = db_get_data(row, "weight");
        gchar *flagsstr = db_get_data(row, "flags");
        gchar *matchtimestr = db_get_data(row, "matchtime");
        gchar *resttimestr = db_get_data(row, "resttime");
        gchar *gstimestr = db_get_data(row, "gstime");
        gchar *reptimestr = db_get_data(row, "reptime");
        gchar *pintimekokastr = db_get_data(row, "pintimekoka");
        gchar *pintimeyukostr = db_get_data(row, "pintimeyuko");
        gchar *pintimewazaaristr = db_get_data(row, "pintimewazaari");
        gchar *pintimeipponstr = db_get_data(row, "pintimeippon");
        gint age = atoi(agestr);
        gint weight = atoi(weightstr);
        gint flags = atoi(flagsstr);
        gint matchtime = atoi(matchtimestr);
        gint resttime = atoi(resttimestr);
        gint gstime = atoi(gstimestr);
        gint reptime = atoi(reptimestr);
        gint pintimekoka = atoi(pintimekokastr);
        gint pintimeyuko = atoi(pintimeyukostr);
        gint pintimewazaari = atoi(pintimewazaaristr);
        gint pintimeippon = atoi(pintimeipponstr);

        //g_print("CAT DEF LINE: %s %d %s %d %d matchtime=%d\n",
        //	agetext, age, weighttxt, weight, flags, matchtime);

        /* find existing category line */
        gint i;
        for (i = 0; i < num_categories; i++) {
            if (strcmp(category_definitions[i].agetext, agetext) == 0 &&
                category_definitions[i].gender == flags)
                break;
        }
        /* new category */
        if (i >= num_categories)
            num_categories = i + 1;

        category_definitions[i].age = age;
        category_definitions[i].gender = flags;
        strcpy(category_definitions[i].agetext, agetext);
        category_definitions[i].match_time = matchtime;
        category_definitions[i].rest_time = resttime;
        category_definitions[i].gs_time = gstime;
        category_definitions[i].rep_time = reptime;
        category_definitions[i].pin_time_koka = pintimekoka;
        category_definitions[i].pin_time_yuko = pintimeyuko;
        category_definitions[i].pin_time_wazaari = pintimewazaari;
        category_definitions[i].pin_time_ippon = pintimeippon;

        gint j;
        for (j = 0; category_definitions[i].weights[j].weight && j < NUM_CAT_DEF_WEIGHTS; j++)
            ;
        if (j < NUM_CAT_DEF_WEIGHTS) {
            category_definitions[i].weights[j].weight = weight;
            strcpy(category_definitions[i].weights[j].weighttext, weighttxt);
        }
    }

out:
    db_close_table();
}

#define Q_RESET 1000

void set_categories_dialog(GtkWidget *w, gpointer arg)
{
    GtkWidget *dialog, *tmp, *scrolled_window1, *scrolled_window2,
	*vbox1, *vbox2, *tables[NUM_CATEGORIES];
    struct {
        GtkWidget *age, *agetext, *matchtime, *resttime, *gstime, *reptime, *pink, *piny, *pinw, *pini;
        gint gender;
        struct {
            GtkWidget *weight, *weighttext;
        } weights[NUM_CAT_DEF_WEIGHTS];
    } fields[NUM_CATEGORIES];
    gint i, j, m = 0, f = NUM_CATEGORIES/2;

    dialog = gtk_dialog_new_with_buttons (_("Categories"),
                                          NULL,
                                          GTK_DIALOG_MODAL,
                                          _("Reset to Defaults"), Q_RESET,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_OK, GTK_RESPONSE_OK,
                                          NULL);

    scrolled_window1 = gtk_scrolled_window_new(NULL, NULL);
    scrolled_window2 = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(scrolled_window1, FRAME_WIDTH, FRAME_HEIGHT/2);
    gtk_widget_set_size_request(scrolled_window2, FRAME_WIDTH, FRAME_HEIGHT/2);
    gtk_container_set_border_width(GTK_CONTAINER(scrolled_window1), 4);
    gtk_container_set_border_width(GTK_CONTAINER(scrolled_window2), 4);

    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
                       scrolled_window1, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
                       scrolled_window2, FALSE, FALSE, 0);
    vbox1 = gtk_grid_new();
    vbox2 = gtk_grid_new();
    gint r = 0;

    for (i = 0; i < NUM_CATEGORIES; i++)
        tables[i] = gtk_grid_new();

    tmp = gtk_label_new(_("----- Men -----"));
    gtk_grid_attach(GTK_GRID(vbox1), tmp, 0, r++, 1, 1);
    gtk_widget_set_halign(tmp, GTK_ALIGN_START);	\
    for (i = 0; i < NUM_CATEGORIES/2; i++) {
        gtk_grid_attach(GTK_GRID(vbox1), tables[i], 0, r++, 1, 1);
        gtk_grid_attach(GTK_GRID(vbox1), gtk_label_new(" "), 0, r++, 1, 1);
    }
    r = 0;
    tmp = gtk_label_new(_("----- Women -----"));
    gtk_grid_attach(GTK_GRID(vbox2), tmp, 0, r++, 1, 1);
    gtk_widget_set_halign(tmp, GTK_ALIGN_START);	\
    for (i = NUM_CATEGORIES/2; i < NUM_CATEGORIES; i++) {
        gtk_grid_attach(GTK_GRID(vbox2), tables[i], 0, r++, 1, 1);
        gtk_grid_attach(GTK_GRID(vbox2), gtk_label_new(" "), 0, r++, 1, 1);
    }

#if (GTKVER == 3) && GTK_CHECK_VERSION(3,8,0)
    gtk_container_add(GTK_CONTAINER(scrolled_window1), vbox1);
    gtk_container_add(GTK_CONTAINER(scrolled_window2), vbox2);
#else
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled_window1), vbox1);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled_window2), vbox2);
#endif

    for (i = 0; i < NUM_CATEGORIES; i++) {
        gtk_grid_attach(GTK_GRID(tables[i]), gtk_label_new(_("Highest age:")),      0, 0, 1, 1);
        gtk_grid_attach(GTK_GRID(tables[i]), gtk_label_new(_("Age text:")),         0, 1, 1, 1);
        gtk_grid_attach(GTK_GRID(tables[i]), gtk_label_new(_("Match time:")),       2, 0, 1, 1);
        gtk_grid_attach(GTK_GRID(tables[i]), gtk_label_new(_("Rest time:")),        2, 2, 1, 1);
        gtk_grid_attach(GTK_GRID(tables[i]), gtk_label_new(_("Golden Score:")),     2, 1, 1, 1);
        gtk_grid_attach(GTK_GRID(tables[i]), gtk_label_new(_("Pin times (IWYK):")), 4, 0, 4, 1);
        gtk_grid_attach(GTK_GRID(tables[i]), gtk_label_new(_("Rep. time:")),        4, 2, 3, 1);

#define ATTACH_ENTRY(_what, _len, _width, _col, _row)			\
	do {								\
	    tmp = fields[i]._what = gtk_entry_new();			\
	    gtk_entry_set_max_length(GTK_ENTRY(tmp), _len);		\
	    gtk_entry_set_width_chars(GTK_ENTRY(tmp), _width);		\
	    gtk_grid_attach(GTK_GRID(tables[i]), tmp, _col, _row, 1, 1);	\
	} while (0)

        /* age */
	ATTACH_ENTRY(age, 4, 10, 1, 0);
        /* age text */
	ATTACH_ENTRY(agetext, 20, 10, 1, 1);
        /* match time */
	ATTACH_ENTRY(matchtime, 3, 3, 3, 0);
        /* rest time */
	ATTACH_ENTRY(resttime, 3, 3, 3, 2);
        /* golden score time */
	ATTACH_ENTRY(gstime, 3, 3, 3, 1);
        /* repechage time */
	ATTACH_ENTRY(reptime, 3, 3, 7, 2);
        /* pin time ippon */
	ATTACH_ENTRY(pini, 2, 2, 4, 1);
        /* pin time waza-ari */
	ATTACH_ENTRY(pinw, 2, 2, 5, 1);
        /* pin time yuko */
	ATTACH_ENTRY(piny, 2, 2, 6, 1);
        /* pin time koka */
	ATTACH_ENTRY(pink, 2, 2, 7, 1);

        /* weights */
        gtk_grid_attach(GTK_GRID(tables[i]), gtk_label_new(_("Highest weight (g):")), 8, 0, 1, 1);
        gtk_grid_attach(GTK_GRID(tables[i]), gtk_label_new(_("Weight text:")),        8, 1, 1, 1);

        for (j = 0; j < NUM_CAT_DEF_WEIGHTS; j++) {
            tmp = fields[i].weights[j].weight = gtk_entry_new();
            gtk_entry_set_max_length(GTK_ENTRY(tmp), 12);
            gtk_entry_set_width_chars(GTK_ENTRY(tmp), 6);
            gtk_grid_attach(GTK_GRID(tables[i]), tmp, j+9, 0, 1, 1);
            tmp = fields[i].weights[j].weighttext = gtk_entry_new();
            gtk_entry_set_max_length(GTK_ENTRY(tmp), 12);
            gtk_entry_set_width_chars(GTK_ENTRY(tmp), 6);
            gtk_grid_attach(GTK_GRID(tables[i]), tmp, j+9, 1, 1, 1);
        }
    }

    for (i = 0; i < NUM_CATEGORIES; i++) {
        gchar buf[32], mt[8], rt[8], gt[8], pt[8], it[8], wt[8], yt[8], kt[8];

        for (j = 0; j < NUM_CAT_DEF_WEIGHTS; j++) {
            if (category_definitions[i].weights[j].weight == 0)
                continue;

            sprintf(buf, "%d", category_definitions[i].weights[j].weight);
            if (category_definitions[i].gender & IS_MALE) {
                gtk_entry_set_text(GTK_ENTRY(fields[m].weights[j].weight), buf);
                gtk_entry_set_text(GTK_ENTRY(fields[m].weights[j].weighttext),
                                   category_definitions[i].weights[j].weighttext);
            } else if (category_definitions[i].gender & IS_FEMALE) {
                gtk_entry_set_text(GTK_ENTRY(fields[f].weights[j].weight), buf);
                gtk_entry_set_text(GTK_ENTRY(fields[f].weights[j].weighttext),
                                   category_definitions[i].weights[j].weighttext);
            }
        }

        sprintf(buf, "%d", category_definitions[i].age);
        sprintf(mt, "%d", category_definitions[i].match_time);
        sprintf(rt, "%d", category_definitions[i].rest_time);
        sprintf(gt, "%d", category_definitions[i].gs_time);
        sprintf(pt, "%d", category_definitions[i].rep_time);
        sprintf(it, "%d", category_definitions[i].pin_time_ippon);
        sprintf(wt, "%d", category_definitions[i].pin_time_wazaari);
        sprintf(yt, "%d", category_definitions[i].pin_time_yuko);
        sprintf(kt, "%d", category_definitions[i].pin_time_koka);

	gint x = m;
	if (category_definitions[i].gender & IS_FEMALE) x = f;

	gtk_entry_set_text(GTK_ENTRY(fields[x].age), buf);
	gtk_entry_set_text(GTK_ENTRY(fields[x].agetext), category_definitions[i].agetext);
	gtk_entry_set_text(GTK_ENTRY(fields[x].matchtime), mt);
	gtk_entry_set_text(GTK_ENTRY(fields[x].resttime), rt);
	gtk_entry_set_text(GTK_ENTRY(fields[x].gstime), gt);
	gtk_entry_set_text(GTK_ENTRY(fields[x].reptime), pt);
	gtk_entry_set_text(GTK_ENTRY(fields[x].pini), it);
	gtk_entry_set_text(GTK_ENTRY(fields[x].pinw), wt);
	gtk_entry_set_text(GTK_ENTRY(fields[x].piny), yt);
	gtk_entry_set_text(GTK_ENTRY(fields[x].pink), kt);

	if (category_definitions[i].gender & IS_FEMALE) {
            if (f < NUM_CATEGORIES-1) f++;
	} else {
            if (m < NUM_CATEGORIES/2-1) m++;
	}
    }

    gtk_widget_show_all(dialog);

    gint response = gtk_dialog_run(GTK_DIALOG(dialog));

    if (response == GTK_RESPONSE_OK) {
        struct cat_def def;

        db_delete_cat_def_table_data();
	db_insert_cat_def_table_data_begin();

        for (i = 0; i < NUM_CATEGORIES; i++) {
            def.age = atoi(gtk_entry_get_text(GTK_ENTRY(fields[i].age)));
            strcpy(def.agetext, gtk_entry_get_text(GTK_ENTRY(fields[i].agetext)));
            def.match_time = atoi(gtk_entry_get_text(GTK_ENTRY(fields[i].matchtime)));
            def.rest_time = atoi(gtk_entry_get_text(GTK_ENTRY(fields[i].resttime)));
            def.gs_time = atoi(gtk_entry_get_text(GTK_ENTRY(fields[i].gstime)));
            def.rep_time = atoi(gtk_entry_get_text(GTK_ENTRY(fields[i].reptime)));
            def.pin_time_ippon = atoi(gtk_entry_get_text(GTK_ENTRY(fields[i].pini)));
            def.pin_time_wazaari = atoi(gtk_entry_get_text(GTK_ENTRY(fields[i].pinw)));
            def.pin_time_yuko = atoi(gtk_entry_get_text(GTK_ENTRY(fields[i].piny)));
            def.pin_time_koka = atoi(gtk_entry_get_text(GTK_ENTRY(fields[i].pink)));

            if (i < NUM_CATEGORIES/2)
                def.gender = IS_MALE;
            else
                def.gender = IS_FEMALE;

            for (j = 0; j < NUM_CAT_DEF_WEIGHTS; j++) {
                def.weights[0].weight =
                    atoi(gtk_entry_get_text(GTK_ENTRY(fields[i].weights[j].weight)));
                strcpy(def.weights[0].weighttext,
                       gtk_entry_get_text(GTK_ENTRY(fields[i].weights[j].weighttext)));
                if (def.age && def.weights[0].weight)
                    db_insert_cat_def_table_data(&def);
            }
        }

	db_insert_cat_def_table_data_end();
        read_cat_definitions();
        db_cat_def_table_done();

        // update next matches
        gint t;
        for (t = 1; t <= number_of_tatamis; t++) {
            struct match *nm = get_cached_next_matches(t);
            if (nm) send_next_matches(0, t, nm);
        }
    } else if (response == Q_RESET) {
        db_delete_cat_def_table_data();
        init_cat_data();
        read_cat_definitions();
        db_cat_def_table_done();
    }

    update_category_status_info_all();
    gtk_widget_destroy(dialog);
}

struct catdefwidgets {
    GtkWidget *age, *agetext, *matchtime, *resttime, *gstime, *reptime,
	*pink, *piny, *pinw, *pini, *gen, *copysrc;
    struct {
	GtkWidget *weight, *weighttext;
    } weights[NUM_CAT_DEF_WEIGHTS];
    gchar newname[32];
    gboolean new_team_cat;
};

static void set_dialog_values(struct catdefwidgets *fields, gint i)
{
    gchar buf[32];
    gint j;

    if (fields->newname[0])
	gtk_entry_set_text(GTK_ENTRY(fields->agetext), fields->newname);

    if (i < 0) {
	if (fields->new_team_cat) {
	    gchar *weighs[5] = {"66000", "73000", "81000", "90000", "1000000"};
	    gchar *weightexts[5] = {"-66", "-73", "-81", "-90", "+90"};
	    for (j = 0; j < 5; j++) {
		gtk_entry_set_text(GTK_ENTRY(fields->weights[j].weight), weighs[j]);
		gtk_entry_set_text(GTK_ENTRY(fields->weights[j].weighttext), weightexts[j]);
	    }
	    gtk_combo_box_set_active(GTK_COMBO_BOX(fields->gen), 0);
	    gtk_entry_set_text(GTK_ENTRY(fields->age), "1");
	    gtk_editable_set_editable(GTK_EDITABLE(fields->age), FALSE);
	    gtk_entry_set_text(GTK_ENTRY(fields->matchtime), "240");
	    gtk_entry_set_text(GTK_ENTRY(fields->resttime), "240");
	    gtk_entry_set_text(GTK_ENTRY(fields->gstime), "0");
	    gtk_entry_set_text(GTK_ENTRY(fields->reptime), "0");
	    gtk_entry_set_text(GTK_ENTRY(fields->pini), "20");
	    gtk_entry_set_text(GTK_ENTRY(fields->pinw), "10");
	    gtk_entry_set_text(GTK_ENTRY(fields->piny), "0");
	    gtk_entry_set_text(GTK_ENTRY(fields->pink), "0");
	}
	return;
    }

    for (j = 0; j < NUM_CAT_DEF_WEIGHTS; j++) {
	if (category_definitions[i].weights[j].weight) {
	    sprintf(buf, "%d", category_definitions[i].weights[j].weight);
	    gtk_entry_set_text(GTK_ENTRY(fields->weights[j].weight), buf);
	} else
	    gtk_entry_set_text(GTK_ENTRY(fields->weights[j].weight), "");
	gtk_entry_set_text(GTK_ENTRY(fields->weights[j].weighttext),
			   category_definitions[i].weights[j].weighttext);
    }

    if (category_definitions[i].gender == IS_FEMALE)
	gtk_combo_box_set_active(GTK_COMBO_BOX(fields->gen), 1);
    else
	gtk_combo_box_set_active(GTK_COMBO_BOX(fields->gen), 0);

#define SET_FIELD_TEXT(_dst, _src) do {			\
	sprintf(buf, "%d", category_definitions[i]._src);	\
	gtk_entry_set_text(GTK_ENTRY(fields->_dst), buf);	\
    } while (0)

    SET_FIELD_TEXT(age, age);
    SET_FIELD_TEXT(matchtime, match_time);
    SET_FIELD_TEXT(resttime, rest_time);
    SET_FIELD_TEXT(gstime, gs_time);
    SET_FIELD_TEXT(reptime, rep_time);
    SET_FIELD_TEXT(pini, pin_time_ippon);
    SET_FIELD_TEXT(pinw, pin_time_wazaari);
    SET_FIELD_TEXT(piny, pin_time_yuko);
    SET_FIELD_TEXT(pink, pin_time_koka);
}

static void edit_category_dialog_callback(GtkWidget *widget,
					  GdkEvent *event,
					  gpointer data)
{
    struct catdefwidgets *fields = data;
    gint event_id = ptr_to_gint(event);
    gint j;

    if (event_id == GTK_RESPONSE_OK) {
        struct cat_def def;
	db_delete_cat_def_table_age(gtk_entry_get_text(GTK_ENTRY(fields->agetext)));
	db_insert_cat_def_table_data_begin();

	def.age = atoi(gtk_entry_get_text(GTK_ENTRY(fields->age)));
	strcpy(def.agetext, gtk_entry_get_text(GTK_ENTRY(fields->agetext)));
	def.match_time = atoi(gtk_entry_get_text(GTK_ENTRY(fields->matchtime)));
	def.rest_time = atoi(gtk_entry_get_text(GTK_ENTRY(fields->resttime)));
	def.gs_time = atoi(gtk_entry_get_text(GTK_ENTRY(fields->gstime)));
	def.rep_time = atoi(gtk_entry_get_text(GTK_ENTRY(fields->reptime)));
	def.pin_time_ippon = atoi(gtk_entry_get_text(GTK_ENTRY(fields->pini)));
	def.pin_time_wazaari = atoi(gtk_entry_get_text(GTK_ENTRY(fields->pinw)));
	def.pin_time_yuko = atoi(gtk_entry_get_text(GTK_ENTRY(fields->piny)));
	def.pin_time_koka = atoi(gtk_entry_get_text(GTK_ENTRY(fields->pink)));

	if (gtk_combo_box_get_active(GTK_COMBO_BOX(fields->gen)) == 0)
	    def.gender = IS_MALE;
	else
	    def.gender = IS_FEMALE;

	for (j = 0; j < NUM_CAT_DEF_WEIGHTS; j++) {
	    def.weights[0].weight =
		atoi(gtk_entry_get_text(GTK_ENTRY(fields->weights[j].weight)));
	    strcpy(def.weights[0].weighttext,
		   gtk_entry_get_text(GTK_ENTRY(fields->weights[j].weighttext)));
	    if (def.age && def.weights[0].weight)
		db_insert_cat_def_table_data(&def);
	}

	db_insert_cat_def_table_data_end();
        read_cat_definitions();
        db_cat_def_table_done();
    }

    if (event_id == GTK_RESPONSE_OK &&
	fields->new_team_cat) {
	teams_dialog(fields);
    }

    gtk_widget_destroy(widget);
}

static gboolean copy_from_existing(GtkWidget *w,
				   GdkEventButton *event,
				   gpointer userdata)
{
    struct catdefwidgets *fields = userdata;
    gint i = gtk_combo_box_get_active(GTK_COMBO_BOX(fields->copysrc));
    if (i < 0)
	return TRUE;
    set_dialog_values(fields, i);
    return TRUE;
}

void edit_category_dialog(gint ix, gboolean is_new_team)
{
    GtkWidget *dialog, *scrolled_window, *table, *tmp;
    struct catdefwidgets *fields = NULL;
    gint i, j;
    struct category_data *catdata = avl_get_category(ix);
    if (!catdata)
	return;

    is_new_team = catdata->deleted & TEAM_EVENT;

    fields = g_malloc0(sizeof(*fields));
    if (!fields) return;

    g_strlcpy(fields->newname, catdata->category,
	      sizeof(fields->newname));
    gchar *p = strrchr(fields->newname, '-');
    if (!p) p = strrchr(fields->newname, '+');
    if (p) *p = 0;
    fields->new_team_cat = is_new_team;

    i = find_age_index(catdata->category);

    dialog = gtk_dialog_new_with_buttons (catdata->category,
                                          GTK_WINDOW(main_window),
					  GTK_DIALOG_DESTROY_WITH_PARENT,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_OK, GTK_RESPONSE_OK,
                                          NULL);

    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(scrolled_window, FRAME_WIDTH, 240 /*FRAME_HEIGHT*/);
    gtk_container_set_border_width(GTK_CONTAINER(scrolled_window), 4);
    table = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(scrolled_window), table);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		       scrolled_window, FALSE, FALSE, 0);

#define ATTACH_TABLE(_what, _col, _row)				\
    gtk_grid_attach(GTK_GRID(table), _what, _col, _row, 1, 1)

#define ATTACH_TABLE_W(_what, _col, _row, _w)			\
    gtk_grid_attach(GTK_GRID(table), _what, _col, _row, _w, 1)

#define ATTACH_ENTRY2(_what, _len, _width, _col, _row)			\
    do {								\
	tmp = fields->_what = gtk_entry_new();				\
	gtk_entry_set_max_length(GTK_ENTRY(tmp), _len);			\
	gtk_entry_set_width_chars(GTK_ENTRY(tmp), _width);		\
	ATTACH_TABLE(tmp, _col, _row);					\
    } while (0)

#define ATTACH_LABEL_W(_label, _col, _row, _w)			\
    do {							\
	GtkWidget *_tmp = gtk_label_new(_label);		\
	ATTACH_TABLE_W(_tmp, _col, _row, _w);			\
	gtk_widget_set_halign(_tmp, GTK_ALIGN_END);		\
    } while (0)

#define ATTACH_LABEL(_label, _col, _row)			\
    ATTACH_LABEL_W(_label, _col, _row, 1)	\


    ATTACH_LABEL_W(_("Highest age:"),    2, 0, 2);
    ATTACH_LABEL(_("Age text:"),         0, 0);
    ATTACH_LABEL(_("Match time:"),       0, 1);
    ATTACH_LABEL_W(_("Rest time:"),      2, 1, 2);
    ATTACH_LABEL_W(_("Golden Score:"),   5, 1, 2);
    ATTACH_LABEL_W(_("Rep. time:"),      8, 1, 2);
    ATTACH_LABEL(_("Pin times (IWYK):"), 0, 2);
    ATTACH_LABEL(_("Gender:"),           0, 5);

    /* age */
    ATTACH_ENTRY2(age, 4, 4, 4, 0);
    /* age text */
    ATTACH_ENTRY2(agetext, 20, 8, 1, 0);
    gtk_editable_set_editable(GTK_EDITABLE(fields->agetext), FALSE);
    /* match time */
    ATTACH_ENTRY2(matchtime, 3, 3, 1, 1);
    /* rest time */
    ATTACH_ENTRY2(resttime, 3, 3, 4, 1);
    /* golden score time */
    ATTACH_ENTRY2(gstime, 3, 3, 7, 1);
    /* repechage time */
    ATTACH_ENTRY2(reptime, 3, 3, 10, 1);
    /* pin time ippon */
    ATTACH_ENTRY2(pini, 2, 2, 1, 2);
    /* pin time waza-ari */
    ATTACH_ENTRY2(pinw, 2, 2, 2, 2);
    /* pin time yuko */
    ATTACH_ENTRY2(piny, 2, 2, 3, 2);
    /* pin time koka */
    ATTACH_ENTRY2(pink, 2, 2, 4, 2);

    /* weights */
    ATTACH_LABEL(_("Highest weight (g):"), 0, 3);
    ATTACH_LABEL(_("Weight text:"),        0, 4);
    for (j = 0; j < NUM_CAT_DEF_WEIGHTS; j++) {
	ATTACH_ENTRY2(weights[j].weight, 12, 6, j+1, 3);
	ATTACH_ENTRY2(weights[j].weighttext, 12, 6, j+1, 4);
    }

    /* Gender */
    tmp = fields->gen = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(tmp), NULL, _("Male"));
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(tmp), NULL, _("Female"));
    ATTACH_TABLE(tmp, 1, 5);

    /* Copy */
    tmp = fields->copysrc = gtk_combo_box_text_new();
    for (j = 0; j < NUM_CATEGORIES; j++) {
	if (category_definitions[j].age &&
	    category_definitions[j].agetext[0])
	    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(tmp), NULL,
				      category_definitions[j].agetext);
    }
    ATTACH_TABLE(tmp, 1, 6);
    tmp = gtk_button_new_with_label(_("Copy"));
    ATTACH_TABLE(tmp, 2, 6);
    g_signal_connect(G_OBJECT(tmp), "button-press-event",
		     (GCallback)copy_from_existing, fields);

    set_dialog_values(fields, i);
    gtk_widget_show_all(dialog);
    g_signal_connect(G_OBJECT(dialog), "response",
                     G_CALLBACK(edit_category_dialog_callback), fields);
}

struct competitor_w {
    GtkWidget *team;
    gchar eventname[32];
    struct {
	GtkWidget *last, *first;
	gchar cat[32];
    } names[NUM_CAT_DEF_WEIGHTS];
    gint numw;
};

static void judoka_defaults(struct judoka *j)
{
    memset(j, 0, sizeof(*j));
    j->last = "";
    j->first = "";
    j->club = "";
    j->regcategory = "";
    j->category = "";
    j->country = "";
    j->comment = "";
    j->coachid = "";
}

static void team_changed_callback(GtkWidget *w, gpointer arg) 
{
    struct competitor_w *comps = arg;
    gint i;
    for (i = 0; i < comps->numw; i++) {
	gtk_entry_set_text(GTK_ENTRY(comps->names[i].last),
			   gtk_entry_get_text(GTK_ENTRY(comps->team)));
    }
}

static void teams_dialog_callback(GtkWidget *widget,
				  GdkEvent *event,
				  gpointer data)
{
    struct competitor_w *comps = data;
    gint event_id = ptr_to_gint(event);

    if (event_id == GTK_RESPONSE_OK) {
	struct judoka j;
	const gchar *name = gtk_entry_get_text(GTK_ENTRY(comps->team));
	struct compsys system;
	system.numcomp = comps->numw;
	system.system = system.table = system.wishsys = 0;

        GtkTreeIter tmp_iter;
        if (find_iter_category(&tmp_iter, name)) {
            struct judoka *g = get_data_by_iter(&tmp_iter);
            gint gix = g ? g->index : 0;
            free_judoka(g);

            if (gix) {
                SHOW_MESSAGE("%s %s %s", _("Team"), name, _("already exists!"));
                return;
            }
        }

	judoka_defaults(&j);

	// add team as competitor
	j.last = name;
	j.category = comps->eventname;
        j.visible = TRUE;
        j.index = current_index++;
        db_add_judoka(j.index, &j);
	gint ret = display_one_judoka(&j);
	if (ret >= 0) {
            db_update_judoka(j.index, &j);
	    update_competitors_categories(j.index);
	}

	// add team
        j.index = 0;
	j.last = name;
	j.deleted = TEAM;
        j.weight = compress_system(system);
	j.category = "";
        j.visible = FALSE;
	ret = display_one_judoka(&j);
	if (ret >= 0) {
	    j.index = ret;
	    db_add_category(ret, &j);
            db_set_system(ret, system);
            db_update_judoka(ret, &j);
	    //db_update_category(ret, &j);
	}

	// add team members
        j.visible = TRUE;
        j.weight = 0;
        j.deleted = 0;
        j.club = name;

        gint k;
        for (k = 0; k < comps->numw; k++) {
            j.index = current_index++;
            j.first = gtk_entry_get_text(GTK_ENTRY(comps->names[k].first));
            j.last = gtk_entry_get_text(GTK_ENTRY(comps->names[k].last));
            j.category = name;
            j.regcategory = comps->names[k].cat;

            db_add_judoka(j.index, &j);
            ret = display_one_judoka(&j);
        }

	return;
    }

    g_free(comps);
    gtk_widget_destroy(widget);
}

static void teams_dialog(struct catdefwidgets *fields)
{
    GtkWidget *dialog, *scrolled_window, *table, *tmp;
    gint i, wclasses = 0;
    struct competitor_w *comps;

    comps = g_malloc0(sizeof(*comps));
    if (!comps) return;

    dialog = gtk_dialog_new_with_buttons (gtk_entry_get_text(GTK_ENTRY(fields->agetext)),
                                          GTK_WINDOW(main_window),
					  GTK_DIALOG_DESTROY_WITH_PARENT,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_NEW, GTK_RESPONSE_OK,
                                          NULL);

    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(scrolled_window, FRAME_WIDTH, FRAME_HEIGHT);
    gtk_container_set_border_width(GTK_CONTAINER(scrolled_window), 4);
    table = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(scrolled_window), table);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		       scrolled_window, FALSE, FALSE, 0);

    GtkWidget *w = gtk_label_new(_("Team:"));
    gtk_grid_attach(GTK_GRID(table), w, 0, 0, 1, 1);

    comps->team = tmp = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(tmp), 20);
    gtk_entry_set_width_chars(GTK_ENTRY(tmp), 10);
    gtk_grid_attach(GTK_GRID(table), tmp, 1, 0, 2, 1);
    g_signal_connect(G_OBJECT(tmp), "changed", G_CALLBACK(team_changed_callback), comps);

    gtk_grid_attach(GTK_GRID(table), gtk_label_new(_("Last Name:")), 0, 2, 2, 1);
    gtk_grid_attach(GTK_GRID(table), gtk_label_new(_("First Name:")), 0, 3, 2, 1);

    for (i = 0; i < NUM_CAT_DEF_WEIGHTS; i++) {
	if (!fields->weights[i].weighttext) continue;
	const gchar *t = gtk_entry_get_text(GTK_ENTRY(fields->weights[i].weighttext));
	if (!t || !t[0]) continue;
	w = gtk_label_new(t);
	gtk_grid_attach(GTK_GRID(table), w, 3+wclasses, 1, 1, 1);

	comps->names[wclasses].last = tmp = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(tmp), 20);
	gtk_entry_set_width_chars(GTK_ENTRY(tmp), 20);
	gtk_grid_attach(GTK_GRID(table), tmp, 3+wclasses, 2, 1, 1);
	gtk_entry_set_text(GTK_ENTRY(tmp), "");

	comps->names[wclasses].first = tmp = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(tmp), 20);
	gtk_entry_set_width_chars(GTK_ENTRY(tmp), 20);
	gtk_grid_attach(GTK_GRID(table), tmp, 3+wclasses, 3, 1, 1);
	gtk_entry_set_text(GTK_ENTRY(tmp), t);

	g_strlcpy(comps->names[wclasses].cat, t, 32);

	wclasses++;
    }

    g_strlcpy(comps->eventname, gtk_entry_get_text(GTK_ENTRY(fields->agetext)), 32);
    comps->numw = wclasses;

    gtk_widget_show_all(dialog);
    g_signal_connect(G_OBJECT(dialog), "response",
                     G_CALLBACK(teams_dialog_callback), comps);
}
