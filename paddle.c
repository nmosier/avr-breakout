/* paddle.c
 * Functions to manipulate player's paddle.
 */

#include <stdint.h>
#include <string.h>
#include "SSD1306.h"
#include "objects.h"
#include "canvas.h"
#include "paddle.h"
#include "physics.h"
#include "util.h"
#include "button.h"

#define PADDLE_MASK 0xf0

#if 0
void paddle_draw(uint8_t *buf, const struct object *paddle, const struct bounds *bnds) {
   uint8_t paddle_page;
   uint8_t buf_page;
   const struct bounds *paddle_pos = &paddle->obj_un.obj_bnded.obj_bnds;

   /* note: paddle will always be fully within one page, so don't need to worry
    * about multipage drawing */
   paddle_page = display_row2page(PADDLE_ROW);
   buf_page = bnds->crds.y; // y is in pages
   if (paddle_page >= buf_page && paddle_page < buf_page + bnds->ext.h
       && paddle_pos->crds.x + paddle_pos->ext.w > bnds->crds.x
       && paddle_pos->crds.x < bnds->crds.x + bnds->ext.w) {
      uint8_t onscreen_width, onscreen_left, onscreen_offset;
      
      /* find onscreen width of paddle */
      onscreen_left = umax8(paddle_pos->crds.x, bnds->crds.x);
      onscreen_width = umin8(paddle_pos->crds.x + paddle_pos->ext.w,
                             bnds->crds.x + bnds->ext.w) - onscreen_left;
      onscreen_offset = onscreen_left - bnds->crds.x;
      
      /* get pointer to beginning of paddle in buffer */
      buf += (paddle_page - buf_page) * bnds->ext.w + onscreen_offset;

      /* draw paddle to buffer */
      for (uint8_t i = 0; i < onscreen_width; ++i, ++buf) {
         *buf |= PADDLE_MASK;
      }
   }
}

#else

void paddle_draw(uint8_t *buf, const struct object *paddle, const struct bounds *bnds) {
   canvas_fill_rectangle(buf, bnds, &paddle->obj_un.obj_bnded.obj_bnds);
}

#endif

void paddle_tick(struct object *paddle) {
   if (button_get_press()) {
      phys_flip_velocity(VEL_X, &paddle->obj_un.obj_bnded.obj_vel);
   }
}
