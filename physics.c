/* physics.c
 * move objects
 */

#include <stdint.h>
#include <string.h>
#include "objects.h"
#include "canvas.h"
#include "util.h"
#include "physics.h"

void phys_adjust_velocity(uint8_t touch, struct velocity *vel) {
   switch (touch) {
   case BOUNDS_TOUCH_TOP:
   case BOUNDS_TOUCH_BOTTOM:
      vel->vy *= -1;
      break;

   case BOUNDS_TOUCH_LEFT:
   case BOUNDS_TOUCH_RIGHT:
      vel->vx *= -1;
      break;

   case BOUNDS_TOUCH_CORNER_TOPLEFT: // invert xy-velocities
   case BOUNDS_TOUCH_CORNER_TOPRIGHT:
   case BOUNDS_TOUCH_CORNER_BOTTOMLEFT:
   case BOUNDS_TOUCH_CORNER_BOTTOMRIGHT:
      vel->vx *= -1;
      vel->vy *= -1;
      break;
      
   default:
      break;
   }
}

/* phys_ball_freebounce: Bounce ball around on screen unobstructed. */
void phys_ball_freebounce(struct bounds *ball_pos,
                          struct velocity *ball_vel,
                          struct bounds *update) {

   /* update velocity if necessary */
   phys_adjust_velocity(bounds_touch_inner(ball_pos, &screen_bnds), ball_vel);
   phys_adjust_velocity(bounds_touch_outer(ball_pos, &paddle_pos), ball_vel);

   /* grid deflection */
   phys_grid_deflect(ball_pos, ball_vel, update);
   
   /* initialize update bounds */
   struct bounds ball_pos_old;
   memcpy(&ball_pos_old, ball_pos, sizeof(*ball_pos));

   /* update position given velocity */
   ball_pos->crds.x += ball_vel->vx;
   ball_pos->crds.y += ball_vel->vy;
   
   /* insert new position into blist */
   bounds_union(update, &ball_pos_old, ball_pos, NULL);
}


#define PHYS_GRID_DEFLECT_MAXBLOCKS 2
void phys_grid_deflect(const struct bounds *bnds, struct velocity *vel,
                       struct bounds *update) {
   struct bounds block_bnds;

   /* get bounding box for bnds in grid */
   project_round(bnds, &block_bnds, &g_proj_pix2grid, PROJ_MODE_FUZZY);

   /* check contact of bnds and bounding box */
   uint8_t touch = bounds_touch_inner(bnds, &block_bnds);

   /* if no contact, then there will be no collision with grid */
   if (touch == BOUNDS_TOUCH_NONE) {
      return;
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
   uint8_t flip_x, flip_y, flip_corner;
   uint8_t row_end = grid_path.crds.y + grid_path.ext.h;
   uint8_t col_end = grid_path.crds.x + grid_path.ext.w;

   flip_x = flip_y = flip_corner = 0;
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
               flip_corner = 1;
            } else if (diff_col) {
               /* flip x component of velocity */
               if (!flip_x) {
                  flip_x = 1;
                  grid_clearblock(row, col);
                  bounds_union(update, &update_block, NULL);
               }
            } else { // if (diff_row) {
               /* flip y component of velocity */
               if (!flip_y) {
                  flip_y = 1;
                  grid_clearblock(row, col);
                  bounds_union(update, &update_block, NULL);
               }
            }
         }
      }
   }

   uint8_t should_flip_corner = !(flip_x || flip_y) && flip_corner;
   if (flip_x || should_flip_corner) {
      vel->vx = -vel->vx;
   }
   if (flip_y || should_flip_corner) {
      vel->vy = -vel->vy;
   }

   /* convert grid path to display space 
    * TODO: fix this to use project_* function. */
   //   grid_path.crds.x *= 8;
   //grid_path.ext.w *= 8;
   
   /* update surrounding blocks */
   //bounds_union(update, &grid_path, NULL);
}

