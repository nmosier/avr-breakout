/* physics.h
 */

#ifndef __PHYSICS_H
#define __PHYSICS_H

void phys_ball_freebounce(struct bounds *ball_pos,
                          struct velocity *ball_vel,
                          struct bounds *update);

void phys_grid_deflect(const struct bounds *bnds, struct velocity *vel);
void phys_adjust_velocity(uint8_t touch, struct velocity *vel);

#endif
