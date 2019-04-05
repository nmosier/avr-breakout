/* canvas.c
 * Functions & sprites related to drawing on grid.
 */

#include <stdint.h>
#include <string.h>
#include "canvas.h"
#include "objects.h"
#include "spi.h"
#include "SSD1306.h"
#include "paddle.h"
#include "ball.h"
#include "util.h"

const uint8_t block_brick[8] = {0b11111111,
                                0b01000100,
                                0b11111111,
                                0b00100010,
                                0b11111111,
                                0b01000100,
                                0b11111111,
                                0b00100010};

const uint8_t block_empty[8] = {0};

const struct projection g_proj_pix2grid =
   {.sx = GRID_BLOCK_WIDTH,
    .sy = GRID_BLOCK_HEIGHT,
   };

inline const uint8_t *grid_getsprite(uint8_t row, uint8_t col) {
   return grid_testblock(row, col) ? block_brick : block_empty;
}

void grid_displayblock(uint8_t row, uint8_t col) {
   const uint8_t *sprite;

   sprite = grid_getsprite(row, col);
   
   display_select(row, col * 8, 1, 8); // just select one block

   /* display block sprite */
   SSD1306_DATA;
   spi_write(sprite, 8);
}

/* grid_display_layer: OR underlying grid to buffer.
 * NOTE: object is passed to conform to standard graphics layer interface, but is not used.
 * TODO: handle buffers that aren't aligned with grid (in x-axis).
 */
void grid_display_layer(uint8_t *buf, const struct object *obj, const struct bounds *bnds) {
   uint8_t col_begin, col_end, row_begin, row_end;

   col_begin = bnds->crds.x;
   col_end = col_begin + bnds->ext.w;
   row_begin = bnds->crds.y;
   row_end = row_begin + bnds->ext.h;
   for (uint8_t row_it = row_begin; row_it < row_end; ++row_it) {
      for (uint8_t col = col_begin; col < col_end;) {
         const uint8_t *sprite = grid_getsprite(row_it, col / 8) + (col % 8);
         uint8_t cnt = umin8(8 - (col % 8), col_end - col);
         col += cnt;
         for (; cnt > 0; --cnt) {
            *(buf++) |= *(sprite++);
         }
         buf += cnt;
      }
   }
}


//////// SCREEN BUFFER /////////

/* TODO: merge graphics layers into object struct. */
#if 0
struct graphics_layer graphics_layers[] =
   {
    {grid_display_layer},
    {paddle_draw},
    {ball_draw},
   };

/* canvas_getbuffer: copies would-be screen buffer for given region
 *                   into provided pointer
 *   *** unifies drawing of all objects ***
 * NOTE: will be stored in the same format as in the screen's internal buffer.
 * NOTE: needs to keep track of everything that can appear in the given area.
 *       Necessary because we can't use a dedicated screen buffer.
 * TODO: inline.
 */
void canvas_getbuffer(uint8_t *buf, const struct bounds *bnds) {
   for (struct graphics_layer *layer = graphics_layers;
        layer < END(graphics_layers); ++layer) {
      layer->draw(buf, bnds);
   }
}
#else
void canvas_getbuffer(uint8_t *buf, const struct bounds *bnds) {
   const struct object *obj_begin = g_objpool.arr,
                       *obj_end = obj_begin + g_objpool.cnt;
   for (const struct object *obj_it = obj_begin; obj_it != obj_end; ++obj_it) {
      if (obj_it->obj_graphics.draw) {
         obj_it->obj_graphics.draw(buf, obj_it, bnds);
      }
   }
}
#endif

/* note: end is one past last pixel drawn */
void canvas_draw_vertical(uint8_t *buf, const struct bounds *bnds,
                          uint8_t col, uint8_t begin, uint8_t end) {
   uint8_t onscreen_begin, onscreen_end;
   uint8_t page_begin;
   uint8_t stride, mask, offset;
   uint8_t *buf_it;

   /* check bounds */
   if (col < bnds->crds.x || col >= bnds->crds.x + bnds->ext.w) {
      return; // not within window
   }
   
   onscreen_begin = umax8(begin, bnds->crds.y * 8);
   onscreen_end = umin8(end, (bnds->crds.y + bnds->ext.h) * 8);
   page_begin = onscreen_begin / 8;
   stride = bnds->ext.w;
   buf_it = buf + (page_begin - bnds->crds.y) * stride + col - bnds->crds.x;


   while (onscreen_begin < onscreen_end) {
      mask = 0xff;
      offset = onscreen_begin & ~SSD1306_PAGE_MASK;
      mask <<= offset;
      
      /* check if line contained w/i one page */
      if ((onscreen_begin & SSD1306_PAGE_MASK) ==
          (onscreen_end & SSD1306_PAGE_MASK)) {
         mask &= ~(0xff << (onscreen_end & ~SSD1306_PAGE_MASK));
         onscreen_begin = onscreen_end;
      } else {
         onscreen_begin += 8 - offset;
      }

      /* update buffer */
      *buf_it |= mask;
      buf_it += stride;
   }
}

void canvas_fill_rectangle_old(uint8_t *buf, const struct bounds *bnds,
                           uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
   for (uint8_t dx = 0; dx < w; ++dx) {
      canvas_draw_vertical(buf, bnds, x + dx, y, y + h);
   }
}

void canvas_fill_rectangle(uint8_t *buf, const struct bounds *canvas_bnds,
                           const struct bounds *obj_bnds) {
   for (uint8_t dx = 0; dx < obj_bnds->ext.w; ++dx) {
      canvas_draw_vertical(buf, canvas_bnds, obj_bnds->crds.x + dx, obj_bnds->crds.y,
                           obj_bnds->crds.y + obj_bnds->ext.h);
   }
}

void bounds_create(uint8_t x, uint8_t y, uint8_t width, uint8_t height,
                   struct bounds *bnds) {
   /* set col & width */
   bnds->crds.x = x;
   bnds->ext.w = width;

   /* set row & height */
   height += y & ~SSD1306_PAGE_MASK;
   bnds->crds.y = SSD1306_PAGE_ROUND_DOWN(y);
   bnds->ext.h = SSD1306_PAGE_ROUND_UP(height);
}


// display should be selected
#define CANVAS_DISPLAY_FULL_STEP 8
void canvas_display_full() {
   struct bounds bnds_full =
      {.crds = {.x = 0, .y = 0},
       .ext = {.w = DISPLAY_WIDTH,
               .h = 8}
      };
   struct bounds bnds_block =
      {.ext = {.w = CANVAS_DISPLAY_FULL_STEP, .h = 1}};
   uint8_t buf[CANVAS_DISPLAY_FULL_STEP];
   
   display_selectbnds(&bnds_full);
   
   /* render 8x8 pixel chunks at a time */
   SSD1306_DATA;
   for (uint8_t page = 0; page < 8; ++page) {
      bnds_block.crds.y = page;
      for (uint8_t col = 0; col < DISPLAY_WIDTH;
           col += CANVAS_DISPLAY_FULL_STEP) {
         bnds_block.crds.x = col;

         memset(buf, 0, CANVAS_DISPLAY_FULL_STEP);
         canvas_getbuffer(buf, &bnds_block);
         spi_write(buf, CANVAS_DISPLAY_FULL_STEP);
      }
   }
}

/* NOTE: modifies blist. */
void canvas_display_updates(struct bounds_list **blist) {
   for (struct bounds_list *it = *blist; it; it = it->next) {
      bounds_downsize(&it->bnds);

      uint8_t bufsize = bounds_area(&it->bnds);
      uint8_t buf[bufsize];
      memset(buf, 0, bufsize);
      
      /* 1. select bounds
       * 2. get buffer for bounds
       * 3. write buffer to screen 
       */
      display_selectbnds(&it->bnds);
      canvas_getbuffer(buf, &it->bnds);
      SSD1306_DATA;
      spi_write(buf, bufsize);
   }
}


void display_update(struct bounds *update_pix) {
   /* convert pixel update bounds to screen coordinates */
   struct bounds update_scrn;
   project_down(update_pix, &update_scrn, &g_proj_pix2scrn, PROJ_MODE_FUZZY);

   display_selectbnds(&update_scrn);

   /* draw update buffer */
   uint8_t bufsize = bounds_area(&update_scrn);
   uint8_t buf[bufsize];

   memset(buf, 0, bufsize);
   canvas_getbuffer(buf, &update_scrn);
   
   SSD1306_DATA;
   spi_write(buf, bufsize);
}
