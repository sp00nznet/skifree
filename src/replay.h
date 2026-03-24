/*
 * SkiFree Replay System
 * Records input events and plays them back.
 */

#ifndef SKIFREE_REPLAY_H
#define SKIFREE_REPLAY_H

#include <SDL.h>

#define REPLAY_MAX_EVENTS 65536

typedef enum {
    REPLAY_INPUT_KEY,
    REPLAY_INPUT_MOUSE_MOVE,
    REPLAY_INPUT_MOUSE_CLICK,
    REPLAY_INPUT_CHAR
} ReplayInputType;

typedef struct {
    uint32_t tick;
    uint8_t type;       /* ReplayInputType */
    int32_t value1;     /* key sym or mouse x */
    int32_t value2;     /* 0 or mouse y */
} ReplayEvent;

typedef struct {
    ReplayEvent *events;
    int count;
    int capacity;
    int playback_idx;
    uint32_t start_tick;
    int recording;
    int playing;
} ReplayState;

/* Start recording inputs. */
void replay_start_recording(void);

/* Stop recording and return the replay data. */
void replay_stop_recording(void);

/* Record a single input event. */
void replay_record_event(ReplayInputType type, int32_t v1, int32_t v2);

/* Save replay to file. */
int replay_save(const char *filepath);

/* Load replay from file. */
int replay_load(const char *filepath);

/* Start playback. */
void replay_start_playback(void);

/* Get the next event if its tick has arrived. Returns NULL if none ready. */
ReplayEvent *replay_next_event(uint32_t current_tick);

/* Check if replay is currently recording or playing. */
int replay_is_recording(void);
int replay_is_playing(void);

/* Clean up replay data. */
void replay_shutdown(void);

#endif /* SKIFREE_REPLAY_H */
