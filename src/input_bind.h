#ifndef SKIFREE_INPUT_BIND_H
#define SKIFREE_INPUT_BIND_H

#include <SDL.h>

typedef struct {
    SDL_Keycode key_left, key_right, key_up, key_down;
    SDL_Keycode key_jump, key_turbo, key_pause, key_reset;
    int pad_jump, pad_turbo, pad_pause, pad_reset;
} InputBindings;

void input_bind_init(void);
void input_bind_load(void);
void input_bind_save(void);
InputBindings *input_bind_get(void);

#endif
