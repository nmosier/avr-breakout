/* util.h
 */

#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include "SSD1306.h"
#include "util.h"

static const uint8_t bounds_touch_lut[3][3] =
   {{BOUNDS_TOUCH_CORNER_TOPLEFT,  BOUNDS_TOUCH_LEFT,  BOUNDS_TOUCH_CORNER_BOTTOMLEFT},
    {BOUNDS_TOUCH_TOP,             BOUNDS_OVERLAP,     BOUNDS_TOUCH_BOTTOM},
    {BOUNDS_TOUCH_CORNER_TOPRIGHT, BOUNDS_TOUCH_RIGHT, BOUNDS_TOUCH_CORNER_BOTTOMRIGHT}
   };

/* bounds_touch_outer: detect where bounds touch each other, assuming 
 * the bounds do not overlap.
 * NOTE: all orientations relative to bnds2 */
uint8_t bounds_touch_outer(const struct bounds *bnds1,
                           const struct bounds *bnds2) {
   uint8_t cmp_x[3], cmp_y[3];

   cmp_x[0] = (bnds1->crds.x + bnds1->ext.w == bnds2->crds.x);
   cmp_x[1] = (umax8(bnds1->crds.x, bnds2->crds.x) <
               umin8(bnds1->crds.x + bnds1->ext.w,
                     bnds2->crds.x + bnds2->ext.w));
   cmp_x[2] = (bnds1->crds.x == bnds2->crds.x + bnds2->ext.w);
   
   cmp_y[0] = (bnds1->crds.y + bnds1->ext.h == bnds2->crds.y);
   cmp_y[1] = (umax8(bnds1->crds.y, bnds2->crds.y) <
               umin8(bnds1->crds.y + bnds1->ext.h,
                     bnds2->crds.y + bnds2->ext.h));
   cmp_y[2] = (bnds1->crds.y == bnds2->crds.y + bnds2->ext.h);

   for (uint8_t x = 0; x < 3; ++x) {
      for (uint8_t y = 0; y < 3; ++y) {
         if (cmp_x[x] && cmp_y[y]) {
            return bounds_touch_lut[x][y];
         }
      }
   }
   return BOUNDS_TOUCH_NONE;
}

uint8_t bounds_touch_inner(const struct bounds *bnds_inner,
                           const struct bounds *bnds_outer) {
   uint8_t cmp_x[3], cmp_y[3];

   cmp_x[0] = (bnds_inner->crds.x == bnds_outer->crds.x);
   cmp_x[1] = (bnds_inner->crds.x > bnds_outer->crds.x &&
               (bnds_inner->crds.x + bnds_inner->ext.w <
                bnds_outer->crds.x + bnds_outer->ext.w));
   cmp_x[2] = (bnds_inner->crds.x + bnds_inner->ext.w ==
               bnds_outer->crds.x + bnds_outer->ext.w);

   cmp_y[0] = (bnds_inner->crds.y == bnds_outer->crds.y);
   cmp_y[1] = (bnds_inner->crds.y > bnds_outer->crds.y &&
               (bnds_inner->crds.y + bnds_inner->ext.h <
                bnds_outer->crds.y + bnds_outer->ext.h));
   cmp_y[2] = (bnds_inner->crds.y + bnds_inner->ext.h ==
               bnds_outer->crds.y + bnds_outer->ext.h);


   for (uint8_t x = 0; x < 3; ++x) {
      for (uint8_t y = 0; y < 3; ++y) {
         if (cmp_x[x] && cmp_y[y]) {
            return bounds_touch_lut[x][y];
         }
      }
   }
   return BOUNDS_TOUCH_NONE;
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

void blist_insert(const struct bounds *bnds, struct bounds_list **blist) {
   struct bounds un;
   struct bounds_list *it, **prev;
   
   for (prev = blist, it = *blist; it; prev = &it->next, it = *prev) {
      if (bounds_should_union(bnds, &it->bnds, &un)) {
         /* 1. remove entry in list.
          * 2. insert union into list.
          */
         memcpy(&it->bnds, &un, sizeof(un));
         break;
      }
   }

   /* if no favorable union found, insert at end of list */
   if (it == NULL) {
      *prev = malloc(sizeof(*it));
      memcpy(&(*prev)->bnds, bnds, sizeof(*bnds));
      (*prev)->next = NULL;
   }
}

void blist_delete(struct bounds_list **blist) {
   struct bounds_list *it, *del;

   it = *blist;
   while (it) {
      del = it;
      it = it->next;
      free(del);
   }
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
