/* util.h
 */

#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include "SSD1306.h"
#include "util.h"

touch_t bounds_touch(const struct bounds * restrict bnds1,
                     const struct bounds * restrict bnds2) {

   touch_t mask = TOUCH_NONE;
   uint8_t cmp_x = (umax8(bnds1->crds.x, bnds2->crds.x) <
                    umin8(bnds1->crds.x + bnds1->ext.w,
                          bnds2->crds.x + bnds2->ext.w));
   uint8_t cmp_y = (umax8(bnds1->crds.y, bnds2->crds.y) <
               umin8(bnds1->crds.y + bnds1->ext.h,
                     bnds2->crds.y + bnds2->ext.h));

   uint8_t bnds_x[2][2] =
      {{bnds1->crds.x, bnds1->crds.x + bnds1->ext.w},
       {bnds2->crds.x, bnds2->crds.x + bnds2->ext.w}};
   uint8_t bnds_y[2][2] =
      {{bnds1->crds.y, bnds1->crds.y + bnds1->ext.h},
       {bnds2->crds.y, bnds2->crds.y + bnds2->ext.h}};

   
   for (uint8_t j = 0; j < 2; ++j) {
      for (uint8_t i = 0; i < 2; ++i) {
         if (cmp_y && bnds_x[0][j] == bnds_x[1][i]) {
            /* equal bounds and other dimension overlaps */
            mask |= (i == 0) ? TOUCH_LEFT : TOUCH_RIGHT;
         }
      }
   }

   for (uint8_t j = 0; j < 2; ++j) {
      for (uint8_t i = 0; i < 2; ++i) {
         if (cmp_x && bnds_y[0][j] == bnds_y[1][i]) {
            /* equal bounds and other dimension overlaps */
            mask |= (i == 0) ? TOUCH_UP : TOUCH_DOWN;
         }
      }
   }

   return mask;
}

/* NOTE: un can be an alias of one of the constituent bounds.
 */
void bounds_union_pair(const struct bounds *bnds1, const struct bounds *bnds2,
                  struct bounds *un) {

   if (bounds_null(bnds1)) {
      memcpy(un, bnds2, sizeof(*un));
      return;
   }
   if (bounds_null(bnds2)) {
      memcpy(un, bnds1, sizeof(*un));
      return;
   }

   struct coords un_crds;
   un_crds.x = umin8(bnds1->crds.x, bnds2->crds.x);
   un_crds.y = umin8(bnds1->crds.y, bnds2->crds.y);
   un->ext.w = umax8(bnds1->crds.x + bnds1->ext.w,
                     bnds2->crds.x + bnds2->ext.w) - un_crds.x;
   un->ext.h = umax8(bnds1->crds.y + bnds1->ext.h,
                     bnds2->crds.y + bnds2->ext.h) - un_crds.y;
   un->crds.x = un_crds.x;
   un->crds.y = un_crds.y;
}

/* bounds_union (varidic form)
 * NOTE: last argument should be NULL.
 */
void bounds_union(struct bounds *un, ...) {
   va_list ap;

   /* varargs init */
   va_start(ap, un);

   /* loop through args */
   const struct bounds *bnds;
   do {
      if ((bnds = va_arg(ap, const struct bounds *))) {
         bounds_union_pair(bnds, un, un);
      }
   } while (bnds);

   /* clean up */
   va_end(ap);
}

void bounds_insersect(const struct bounds *bnds1, const struct bounds *bnds2,
                      struct bounds *intersect) {
   intersect->crds.x = umax8(bnds1->crds.x, bnds2->crds.x);
   intersect->crds.y = umax8(bnds1->crds.y, bnds2->crds.y);
   intersect->ext.w = umin8(bnds1->crds.x + bnds1->ext.w,
                            bnds2->crds.x + bnds2->ext.w) - intersect->crds.x;
   intersect->ext.h = umin8(bnds1->crds.y + bnds1->ext.h,
                            bnds2->crds.y + bnds2->ext.h) - intersect->crds.y;
}



/* bounds_should_union: would it be advantageous to union the two given display bounds,
 * assuming I/O-boundedness?
 * NOTE: _un_ always modified.
 */
uint8_t bounds_should_union(const struct bounds *bnds1,
                            const struct bounds *bnds2,
                            struct bounds *un) {
   int dA;

   memset(un, 0, sizeof(*un));
   bounds_union(un, bnds1, bnds2);
   
   dA = (int) bounds_area(un) - (int) bounds_area(bnds1) - (int) bounds_area(bnds2);

   /* divide by 8 b/c bounds not yet downsized for display */
   return (dA / 8 < DISPLAY_SELECT_IOBYTES) ? 1 : 0;
}

//////////// PROJECTIONS ////////////

// TODO: add function for projecting coordinates -- then use this in project_* functions.

uint8_t project_down(const struct bounds *src, struct bounds *dst,
                     const struct projection *proj, enum proj_mode mode) {
   /* determine division mode */
   uint8_t (*div_crds)(uint8_t, uint8_t) = (mode == PROJ_MODE_SHARP) ? udivup8 : udivdwn8;
   uint8_t (*div_ext)(uint8_t, uint8_t) = (mode == PROJ_MODE_SHARP)  ? udivdwn8 : udivup8;

   /* divide x coords */
   dst->crds.x = div_crds(src->crds.x, proj->sx);
   dst->ext.w = div_ext(src->ext.w - (-src->crds.x % proj->sx), proj->sx);

   /* divide y coords */
   dst->crds.y = div_crds(src->crds.y, proj->sy);
   dst->ext.h = div_ext(src->ext.h - (-src->crds.y % proj->sy), proj->sy);

   return dst->ext.w && dst->ext.h;
}

/* NOTE: _mode_ is ignored. Kept for symmetry. */
uint8_t project_up(const struct bounds *src, struct bounds *dst,
                   const struct projection *proj, enum proj_mode mode) {
   
   dst->crds.x = src->crds.x * proj->sx;
   dst->crds.y = src->crds.y * proj->sy;
   dst->ext.w = src->ext.w * proj->sx;
   dst->ext.h = src->ext.h * proj->sy;

   return dst->ext.w && dst->ext.h;
}

uint8_t project_round(const struct bounds *src, struct bounds *dst,
                      const struct projection *proj, enum proj_mode mode) {

   /* determine rounding mode */
   uint8_t (*rnd_crds)(uint8_t, uint8_t) = (mode == PROJ_MODE_SHARP) ? urndup8 : urnddwn8;
   uint8_t (*rnd_ext)(uint8_t, uint8_t)  = (mode == PROJ_MODE_SHARP) ? urnddwn8 : urndup8;

   /* round coords */
   dst->crds.x = rnd_crds(src->crds.x, proj->sx);
   dst->crds.y = rnd_crds(src->crds.y, proj->sy);

   /* round ext */
   dst->ext.w = rnd_ext(src->ext.w - (-src->crds.x % proj->sx), proj->sx);
   dst->ext.h = rnd_ext(src->ext.h - (-src->crds.y % proj->sy), proj->sy);

   return dst->ext.w && dst->ext.h;
}

uint8_t udivup8(uint8_t dividend, uint8_t divisor) {
   return (dividend + divisor - 1) / divisor;
}

uint8_t udivdwn8(uint8_t dividend, uint8_t divisor) {
   return dividend / divisor;
}

uint8_t urndup8(uint8_t n, uint8_t fact) {
   return udivup8(n, fact) * fact;
}

uint8_t urnddwn8(uint8_t n, uint8_t fact) {
   return udivdwn8(n, fact) * fact;
}
