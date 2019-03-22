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

uint8_t phys_touch_velocity(uint8_t touch, const struct velocity *vel) {
   const uint8_t lut[TOUCH_MAX] = {[TOUCH_UPB] = VEL_FLIP_Y,
                                   [TOUCH_RIGHTB] = VEL_FLIP_X,
                                   [TOUCH_DOWNB] = VEL_FLIP_Y,
                                   [TOUCH_LEFTB] = VEL_FLIP_X};

   uint8_t mask = VEL_FLIP_NONE;
   for (uint8_t i = 0; i < TOUCH_MAX; ++i) {
      if ((touch & (1 << i))) {
         mask |= lut[i];
      }
   }

   return mask;
}

uint8_t phys_adjust_velocity(uint8_t touch, struct velocity *vel) {
   switch (touch) {
   case BOUNDS_TOUCH_TOP:
      return (vel->vy < 0) ? VEL_FLIP_Y : VEL_FLIP_NONE;
   case BOUNDS_TOUCH_BOTTOM:
      return (vel->vy > 0) ? VEL_FLIP_Y : VEL_FLIP_NONE;

   case BOUNDS_TOUCH_LEFT:
      return (vel->vx < 0) ? VEL_FLIP_X : VEL_FLIP_NONE;
   case BOUNDS_TOUCH_RIGHT:
      return (vel->vx > 0) ? VEL_FLIP_X : VEL_FLIP_NONE;

   case BOUNDS_TOUCH_CORNER_TOPLEFT:
      return (vel->vx < 0 && vel->vy < 0) ? (VEL_FLIP_X | VEL_FLIP_Y) : VEL_FLIP_NONE;
   case BOUNDS_TOUCH_CORNER_TOPRIGHT:
      return (vel->vx > 0 && vel->vy < 0) ? (VEL_FLIP_X | VEL_FLIP_Y) : VEL_FLIP_NONE;
   case BOUNDS_TOUCH_CORNER_BOTTOMLEFT:
      return (vel->vx < 0 && vel->vy > 0) ? (VEL_FLIP_X | VEL_FLIP_Y) : VEL_FLIP_NONE;
   case BOUNDS_TOUCH_CORNER_BOTTOMRIGHT:
      return (vel->vx > 0 && vel->vy > 0) ? (VEL_FLIP_X | VEL_FLIP_Y) : VEL_FLIP_NONE;
      
   default:
      return VEL_FLIP_NONE;
   }
}

void phys_flip_velocity(uint8_t flags, struct velocity *vel) {
   if ((flags & VEL_FLIP_X)) {
      vel->vx = -vel->vx;
   }
   if ((flags & VEL_FLIP_Y)) {
      vel->vy = -vel->vy;
   }
}

/* phys_ball_freebounce: Bounce ball around on screen unobstructed. */
// TODO: abstract away velocity deflections to abstract interface.
void phys_ball_freebounce(struct bounds *ball_pos,
                          struct velocity *ball_vel,
                          struct bounds *update) {

   uint8_t flip = VEL_FLIP_NONE; // velocity flip flags

   /* check velocity deflections  */
   flip |= phys_touch_velocity(bounds_touch(ball_pos, &screen_bnds), ball_vel);
   flip |= phys_touch_velocity(bounds_touch(&paddle_pos, ball_pos), ball_vel);
   flip |= phys_grid_deflect(ball_pos, ball_vel, update);

   /* apply velocity flip */
   phys_flip_velocity(flip, ball_vel);
   
   /* initialize update bounds */
   struct bounds ball_pos_old;
   memcpy(&ball_pos_old, ball_pos, sizeof(*ball_pos));

   /* update position given velocity */
   phys_object_move(ball_pos, ball_vel, update);
   
   /* insert new position into blist */
   bounds_union(update, &ball_pos_old, ball_pos, NULL);
}


uint8_t phys_grid_deflect(const struct bounds *bnds, struct velocity *vel,
                       struct bounds *update) {
   struct bounds block_bnds;

   /* get bounding box for bnds in grid */
   project_round(bnds, &block_bnds, &g_proj_pix2grid, PROJ_MODE_FUZZY);

   /* check contact of bnds and bounding box */
   uint8_t touch = bounds_touch(bnds, &block_bnds);

   /* if no contact, then there will be no collision with grid */
   if (touch == TOUCH_NONE) {
      return VEL_FLIP_NONE;
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

   flip = flip_corner = VEL_FLIP_NONE;
   for (uint8_t row = grid_path.crds.y; row < row_end; ++row) {
      for (uint8_t col = grid_path.crds.x; col < col_end; ++col) {
         if (grid_testblock(row, col)) {
            /* found collision */
            uint8_t diff_col = (col != grid_ball.crds.x);
            uint8_t diff_row = (row != grid_ball.crds.y);
            struct bounds update_block = {.crds = {.x = col * 8, .y = row},
                                          .ext = {.w = 8, .h = 1}};
            
            if (diff_col && diff_row) {
               /* corner case (pun [not] intended) */
               flip_corner = VEL_FLIP_X | VEL_FLIP_Y;
            } else {
               /* side case */
               uint8_t flip_mask = diff_col ? VEL_FLIP_X : VEL_FLIP_Y;
               
               if ((flip_mask & flip) == VEL_FLIP_NONE) {
                  flip |= flip_mask;
                  grid_clearblock(row, col);
                  bounds_union(update, &update_block, NULL);
               }
            }
         }
      }
   }

   return flip;
}


void phys_object_move(struct bounds *obj, const struct velocity *vel,
                      struct bounds *update) {

   bounds_union_pair(obj, update, update);

   /* add velocity differential bounds */
   obj->crds.x += vel->vx;
   obj->crds.y += vel->vy;
   
   bounds_union_pair(obj, update, update);
}
