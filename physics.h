/* physics.h
 */

#ifndef __PHYSICS_H
#define __PHYSICS_H

void phys_flip_velocity(uint8_t flags, struct velocity *vel);

void phys_ball_freebounce(struct bounds *ball_pos,
                          struct velocity *ball_vel,
                          struct bounds *update);
uint8_t phys_grid_deflect(const struct bounds *bnds, struct velocity *vel,
                          struct bounds *update);
void phys_object_freemove(struct bounds *obj, const struct velocity *vel,
                      struct bounds *update);
uint8_t phys_adjust_velocity(uint8_t touch, struct velocity *vel);

#endif
