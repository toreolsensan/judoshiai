/* -*- mode: C; c-basic-offset: 4;  -*- */

/*
 * Copyright (C) 2006-2013 by Hannu Jokinen
 * Full copyright text is included in the software package.
 */ 

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <locale.h>

#ifdef WIN32
#include <process.h>
#else
#include <sys/types.h>
#include <unistd.h>
#include <sys/resource.h>
#endif

#include "custom-category.h"

/*
  Description file format:

  Match definition:
  <ident>: c<num> c<num>
  <ident>: <ident>.<num>[.p.<num>]* <ident>.<num>[.p.<num>]*

  rr <ident>: c<num> [c<num>]*

  ko <ident>: c<num> c<num> [c<num> c<num>]*

  pos<num> <ident>.<num>


  Examples:
  -----
  ko a_lohko: c1 c2 c3 c4
  ko b_lohko: c5 c6 c7 c8
  pronssi: a_lohko.2 b_lohko.2
  finaali: a_lohko.1 b_lohko.1

  pos1 finaali.1
  pos2 finaali.2
  pos3 pronssi.1
  -----
  rr a_pooli: c1 c2 c3
  rr b_pooli: c4 c5 c6
  semari_a: a_pooli.1 b_pooli.2
  semari_b: a_pooli.2 b_pooli.1
  finaali: semari_a.1 semari_b.1

  pos1 finaali.1
  pos2 finaali.2
  pos3 semari_a.2
  pos3 semari_b.2
  -----
 */

static void block(void);

static FILE *f;


typedef enum { 
    dummy, rr, ko, match, dot, colon, prev, ident, number, 
    competitor, eol, eof, pos, dash, lparen, rparen, order,
    page, svg, info, err
} Symbol;

static char *labels[] = {
    "", "rr", "ko", "match", "p", "colon", "prev", "ident", "number", 
    "competitor", "eol", "eof", "pos", "dash", "lparen", "rparen", "order",
    "page", "svg", "info", "error"
};

typedef struct symbol {
    char *name;
    int   type;
    int   value;
} sym_t;

typedef struct player {
    int    comp;
    sym_t *match;
    int    pos;
    int    prev[8];
} competitor_t;

typedef struct fight {
    sym_t *name;
    int   type;
#define COMP_STAT_MATCHED 1
    int   status;
    int   level;
    int   pos;
    int   ordernum;
    int   number;
    int   matchord;
    competitor_t c1;
    competitor_t c2;
    int   page;
    double x, y, y1, y2;
    sym_t *reference;
#define FLAG_LONG_NAME 1
#define FLAG_LAST      2
    int flags;
} match_t;

typedef struct position {
    sym_t *match;
    int    pos;
    int    page;
} position_t;

static position_t positions[NUM_POSITIONS]; 

typedef struct round_robin {
    sym_t *name;
    sym_t *rr_matches[NUM_RR_MATCHES];
    int   num_rr_matches;
} round_robin_t;

static round_robin_t round_robin_pools[NUM_ROUND_ROBIN_POOLS];
static int num_round_robin_pools = 0;

#define NUM_SYMBOLS 1024 
static struct symbol symbols[NUM_SYMBOLS];
static int num_symbols = 0;

static match_t matches[NUM_CUSTOM_MATCHES];
static int num_matches = 0;
static int match_list[NUM_CUSTOM_MATCHES];

static Symbol sym;
static int value, numvalue, ordernum, pagenum = 1;
static char strvalue[256];
static int linenum = 1;
static char line[256];
static int n = 0;

static competitor_t compvalue;
static int stop = 0;

static int max_comp = 0;

static struct {
    competitor_t first, second;
} match_order[NUM_CUSTOM_MATCHES];
static int num_ord = 0;

static char name_long[64];
static char name_short[16];
static int  competitors_min, competitors_max;

static double x_shift = 100.0, y_shift = 60.0;

#ifdef MAKE_SVG_FILES

#define NUM_PAGES 16
static struct {
    double current_y;
    FILE *fd;
    int opened;
    char name[128];
    double res_start_y;
    int num_res;
#define PAGE_FLAG_POS_HDR 1
    int    flags;
} page_data[NUM_PAGES];

static double res_x = 200.0, res_w = 150, res_line = 16.0, x_init = 200.0;

static char *svg_start =
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
    "<svg\n"
    "   xmlns=\"http://www.w3.org/2000/svg\"   width=\"630px\"\n"
    "   height=\"891px\"\n"
    "   id=\"judoshiai\"\n"
    "   version=\"1.1\"\n"
    "   xmlns:sodipodi=\"http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd\"\n"
    "   xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\">\n"
    "<style type=\"text/css\"> <![CDATA[ tspan.cmpx { fill: red; } ]]> </style>\n"
    "<sodipodi:namedview\n"
    "id=\"base\"\n"
    "pagecolor=\"#ffffff\"\n"
    "bordercolor=\"#666666\"\n"
    "borderopacity=\"1.0\"\n"
    "inkscape:pageopacity=\"0.0\"\n"
    "inkscape:pageshadow=\"2\"\n"
    "inkscape:zoom=\"1\"\n"
    "inkscape:cx=\"185.84668\"\n"
    "inkscape:cy=\"880.85263\"\n"
    "inkscape:document-units=\"px\"\n"
    "inkscape:current-layer=\"layer1\"\n"
    "inkscape:window-width=\"877\"\n"
    "inkscape:window-height=\"739\"\n"
    "inkscape:window-x=\"177\"\n"
    "inkscape:window-y=\"31\"\n"
    "showgrid=\"false\"\n"
    "inkscape:window-maximized=\"0\" />\n"
    "   <g\n"
    "      id=\"layer1\"\n"
    "      style=\"opacity:1\">\n"
    "<rect x='0.00' y='0.00' width='630.00' height='891.00' style='fill:white;stroke:none'/>\n";

static char *svg_end =
    "   </g>\n"
    "</svg>\n";

static char *svg_page_header_text =
    "<text\n"
    "   style=\"font-size:21.38px;font-style:normal;font-weight:bold;letter-spacing:0px;\n"
    "           word-spacing:0px;fill:#000000;fill-opacity:1;stroke:none;font-family:Arial;\n"
    "           text-anchor:middle;text-align:center\"\n"
    "   x=\"315.00\" y=\"44.55\" id=\"text1100\">\n"
    "  <tspan id=\"tspan1300\">%i-competition  %i-date  %i-place   %i-catname</tspan></text>\n";

#define svg_text \
    "<text x=\"%.2f\" y=\"%.2f\" font-family=\"Arial\" font-size=\"10\" fill=\"black\"><tspan>%s</tspan></text>\n"

static char *competitor_style  = "font-size:10px;font-family:Arial;fill:#000000";
static char *competitor_name_1 = "hm1-first-s-last-s-club";
static char *competitor_name_2 = "hm1-last";
#endif

static char errstr[128];
#define ERR_SYM(_test, _str...) \
    do { if (_test) { snprintf(errstr, sizeof(errstr), _str); sym = err; goto out; } } while (0)

static sym_t *get_sym(char *name) {
    int i;
    for (i = 0; i < num_symbols; i++) {
        if (!strcmp(symbols[i].name, name))
            return &symbols[i];
    }

    symbols[num_symbols].name = strdup(name);
    return &symbols[num_symbols++];
}

static match_t *get_match(char *name) {
    int i;

    for (i = 0; i < num_matches; i++) {
        if (!strcmp(matches[i].name->name, name)) {
            return &matches[i];
        }
    }
    
    matches[num_matches].name = get_sym(name); 
    return &matches[num_matches++];
}

static void getsym(void)
{
    if (stop) {
        sym = eof;
        goto out;
    }

    int c = fgetc(f);
    while (c == ' ') c = fgetc(f);

    if (c == '\n') {
        linenum++;
        sym = eol;
        goto out;
    }

    if (c < 0 || c > 255) {
        sym = eof;
        goto out;
    }

    if (c == '.') {
        sym = dot;
        goto out;
    }

    if (c == ':') {
        sym = colon;
        goto out;
    }

    if (c == '-') {
        sym = dash;
        goto out;
    }

    if (c == '(') {
        sym = lparen;
        goto out;
    }

    if (c == ')') {
        sym = rparen;
        goto out;
    }

    if (c >= '0' && c <= '9') {
        value = c - '0';
        c = fgetc(f);
        while (c >= '0' && c <= '9') {
            value = 10*value + c - '0';
            c = fgetc(f);
        }
        ungetc(c, f);
        sym = number;
        goto out;
    }

    if (c == '"') {
        n = 0;
        c = fgetc(f);
        while (n < sizeof(line)-1 && c != '"') {
            ERR_SYM((c < ' '), "Invalid character");
            if (c == '\\') c = fgetc(f);
            line[n++] = c;
            c = fgetc(f);
        }
        line[n] = 0;
        sym = ident;
        get_sym(line);
        goto out;
    }

    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
        n = 0;
        line[n++] = c;
        c = fgetc(f);
        while (n < sizeof(line)-1 && 
               ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_')) {
            line[n++] = c;
            c = fgetc(f);
        }
        line[n] = 0;
        ungetc(c, f);

        if (!strcmp(line, "rr")) {
            sym = rr;
            goto out;
        }

        if (!strcmp(line, "ko")) {
            sym = ko;
            goto out;
        }

        if (!strcmp(line, "p")) {
            sym = prev;
            goto out;
        }

        if (!strcmp(line, "order")) {
            sym = order;
            goto out;
        }

        if (!strcmp(line, "page")) {
            sym = page;
            goto out;
        }

        if (!strcmp(line, "svg")) {
            sym = svg;
            goto out;
        }

        if (!strcmp(line, "info")) {
            sym = info;
            goto out;
        }

        if (!strncmp(line, "pos", 3) && line[3] >= '0' && line[3] <= '9') {
            sym = pos;
            value = atoi(&line[3]);
            goto out;
        }

        if (line[0] == 'c' && line[1] >= '0' && line[1] <= '9') {
            value = atoi(&line[1]);
            sym = competitor;
            goto out;
        }

        if (line[0] == 'M' && line[1] >= '0' && line[1] <= '9') {
            value = atoi(&line[1]);
            sym = match;
            goto out;
        }

        get_sym(line);
        sym = ident;
        goto out;
    }

 out:
        if (sym == err)
            fprintf(stderr, "Error: %s\n", errstr);
}

/*
static void error(const char msg[])
{
    fprintf(stderr, "line %d: %s %s\n", linenum, msg, labels[sym]);
}
*/

static int accept(Symbol s) {
    if (sym == s) {
        numvalue = value;
        strcpy(strvalue, line);
        getsym();
        return 1;
    }
    return 0;
}
 
static int expect(Symbol s) {
    if (accept(s))
        return 1;
    fprintf(stderr, "Error on line %d: Expected %s: unexpected symbol %s\n", 
            linenum, labels[s], labels[sym]);
    stop = 1;
    return 0;
}

static int player(void) {
    int p = 0;
    memset(&compvalue, 0, sizeof(compvalue));
    if (accept(competitor)) {
        compvalue.comp = numvalue;
        if (max_comp < numvalue)
            max_comp = numvalue;
        return 1;
    } else if (accept(ident)) {
        compvalue.match = get_sym(strvalue);
        expect(dot);
        expect(number);
        compvalue.pos = numvalue;
        while (accept(dot)) {
            expect(prev);
            expect(dot);
            expect(number);
            compvalue.prev[p++] = numvalue;
        }
        return 2;
    }
    return 0;
}
 
static void block(void)
{
    if (accept(number)) {
        ordernum = numvalue;
    } else {
        ordernum = 0;
    }

    if (accept(info)) {
        expect(ident);
        strncpy(name_long, strvalue, sizeof(name_long)-1);
        expect(ident);
        strncpy(name_short, strvalue, sizeof(name_short)-1);
        expect(number);
        competitors_min = numvalue;
        expect(number);
        competitors_max = numvalue;
        expect(eol);
        return;
    }

    if (accept(svg)) {
        if (accept(page)) {
            expect(number);
            pagenum = numvalue;
        }
        expect(eol);
        return;
    }

    if (accept(ident)) {
        match_t *m = get_match(strvalue);
        m->ordernum = ordernum;
        m->number = linenum;
        m->page = pagenum;
        expect(colon);
        player();
        m->c1 = compvalue;
        player();
        m->c2 = compvalue;
        expect(eol);
        return;
    }

    if (accept(rr)) {
        competitor_t c[20];
        int num_players = 0, i, j;
        char buf[64];
        expect(ident);
        round_robin_pools[num_round_robin_pools].name = get_sym(strvalue);
        /*
        match_t *m1 = get_match(strvalue);
        m1->ordernum = ordernum;
        m1->number = linenum;
        m1->page = pagenum;
        m1->type = rr;
        */
        expect(colon);
        while (player()) {
            c[num_players++] = compvalue;
        }
        // Create round robin matches
        for (i = 0; i < num_players-1; i++) {
            for (j = i+1; j < num_players; j++) {
                sprintf(buf, "%s_%d_%d", round_robin_pools[num_round_robin_pools].name->name, i, j);
                match_t *m = get_match(buf);
                m->ordernum = ordernum;
                m->number = linenum;
                m->page = pagenum;
                m->c1 = c[i];
                m->c2 = c[j];

                round_robin_pools[num_round_robin_pools].
                    rr_matches[round_robin_pools[num_round_robin_pools].num_rr_matches++] = get_sym(buf);
            }
        }
        expect(eol);
        num_round_robin_pools++;
        return;
    }

    if (accept(ko)) {
        int num_players = 0;
        char buf[64];
        expect(ident);
        sym_t *name = get_sym(strvalue), *reference = NULL;
        expect(colon);
        int i = 1, n, lev = 1;
        while (player()) {
            sprintf(buf, "%s_%d_%d", name->name, lev, i);
            match_t *m = get_match(buf);
            if (i == 1) reference = get_sym(buf);
            else {
                m->reference = reference;
                m->x = 0.0; m->y = y_shift*(i - 1);
            }
            m->ordernum = ordernum;
            m->number = linenum;
            m->page = pagenum;
            m->c1 = compvalue;
            if (!player()) {
                fprintf(stderr, "Line %d: missing player\n", linenum);
                return;
            }
            m->c2 = compvalue;
            m->flags |= FLAG_LONG_NAME;
            num_players += 2;
            i++;
        }
        // Create knock out matches
        for (n = num_players/4; n >= 1; n = n/2) {
            lev++;
            for (i = 0; i < n; i++) {
                if (n == 1)
                    sprintf(buf, "%s", name->name);
                else
                    sprintf(buf, "%s_%d_%d", name->name, lev, i+1);
                match_t *m = get_match(buf), *m1, *m2;

                if (n == 1) m->flags |= FLAG_LAST;

                m->ordernum = ordernum;
                m->number = linenum;
                m->page = pagenum;

                sprintf(buf, "%s_%d_%d", name->name, lev-1, 2*i+1);
                m->c1.match = get_sym(buf);
                m->c1.pos = 1;

                sprintf(buf, "%s_%d_%d", name->name, lev-1, 2*i+2);
                m->c2.match = get_sym(buf);
                m->c2.pos = 1;

                m1 = get_match(m->c1.match->name);
                m2 = get_match(m->c2.match->name);
                if (m1 && m2) {
                    m->reference = reference;
                    m->x = x_shift*(lev-1); 
                    m->y = (m1->y + m2->y)/2;
                    m->y1 = m1->y - m->y;
                    m->y2 = m2->y - m->y;
                }
            }
        } 
        expect(eol);
        return;
    }

    if (accept(order)) {
        while (player()) {
            match_order[num_ord].first = compvalue;
            expect(dash);
            if (player()) {
                match_order[num_ord++].second = compvalue;
            }
        }
         
        expect(eol);
        return;
    }

    if (accept(pos)) {
        int posval = numvalue-1;
        expect(ident);
        sym_t *s = get_sym(strvalue);
        positions[posval].match = s;
        expect(dot);
        expect(number);
        positions[posval].pos = numvalue;
        positions[posval].page = pagenum;
        expect(eol);
        return;
    }

    accept(eol);
}

static void solve(sym_t *mname, int level, int pos) {
    match_t *m;
    
    if (!mname)
        return;

    m = get_match(mname->name);
    //printf("-- %s level=%d curr=%d\n", mname, level, current_level);

    if (!m->level) {
        m->level = level;
        m->pos = pos;
    }

    if (m->c1.comp && m->c2.comp) {
        //printf("%s: comp%d comp%d\n", m->name, m->c1.comp, m->c2.comp);
        return;
    }
    if (m->c1.comp) {
        solve(m->c2.match, level+1, pos);
        //printf("%s: comp%d match=%s.%d\n", m->name, m->c1.comp, m->c2.match, m->c2.pos);
        return;
    }
    if (m->c2.comp) {
        solve(m->c1.match, level+1, pos);
        //printf("%s: match=%s.%d comp%d\n", m->name, m->c1.match, m->c1.pos, m->c2.comp);
        return;
    }
    solve(m->c1.match, level+1, pos);
    solve(m->c2.match, level+1, pos);
    //printf("%s: match=%s.%d match=%s.%d\n", m->name, m->c1.match, m->c1.pos, m->c2.match, m->c2.pos);
}

static void sort_matches(void) {
    int i, j, i1, j1;

    // find explicite match numbers 
    for (i = 0; i < num_matches; i++) {
        for (j = 0; j < num_ord; j++) {
            if (!memcmp(&matches[i].c1, &match_order[j].first.comp, sizeof(competitor_t)) &&
                !memcmp(&matches[i].c2, &match_order[j].second.comp, sizeof(competitor_t))) {
                matches[i].matchord = j+1;
                break;
            }
        }
    }

    for (i1 = 0; i1 < num_matches-1; i1++) {
        for (j1 = i1+1; j1 < num_matches; j1++) {
            i = match_list[i1];
            j = match_list[j1];
            int order = matches[i].ordernum > matches[j].ordernum && 
                matches[i].ordernum && matches[j].ordernum;
            int level = matches[i].level < matches[j].level &&
                matches[i].level && matches[j].level;
            int number = matches[i].level == matches[j].level && 
                matches[i].number > matches[j].number;
            if (order || level || number) {
                /*printf("sort: %d:%d order=%d level=%d number=%d type=%d\n", i, j,
                  order, level, number, typ);*/
                int tmp = match_list[i1];
                match_list[i1] = match_list[j1];
                match_list[j1] = tmp;
                /*
                match_t m = matches[i];
                matches[i] = matches[j];
                matches[j] = m;
                */
            }
        }
    }

    for (i1 = 0; i1 < num_matches-1; i1++) {
        for (j1 = i1+1; j1 < num_matches; j1++) {
            i = match_list[i1];
            j = match_list[j1];
            if (matches[i].matchord && matches[j].matchord &&
                matches[i].matchord > matches[j].matchord) {
                //printf("ord sort: %d:%d\n", i, j);
                int tmp = match_list[i1];
                match_list[i1] = match_list[j1];
                match_list[j1] = tmp;
                /*
                match_t m = matches[i];
                matches[i] = matches[j];
                matches[j] = m;
                */
            }
        }
    }
}

static void program(void) {
    getsym();
    while (sym != eof) {
        block();
    }
}


static int get_match_num(char *name) {
    int i;
    for (i = 0; i < num_matches; i++) {
        if (!strcmp(matches[match_list[i]].name->name, name))
            return i+1;
    }
    return 0;
}

#ifdef MAKE_SVG_FILES

static char *print_comp(competitor_t *c) {
    static char buf[2][32];
    static int sel = 0;
    int n = 0;

    sel = sel ? 0 : 1;

    if (c->comp)
        sprintf(buf[sel], "comp%d", c->comp);
    else if (c->match) {
        int mn = get_match_num(c->match->name);
        match_t *m = get_match(c->match->name);
        if (m->type == rr)
            n = sprintf(buf[sel], "%s.%d", c->match->name, c->pos);
        else
            n = sprintf(buf[sel], "M%d.%d", mn, c->pos);
        int i = 0;
        while (c->prev[i]) {
            n += sprintf(buf[sel]+n, ".p.%d", c->prev[i]);
            i++;
        }
    }

    return buf[sel];
}

#define TEXT(_x, _y, _t) fprintf(out, svg_text, _x, _y, _t)

static void draw_match(FILE *out, int num, match_t *m)
{
    int longname = m->flags & FLAG_LONG_NAME;
    double xs = m->x - (longname ? x_init : x_shift);
    double ys1 = m->y + (m->y1 ? m->y1 : -y_shift/4.0);
    double ys2 = m->y + (m->y2 ? m->y2 : y_shift/4.0);
    double r = 8.05;

    fprintf(out, "<g>\n");
    fprintf(out, "  <circle cx='%.2f' cy='%.2f' r='%.2f' stroke='black' stroke-width='1' fill='white'/>\n",
            m->x, m->y, r);
    fprintf(out, "<text x='%.2f' y='%.2f' style='%s;text-anchor:middle;text-align:center'>%d</text>\n", 
            m->x, m->y+4.0, competitor_style, num);
    fprintf(out, "<text x='%.2f' y='%.2f' style='%s'>%%m%d-1-%s</text>\n",
            xs + r + 4.0, ys1, competitor_style, num, 
            longname ? competitor_name_1 : competitor_name_2);
    fprintf(out, "<text x='%.2f' y='%.2f' style='%s'>%%m%d-2-%s</text>\n",
            xs + r + 4.0, ys2,  competitor_style, num,
            longname ? competitor_name_1 : competitor_name_2);

    fprintf(out, "<path "
            "d='M %.2f,%.2f l %.2f,0.00 "
            "a%.2f,%.2f 0 0,1 %.2f,%.2f "
            "l 0.00,%.2f "
            "M %.2f,%.2f l %.2f,0.00 "
            "a%.2f,%.2f 0 0,0 %.2f,-%.2f "
            "l 0.00,-%.2f' "
            "style='fill:none;stroke:black;stroke-width:1' />\n",
            xs+r,ys1, (longname ? x_init : x_shift) - r*0.5 - r,
            r*0.5,r*0.5, r*0.5,r*0.5,
            m->y - ys1 - r - 0.5*r,
            xs+r,ys2, (longname ? x_init : x_shift) - r*0.5 - r,
            r*0.5,r*0.5, r*0.5,r*0.5,
            m->y - ys1 - r - 0.5*r);

    if (m->flags & FLAG_LAST) {
        fprintf(out, "<path d='M %.2f,%.2f l %.2f,0' style='fill:none;stroke:black;stroke-width:1'/>\n",
                m->x + r, m->y, x_shift - r);
        fprintf(out, "<text x='%.2f' y='%.2f' style='%s'>%%m%d-winner-%s</text>\n",
                m->x + r + 4.0, m->y,  competitor_style, num,
                longname ? competitor_name_1 : competitor_name_2);
    }

    fprintf(out, "<text x='%.2f' y='%.2f' style='%s'>%%m%dp1-1</text>\n",
            m->x+2.0, ys1 + 10.0,  competitor_style, num);
    
    fprintf(out, "<text x='%.2f' y='%.2f' style='%s'>%%m%dp2-1</text>\n",
            m->x+2.0, ys2,  competitor_style, num);
    
    fprintf(out, "</g>\n");
}

static int current_page = 0;

static FILE *open_file(int page, double *y)
{
    if (page < 1 || page > NUM_PAGES) {
        fprintf(stderr, "Error: page = %d\n", page);
        return NULL;
    }

    if (!page_data[page-1].fd) {
        page_data[page-1].fd = fopen(page_data[page-1].name, "w");
        if (!page_data[page-1].fd) return NULL;
        fprintf(page_data[page-1].fd, "%s", svg_start);
        fprintf(page_data[page-1].fd, "%s", svg_page_header_text);
        page_data[page-1].current_y = 60.0; 
    }

    if (current_page != page) {
        if (current_page > 0)
            page_data[current_page-1].current_y = *y;
        current_page = page;
        *y = page_data[current_page-1].current_y;
    }

    return page_data[page-1].fd;
}

#endif


int read_custom_category(char *name, struct custom_data *data)
{
    int i;

    memset(&matches, 0, sizeof(matches));
    memset(&positions, 0, sizeof(positions));
    memset(&round_robin_pools, 0, sizeof(round_robin_pools));
    memset(&symbols, 0, sizeof(symbols));
    memset(&match_order, 0, sizeof(match_order));
    num_round_robin_pools = 0;
    num_symbols = 0;
    num_matches = 0;
    linenum = 1;
    stop = 0;
    max_comp = 0;
    num_ord = 0;
    competitors_min = competitors_max = 0;

    for (i = 0; i < NUM_CUSTOM_MATCHES; i++)
        match_list[i] = i;

    f = fopen(name, "r");
    if (!f)
        return -1;

    program();

    fclose(f);

    if (stop)
        return -1;

    for (i = 1; i < NUM_POSITIONS ; i++) {
        if (!positions[i].match)
            continue;
        
        solve(positions[i].match, 1, i);
    }

    sort_matches();

    data->num_round_robin_pools = 0;
    data->num_matches = 0;
    data->num_positions = 0;
    
    for (i = 0; i < num_round_robin_pools; i++) {
        int j;
        for (j = 0; j < round_robin_pools[i].num_rr_matches; j++)
            data->round_robin_pools[i].rr_matches[j] = 
                get_match_num(round_robin_pools[i].rr_matches[j]->name);

        data->round_robin_pools[i].num_rr_matches = round_robin_pools[i].num_rr_matches;
    }
    data->num_round_robin_pools = num_round_robin_pools;

    for (i = 0; i < num_matches; i++) {
        match_t *m = &matches[match_list[i]];
        if (m->c1.comp) {
            data->matches[i].c1.type = COMP_TYPE_COMPETITOR;
            data->matches[i].c1.num = m->c1.comp;
        } else {
            int j;
            data->matches[i].c1.type = COMP_TYPE_MATCH;
            data->matches[i].c1.num = get_match_num(m->c1.match->name);
            data->matches[i].c1.pos = m->c1.pos;
            for (j = 0; j < 8; j++)
                data->matches[i].c1.prev[j] = m->c1.prev[j];
        }
        if (m->c2.comp) {
            data->matches[i].c2.type = COMP_TYPE_COMPETITOR;
            data->matches[i].c2.num = m->c2.comp;
        } else {
            int j;
            data->matches[i].c2.type = COMP_TYPE_MATCH;
            data->matches[i].c2.num = get_match_num(m->c2.match->name);
            data->matches[i].c2.pos = m->c2.pos;
            for (j = 0; j < 8; j++)
                data->matches[i].c2.prev[j] = m->c2.prev[j];
        }
    }
    data->num_matches = num_matches;

    for (i = 0; positions[i].match; i++) {
        data->positions[i].type = COMP_TYPE_MATCH;
        data->positions[i].match = get_match_num(positions[i].match->name);
        data->positions[i].pos = positions[i].pos;
    }
    data->num_positions = i;

    data->competitors_min = competitors_min;
    data->competitors_max = competitors_max;
    strncpy(data->name_long, name_long, sizeof(data->name_long)-1);
    strncpy(data->name_short, name_short, sizeof(data->name_short)-1);

    return 0;
}

#ifdef MAKE_SVG_FILES

struct custom_data test;

int main(int argc, char *argv[])
{
    int i, j;
    char outfile[256], template[256];

    if (argc < 2)
        return -1;

    read_custom_category(argv[1], &test);

    for (i = 0; i < test.num_matches; i++) {
        match_bare_t *m = &test.matches[i];
        fprintf(stderr, "M%d: ", i+1);
        if (m->c1.type == COMP_TYPE_COMPETITOR) 
            fprintf(stderr, "c%d", m->c1.num);
        else if (m->c1.type == COMP_TYPE_MATCH) {
            fprintf(stderr, "M%d.%d", m->c1.num, m->c1.pos);
            for (j = 0; j < 8 && m->c1.prev[j]; j++)
                fprintf(stderr, ".p.%d", m->c1.prev[j]);
        }
        fprintf(stderr, " - ");
        if (m->c2.type == COMP_TYPE_COMPETITOR) 
            fprintf(stderr, "c%d", m->c2.num);
        else if (m->c2.type == COMP_TYPE_MATCH) {
            fprintf(stderr, "M%d.%d", m->c2.num, m->c2.pos);
            for (j = 0; j < 8 && m->c2.prev[j]; j++)
                fprintf(stderr, ".p.%d", m->c2.prev[j]);
        }
        fprintf(stderr, "\n");
    }
    return;

    for (i = 0; i < NUM_CUSTOM_MATCHES; i++)
        match_list[i] = i;

    for (i = 0; i < NUM_PAGES; i++) {
        page_data[i].current_y = 60.0;
    }

    strcpy(template, argv[1]);
    char *p = strrchr(template, '.');
    if (p) *p = 0;

    // delete old files
    for (i = 1; i <= NUM_PAGES; i++) {
        snprintf(outfile, sizeof(outfile), "%s-%d.svg", template, i);
        strncpy(page_data[i-1].name, outfile, 127);
        FILE *tmp = fopen(outfile, "r");
        if (tmp) {
            fclose(tmp);
            unlink(outfile);
        }
    }

    f = fopen(argv[1], "r");
    if (!f)
        return 1;

    program();

#if 0
    for (i = 0; i < num_symbols; i++) {
        fprintf(stderr, "Symbol %s\n", symbols[i].name);
    }
    for (i = 0; i < num_matches; i++) {
        fprintf(stderr, "Match %s comp1=%d/%s/%d comp2=%d/%s/%d ord=%d\n", matches[i].name->name,
                matches[i].c1.comp, matches[i].c1.match ? matches[i].c1.match->name : "", matches[i].c1.pos,
                matches[i].c2.comp, matches[i].c2.match ? matches[i].c2.match->name : "", matches[i].c2.pos,
                matches[i].ordernum);
    }
    for (i = 0; i < num_ord; i++) {
        fprintf(stderr, "Order c%d-c%d\n", match_order[i].first.comp, match_order[i].second.comp);
    }
    for (i = 0; i < NUM_POSITIONS; i++) {
        if (positions[i].match)
            fprintf(stderr, "Position %d match=%s pos=%d page=%d\n", 
                    i, positions[i].match->name, positions[i].pos, positions[i].page);
    }
#endif

    for (i = 1; i < NUM_POSITIONS ; i++) {
        if (!positions[i].match)
            continue;
        
        solve(positions[i].match, 1, i);
    }

    sort_matches();

#if 1
    fprintf(stderr, "MATCHES:\n");
    for (j = 0; j < num_matches; j++) {
        match_t *m = &matches[match_list[j]];

        //printf("type=%d level=%d pos=%d -- ", m->type, m->level, m->pos);

        if (m->type == rr)
            fprintf(stderr, "%s: Round robin virtual\n", m->name->name);
        else
            fprintf(stderr, "M%d (%s): %s %s\n", j+1, m->name->name, 
                    print_comp(&m->c1), print_comp(&m->c2));
    }
#endif

    FILE *out = NULL;
    double y = 0.0;

    for (j = 0; j < num_matches; j++) {
        match_t *m = &matches[j];
        out = open_file(m->page, &y);
        if (!out) return 1;

        if (m->reference) {
            match_t *m1 = get_match(m->reference->name);
            if (m1) {
                m->x += m1->x;
                m->y += m1->y;
            }
        } else {
            m->x = x_init;
            y += y_shift;
            m->y = y;
        }
        if (m->y > y) y = m->y;
        //TEXT(m->x, m->y, buf);
        draw_match(out, j+1, m);
    }

    for (j = 0; j < NUM_POSITIONS; j++) {
        if (!positions[j].page || !positions[j].match) 
            continue;

        out = open_file(positions[j].page, &y);
        if (!out) return 1;

        if (!(page_data[current_page-1].flags & PAGE_FLAG_POS_HDR)) {
            page_data[current_page-1].flags |= PAGE_FLAG_POS_HDR;
            y += 40.0;
            page_data[current_page-1].res_start_y = y;
            fprintf(out, "<text x='%.2f' y='%.2f' style='%s;font-weight:bold;"
                    "text-anchor:middle;text-align:center'>%%t18</text>\n",
                    res_x + res_w*0.5, y + res_line - 4.0, competitor_style);
            fprintf(out, "<text x='%.2f' y='%.2f' "
                    "style='%s;font-weight:bold;text-anchor:middle;text-align:center'>%%t22</text>\n",
                    res_x + 12.0, y + res_line - 4.0, competitor_style);
        }

        y += res_line;
        fprintf(out, "<text x='%.2f' y='%.2f' style='%s'>%%r%dhm2-first-s-last-s-club</text>\n",
                res_x + 30.0, y + res_line - 4.0, competitor_style, j+1);
        fprintf(out, "<text x='%.2f' y='%.2f' style='%s;text-anchor:middle;text-align:center'>%d</text>\n",
                res_x + 12.0, y + res_line - 4.0, competitor_style, j+1);
        page_data[current_page-1].num_res++;
    }

    // draw result boxes
    for (i = 0; i < NUM_PAGES; i++) {
        if (!page_data[i].num_res) continue;

        out = open_file(i+1, &y);
        if (!out) return 1;

        fprintf(out, "<path "
                "d='M %.2f,%.2f "
                "l %.2f,0 l 0,%.2f l -%.2f,0 l 0,-%.2f' "
                "style='fill:none;stroke:black;stroke-width:2' />\n",
                res_x, page_data[i].res_start_y,
                res_w, (page_data[i].num_res + 1)*res_line,
                res_w, (page_data[i].num_res + 1)*res_line);

        fprintf(out, "<path "
                "d='M %.2f,%.2f l 0,%.2f' "
                "style='fill:none;stroke:black;stroke-width:1' />\n",
                res_x + 24.0, page_data[i].res_start_y + res_line, page_data[i].num_res*res_line);

        for (j = 1; j <= page_data[i].num_res; j++) {
            fprintf(out, "<path "
                    "d='M %.2f,%.2f "
                    "l %.2f,0' "
                    "style='fill:none;stroke:black;stroke-width:1' />\n",
                    res_x, page_data[i].res_start_y + j*res_line,
                    res_w);
            
        }

    }

    // close svg files
    for (i = 0; i < NUM_PAGES; i++) {
        if (page_data[i].fd) {
            fprintf(page_data[i].fd, "%s", svg_end);
            fclose(page_data[i].fd);
        }
    }

    fclose(f);
    return 0;
}

#endif

