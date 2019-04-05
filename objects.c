/* objects.c
 * Global definitions of shared game objects:
 *  - Game grid
 *  - Paddle
 *  - Ball(s)
 */


#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include "canvas.h"
#include "SSD1306.h"
#include "objects.h"
#include "physics.h"
#include "canvas.h"
#include "util.h"
#include "ball.h"
#include "paddle.h"
#include "breakout.h"

static void objpool_interact_pair(struct object *obj1, struct object *obj2);

//////////// OBJECTS ////////////
/* object pool */
struct object_pool g_objpool =
   {.arr = {[0] = OBJ_SCREEN_DEF(DISPLAY_WIDTH, DISPLAY_HEIGHT),
            [1] = OBJ_GRID_DEF,
            [2] = OBJ_PADDLE_DEF(50 - 10, 30 + BALL_HEIGHT * 2 + 2, PADDLE_WIDTH * 2, PADDLE_HEIGHT, 0, -1),
            // [2] = OBJ_BALL_DEF(BALL_COL, BALL_ROW, BALL_WIDTH * 3, BALL_HEIGHT * 3, BALL_VX, BALL_VY),
            [3] = OBJ_BALL_DEF(50, 30, BALL_WIDTH * 2, BALL_HEIGHT * 2, 1, 1),
            //[5] = OBJ_BALL_DEF(BALL_COL + 20, BALL_ROW + 5, BALL_WIDTH * 3, BALL_HEIGHT * 3, -1, -1),
            //[6] = OBJ_BALL_DEF(BALL_COL - 10, BALL_ROW - 10, BALL_WIDTH / 2, BALL_HEIGHT / 2, 1, 1)
            },
    .cnt = 4};

/* interact a pair of objects */
static void objpool_interact_pair(struct object *obj1, struct object *obj2) {
   uint8_t obj1_k = obj1->obj_kind,
           obj2_k = obj2->obj_kind;
   touch_t touch;
   uint8_t flip;

   /* detect collision & collide */
   switch (obj1_k) {
   case OBJ_K_BOUNDED:
      {
         switch (obj2_k) {
         case OBJ_K_BOUNDED: // collide bounded objects
            touch = obj_bnded_detect_collision(obj1, obj2);
            obj_bnded_collide(obj1, obj2, touch);
            break;
            
         case OBJ_K_GRID: // collide bounded object with grid
            flip = phys_grid_deflect(&obj1->obj_un.obj_bnded.obj_bnds,
                                     &obj1->obj_un.obj_bnded.obj_vel);
            phys_flip_velocity(flip, &obj1->obj_un.obj_bnded.obj_vel);
            break;
            
         default: // do nothing
            break;
         }
      }
      break;

   case OBJ_K_GRID:
   default: // do nothing
      break; 
   }
}

void objpool_map(struct object_pool *objpool, objpool_map_func_t func) {
   /* iterate over object pool array */
   struct object *obj_begin = objpool->arr,
                 *obj_end = obj_begin + objpool->cnt;
   for (struct object *obj_it = obj_begin; obj_it != obj_end; ++obj_it) {
      func(obj_it);
   }
}

void object_update(struct object *obj) {
   display_update(&obj->obj_update);
   memset(&obj->obj_update, 0, sizeof(obj->obj_update));
}

/* NOTE: works on ALL kinds of objects. */
void object_move(struct object *obj) {
   switch (obj->obj_kind) {
   case OBJ_K_BOUNDED:
      bounds_union_pair(&obj->obj_un.obj_bnded.obj_bnds, &obj->obj_update, &obj->obj_update);
      obj->obj_un.obj_bnded.obj_bnds.crds.x += obj->obj_un.obj_bnded.obj_vel.vx;
      obj->obj_un.obj_bnded.obj_bnds.crds.y += obj->obj_un.obj_bnded.obj_vel.vy;
      bounds_union_pair(&obj->obj_un.obj_bnded.obj_bnds, &obj->obj_update, &obj->obj_update);
      break;
      
   case OBJ_K_GRID:
   default:
      break;
   }
}

void object_special(struct object *obj) {
   if (obj->obj_special) {
      obj->obj_special(obj);
   }
}

/* object interaction */
void objpool_interact(struct object_pool *objpool) {
   struct object *obj_begin = objpool->arr,
                 *obj_end = obj_begin + objpool->cnt;
   for (struct object *obj_i = obj_begin; obj_i != obj_end; ++obj_i) {
      for (struct object *obj_j = obj_begin; obj_j != obj_end; ++obj_j) {
         if (obj_i != obj_j) {
            objpool_interact_pair(obj_i, obj_j);
         }
      }
   }
}

/////// GRID /////////
uint8_t grid[GRID_HEIGHT_BYTES][GRID_WIDTH_BYTES] =
   {{
     0b11000111,
     0b01000001,
     0b01000001,
     0b00100011,
     0b00000001,
     0b00000011,
     0b00000011,
     0b00000001,
     }};
