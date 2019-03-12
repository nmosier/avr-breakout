/* util.h
 * basic utility functions
 */

#ifndef __UTIL_H
#define __UTIL_H

#include <stdint.h>

struct coords {
   uint8_t x;
   uint8_t y;
};

struct extent {
   uint8_t w;
   uint8_t h;
};

struct bounds {
   struct coords crds;
   struct extent ext;
};



/* general util macros */
#define LEN(arr) (sizeof(arr) / sizeof(*arr))
#define END(arr) (arr + LEN(arr))

/* min/max functions */
#define MIN(i1, i2) ((i1) < (i2) ? (i1) : (i2))
#define MAX(i1, i2) ((i2) < (i2) ? (i2) : (i1))

#define INTTYPE(intsize) uint##intsize##_t
#define UMIN_TEMPLATE(intsize)                                           \
   uint##intsize##_t umin##intsize(uint##intsize##_t i1,          \
                                         uint##intsize##_t i2) {        \
      return i1 < i2 ? i1 : i2;                                         \
   }
#define UMAX_TEMPLATE(intsize)                                           \
   uint##intsize##_t umax##intsize(uint##intsize##_t i1,         \
                                         uint##intsize##_t i2) {        \
      return i1 < i2 ? i2 : i1;                                         \
   }


UMIN_TEMPLATE(8);
UMAX_TEMPLATE(8);


uint8_t bounds_touch(const struct bounds *bnds1, const struct bounds *bnds2);


#endif

