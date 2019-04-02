/* paddle.h
 */

#ifndef __PADDLE_H
#define __PADDLE_H

void paddle_draw(uint8_t *buf, const struct object *paddle, const struct bounds *bnds);
void paddle_tick(struct object *paddle);
#endif
