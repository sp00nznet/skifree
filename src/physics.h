/*
 * SkiFree Enhanced Physics
 * Optional physics enhancements gated behind classic_mode config.
 * Wind, ice patches, smooth acceleration, momentum conservation.
 */

#ifndef SKIFREE_PHYSICS_H
#define SKIFREE_PHYSICS_H

#include "types.h"

/* Apply wind force to an actor's position delta. */
void physics_apply_wind(short *dx, short *dy);

/* Smooth acceleration: interpolate toward target instead of snapping. */
short physics_smooth_accel(short current, short target, short rate);

/* Apply ice friction modifier to velocity. Returns modified velocity. */
short physics_apply_ice(short velocity, short sprite_idx);

/* Apply momentum conservation on collision.
 * Returns modified velocity (partial retention instead of zeroing). */
short physics_collision_momentum(short velocity, float retention);

/* Check if a sprite index represents an ice tile. */
int physics_is_ice_tile(short sprite_idx);

#endif /* SKIFREE_PHYSICS_H */
