/* objects.c
 * Global definitions of shared game objects:
 *  - Game grid
 *  - Paddle
 *  - Ball(s)
 */


#include <stdint.h>
#include <string.h>
#include "canvas.h"
#include "SSD1306.h"
#include "objects.h"
#include "physics.h"
#include "util.h"
#include "ball.h"
#include "paddle.h"

static struct object g_obj_screen;
static struct object g_obj_grid;
static struct object g_obj_paddle;
static struct object g_obj_ball;

static void objpool_interact_pair(struct object *obj1, struct object *obj2);
static void object_move(struct object *obj);
static void object_update(struct object *obj);

//////////// OBJECTS ////////////
/* object pool */
struct object_pool g_objpool;
void objpool_init(struct object_pool *objpool) {
   objpool->arr[0] = g_obj_screen;
   // objpool->arr[2] = g_obj_grid;
   objpool->arr[2] = g_obj_paddle;
   objpool->arr[1] = g_obj_ball;

   objpool->cnt = 3;
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

/* interact a pair of objects */
static void objpool_interact_pair(struct object *obj1, struct object *obj2) {
   uint8_t obj1_k = obj1->obj_kind,
           obj2_k = obj2->obj_kind;
   touch_t touch;

   /* detect collision & collide */
   if (obj1_k == OBJ_K_BOUNDED && obj2_k == OBJ_K_BOUNDED) {
      touch = obj_bnded_detect_collision(obj1, obj2);
      obj_bnded_collide(obj1, obj2, touch);
   } else {
      /* NOT YET IMPLEMENTED */
      touch = TOUCH_NONE;
   }
}

/* NOTE: works on ALL kinds of objects. */
static void object_move(struct object *obj) {
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

void objpool_move(struct object_pool *objpool) {
   struct object *obj_begin = objpool->arr,
                 *obj_end = obj_begin + objpool->cnt;
   for (struct object *obj_it = obj_begin; obj_it != obj_end; ++obj_it) {
      object_move(obj_it);
   }
}

/* object_update: display changes due to object on screen and reset update bounds. */
static void object_update(struct object *obj) {
   display_update(&obj->obj_update);
   memset(&obj->obj_update, 0, sizeof(obj->obj_update));
}

void objpool_update(struct object_pool *objpool) {
   struct object *obj_begin = objpool->arr,
                 *obj_end = obj_begin + objpool->cnt;
   for (struct object *obj_it = obj_begin; obj_it != obj_end; ++obj_it) {
      object_update(obj_it);
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
    .obj_graphics = {.draw = NULL}
   };

struct bounds screen_bnds = {.crds = {.x = 0,
                                      .y = 0},
                             .ext = {.w = DISPLAY_WIDTH,
                                     .h = DISPLAY_HEIGHT}};
                                      

//////////////// GRID ////////////////

static struct object g_obj_grid =
   {.obj_kind = OBJ_K_GRID,
    .obj_update = {{0}}
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
    .obj_graphics = {.draw = paddle_draw}
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
        .y = DISPLAY_HEIGHT - PADDLE_HEIGHT - BALL_HEIGHT - 1
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
    .obj_graphics = {.draw = ball_draw}
   };

struct bounds ball_pos = {.crds = {.x = 60, .y = 28},
                          .ext = {.w = BALL_WIDTH, .h = BALL_HEIGHT}};
struct velocity ball_vel = {.vx = 1, .vy = 1}; // in pixels/sec

