/* -*- mode: C; c-basic-offset: 4;  -*- */

/*
 * Copyright (C) 2006-2011 by Hannu Jokinen
 * Full copyright text is included in the software package.
 */ 

/* Messages */

#ifndef _COMM_H_
#define _COMM_H_

#define SHIAI_PORT     2310
#define JUDOTIMER_PORT 2311

#define COMM_VERSION 2

#define APPLICATION_TYPE_UNKNOWN 0
#define APPLICATION_TYPE_SHIAI   1
#define APPLICATION_TYPE_TIMER   2
#define APPLICATION_TYPE_INFO    3
#define APPLICATION_TYPE_WEIGHT  4
#define APPLICATION_TYPE_JUDOGI  5
#define NUM_APPLICATION_TYPES    6

#define COMM_ESCAPE 0xff
#define COMM_FF     0xfe
#define COMM_BEGIN  0xfd
#define COMM_END    0xfc

enum message_types {
    MSG_NEXT_MATCH = 1,
    MSG_RESULT,
    MSG_ACK,
    MSG_SET_COMMENT,
    MSG_SET_POINTS,
    MSG_HELLO,
    MSG_DUMMY,
    MSG_MATCH_INFO,
    MSG_NAME_INFO,
    MSG_NAME_REQ,
    MSG_ALL_REQ,
    MSG_CANCEL_REST_TIME,
    MSG_UPDATE_LABEL,
    MSG_EDIT_COMPETITOR,
    MSG_SCALE,
    NUM_MESSAGES
};

#define MATCH_FLAG_BLUE_DELAYED  0x0001
#define MATCH_FLAG_WHITE_DELAYED 0x0002
#define MATCH_FLAG_REST_TIME     0x0004
#define MATCH_FLAG_BLUE_REST     0x0008
#define MATCH_FLAG_WHITE_REST    0x0010
#define MATCH_FLAG_SEMIFINAL_A   0x0020  
#define MATCH_FLAG_SEMIFINAL_B   0x0040
#define MATCH_FLAG_BRONZE_A      0x0080
#define MATCH_FLAG_BRONZE_B      0x0100
#define MATCH_FLAG_GOLD          0x0200
#define MATCH_FLAG_SILVER        0x0400
#define MATCH_FLAG_JUDOGI1_OK    0x0800
#define MATCH_FLAG_JUDOGI1_NOK   0x1000
#define MATCH_FLAG_JUDOGI2_OK    0x2000
#define MATCH_FLAG_JUDOGI2_NOK   0x4000
#define MATCH_FLAG_JUDOGI_MASK (MATCH_FLAG_JUDOGI1_OK | MATCH_FLAG_JUDOGI1_NOK | \
                                MATCH_FLAG_JUDOGI2_OK | MATCH_FLAG_JUDOGI2_NOK)

struct msg_next_match {
    int tatami;
    int category;
    int match;
    int minutes;
    int match_time;
    int gs_time;
    int rest_time;
    char pin_time_ippon;
    char pin_time_wazaari;
    char pin_time_yuko;
    char pin_time_koka;
    char cat_1[32];
    char blue_1[64];
    char white_1[64];
    char cat_2[32];
    char blue_2[64];
    char white_2[64];
    int  flags;
};

struct msg_result {
    int tatami;
    int category;
    int match;
    int minutes;
    int blue_score;
    int white_score;
    char blue_vote;
    char white_vote;
    char blue_hansokumake;
    char white_hansokumake;
};

struct msg_set_comment {
    int data;
    int category;
    int number;
    int cmd;
    int sys;
};

struct msg_set_points {
    int category;
    int number;
    int sys;
    int blue_points;
    int white_points;
};

struct msg_ack {
    int tatami;
};

struct msg_hello {
    char          info_competition[30];
    char          info_date[30];
    char          info_place[30];
};

struct msg_dummy {
    int application_type;
    int tatami;
};

enum info_types {
    INFO_RUN_COLOR,
    INFO_OSAEKOMI_COLOR,
    INFO_TIMER,
    INFO_OSAEKOMI,
    INFO_POINTS,
    INFO_SCORE,
    NUM_INFOS
};

struct msg_match_info {
    int tatami;
    int position;
    int category;
    int number;
    int blue;
    int white;
    int flags;
    int rest_time;
};

struct msg_name_info {
    int index;
    char last[64];
    char first[64];
    char club[64];
};

struct msg_name_req {
    int index;
};

struct msg_cancel_rest_time {
    int category;
    int number;
    int blue;
    int white;
};

struct msg_update_label {
    gchar expose[64];
    gchar text[64];
    gchar text2[64];
    gchar text3[16];
    gdouble x, y;
    gdouble w, h;
    gdouble fg_r, fg_g, fg_b;
    gdouble bg_r, bg_g, bg_b;
    gdouble size;
    gint label_num;
    gint xalign;
};

#define EDIT_OP_GET        0
#define EDIT_OP_SET        1
#define EDIT_OP_SET_WEIGHT 2
#define EDIT_OP_SET_FLAGS  4
#define EDIT_OP_GET_BY_ID  8
#define EDIT_OP_SET_JUDOGI 16
#define EDIT_OP_CONFIRM    32

struct msg_edit_competitor {
    gint operation;
    gint index; // == 0 -> new competitor
    gchar last[32];
    gchar first[32];
    gint birthyear;
    gchar club[32];
    gchar regcategory[20];
    gint belt;
    gint weight;
    gint visible;
    gchar category[20];
    gint deleted;
    gchar country[20];
    gchar id[20];
    gint seeding;
    gint clubseeding;
    gint matchflags;
};

struct msg_scale {
    gint weight;
};

struct message {
    long  src_ip_addr; // Source address of the packet. Added by the comm node (network byte order).
    char  type;
    int   sender;
    union {
        struct msg_next_match  next_match;
        struct msg_result      result;
        struct msg_ack         ack;
        struct msg_set_comment set_comment;
        struct msg_set_points  set_points;
        struct msg_hello       hello;
        struct msg_match_info  match_info;
        struct msg_name_info   name_info;
        struct msg_name_req    name_req;
        struct msg_cancel_rest_time cancel_rest_time;
        struct msg_update_label update_label;
        struct msg_edit_competitor edit_competitor;
        struct msg_scale       scale;
        struct msg_dummy       dummy;
    } u;
};

#define MSG_QUEUE_LEN 2048
extern volatile gint msg_queue_put, msg_queue_get;
extern struct message msg_to_send[MSG_QUEUE_LEN];
extern GStaticMutex send_mutex;

/* comm */
extern void open_comm_socket(void);
extern void send_packet(struct message *msg);
extern gint send_msg(gint fd, struct message *msg);
extern gboolean msg_accepted(struct message *m);
extern gboolean keep_connection(void);
extern gint get_port(void);
extern gint application_type(void);
extern gint pwcrc32(const guchar *str, gint len);
extern gint encode_msg(struct message *m, guchar *buf, gint buflen);
extern gint decode_msg(struct message *m, guchar *buf, gint buflen);

#endif
