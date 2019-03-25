/* physics.c
 * move objects
 */

#include <stdint.h>
#include <string.h>
#include "objects.h"
#include "canvas.h"
#include "util.h"
#include "physics.h"

/* prototypes */

static uint8_t phys_touch_velocity(touch_t touch, uint8_t vel) {
   const uint8_t lut[TOUCH_MAX] = {[TOUCH_UPB] = VEL_Y,
                                   [TOUCH_RIGHTB] = VEL_X,
                                   [TOUCH_DOWNB] = VEL_Y,
                                   [TOUCH_LEFTB] = VEL_X};

   uint8_t mask = VEL_NONE;
   for (uint8_t i = 0; i < TOUCH_MAX; ++i) {
      if ((touch & (1 << i))) {
         mask |= lut[i];
      }
   }

   return (mask & vel);
}

void phys_flip_velocity(uint8_t flags, struct velocity *vel) {
   if ((flags & VEL_X)) {
      vel->vx = -vel->vx;
   }
   if ((flags & VEL_Y)) {
      vel->vy = -vel->vy;
   }
}

void ball_collide(const struct bounds *pos, struct velocity *vel, uint8_t velmask) {
   uint8_t flip = VEL_NONE;

   /* check collisions */
   flip |= phys_touch_velocity(bounds_touch(pos, &screen_bnds), velmask);
   flip |= phys_touch_velocity(bounds_touch(&paddle_pos, pos), velmask);
   flip |= phys_grid_deflect(pos, vel);

   /* update velocity */
   phys_flip_velocity(flip, vel);
}

/* phys_ball_freebounce: Bounce ball around on screen unobstructed. */
// TODO: abstract away velocity deflections to abstract interface.
void phys_ball_freebounce(struct bounds *ball_pos,
                          struct velocity *ball_vel,
                          struct bounds *update) {

   /* check velocity deflections  */
#if 1
   uint8_t velmask = velocity_mask(ball_vel);
   ball_collide(ball_pos, ball_vel, velmask);
#else
   // obj_bnded_detect_collision(
#endif
   
   /* initialize update bounds */
   struct bounds ball_pos_old;
   memcpy(&ball_pos_old, ball_pos, sizeof(*ball_pos));

   /* update position given velocity */
   phys_object_freemove(ball_pos, ball_vel, update);
   
   /* insert new position into blist */
   bounds_union(update, &ball_pos_old, ball_pos, NULL);
}

uint8_t phys_grid_deflect(const struct bounds *bnds, struct velocity *vel) {
   struct bounds block_bnds;

   /* get bounding box for bnds in grid */
   project_round(bnds, &block_bnds, &g_proj_pix2grid, PROJ_MODE_FUZZY);

   /* check contact of bnds and bounding box */
   uint8_t touch = bounds_touch(bnds, &block_bnds);

   /* if no contact, then there will be no collision with grid */
   if (touch == TOUCH_NONE) {
      return VEL_NONE;
   }

   /* otherwise, advance ball along current trajectory and find the union grid bounds */
   struct bounds newbnds, path, grid_path, grid_ball;
   newbnds.crds.x = bnds->crds.x + vel->vx;
   newbnds.crds.y = bnds->crds.y + vel->vy;
   newbnds.ext = bnds->ext;
   bounds_union_pair(bnds, &newbnds, &path);

   /* project swept out path to gridspace */
   project_down(&path, &grid_path, &g_proj_pix2grid, PROJ_MODE_FUZZY);

   /* convert ball bounds to grid bounds */
   project_down(&block_bnds, &grid_ball, &g_proj_pix2grid, PROJ_MODE_FUZZY);

   /* search for nonempty blocks in path */
   uint8_t flip, flip_corner;
   uint8_t row_end = grid_path.crds.y + grid_path.ext.h;
   uint8_t col_end = grid_path.crds.x + grid_path.ext.w;

   flip = flip_corner = VEL_NONE;
   for (uint8_t row = grid_path.crds.y; row < row_end; ++row) {
      for (uint8_t col = grid_path.crds.x; col < col_end; ++col) {
         if (grid_testblock(row, col)) {
            /* found collision */
            uint8_t diff_col = (col != grid_ball.crds.x);
            uint8_t diff_row = (row != grid_ball.crds.y);

            if (diff_col && diff_row) {
               /* corner case (pun [not] intended) */
               flip_corner = VEL_X | VEL_Y;
            } else {
               /* side case */
               uint8_t flip_mask = diff_col ? VEL_X : VEL_Y;
               
               if ((flip_mask & flip) == VEL_NONE) {
                  flip |= flip_mask;
                  grid_clearblock(row, col);
                  grid_displayblock(row, col);
               }
            }
         }
      }
   }

   return flip;
}


void phys_object_freemove(struct bounds *obj, const struct velocity *vel,
                      struct bounds *update) {

   bounds_union_pair(obj, update, update);

   /* add velocity differential bounds */
   obj->crds.x += vel->vx;
   obj->crds.y += vel->vy;
   
   bounds_union_pair(obj, update, update);
}


/* collision detection for bounded objects */
touch_t obj_bnded_detect_collision(const struct object * restrict obj1,
                                   const struct object * restrict obj2) {

   const struct bounds * restrict bnds1 = &obj1->obj_un.obj_bnded.obj_bnds,
                       * restrict bnds2 = &obj2->obj_un.obj_bnded.obj_bnds;
   touch_t touch = bounds_touch(bnds1, bnds2);
   return touch;
}

/* collide two bounded objects */
void obj_bnded_collide(struct object *obj1, struct object *obj2, touch_t touch) {
   struct velocity *vel1 = &obj1->obj_un.obj_bnded.obj_vel,
                   *vel2 = &obj2->obj_un.obj_bnded.obj_vel;
   uint8_t velmask = velocity_mask(vel1);
   uint8_t velflip = phys_touch_velocity(touch, velmask);
   phys_flip_velocity(velflip, vel1);
   phys_flip_velocity(velflip, vel2);
}
