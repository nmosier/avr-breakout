/* canvas.h
 * Nicholas Mosier 2019
 */

#ifndef __CANVAS_H
#define __CANVAS_H

#include "SSD1306.h"
#include "objects.h"
#include "util.h"

// note: bounds should always be multiples of 8
struct graphics_layer {
   void (*draw)(uint8_t *buf, const struct bounds *bnds);
   // future: perhaps add bounds? 
};


inline void grid_setblock(uint8_t row, uint8_t col) {
   grid[row / 8][col] |= 1 << (row % 8);
}

inline void grid_clearblock(uint8_t row, uint8_t col) {
   grid[row / 8][col] &= ~(1 << (row % 8));
}

inline uint8_t grid_testblock(uint8_t row, uint8_t col) {
   return (grid[row / 8][col] & (1 << (row % 8)));
}

void grid_displayblock(uint8_t row, uint8_t col);
void grid_display_full();

void canvas_getbuffer(uint8_t *buf, const struct bounds *bnds);
void canvas_draw_vertical(uint8_t *buf, const struct bounds *bnds,
                          uint8_t col, uint8_t begin, uint8_t end);
void canvas_fill_rectangle(uint8_t *buf, const struct bounds *bnds,
                           uint8_t x, uint8_t y, uint8_t w, uint8_t h);

void bounds_create(uint8_t x, uint8_t y, uint8_t width, uint8_t height,
                   struct bounds *bnds);
void canvas_display_full();


#endif
