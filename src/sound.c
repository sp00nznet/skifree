/*
 * SkiFree Sound System
 * A small mixer on SDL's own audio, so no SDL_mixer: it runs anywhere SDL2 does.
 *
 * The original SkiFree used sndPlaySound, one sound at a time. This mixes up to
 * 8 at once; classic mode keeps it to 1, like the original.
 */

#include "sound.h"
#include "config.h"
#include "resources.h"
#include <stdio.h>
#include <string.h>

#define MAX_VOICES 8
#define RATE 44100

typedef struct { const Sint16 *data; int len, pos; } Voice;   /* len, pos in samples (stereo pairs * 2) */

static int sound_initialized = 0;
static SDL_AudioDeviceID dev = 0;
static Voice voices[MAX_VOICES];
static int num_voices = MAX_VOICES;
static int volume = 128;

static void mix(void *userdata, Uint8 *stream, int bytes) {
    Sint16 *out = (Sint16 *)stream;
    int n = bytes / 2, i, v;
    (void)userdata;
    for (i = 0; i < n; i++) {
        int s = 0;
        for (v = 0; v < num_voices; v++) {
            Voice *vc = &voices[v];
            if (vc->data && vc->pos < vc->len) s += vc->data[vc->pos++];
        }
        s = s * volume / 128;
        out[i] = (Sint16)(s > 32767 ? 32767 : s < -32768 ? -32768 : s);
    }
}

int sound_init(void) {
    SDL_AudioSpec want, have;
    if (sound_initialized) return 1;
    SDL_zero(want);
    want.freq = RATE;
    want.format = AUDIO_S16SYS;
    want.channels = 2;
    want.samples = 2048;
    want.callback = mix;
    dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (!dev) {
        printf("[sound] SDL_OpenAudioDevice failed: %s\n", SDL_GetError());
        return 0;
    }
    num_voices = config_classic_mode() ? 1 : MAX_VOICES;
    volume = config_sound_volume();
    SDL_PauseAudioDevice(dev, 0);
    sound_initialized = 1;
    printf("[sound] Initialized (%d channels)\n", num_voices);
    return 1;
}

void sound_shutdown(void) {
    if (!sound_initialized) return;
    SDL_CloseAudioDevice(dev);
    sound_initialized = 0;
}

int sound_load(SoundEffect *snd, const char *name) {
    Resource *res;
    SDL_AudioSpec spec;
    Uint8 *buf;
    Uint32 len;
    SDL_AudioCVT cvt;

    snd->samples = NULL;
    snd->len = 0;
    snd->name = name;
    res = resource_load(name);
    if (!res) return 0;   /* sounds are optional */
    if (!SDL_LoadWAV_RW(SDL_RWFromConstMem(res->content, res->len), 1, &spec, &buf, &len)) {
        printf("[sound] Failed to load %s: %s\n", name, SDL_GetError());
        resource_free(res);
        return 0;
    }
    resource_free(res);
    /* to the device's format once, here, so mixing is a plain add */
    if (SDL_BuildAudioCVT(&cvt, spec.format, spec.channels, spec.freq, AUDIO_S16SYS, 2, RATE) < 0) {
        SDL_FreeWAV(buf);
        return 0;
    }
    cvt.len = (int)len;
    cvt.buf = (Uint8 *)SDL_malloc(len * cvt.len_mult);
    if (!cvt.buf) { SDL_FreeWAV(buf); return 0; }
    SDL_memcpy(cvt.buf, buf, len);
    SDL_FreeWAV(buf);
    SDL_ConvertAudio(&cvt);
    snd->samples = (Sint16 *)cvt.buf;
    snd->len = cvt.len_cvt / 2;
    printf("[sound] Loaded: %s\n", name);
    return 1;
}

void sound_play(SoundEffect *snd) {
    int v, best = 0;
    if (!sound_initialized || !snd || !snd->samples) return;
    SDL_LockAudioDevice(dev);
    /* a free voice, else the one nearest its end */
    for (v = 0; v < num_voices; v++) {
        if (!voices[v].data || voices[v].pos >= voices[v].len) { best = v; break; }
        if (voices[v].len - voices[v].pos < voices[best].len - voices[best].pos) best = v;
    }
    voices[best].data = snd->samples;
    voices[best].len = snd->len;
    voices[best].pos = 0;
    SDL_UnlockAudioDevice(dev);
}

void sound_free(SoundEffect *snd) {
    int v;
    if (!snd || !snd->samples) return;
    if (sound_initialized) SDL_LockAudioDevice(dev);
    for (v = 0; v < MAX_VOICES; v++)
        if (voices[v].data == snd->samples) voices[v].data = NULL;
    if (sound_initialized) SDL_UnlockAudioDevice(dev);
    SDL_free(snd->samples);
    snd->samples = NULL;
}

void sound_set_volume(int vol) {
    volume = vol < 0 ? 0 : vol > 128 ? 128 : vol;
}

void sound_set_classic_mode(int enabled) {
    if (sound_initialized) SDL_LockAudioDevice(dev);
    num_voices = enabled ? 1 : MAX_VOICES;
    if (sound_initialized) SDL_UnlockAudioDevice(dev);
}
