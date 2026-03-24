/*
 * SkiFree AI Opponents
 * AI-controlled skiers that race you on courses.
 *
 * AI skiers use the same actor system as everything else — same physics,
 * same collisions, same sprites. They steer toward gates, avoid obstacles,
 * and compete on timed courses.
 */

#include "ai.h"
#include "skifree.h"
#include "data.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

static AIOpponent opponents[MAX_AI_OPPONENTS];
static int num_active = 0;

/* Difficulty affects reaction time and line precision */
static const int DECISION_INTERVALS[] = {15, 10, 6, 3}; /* frames between decisions */
static const int AVOIDANCE_RANGE[] = {80, 100, 120, 150}; /* lookahead pixels */

void ai_init(void) {
    int i;
    for (i = 0; i < MAX_AI_OPPONENTS; i++) {
        opponents[i].active = 0;
        opponents[i].actor = NULL;
    }
    num_active = 0;
}

void ai_spawn(int num_opponents, int difficulty) {
    int i;
    Actor *actor;
    short spawn_x, spawn_y;

    if (num_opponents > MAX_AI_OPPONENTS)
        num_opponents = MAX_AI_OPPONENTS;

    for (i = 0; i < num_opponents; i++) {
        /* Spawn near the player but offset horizontally */
        spawn_x = (short)(playerX + (i + 1) * 40 - 80);
        spawn_y = (short)(playerY - 20);

        /* Use beginner sprites (frames 22-24) for AI skiers */
        actor = addActorOfType(ACTOR_TYPE_19_AI_SKIER, 22);
        if (!actor) continue;

        actor = updateActorPositionMaybe(actor, spawn_x, spawn_y, 0);

        opponents[i].actor = actor;
        opponents[i].active = 1;
        opponents[i].difficulty = difficulty;
        opponents[i].target_gate_idx = 0;
        opponents[i].target_x = spawn_x;
        opponents[i].target_y = spawn_y + 200;
        opponents[i].style_points = 0;
        opponents[i].elapsed_time = 0;
        opponents[i].finished = 0;
        opponents[i].decision_timer = 0;
        num_active++;
    }

    printf("[ai] Spawned %d AI opponents (difficulty %d)\n", num_active, difficulty);
}

/* Choose a target position based on game mode and obstacles */
static void ai_choose_target(AIOpponent *ai) {
    short ahead_y;
    int range;

    if (!ai->actor) return;

    range = AVOIDANCE_RANGE[ai->difficulty];
    ahead_y = ai->actor->yPosMaybe + (short)range;

    /* Default: ski roughly straight down with some variation */
    ai->target_x = ai->actor->xPosMaybe + (short)(ski_random(60) - 30);
    ai->target_y = ahead_y;

    /* In slalom mode, steer toward the next gate */
    if (isSsGameMode && currentSlalomFlag) {
        ai->target_x = currentSlalomFlag->maybeX;
        ai->target_y = currentSlalomFlag->maybeY;
    } else if (isGsGameMode && currentSlalomFlag) {
        ai->target_x = currentSlalomFlag->maybeX;
        ai->target_y = currentSlalomFlag->maybeY;
    }
}

/* Simple obstacle avoidance: check if any nearby actors are in the way */
static int ai_check_obstacle(AIOpponent *ai, short check_x, short check_y) {
    Actor *other;
    short dx, dy;
    int range = AVOIDANCE_RANGE[ai->difficulty];

    for (other = actorListPtr; other != NULL; other = other->next) {
        if (other == ai->actor) continue;
        if (other->typeMaybe == ACTOR_TYPE_0_PLAYER) continue;
        if (other->typeMaybe == ACTOR_TYPE_19_AI_SKIER) continue;
        if (other->typeMaybe >= ACTOR_TYPE_11_MOGULS &&
            other->typeMaybe != ACTOR_TYPE_13_TREE &&
            other->typeMaybe != ACTOR_TYPE_14_ROCK_STUMP) continue;

        dx = other->xPosMaybe - check_x;
        dy = other->yPosMaybe - check_y;

        if (abs(dx) < 30 && dy > 0 && dy < range) {
            return 1; /* obstacle ahead */
        }
    }
    return 0;
}

Actor *updateActorType19_aiSkier(Actor *actor) {
    int i;
    AIOpponent *ai = NULL;
    int frame;
    short dx;

    /* Find which AI opponent this actor belongs to */
    for (i = 0; i < MAX_AI_OPPONENTS; i++) {
        if (opponents[i].actor == actor && opponents[i].active) {
            ai = &opponents[i];
            break;
        }
    }

    if (!ai || ai->finished) {
        return updateActorPositionWithVelocityMaybe(actor);
    }

    /* Update decision timer */
    ai->decision_timer--;
    if (ai->decision_timer <= 0) {
        ai->decision_timer = DECISION_INTERVALS[ai->difficulty];
        ai_choose_target(ai);

        /* Try to avoid obstacles */
        if (ai_check_obstacle(ai, ai->target_x, actor->yPosMaybe)) {
            /* Dodge left or right */
            if (ski_random(2) == 0) {
                ai->target_x -= 60;
            } else {
                ai->target_x += 60;
            }
        }
    }

    /* Steer toward target */
    dx = ai->target_x - actor->xPosMaybe;

    if (dx < -20) {
        /* Turn left */
        frame = 1; /* down-left */
        if (dx < -60) frame = 2; /* sharp left */
    } else if (dx > 20) {
        /* Turn right */
        frame = 4; /* down-right */
        if (dx > 60) frame = 5; /* sharp right */
    } else {
        /* Go straight */
        frame = 0; /* straight down */
    }

    actor = updateActorPositionWithVelocityMaybe(actor);
    actor = updateActorVelMaybe(actor, &ActorVelStruct_ARRAY_0040a308[frame]);
    return setActorFrameNo(actor, frame);
}

void ai_update_all(void) {
    /* AI actors are updated through the normal updateActor() dispatch.
     * This function handles higher-level AI state like finish detection. */
    int i;
    for (i = 0; i < MAX_AI_OPPONENTS; i++) {
        if (!opponents[i].active || !opponents[i].actor) continue;

        /* Check if AI reached the finish line */
        if (isSsGameMode && opponents[i].actor->yPosMaybe > 8640) {
            opponents[i].finished = 1;
            opponents[i].elapsed_time = elapsedTime;
        } else if ((isFsGameMode || isGsGameMode) && opponents[i].actor->yPosMaybe > 16640) {
            opponents[i].finished = 1;
            opponents[i].elapsed_time = elapsedTime;
        }

        /* Remove dead AI actors */
        if (opponents[i].actor->flags & FLAG_8) {
            opponents[i].actor = NULL;
            opponents[i].active = 0;
            num_active--;
        }
    }
}

void ai_reset(void) {
    int i;
    for (i = 0; i < MAX_AI_OPPONENTS; i++) {
        if (opponents[i].active && opponents[i].actor) {
            actorSetFlag8IfFlag1IsUnset(opponents[i].actor);
        }
        opponents[i].active = 0;
        opponents[i].actor = NULL;
    }
    num_active = 0;
}

AIOpponent *ai_get_opponents(int *count) {
    if (count) *count = MAX_AI_OPPONENTS;
    return opponents;
}
