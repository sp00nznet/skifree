/*
 * SkiFree Replay System
 * Records {tick, type, value} tuples for input events.
 * ~8 bytes per event, a 5-minute run at ~25 FPS with sparse inputs is under 100KB.
 */

#include "replay.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static ReplayState state = {0};

void replay_start_recording(void) {
    if (state.events) free(state.events);
    state.events = (ReplayEvent *)malloc(REPLAY_MAX_EVENTS * sizeof(ReplayEvent));
    state.count = 0;
    state.capacity = REPLAY_MAX_EVENTS;
    state.start_tick = SDL_GetTicks();
    state.recording = 1;
    state.playing = 0;
    printf("[replay] Recording started\n");
}

void replay_stop_recording(void) {
    state.recording = 0;
    printf("[replay] Recording stopped (%d events)\n", state.count);
}

void replay_record_event(ReplayInputType type, int32_t v1, int32_t v2) {
    if (!state.recording || !state.events) return;
    if (state.count >= state.capacity) return;

    state.events[state.count].tick = SDL_GetTicks() - state.start_tick;
    state.events[state.count].type = (uint8_t)type;
    state.events[state.count].value1 = v1;
    state.events[state.count].value2 = v2;
    state.count++;
}

int replay_save(const char *filepath) {
    FILE *f;
    uint32_t magic = 0x534B4952; /* "SKIR" */
    uint32_t version = 1;

    if (!state.events || state.count == 0) return 0;

    f = fopen(filepath, "wb");
    if (!f) return 0;

    fwrite(&magic, 4, 1, f);
    fwrite(&version, 4, 1, f);
    fwrite(&state.count, 4, 1, f);
    fwrite(state.events, sizeof(ReplayEvent), state.count, f);
    fclose(f);

    printf("[replay] Saved %d events to %s\n", state.count, filepath);
    return 1;
}

int replay_load(const char *filepath) {
    FILE *f;
    uint32_t magic, version;
    int count;

    f = fopen(filepath, "rb");
    if (!f) return 0;

    fread(&magic, 4, 1, f);
    fread(&version, 4, 1, f);
    if (magic != 0x534B4952 || version != 1) {
        fclose(f);
        return 0;
    }

    fread(&count, 4, 1, f);
    if (state.events) free(state.events);
    state.events = (ReplayEvent *)malloc(count * sizeof(ReplayEvent));
    state.count = count;
    state.capacity = count;
    fread(state.events, sizeof(ReplayEvent), count, f);
    fclose(f);

    printf("[replay] Loaded %d events from %s\n", count, filepath);
    return 1;
}

void replay_start_playback(void) {
    state.playback_idx = 0;
    state.start_tick = SDL_GetTicks();
    state.playing = 1;
    state.recording = 0;
    printf("[replay] Playback started\n");
}

ReplayEvent *replay_next_event(uint32_t current_tick) {
    uint32_t elapsed;
    if (!state.playing || !state.events) return NULL;
    if (state.playback_idx >= state.count) {
        state.playing = 0;
        printf("[replay] Playback finished\n");
        return NULL;
    }

    elapsed = current_tick - state.start_tick;
    if (state.events[state.playback_idx].tick <= elapsed) {
        return &state.events[state.playback_idx++];
    }
    return NULL;
}

int replay_is_recording(void) { return state.recording; }
int replay_is_playing(void) { return state.playing; }

void replay_shutdown(void) {
    if (state.events) {
        free(state.events);
        state.events = NULL;
    }
    state.count = 0;
    state.recording = 0;
    state.playing = 0;
}
