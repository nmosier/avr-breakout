/* objects.h
 */

#ifndef __OBJECTS_H
#define __OBJECTS_H

#include <stdint.h>
#include "SSD1306.h"
//#include "canvas.h"
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
void objpool_init(struct object_pool *objpool);
void objpool_move(struct object_pool *objpool);
void objpool_update(struct object_pool *objpool);
void objpool_interact(struct object_pool *objpool);
void objpool_special(struct object_pool *objpool);

//////////// SCREEN ///////////
extern struct bounds screen_bnds;

/////////// GRID ////////////////
#define GRID_BLOCK_WIDTH  8
#define GRID_BLOCK_HEIGHT 8
#define GRID_BLOCKS_PER_BYTE 8
#define GRID_WIDTH (DISPLAY_WIDTH / GRID_BLOCK_WIDTH)
#define GRID_HEIGHT (DISPLAY_HEIGHT / GRID_BLOCK_HEIGHT)
#define GRID_WIDTH_BYTES GRID_WIDTH
#define GRID_HEIGHT_BYTES (GRID_HEIGHT / GRID_BLOCKS_PER_BYTE)

extern uint8_t grid[GRID_HEIGHT_BYTES][GRID_WIDTH_BYTES];

////////// PADDLE ////////////////
#define PADDLE_WIDTH 16
#define PADDLE_HEIGHT 4
#define PADDLE_ROW   (DISPLAY_HEIGHT - PADDLE_HEIGHT)

extern struct bounds paddle_pos;
extern struct velocity paddle_vel;

//////////// BALL /////////////////
#define BALL_WIDTH  4
#define BALL_HEIGHT 4

extern struct bounds ball_pos;
extern struct velocity ball_vel; // in pixels/sec

#endif
