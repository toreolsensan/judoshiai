/* -*- mode: C; c-basic-offset: 4;  -*- */

/*
 * Copyright (C) 2006-2016 by Hannu Jokinen
 * Full copyright text is included in the software package.
 */ 

#if 0
#if defined(__WIN32__) || defined(WIN32)

#define  __USE_W32_SOCKETS

#include <windows.h>
#include <stdio.h>
#include <initguid.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#else /* UNIX */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <fcntl.h>
#include <signal.h>

#endif /* WIN32 */
#endif

#include <stdio.h>
#include <string.h>
#ifndef EMSCRIPTEN
#include <glib.h>
#endif
#include "comm.h"
#include "cJSON.h"



static int putstr_esc(char *p, int len, char *s)
{
    int a = 0;

    p[a++] = '"';
    while (a < len - 2) {
	if (*s == 0)
	    break;
	if (*s == '"' || *s == '\t') {
	    p[a++] = '\\';
	}
	if (*s == '\t') {
	    p[a++] = 't';
	    s++;
	} else {
	    p[a++] = *s++;
	}
    }
    p[a++] = '"';
    p[a++] = ',';
    return a;
}

#define put8(_n) len += snprintf(p+len, buflen-len, "%d,", _n)
#define put16(_n) len += snprintf(p+len, buflen-len, "%d,", _n)
#define put32(_n) len += snprintf(p+len, buflen-len, "%d,", _n)
#define putdbl(_n) len += snprintf(p+len, buflen-len, "%f,", _n)
#define putstr(_s) len += putstr_esc(p+len, buflen-len, _s)

int websock_encode_msg(struct message *m, unsigned char *buf, int buflen)
{
    char *p = (char *)buf;
    int i;
    int len = 0;

    len += snprintf(p+len, buflen-len, "{\"msg\":[");
    put8(COMM_VERSION);
    put8(m->type);
    put16(0); // length
    put32(m->sender);

    switch (m->type) {
    case MSG_NEXT_MATCH:
	put32(m->u.next_match.tatami);
	put32(m->u.next_match.category);
	put32(m->u.next_match.match);
	put32(m->u.next_match.minutes);
	put32(m->u.next_match.match_time);
	put32(m->u.next_match.gs_time);
	put32(m->u.next_match.rep_time);
	put32(m->u.next_match.rest_time);
	put8(m->u.next_match.pin_time_ippon);
	put8(m->u.next_match.pin_time_wazaari);
	put8(m->u.next_match.pin_time_yuko);
	put8(m->u.next_match.pin_time_koka);
	putstr(m->u.next_match.cat_1);
	putstr(m->u.next_match.blue_1);
	putstr(m->u.next_match.white_1);
	putstr(m->u.next_match.cat_2);
	putstr(m->u.next_match.blue_2);
	putstr(m->u.next_match.white_2);
	put32(m->u.next_match.flags);
	put32(m->u.next_match.round);
	break;
    case MSG_RESULT:
	put32(m->u.result.tatami);
	put32(m->u.result.category);
	put32(m->u.result.match);
	put32(m->u.result.minutes);
	put32(m->u.result.blue_score);
	put32(m->u.result.white_score);
	put8(m->u.result.blue_vote);
	put8(m->u.result.white_vote);
	put8(m->u.result.blue_hansokumake);
	put8(m->u.result.white_hansokumake);
	put32(m->u.result.legend);
	break;
    case MSG_ACK:
	put32(m->u.ack.tatami);
	break;
    case MSG_SET_COMMENT:
	put32(m->u.set_comment.data);
	put32(m->u.set_comment.category);
	put32(m->u.set_comment.number);
	put32(m->u.set_comment.cmd);
	put32(m->u.set_comment.sys);
	break;
    case MSG_SET_POINTS:
	put32(m->u.set_points.category);
	put32(m->u.set_points.number);
	put32(m->u.set_points.sys);
	put32(m->u.set_points.blue_points);
	put32(m->u.set_points.white_points);
	break;
    case MSG_HELLO:
	putstr(m->u.hello.info_competition);
	putstr(m->u.hello.info_date);
	putstr(m->u.hello.info_place);
	break;
    case MSG_DUMMY:
	put32(m->u.dummy.application_type);
	put32(m->u.dummy.tatami);
	break;
    case MSG_MATCH_INFO:
	put32(m->u.match_info.tatami);
	put32(m->u.match_info.position);
	put32(m->u.match_info.category);
	put32(m->u.match_info.number);
	put32(m->u.match_info.blue);
	put32(m->u.match_info.white);
	put32(m->u.match_info.flags);
	put32(m->u.match_info.rest_time);
	put32(m->u.match_info.round);
	break;
    case MSG_11_MATCH_INFO:
	for (i = 0; i < 11; i++) {
	    put32(m->u.match_info_11.info[i].tatami);
	    put32(m->u.match_info_11.info[i].position);
	    put32(m->u.match_info_11.info[i].category);
	    put32(m->u.match_info_11.info[i].number);
	    put32(m->u.match_info_11.info[i].blue);
	    put32(m->u.match_info_11.info[i].white);
	    put32(m->u.match_info_11.info[i].flags);
	    put32(m->u.match_info_11.info[i].rest_time);
	    put32(m->u.match_info_11.info[i].round);
	}
	break;
    case MSG_NAME_INFO:
	put32(m->u.name_info.index);
	putstr(m->u.name_info.last);
	putstr(m->u.name_info.first);
	putstr(m->u.name_info.club);
	break;
    case MSG_NAME_REQ:
	put32(m->u.name_req.index);
	break;
    case MSG_ALL_REQ:
	break;
    case MSG_CANCEL_REST_TIME:
	put32(m->u.cancel_rest_time.category);
	put32(m->u.cancel_rest_time.number);
	put32(m->u.cancel_rest_time.blue);
	put32(m->u.cancel_rest_time.white);
	break;
    case MSG_UPDATE_LABEL:
	putstr(m->u.update_label.expose);
	putstr(m->u.update_label.text);
	putstr(m->u.update_label.text2);
	putstr(m->u.update_label.text3);
	putdbl(m->u.update_label.x);
	putdbl(m->u.update_label.y);
	putdbl(m->u.update_label.w);
	putdbl(m->u.update_label.h);
	putdbl(m->u.update_label.fg_r);
	putdbl(m->u.update_label.fg_g);
	putdbl(m->u.update_label.fg_b);
	putdbl(m->u.update_label.bg_r);
	putdbl(m->u.update_label.bg_g);
	putdbl(m->u.update_label.bg_b);
	putdbl(m->u.update_label.size);
	put32(m->u.update_label.label_num);
	put32(m->u.update_label.xalign);
	break;
    case MSG_EDIT_COMPETITOR:
	put32(m->u.edit_competitor.operation);
	put32(m->u.edit_competitor.index);
	putstr(m->u.edit_competitor.last);
	putstr(m->u.edit_competitor.first);
	put32(m->u.edit_competitor.birthyear);
	putstr(m->u.edit_competitor.club);
	putstr(m->u.edit_competitor.regcategory);
	put32(m->u.edit_competitor.belt);
	put32(m->u.edit_competitor.weight);
	put32(m->u.edit_competitor.visible);
	putstr(m->u.edit_competitor.category);
	put32(m->u.edit_competitor.deleted);
	putstr(m->u.edit_competitor.country);
	putstr(m->u.edit_competitor.id);
	put32(m->u.edit_competitor.seeding);
	put32(m->u.edit_competitor.clubseeding);
	put32(m->u.edit_competitor.matchflags);
	putstr(m->u.edit_competitor.comment);
	putstr(m->u.edit_competitor.coachid);
	putstr(m->u.edit_competitor.beltstr);
	putstr(m->u.edit_competitor.estim_category);
	break;
    case MSG_SCALE:
	put32(m->u.scale.weight);
	putstr(m->u.scale.config);
	break;
    case MSG_LANG:
	putstr(m->u.lang.english);
	putstr(m->u.lang.translation);
	break;
    case MSG_LOOKUP_COMP:
	putstr(m->u.lookup_comp.name);
	for (i = 0; i < NUM_LOOKUP; i++) {
	    put32(m->u.lookup_comp.result[i].index);
	    putstr(m->u.lookup_comp.result[i].fullname);
	}
	break;
    }

    len += snprintf(p+len, buflen-len, "0]}");
    return len;
}

#define get8(_n)   if (!val) return -1; if (val->type != cJSON_Number) printf("E:%d\n", __LINE__);_n = val->valueint; val = val->next
#define get16(_n)  if (!val) return -1; if (val->type != cJSON_Number) printf("E:%d\n", __LINE__); _n = val->valueint; val = val->next
#define get32(_n)  if (!val) return -1; if (val->type != cJSON_Number) printf("E:%d\n", __LINE__); _n = val->valueint; val = val->next
#define getdbl(_n) if (!val) return -1; if (val->type != cJSON_Number) printf("E:%d\n", __LINE__); _n = val->valuedouble; val = val->next
#define getstr(_s) if (!val) return -1; if (val->type != cJSON_String) printf("E:%d\n", __LINE__); strncpy((char *)_s, val->valuestring, sizeof(_s)-1); val = val->next

int websock_decode_msg(struct message *m, cJSON *json)
{
    int ver, len, i;
    cJSON *msg = json->child;

    if (msg->type != cJSON_Array)
	return -1;

    cJSON *val = msg->child;

    get8(ver);

    if (ver != COMM_VERSION) {
	return -1;
    }
    get8(m->type);
    get16(len);
    (void)len;
    get32(m->sender);

    switch (m->type) {
    case MSG_NEXT_MATCH:
	get32(m->u.next_match.tatami);
	get32(m->u.next_match.category);
	get32(m->u.next_match.match);
	get32(m->u.next_match.minutes);
	get32(m->u.next_match.match_time);
	get32(m->u.next_match.gs_time);
	get32(m->u.next_match.rep_time);
	get32(m->u.next_match.rest_time);
	get8(m->u.next_match.pin_time_ippon);
	get8(m->u.next_match.pin_time_wazaari);
	get8(m->u.next_match.pin_time_yuko);
	get8(m->u.next_match.pin_time_koka);
	getstr(m->u.next_match.cat_1);
	getstr(m->u.next_match.blue_1);
	getstr(m->u.next_match.white_1);
	getstr(m->u.next_match.cat_2);
	getstr(m->u.next_match.blue_2);
	getstr(m->u.next_match.white_2);
	get32(m->u.next_match.flags);
	get32(m->u.next_match.round);
	break;
    case MSG_RESULT:
	get32(m->u.result.tatami);
	get32(m->u.result.category);
	get32(m->u.result.match);
	get32(m->u.result.minutes);
	get32(m->u.result.blue_score);
	get32(m->u.result.white_score);
	get8(m->u.result.blue_vote);
	get8(m->u.result.white_vote);
	get8(m->u.result.blue_hansokumake);
	get8(m->u.result.white_hansokumake);
	get32(m->u.result.legend);
	break;
    case MSG_ACK:
	get32(m->u.ack.tatami);
	break;
    case MSG_SET_COMMENT:
	get32(m->u.set_comment.data);
	get32(m->u.set_comment.category);
	get32(m->u.set_comment.number);
	get32(m->u.set_comment.cmd);
	get32(m->u.set_comment.sys);
	break;
    case MSG_SET_POINTS:
	get32(m->u.set_points.category);
	get32(m->u.set_points.number);
	get32(m->u.set_points.sys);
	get32(m->u.set_points.blue_points);
	get32(m->u.set_points.white_points);
	break;
    case MSG_HELLO:
	getstr(m->u.hello.info_competition);
	getstr(m->u.hello.info_date);
	getstr(m->u.hello.info_place);
	break;
    case MSG_DUMMY:
	get32(m->u.dummy.application_type);
	get32(m->u.dummy.tatami);
	break;
    case MSG_MATCH_INFO:
	get32(m->u.match_info.tatami);
	get32(m->u.match_info.position);
	get32(m->u.match_info.category);
	get32(m->u.match_info.number);
	get32(m->u.match_info.blue);
	get32(m->u.match_info.white);
	get32(m->u.match_info.flags);
	get32(m->u.match_info.rest_time);
	get32(m->u.match_info.round);
	break;
    case MSG_11_MATCH_INFO:
	for (i = 0; i < 11; i++) {
	    get32(m->u.match_info_11.info[i].tatami);
	    get32(m->u.match_info_11.info[i].position);
	    get32(m->u.match_info_11.info[i].category);
	    get32(m->u.match_info_11.info[i].number);
	    get32(m->u.match_info_11.info[i].blue);
	    get32(m->u.match_info_11.info[i].white);
	    get32(m->u.match_info_11.info[i].flags);
	    get32(m->u.match_info_11.info[i].rest_time);
	    get32(m->u.match_info_11.info[i].round);
	}
	break;
    case MSG_NAME_INFO:
	get32(m->u.name_info.index);
	getstr(m->u.name_info.last);
	getstr(m->u.name_info.first);
	getstr(m->u.name_info.club);
	break;
    case MSG_NAME_REQ:
	get32(m->u.name_req.index);
	break;
    case MSG_ALL_REQ:
	break;
    case MSG_CANCEL_REST_TIME:
	get32(m->u.cancel_rest_time.category);
	get32(m->u.cancel_rest_time.number);
	get32(m->u.cancel_rest_time.blue);
	get32(m->u.cancel_rest_time.white);
	break;
    case MSG_UPDATE_LABEL:
	getstr(m->u.update_label.expose);
	getstr(m->u.update_label.text);
	getstr(m->u.update_label.text2);
	getstr(m->u.update_label.text3);
	getdbl(m->u.update_label.x);
	getdbl(m->u.update_label.y);
	getdbl(m->u.update_label.w);
	getdbl(m->u.update_label.h);
	getdbl(m->u.update_label.fg_r);
	getdbl(m->u.update_label.fg_g);
	getdbl(m->u.update_label.fg_b);
	getdbl(m->u.update_label.bg_r);
	getdbl(m->u.update_label.bg_g);
	getdbl(m->u.update_label.bg_b);
	getdbl(m->u.update_label.size);
	get32(m->u.update_label.label_num);
	get32(m->u.update_label.xalign);
	break;
    case MSG_EDIT_COMPETITOR:
	get32(m->u.edit_competitor.operation);
	get32(m->u.edit_competitor.index);
	getstr(m->u.edit_competitor.last);
	getstr(m->u.edit_competitor.first);
	get32(m->u.edit_competitor.birthyear);
	getstr(m->u.edit_competitor.club);
	getstr(m->u.edit_competitor.regcategory);
	get32(m->u.edit_competitor.belt);
	get32(m->u.edit_competitor.weight);
	get32(m->u.edit_competitor.visible);
	getstr(m->u.edit_competitor.category);
	get32(m->u.edit_competitor.deleted);
	getstr(m->u.edit_competitor.country);
	getstr(m->u.edit_competitor.id);
	get32(m->u.edit_competitor.seeding);
	get32(m->u.edit_competitor.clubseeding);
	get32(m->u.edit_competitor.matchflags);
	getstr(m->u.edit_competitor.comment);
	getstr(m->u.edit_competitor.coachid);
	getstr(m->u.edit_competitor.beltstr);
	getstr(m->u.edit_competitor.estim_category);
	break;
    case MSG_SCALE:
	get32(m->u.scale.weight);
	getstr(m->u.scale.config);
	break;
    case MSG_LANG:
	getstr(m->u.lang.english);
	getstr(m->u.lang.translation);
	break;
    case MSG_LOOKUP_COMP:
	getstr(m->u.lookup_comp.name);
	for (i = 0; i < NUM_LOOKUP; i++) {
	    get32(m->u.lookup_comp.result[i].index);
	    getstr(m->u.lookup_comp.result[i].fullname);
	}
	break;
    }

    return 0;
}
