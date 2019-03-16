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
   phys_grid_deflect(ball_pos, ball_vel);
   
   /* initialize update bounds */
   struct bounds ball_pos_old;
   memcpy(&ball_pos_old, ball_pos, sizeof(*ball_pos));

   /* insert old position into blist */
   //blist_insert(ball_pos, update);
   
   /* update position given velocity */
   ball_pos->crds.x += ball_vel->vx;
   ball_pos->crds.y += ball_vel->vy;
   
   /* insert new position into blist */
   //blist_insert(ball_pos, update);
   bounds_union(&ball_pos_old, ball_pos, update);
}


// TODO: revise this monstrosity.
#if 0
void phys_grid_deflect(const struct bounds *bnds, struct velocity *vel) {
   uint8_t cmp_x[2], cmp_y[2];
   
   cmp_x[0] = (bnds->crds.x % 8 == 0 && bnds->crds.x > 0 && vel->vx < 0);
   cmp_x[1] = (((bnds->crds.x + bnds->ext.w) % 8 == 0) &&
               ((bnds->crds.x + bnds->ext.w) < DISPLAY_WIDTH - 1) &&
               vel->vx > 0);
   cmp_y[0] = ((bnds->crds.y % 8 == 0) && (bnds->crds.y > 0) && vel->vy < 0);
   cmp_y[1] = (((bnds->crds.y + bnds->ext.h) % 8 == 0) &&
               (bnds->crds.y + bnds->ext.h < DISPLAY_HEIGHT - 1) &&
               vel->vy > 0);
   
   struct bounds gbnds;
   project_down(bnds, &gbnds, &g_proj_pix2grid, PROJ_MODE_FUZZY);
   
   uint8_t flip_vx = 0; // whether to invert x velocity
   uint8_t flip_vy = 0; // whether to invert y velocity

   if (cmp_x[0]) {
      for (uint8_t y = gbnds.crds.y; y < gbnds.crds.y + gbnds.ext.h; ++y) {
         if (grid_testblock(y, gbnds.crds.x - 1)) {
            flip_vx = 1;
            grid_clearblock(y, gbnds.crds.x - 1);
            grid_displayblock(y, gbnds.crds.x - 1);
         }
      }
   }
   if (cmp_x[1]) {
      for (uint8_t y = gbnds.crds.y; y < gbnds.crds.y + gbnds.ext.h; ++y) {
         if (grid_testblock(y, gbnds.crds.x + gbnds.ext.w)) {
            flip_vx = 1;
            grid_clearblock(y, gbnds.crds.x + gbnds.ext.w);
            grid_displayblock(y, gbnds.crds.x + gbnds.ext.w);
         }
      }
   }
   if (cmp_y[0]) {
      for (uint8_t x = gbnds.crds.x; x < gbnds.crds.x + gbnds.ext.h; ++x) {
         if (grid_testblock(gbnds.crds.y - 1, x)) {
            flip_vy = 1;
            grid_clearblock(gbnds.crds.y -1 , gbnds.crds.x - 1);
            grid_displayblock(gbnds.crds.y -1 , gbnds.crds.x - 1);
         }
      }
   }
   if (cmp_y[1]) {
      for (uint8_t x = gbnds.crds.x; x < gbnds.crds.x + gbnds.ext.h; ++x) {
         if (grid_testblock(gbnds.crds.y + gbnds.ext.h, x)) {
            flip_vy = 1;
            grid_clearblock(gbnds.crds.y + gbnds.ext.h, x);
            grid_displayblock(gbnds.crds.y + gbnds.ext.h, x);
         }
      }
   }

   if (flip_vx == 0 && flip_vy == 0) {
      /* check corners */
      uint8_t grid_x, grid_y;

      grid_x = gbnds.crds.x - 1;
      grid_y = gbnds.crds.y - 1;
      if (cmp_x[1]) {
         grid_x = gbnds.crds.x + gbnds.ext.w;
      }
      if (cmp_y[1]) {
         grid_y = gbnds.crds.y + gbnds.ext.h;
      }

      for (uint8_t i = 0; i < 2; ++i) {
         for (uint8_t j = 0; j < 2; ++j) {
            if (cmp_x[i] && cmp_y[j] && grid_testblock(grid_y, grid_x)) {
               flip_vx = 1;
               flip_vy = 1;
               grid_clearblock(grid_y, grid_x);
               grid_displayblock(grid_y, grid_x);
            }
         }
      }
   }

   if (flip_vx) {
      vel->vx *= -1;
   }
   if (flip_vy) {
      vel->vy *= -1;
   }
}
#else
void phys_grid_deflect(const struct bounds *bnds, struct velocity *vel) {
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
   bounds_union(bnds, &newbnds, &path);

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
            
            if (diff_col && diff_row) {
               /* corner case (pun [not] intended) */
               flip_corner = 1;
            } else if (diff_col) {
               /* flip x component of velocity */
               flip_x = 1;
            } else { // if (diff_row) {
               /* flip y component of velocity */
               flip_y = 1;
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
}
#endif
