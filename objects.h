/* objects.h
 */

#ifndef __OBJECTS_H
#define __OBJECTS_H

#include <stdint.h>
#include <stdarg.h>
#include "SSD1306.h"
#include "util.h"

//////////// OBJECT ////////////

// note: bounds should always be multiples of 8
struct object;
struct graphics_layer {
   void (*draw)(uint8_t *buf, const struct object *obj, const struct bounds *bnds);
   // future: perhaps add bounds? 
};

struct object {
   enum object_kind {OBJ_K_BOUNDED, OBJ_K_GRID} obj_kind;
   union {
      struct {
         struct bounds obj_bnds;
         struct velocity obj_vel;
         uint8_t obj_velupdate;
      } obj_bnded;
      struct {
      } obj_grid;
   } obj_un;
   struct bounds obj_update;
   struct graphics_layer obj_graphics;
   void (*obj_special)(struct object *obj);
};

#define OBJ_POOL_MAXSIZE 16
struct object_pool {
   struct object arr[OBJ_POOL_MAXSIZE];
   uint8_t cnt;
};
extern struct object_pool g_objpool;
typedef void (*objpool_map_func_t)(struct object *obj);
void objpool_interact(struct object_pool *objpool);
void objpool_map(struct object_pool *objpool, objpool_map_func_t func);
void object_move(struct object *obj);
void object_update(struct object *obj);
void object_special(struct object *obj);

inline void objpool_update(struct object_pool *objpool);
inline void objpool_move(struct object_pool *objpool);
inline void objpool_special(struct object_pool *objpool);

inline void objpool_update(struct object_pool *objpool) {
   objpool_map(objpool, object_update);
}

inline void objpool_move(struct object_pool *objpool) {
   objpool_map(objpool, object_move);
}

inline void objpool_special(struct object_pool *objpool) {
   objpool_map(objpool, object_special);
}


//////////// SCREEN ///////////
extern struct bounds screen_bnds;

#define OBJ_SCREEN_DEF(width, height)       \
   {.obj_kind = OBJ_K_BOUNDED,              \
    .obj_un =                               \
    {.obj_bnded =                           \
     {.obj_bnds =                           \
      {.crds = {.x = 0, .y = 0},            \
       .ext = {.w = width, .h = height}     \
      },                                    \
      .obj_vel = {.vx = 0, .vy = 0},        \
     }                                      \
    },                                      \
    .obj_update = {{0}},                    \
    .obj_graphics = {.draw = NULL},         \
    .obj_special = NULL                     \
   }


/////////// GRID ////////////////
#define GRID_BLOCK_WIDTH  8
#define GRID_BLOCK_HEIGHT 8
#define GRID_BLOCKS_PER_BYTE 8
#define GRID_WIDTH (DISPLAY_WIDTH / GRID_BLOCK_WIDTH)
#define GRID_HEIGHT (DISPLAY_HEIGHT / GRID_BLOCK_HEIGHT)
#define GRID_WIDTH_BYTES GRID_WIDTH
#define GRID_HEIGHT_BYTES (GRID_HEIGHT / GRID_BLOCKS_PER_BYTE)

extern uint8_t grid[GRID_HEIGHT_BYTES][GRID_WIDTH_BYTES];

#define OBJ_GRID_DEF                             \
   {.obj_kind = OBJ_K_GRID,                      \
    .obj_update = {{0}},                         \
    .obj_graphics = {.draw = grid_display_layer},\
    .obj_special = NULL                          \
   }

////////// PADDLE ////////////////
#define PADDLE_WIDTH 16
#define PADDLE_HEIGHT 4
#define PADDLE_ROW   (DISPLAY_HEIGHT - PADDLE_HEIGHT)
#define PADDLE_COL   ((DISPLAY_WIDTH - PADDLE_WIDTH) / 2)
#define PADDLE_VX  1
#define PADDLE_VY  0

extern struct bounds paddle_pos;
extern struct velocity paddle_vel;

#define OBJ_PADDLE_DEF(x_, y_, w_, h_, vx_, vy_)    \
   {.obj_kind = OBJ_K_BOUNDED,            \
    .obj_un =                             \
    {.obj_bnded =                         \
     {.obj_bnds =                         \
      {.crds =                            \
       {.x = x_,                           \
        .y = y_                            \
       },                                 \
       .ext =                             \
       {.w = w_,                           \
        .h = h_                            \
       }                                  \
      },                                  \
      .obj_vel =                          \
      {.vx = vx_,                           \
       .vy = vy_                            \
      }                                   \
     }                                    \
    },                                    \
    .obj_update = {{0}},                  \
    .obj_graphics = {.draw = paddle_draw},\
    .obj_special = paddle_tick            \
   }

//////////// BALL /////////////////
#define BALL_WIDTH  4
#define BALL_HEIGHT 4
#define BALL_COL    ((DISPLAY_WIDTH - BALL_WIDTH) / 2)
#define BALL_ROW    ((DISPLAY_HEIGHT - BALL_HEIGHT) / 2)
#define BALL_VX     1
#define BALL_VY     1

extern struct bounds ball_pos;
extern struct velocity ball_vel; // in pixels/sec

#define OBJ_BALL_DEF(x_, y_, w_, h_, vx_, vy_)        \
   {.obj_kind = OBJ_K_BOUNDED,                  \
      .obj_un =                                 \
      {.obj_bnded =                             \
      {.obj_bnds =                              \
         {.crds =                               \
         {.x = x_,                               \
          .y = y_                                \
         },                                     \
      .ext =                                    \
            {.w = w_,                            \
      .h = h_                                    \
      }                                         \
       },                                       \
      .obj_vel =                                \
         {.vx = vx_,                             \
          .vy = vy_                             \
         }                                      \
          }                                     \
           },                                   \
    .obj_update = {{0}},                        \
        .obj_graphics = {.draw = ball_draw},    \
        .obj_special = ball_special             \
        }                                      

#endif
