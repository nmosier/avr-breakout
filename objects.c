/* Objects.c
 * Global definitions of shared game objects:
 *  - Game grid
 *  - Paddle
 *  - Ball(s)
 */


#include <stdint.h>
#include "canvas.h"
#include "SSD1306.h"
#include "objects.h"
#include "util.h"

/////////////// SCREEN ///////////////
struct bounds screen_bnds = {.crds = {.x = 0,
                                      .y = 0},
                             .ext = {.w = DISPLAY_WIDTH,
                                     .h = DISPLAY_HEIGHT}};
                                      

//////////////// GRID ////////////////
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
/* location represented by left display column of paddle */
//uint8_t paddle_col = (DISPLAY_WIDTH - PADDLE_WIDTH) / 2;
//uint8_t paddle_col = 10;
struct bounds paddle_pos = {.crds = {.x = 10,
                                     .y = DISPLAY_HEIGHT - PADDLE_HEIGHT},
                            .ext = {.w = PADDLE_WIDTH, .h = PADDLE_HEIGHT}};

//////////////// BALL  ///////////////
/* note: currently only supports one ball
 * note: y-coord is pixels, NOT pages
 */
struct bounds ball_pos = {.crds = {.x = 60, .y = 28},
                          .ext = {.w = BALL_WIDTH, .h = BALL_HEIGHT}};
struct velocity ball_vel = {.vx = 1, .vy = 1}; // in pixels/sec
