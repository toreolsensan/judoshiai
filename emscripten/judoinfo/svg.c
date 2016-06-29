/* -*- mode: C; c-basic-offset: 4;  -*- */

/*
 * Copyright (C) 2006-2016 by Hannu Jokinen
 * Full copyright text is included in the software package.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "judoinfo.h"

#define CODE_LEN 16

static struct {
    gchar code[CODE_LEN];
    gint codecnt;
    gint value;
} attr[16];
static gint cnt = 0;

char *svg_data = NULL;
int   svg_datalen = 0;
int   svg_ok = 0;
char *svg_out = NULL;
int   svg_outlen = 0;
int   svg_outsize = 0;
char *datamax = NULL;

void WRITE2(char *buf, int len)
{
    int i;

    for (i = 0; i < len; i++) {
	if (svg_outlen == 0) {
	    strcpy(svg_out, "matchlistcontent += \"");
	    svg_outlen = strlen(svg_out);
	}

	if (buf[i] == '"') {
	    strcpy(svg_out + svg_outlen, "\\\"");
	    svg_outlen += 2;
	} else if (buf[i] == '\n') {
	    strcpy(svg_out + svg_outlen, "\\n\"");
	    //printf("CMD=%s\n", svg_out);
	    emscripten_run_script(svg_out);
	    svg_outlen = 0;
	} else {
	    svg_out[svg_outlen] = buf[i];
	    svg_outlen++;
	}
    }


#if 0
    while (len >= svg_outsize) {
	svg_outsize *= 2;
	svg_out = realloc(svg_out, svg_outsize);
    }

    memcpy(svg_out /*+ svg_outlen*/, buf, len);
    svg_outlen = len;
    svg_out[svg_outlen] = 0;
#endif
}

#define WRITE1(_s, _l)                                                  \
    do { gint _i; for (_i = 0; _i < _l; _i++) {                         \
        if (_s[_i] == '&')                                              \
            WRITE2("&amp;", 5);                                         \
        else if (_s[_i] == '<')                                         \
            WRITE2("&lt;", 4);                                          \
        else if (_s[_i] == '>')                                         \
            WRITE2("&gt;", 4);                                          \
        else if (_s[_i] == '\'')                                        \
            WRITE2("&apos;", 6);                                        \
        else if (_s[_i] == '"')                                         \
            WRITE2("&quot;", 6);                                        \
        else                                                            \
            WRITE2(&_s[_i], 1);                                         \
        }} while (0)

#define WRITE(_a) WRITE1(_a, strlen(_a))

#define IS_LABEL_CHAR(_x) ((_x >= 'a' && _x <= 'z') || _x == 'C' || _x == 'M' || _x == '#' || _x == '=')
#define IS_VALUE_CHAR(_x) (_x >= '0' && _x <= '9')

#define IS_SAME(_a, _b) (!strcmp((char *)_a, (char *)_b))

void svg_file1_read(const char *svg_file)
{

    if (svg_data)
	free(svg_data);
    svg_data = NULL;
    svg_datalen = 0;
    svg_ok = FALSE;

    if (svg_file == NULL || svg_file[0] == 0)
        return;

    struct stat sb;
    if (stat(svg_file, &sb) < 0) {
	printf("File %s does not exist!\n", svg_file);
	return;
    }

    FILE *f = fopen(svg_file, "r");
    if (!f) return;

    int n;
    svg_data = malloc(sb.st_size);
    while ((n = fread(svg_data + svg_datalen, 1, 1024, f)) > 0) {
	svg_datalen += n;
    }
    printf("READ SVG/HTML FILE %d bytes\n", svg_datalen);

    fclose(f);
    datamax = svg_data + svg_datalen;
    expose();
}

gint write_judoka(gint start, struct name_data *j)
{
    gint i;

    for (i = start; i < cnt; i++) {
        if (attr[i].code[0] == '\'') {
            // quoted text
            WRITE1((attr[i].code+1), attr[i].codecnt - 1);
        } else if (IS_SAME(attr[i].code, "first")) {
            WRITE(j->first);
        } else if (IS_SAME(attr[i].code, "last")) {
            WRITE(j->last);
        } else if (IS_SAME(attr[i].code, "club")) {
            gchar *p = strchr(j->club, '/');
            if (p) WRITE((p+1));
            else WRITE(j->club);
        } else if (IS_SAME(attr[i].code, "country")) {
            gchar *p = strchr(j->club, '/');
            if (p) {
                gchar buf[32];
                strncpy(buf, j->club, sizeof(buf)-1);
                buf[31] = 0;
                p = strchr(buf, '/');
                if (p) *p = 0;
                WRITE(buf);
            } else
                WRITE(j->club);
        } else if (IS_SAME(attr[i].code, "cntclub")) {
            WRITE(j->club);
        } else if (IS_SAME(attr[i].code, "s")) {
            WRITE(" ");
        }
    }

    return 0;
}

gint paint_svg(struct paint_data *pd)
{
    int current_tatami = 0, j;

    for (j = 0; j < NUM_TATAMIS; j++)
	if (show_tatami[j]) {
	    current_tatami = j;
	    break;
	}

    if (!svg_data) {
	printf("No svg file!\n");
	return -1;
    }

    emscripten_run_script("matchlistcontent=''");

    if (!svg_out) {
	svg_outsize = 1024;
	svg_out = malloc(svg_outsize);
    }

    svg_outlen = 0;

    char *p = svg_data;

    while (p < datamax && *p) {
        if (*p == '%' && IS_LABEL_CHAR(p[1])) {
            memset(attr, 0, sizeof(attr));
            cnt = 0;
            p++;
            while (IS_LABEL_CHAR(*p) || IS_VALUE_CHAR(*p) || *p == '-' ||
		   *p == '\'' || *p == '|' || *p == '!') {
                while (IS_LABEL_CHAR(*p))
                    attr[cnt].code[attr[cnt].codecnt++] = *p++;

                if (*p == '-') p++;

                while (IS_VALUE_CHAR(*p))
                    attr[cnt].value = attr[cnt].value*10 + *p++ - '0';

                if (*p == '-') p++;

                if (*p == '\'' || *p == '|') {
                    cnt++;
                    p++;
                    attr[cnt].code[0] = '\'';
                    attr[cnt].codecnt = 1;
                    while (*p && *p != '\'' && *p != '|') {
                        attr[cnt].code[attr[cnt].codecnt] = *p++;
                        if (attr[cnt].codecnt < CODE_LEN)
                            attr[cnt].codecnt++;
                    }
                    if (*p == '\'' || *p == '|')
                        p++;
                }

                cnt++;

                if (*p == '!') {
                    p++;
                    break;
                }
            } // while IS_LABEL

            if (attr[0].code[0] == 'm') {
                struct name_data *j = NULL;
                gint ix;
                gint tatami = attr[0].value-1;
                gint fight = attr[1].value;
                gint who = attr[2].value;

		if (tatami < 0)
		    tatami = current_tatami;

                if (attr[2].code[0] == '#') {
                    if (match_list[tatami][fight].number < 1000) {
                        gchar buf[16];
                        snprintf(buf, sizeof(buf), "%d", match_list[tatami][fight].number);
                        WRITE(buf);
                    }
                } else {
                    if (who == 1) ix = match_list[tatami][fight].blue;
                    else ix = match_list[tatami][fight].white;
                    j = avl_get_data(ix);
                    if (j) {
                        write_judoka(3, j);
                    }
                }
            } else if (attr[0].code[0] == 'c') {
                struct name_data *j = NULL;
                gint tatami = attr[0].value-1;
                gint fight = attr[1].value;

		if (tatami < 0)
		    tatami = current_tatami;

                j = avl_get_data(match_list[tatami][fight].category);

                if (j) {
                    WRITE(j->last);
                }
            } else if (attr[0].code[0] == 't') {
		char buf[8];
		snprintf(buf, sizeof(buf), "%d", current_tatami+1);
		WRITE(buf);
            }
        } // *p = %
        else {
            WRITE2(p, 1);
            p++;
        }
    }

    emscripten_run_script("document.getElementById('matchlist').innerHTML=matchlistcontent");

    char buf[128];
    extern int split_x;
    snprintf(buf, sizeof(buf),
	     "el = document.getElementById('matchtable');"
	     "if (el) { el.style.width = %d; }",
	     split_x);
    emscripten_run_script(buf);

    return TRUE;
}
