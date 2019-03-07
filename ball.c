/* ball.c
 * Functions for controlling the ball.
 */

#include <stdint.h>
#include "SSD1306.h"
#include "objects.h"
#include "util.h"
#include "canvas.h"
#include "ball.h"

void ball_draw(uint8_t *buf, const struct bounds *bnds) {
   canvas_fill_rectangle(buf, bnds, ball_pos.x, ball_pos.y,
                         BALL_WIDTH, BALL_HEIGHT);
}
