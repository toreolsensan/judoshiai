#ifndef _CUSTOM_CATEGORY_H_
#define _CUSTOM_CATEGORY_H_

/** Custom brackets */

#define NUM_CUSTOM_MATCHES   512
#define NUM_CUST_POS          16
#define NUM_ROUND_ROBIN_POOLS 16
#define NUM_RR_MATCHES        64

typedef struct player_bare {
#define COMP_TYPE_COMPETITOR  1
#define COMP_TYPE_MATCH       2
#define COMP_TYPE_ROUND_ROBIN 3
    int    type;
    int    num; // competitor num or match num or rr num
    int    pos; // winner = 1, loser = 2, or rr position
    char   prev[8];
} competitor_bare_t;

typedef struct fight2 {
    competitor_bare_t c1;
    competitor_bare_t c2;
} match_bare_t;

typedef struct position_bare {
    int    type;
    int    match;
    int    pos;
    int    real_contest_pos;
} position_bare_t;

typedef struct round_robin_bare {
    int rr_matches[NUM_RR_MATCHES];
    int num_rr_matches;
} round_robin_bare_t;

struct custom_data {
    char name_long[64];
    char name_short[16];
    int  competitors_min, competitors_max;

    round_robin_bare_t round_robin_pools[NUM_ROUND_ROBIN_POOLS];
    int num_round_robin_pools;

    match_bare_t matches[NUM_CUSTOM_MATCHES];
    int num_matches;

    position_bare_t positions[NUM_CUST_POS];
    int num_positions;
};

extern int read_custom_category(char *name, struct custom_data *data);

#endif
