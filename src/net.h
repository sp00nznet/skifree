/*
 * SkiFree Network Multiplayer
 * Competitive multiplayer with UDP position sync and TCP session management.
 */

#ifndef SKIFREE_NET_H
#define SKIFREE_NET_H

#include "types.h"
#include <stdint.h>

#define NET_MAX_PLAYERS 4
#define NET_DEFAULT_PORT 27015
#define NET_TICK_RATE 4     /* send state every N game ticks */

/* Network packet types */
#define NET_PKT_PLAYER_STATE 1
#define NET_PKT_PLAYER_JOIN  2
#define NET_PKT_PLAYER_LEAVE 3
#define NET_PKT_GAME_STATE   4

#pragma pack(push, 1)
typedef struct {
    uint8_t type;
    uint8_t player_id;
    short x;
    short y;
    uint16_t sprite_idx;
    short is_in_air;
    short vel_x;
    short vel_y;
    uint32_t tick;
} NetPlayerState;

typedef struct {
    uint8_t type;
    uint8_t player_id;
    char name[16];
} NetPlayerJoin;
#pragma pack(pop)

typedef struct {
    int connected;
    int player_id;
    char name[16];
    short x, y;
    uint16_t sprite_idx;
    short is_in_air;
    uint32_t last_update_tick;
    Actor *actor;
} NetPlayer;

/* Initialize networking. Returns 1 on success. */
int net_init(void);

/* Host a game on the given port. */
int net_host(int port);

/* Connect to a host. */
int net_connect(const char *host, int port);

/* Disconnect and clean up. */
void net_disconnect(void);

/* Shut down networking. */
void net_shutdown(void);

/* Send local player state. Called every NET_TICK_RATE game ticks. */
void net_send_player_state(short x, short y, uint16_t sprite_idx,
                           short is_in_air, short vel_x, short vel_y);

/* Process incoming network packets. Call each game tick. */
void net_update(void);

/* Create/update actor representations for remote players. */
void net_update_actors(void);

/* Check if we're in a network game. */
int net_is_active(void);

/* Check if we're the host. */
int net_is_host(void);

/* Get remote player data. */
NetPlayer *net_get_players(int *count);

#endif /* SKIFREE_NET_H */
