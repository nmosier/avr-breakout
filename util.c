/* util.h
 */

#include <stdint.h>
#include <string.h>
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




void bounds_union(const struct bounds *bnds1, const struct bounds *bnds2,
                  struct bounds *un) {
   un->crds.x = umin8(bnds1->crds.x, bnds2->crds.x);
   un->crds.y = umin8(bnds1->crds.y, bnds2->crds.y);
   un->ext.w = umax8(bnds1->crds.x + bnds1->ext.w,
                     bnds2->crds.x + bnds2->ext.w) - un->crds.x;
   un->ext.h = umax8(bnds1->crds.y + bnds1->ext.h,
                     bnds2->crds.y + bnds2->ext.h) - un->crds.y;
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

   bounds_union(bnds1, bnds2, un);
   dA = (int) bounds_area(un) - (int) bounds_area(bnds1) - (int) bounds_area(bnds2);

   return (dA < DISPLAY_SELECT_IOBYTES) ? 1 : 0;
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
