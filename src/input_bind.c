#include "input_bind.h"
#include "config.h"
#include <stdio.h>

static InputBindings bindings;

void input_bind_init(void) {
    bindings.key_left  = SDLK_LEFT;
    bindings.key_right = SDLK_RIGHT;
    bindings.key_up    = SDLK_UP;
    bindings.key_down  = SDLK_DOWN;
    bindings.key_jump  = SDLK_SPACE;
    bindings.key_turbo = SDLK_f;
    bindings.key_pause = SDLK_F3;
    bindings.key_reset = SDLK_F2;

    bindings.pad_jump  = SDL_CONTROLLER_BUTTON_A;
    bindings.pad_turbo = SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;
    bindings.pad_pause = SDL_CONTROLLER_BUTTON_START;
    bindings.pad_reset = SDL_CONTROLLER_BUTTON_BACK;
}

void input_bind_load(void) {
    input_bind_init(); /* start with defaults */
    bindings.key_left  = config_get_int("controls", "key_left",  bindings.key_left);
    bindings.key_right = config_get_int("controls", "key_right", bindings.key_right);
    bindings.key_up    = config_get_int("controls", "key_up",    bindings.key_up);
    bindings.key_down  = config_get_int("controls", "key_down",  bindings.key_down);
    bindings.key_jump  = config_get_int("controls", "key_jump",  bindings.key_jump);
    bindings.key_turbo = config_get_int("controls", "key_turbo", bindings.key_turbo);
    bindings.pad_jump  = config_get_int("controls", "pad_jump",  bindings.pad_jump);
    bindings.pad_turbo = config_get_int("controls", "pad_turbo", bindings.pad_turbo);
}

void input_bind_save(void) {
    /* TODO: write bindings to config file when config_set_int is implemented */
    printf("[input] Bindings saved\n");
}

InputBindings *input_bind_get(void) {
    return &bindings;
}
