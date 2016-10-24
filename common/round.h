#ifndef _ROUND_H_
#define _ROUND_H_

#define ROUND_MASK            0x00ff
#define ROUND_TYPE_MASK       0x0f00
#define ROUND_UP_DOWN_MASK    0xf000
#define ROUND_UPPER           0x1000
#define ROUND_LOWER           0x2000
#define ROUND_ROBIN           (1<<8)
#define ROUND_REPECHAGE       (2<<8)
#define ROUND_REPECHAGE_1     (ROUND_REPECHAGE | ROUND_UPPER)
#define ROUND_REPECHAGE_2     (ROUND_REPECHAGE | ROUND_LOWER)
#define ROUND_SEMIFINAL       (3<<8)
#define ROUND_SEMIFINAL_1     (ROUND_SEMIFINAL | ROUND_UPPER)
#define ROUND_SEMIFINAL_2     (ROUND_SEMIFINAL | ROUND_LOWER)
#define ROUND_BRONZE          (4<<8)
#define ROUND_BRONZE_1        (ROUND_BRONZE | ROUND_UPPER)
#define ROUND_BRONZE_2        (ROUND_BRONZE | ROUND_LOWER)
#define ROUND_SILVER          (5<<8)
#define ROUND_FINAL           (6<<8)
#define ROUND_IS_FRENCH(_n)   ((_n & ROUND_TYPE_MASK) == 0 || \
			       (_n & ROUND_TYPE_MASK) == ROUND_REPECHAGE)
#define ROUND_TYPE(_n)        (_n & ROUND_TYPE_MASK)
#define ROUND_NUMBER(_n)      (_n & ROUND_MASK)
#define ROUND_TYPE_NUMBER(_n) (ROUND_TYPE(_n) | ROUND_NUMBER(_n))

#endif
