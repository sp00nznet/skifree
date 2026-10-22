/*
 * SkiFree Sound System
 * Multi-channel sound effects on SDL's own audio (sound.c).
 */

#ifndef SKIFREE_SOUND_H
#define SKIFREE_SOUND_H

#include <SDL.h>

typedef struct {
    Sint16 *samples;   /* converted to the device format: S16 stereo 44.1 kHz */
    int len;           /* in Sint16s */
    const char *name;  /* resource name for mod loading */
} SoundEffect;

/* Initialize the sound system. Returns 1 on success, 0 on failure. */
int sound_init(void);

/* Shut down the sound system. */
void sound_shutdown(void);

/* Load a sound effect from the resource system.
 * name is like "sounds/ouch.wav". Returns 1 on success. */
int sound_load(SoundEffect *snd, const char *name);

/* Play a sound effect. */
void sound_play(SoundEffect *snd);

/* Free a loaded sound effect. */
void sound_free(SoundEffect *snd);

/* Set master volume (0-128). */
void sound_set_volume(int vol);

/* Set classic mode (single channel, like original sndPlaySound). */
void sound_set_classic_mode(int enabled);

#endif /* SKIFREE_SOUND_H */
