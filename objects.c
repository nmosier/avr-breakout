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

//////////////// GRID ////////////////
/* currently, only one byte per column */
uint8_t grid[GRID_HEIGHT_BYTES][GRID_WIDTH_BYTES] =
   {{
     0b11111111,
     0b01111111,
     0b00111111,
     0b00011111,
     0b00001111,
     0b00000111,
     0b00000011,
     0b00000001,
     }};

/////////////// PADDLE ///////////////
/* location represented by left display column of paddle */
//uint8_t paddle_col = (DISPLAY_WIDTH - PADDLE_WIDTH) / 2;
uint8_t paddle_col = 10;
struct bounds paddle_pos = {.crds = {.x = 10,
                                     .y = DISPLAY_HEIGHT - PADDLE_HEIGHT},
                            .ext = {.w = PADDLE_WIDTH, .h = PADDLE_HEIGHT}};

//////////////// BALL  ///////////////
/* note: currently only supports one ball
 * note: y-coord is pixels, NOT pages
 */
struct coords ball_pos = {.x = 64, .y = 32}; // in pixels
struct coords ball_vel = {.x = 1, .y = 1}; // in pixels/sec
