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

static struct object g_obj_screen;
static struct object g_obj_grid;
static struct object g_obj_paddle;
static struct object g_obj_ball;

static void objpool_interact_pair(struct object *obj1, struct object *obj2);

//////////// OBJECTS ////////////
/* object pool */
struct object_pool g_objpool =
   {.arr = {[0] = OBJ_SCREEN_DEF(DISPLAY_WIDTH, DISPLAY_HEIGHT),
            [1] = OBJ_GRID_DEF,
            [2] = OBJ_PADDLE_DEF(PADDLE_COL, PADDLE_ROW, PADDLE_WIDTH, PADDLE_HEIGHT, 1, 1),
            [3] = OBJ_BALL_DEF(BALL_COL, BALL_ROW, BALL_WIDTH, BALL_HEIGHT, BALL_VX, BALL_VY),
            [4] = OBJ_BALL_DEF(BALL_COL - 20, BALL_ROW + 5, BALL_WIDTH * 2, BALL_HEIGHT * 2, -1, 1),
            [5] = OBJ_BALL_DEF(BALL_COL + 20, BALL_ROW + 5, BALL_WIDTH * 3, BALL_HEIGHT * 3, -1, -1),
            [6] = OBJ_BALL_DEF(BALL_COL - 10, BALL_ROW - 10, BALL_WIDTH / 2, BALL_HEIGHT / 2, 1, 1)
            },
    .cnt = 7};

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

#if 0
void objpool_move(struct object_pool *objpool) {
   struct object *obj_begin = objpool->arr,
                 *obj_end = obj_begin + objpool->cnt;
   for (struct object *obj_it = obj_begin; obj_it != obj_end; ++obj_it) {
      object_move(obj_it);
   }
}


void objpool_update(struct object_pool *objpool) {
   struct object *obj_begin = objpool->arr,
                 *obj_end = obj_begin + objpool->cnt;
   for (struct object *obj_it = obj_begin; obj_it != obj_end; ++obj_it) {
      object_update(obj_it);
   }
}

void objpool_special(struct object_pool *objpool) {
   struct object *obj_begin = objpool->arr,
                 *obj_end = obj_begin + objpool->cnt;
   for (struct object *obj_it = obj_begin; obj_it != obj_end; ++obj_it) {
      if (obj_it->obj_special) {
         obj_it->obj_special(obj_it);
      }
   }
}
#endif

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

/////////////// SCREEN ///////////////


static struct object g_obj_screen =
   {.obj_kind = OBJ_K_BOUNDED,
    .obj_un =
    {.obj_bnded =
     {.obj_bnds =
      {.crds = {.x = 0, .y = 0},
       .ext = {.w = DISPLAY_WIDTH, .h = DISPLAY_HEIGHT}
      },
      .obj_vel = {.vx = 0, .vy = 0},
     }
    },
    .obj_update = {{0}},
    .obj_graphics = {.draw = NULL},
    .obj_special = NULL
   };

struct bounds screen_bnds = {.crds = {.x = 0,
                                      .y = 0},
                             .ext = {.w = DISPLAY_WIDTH,
                                     .h = DISPLAY_HEIGHT}};
                                      

//////////////// GRID ////////////////

static struct object g_obj_grid =
   {.obj_kind = OBJ_K_GRID,
    .obj_update = {{0}},
    .obj_graphics = {.draw = grid_display_layer},
    .obj_special = NULL
   };

/* currently, only one byte per column */
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

/////////////// PADDLE ///////////////

static struct object g_obj_paddle =
   {.obj_kind = OBJ_K_BOUNDED,
    .obj_un =
    {.obj_bnded =
     {.obj_bnds =
      {.crds =
       {.x = (DISPLAY_WIDTH - PADDLE_WIDTH) / 2,
        .y = DISPLAY_HEIGHT - PADDLE_HEIGHT
       },
       .ext =
       {.w = PADDLE_WIDTH,
        .h = PADDLE_HEIGHT
       }
      },
      .obj_vel =
      {.vx = 1,
       .vy = 0
      }
     }
    },
    .obj_update = {{0}},
    .obj_graphics = {.draw = paddle_draw},
    .obj_special = paddle_tick
   };
      
      
struct bounds paddle_pos = {.crds = {.x = (DISPLAY_WIDTH - PADDLE_WIDTH) / 2,
                                     .y = DISPLAY_HEIGHT - PADDLE_HEIGHT},
                            .ext = {.w = PADDLE_WIDTH, .h = PADDLE_HEIGHT}};

struct velocity paddle_vel = {.vx = 2, .vy = 0};

//////////////// BALL  ///////////////


static struct object g_obj_ball =
   {.obj_kind = OBJ_K_BOUNDED,
    .obj_un =
    {.obj_bnded =
     {.obj_bnds =
      {.crds =
       {.x = (DISPLAY_WIDTH - BALL_WIDTH) / 2,
        .y = DISPLAY_HEIGHT - PADDLE_HEIGHT - BALL_HEIGHT - 5
       },
       .ext =
       {.w = BALL_WIDTH,
        .h = BALL_HEIGHT
       }
      },
      .obj_vel =
      {.vx = 1,
       .vy = 1
      }
     }
    },
    .obj_update = {{0}},
    .obj_graphics = {.draw = ball_draw},
    .obj_special = ball_special
   };

struct bounds ball_pos = {.crds = {.x = 60, .y = 28},
                          .ext = {.w = BALL_WIDTH, .h = BALL_HEIGHT}};
struct velocity ball_vel = {.vx = 1, .vy = 1}; // in pixels/sec

