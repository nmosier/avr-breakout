/* ball.c
 * Functions for controlling the ball.
 */

#include <stdint.h>
#include "SSD1306.h"
#include "objects.h"
#include "util.h"
#include "canvas.h"
#include "ball.h"

void ball_draw(uint8_t *buf, const struct object *ball, const struct bounds *bnds) {
   const struct bounds *ball_pos = &ball->obj_un.obj_bnded.obj_bnds;
   canvas_fill_rectangle(buf, bnds, ball_pos->crds.x, ball_pos->crds.y,
                         BALL_WIDTH, BALL_HEIGHT);
}
