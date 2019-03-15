/* util.h
 * basic utility functions
 */

#ifndef __UTIL_H
#define __UTIL_H

#include <stdint.h>
#include <stdlib.h>
#include "SSD1306.h"

struct coords {
   uint8_t x;
   uint8_t y;
};

struct velocity {
   int8_t vx;
   int8_t vy;
};

struct extent {
   uint8_t w;
   uint8_t h;
};

struct bounds {
   struct coords crds;
   struct extent ext;
};

struct bounds_list {
   struct bounds bnds;
   struct bounds_list *next;
};

#define PROJ_FUNC_MUL 1
#define PROJ_FUNC_DIV 0
enum proj_mode {PROJ_MODE_SHARP, PROJ_MODE_FUZZY};
struct projection {
   uint8_t fx: 1; // x function (mul./div.)
   uint8_t fy: 1; // y function (mul./div.)
   uint8_t sx; // x scalar
   uint8_t sy; // y scalar
};

/* general util macros */
#define LEN(arr) (sizeof(arr) / sizeof(*arr))
#define END(arr) (arr + LEN(arr))

/* min/max functions */
#define MIN(i1, i2) ((i1) < (i2) ? (i1) : (i2))
#define MAX(i1, i2) ((i2) < (i2) ? (i2) : (i1))

#define UMIN_TEMPLATE(intsize)                                          \
   inline uint##intsize##_t umin##intsize(uint##intsize##_t i1,         \
                                          uint##intsize##_t i2) {       \
      return i1 < i2 ? i1 : i2;                                         \
   }
#define UMAX_TEMPLATE(intsize)                                          \
   inline uint##intsize##_t umax##intsize(uint##intsize##_t i1,          \
                                         uint##intsize##_t i2) {        \
      return i1 < i2 ? i2 : i1;                                         \
   }
#define ABSU_TEMPLATE(intsize)                                    \
   inline uint##intsize##_t absu##intsize(int##intsize##_t i) {   \
      return i < 0 ? -i : i;                                      \
   }

uint8_t udivup8(uint8_t dividend, uint8_t divisor);
uint8_t udivdwn8(uint8_t dividend, uint8_t divisor);
uint8_t urndup8(uint8_t n, uint8_t fact);
uint8_t urnddwn8(uint8_t n, uint8_t fact);

UMIN_TEMPLATE(8);
UMAX_TEMPLATE(8);
ABSU_TEMPLATE(8);

enum {BOUNDS_TOUCH_NONE,
      BOUNDS_TOUCH_LEFT,
      BOUNDS_TOUCH_RIGHT,
      BOUNDS_TOUCH_TOP,
      BOUNDS_TOUCH_BOTTOM,
      BOUNDS_TOUCH_CORNER_TOPLEFT,
      BOUNDS_TOUCH_CORNER_TOPRIGHT,
      BOUNDS_TOUCH_CORNER_BOTTOMLEFT,
      BOUNDS_TOUCH_CORNER_BOTTOMRIGHT,
      BOUNDS_OVERLAP
};

uint8_t bounds_touch_outer(const struct bounds *bnds1,
                           const struct bounds *bnds2);
uint8_t bounds_touch_inner(const struct bounds *bnds_inner,
                           const struct bounds *bnds_outer);
void bounds_union(const struct bounds *bnds1, const struct bounds *bnds2,
                  struct bounds *un);
inline uint16_t bounds_area(const struct bounds *bnds) {
   return bnds->ext.w * bnds->ext.h;
}

inline void bounds_downsize(struct bounds *bnds) {
   bnds->ext.h = (bnds->ext.h + (bnds->crds.y & ~SSD1306_PAGE_MASK) + 7) / 8;
   bnds->crds.y = bnds->crds.y / 8;
}

///////////// BOUNDS LIST /////////////
inline void blist_init(struct bounds_list **blist) {
   *blist = NULL;
}

void blist_insert(const struct bounds *bnds, struct bounds_list **blist);
void blist_delete(struct bounds_list **blist);

/////////// PROJECTIONS ///////////
uint8_t project(const struct bounds *src, struct bounds *dst,
             const struct projection *proj, enum proj_mode mode);

#endif

