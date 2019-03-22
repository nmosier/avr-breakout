/* paddle.h
 */

#ifndef __PADDLE_H
#define __PADDLE_H

void paddle_draw(uint8_t *buf, const struct bounds *bnds);
void paddle_tick(struct bounds *paddle_bnds, struct velocity *paddle_vel,
                 struct bounds *update);

#endif
