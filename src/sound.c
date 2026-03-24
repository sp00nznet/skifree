/*
 * SkiFree Sound System
 * SDL_mixer-based sound effects.
 *
 * The original SkiFree used sndPlaySound which only supports one sound
 * at a time. SDL_mixer gives us proper multi-channel mixing.
 * In classic mode, we allocate only 1 channel to match original behavior.
 */

#include "sound.h"
#include "config.h"
#include "resources.h"
#include <stdio.h>
#include <string.h>

static int sound_initialized = 0;

int sound_init(void) {
    int channels;

    if (sound_initialized) return 1;

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("[sound] Mix_OpenAudio failed: %s\n", Mix_GetError());
        return 0;
    }

    /* Classic mode: single channel like original sndPlaySound */
    channels = config_classic_mode() ? 1 : 8;
    Mix_AllocateChannels(channels);
    Mix_Volume(-1, config_sound_volume());

    sound_initialized = 1;
    printf("[sound] Initialized (%d channels)\n", channels);
    return 1;
}

void sound_shutdown(void) {
    if (!sound_initialized) return;
    Mix_CloseAudio();
    sound_initialized = 0;
}

int sound_load(SoundEffect *snd, const char *name) {
    Resource *res;
    SDL_RWops *rw;

    snd->chunk = NULL;
    snd->name = name;

    res = resource_load(name);
    if (!res) {
        /* Not an error — sounds are optional */
        return 0;
    }

    rw = SDL_RWFromConstMem(res->content, res->len);
    if (!rw) {
        resource_free(res);
        return 0;
    }

    snd->chunk = Mix_LoadWAV_RW(rw, 1);
    resource_free(res);

    if (!snd->chunk) {
        printf("[sound] Failed to load %s: %s\n", name, Mix_GetError());
        return 0;
    }

    printf("[sound] Loaded: %s\n", name);
    return 1;
}

void sound_play(SoundEffect *snd) {
    if (!sound_initialized || !snd || !snd->chunk) return;
    Mix_PlayChannel(-1, snd->chunk, 0);
}

void sound_free(SoundEffect *snd) {
    if (snd && snd->chunk) {
        Mix_FreeChunk(snd->chunk);
        snd->chunk = NULL;
    }
}

void sound_set_volume(int vol) {
    if (vol < 0) vol = 0;
    if (vol > 128) vol = 128;
    Mix_Volume(-1, vol);
}

void sound_set_classic_mode(int enabled) {
    Mix_AllocateChannels(enabled ? 1 : 8);
}
