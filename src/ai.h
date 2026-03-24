/*
 * SkiFree AI Opponents
 * AI-controlled skiers that race you on courses.
 */

#ifndef SKIFREE_AI_H
#define SKIFREE_AI_H

#include "types.h"
#include "consts.h"

#define MAX_AI_OPPONENTS 4

typedef struct {
    Actor *actor;
    int active;
    int difficulty;       /* 0=easy, 1=medium, 2=hard, 3=expert */
    int target_gate_idx;
    short target_x;
    short target_y;
    int style_points;
    int elapsed_time;
    int finished;
    int decision_timer;   /* frames until next direction change */
} AIOpponent;

/* Initialize the AI system. */
void ai_init(void);

/* Spawn AI opponents for the current game mode. */
void ai_spawn(int num_opponents, int difficulty);

/* Update all AI opponents. Called each game tick. */
void ai_update_all(void);

/* Reset/remove all AI opponents. */
void ai_reset(void);

/* Get AI opponent data for display. */
AIOpponent *ai_get_opponents(int *count);

/* Update function for ACTOR_TYPE_19_AI_SKIER. */
Actor *updateActorType19_aiSkier(Actor *actor);

#endif /* SKIFREE_AI_H */
