/*
 * SkiFree Enhanced Physics
 * All functions are no-ops when classic_mode is enabled.
 */

#include "physics.h"
#include "config.h"
#include <stdlib.h>

void physics_apply_wind(short *dx, short *dy) {
    if (!config_physics_enhanced()) return;

    *dx += (short)config_get_int("physics", "wind_x", 0);
    *dy += (short)config_get_int("physics", "wind_y", 0);
}

short physics_smooth_accel(short current, short target, short rate) {
    short diff;

    if (!config_physics_enhanced()) return target;

    diff = target - current;
    if (abs(diff) <= rate) return target;

    return (diff > 0) ? current + rate : current - rate;
}

short physics_apply_ice(short velocity, short sprite_idx) {
    if (!config_physics_enhanced()) return velocity;
    if (!physics_is_ice_tile(sprite_idx)) return velocity;

    /* Ice: velocity decays slower (multiply by 15/16 instead of normal decay) */
    return (short)((velocity * 15) / 16);
}

short physics_collision_momentum(short velocity, float retention) {
    if (!config_physics_enhanced()) return 0;

    /* Retain a fraction of velocity on collision instead of zeroing */
    return (short)(velocity * retention);
}

int physics_is_ice_tile(short sprite_idx) {
    /* Mogul tiles (0x1b = 27) repurposed as ice in enhanced mode */
    /* Can be extended with custom ice sprites via mods */
    return (sprite_idx == 27);
}
