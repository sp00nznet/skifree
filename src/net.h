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
#define NET_PKT_PLAYER_INFO  6
#define NET_PKT_READY        7
#define NET_PKT_GAME_START   8
#define NET_PKT_KICK         9

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

typedef struct {
    uint8_t type;
    uint8_t player_id;
    char name[16];
    uint8_t r, g, b;
} NetPlayerInfo;

typedef struct {
    uint8_t type;
    uint8_t player_id;
    uint8_t ready;
} NetReady;

typedef struct {
    uint8_t type;
    int8_t spawn_offsets[NET_MAX_PLAYERS];
} NetGameStart;

typedef struct {
    uint8_t type;
    uint8_t player_id;
} NetKick;
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
    uint8_t r, g, b;
    uint8_t ready;
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

/* Set local player info (name + color) and send to peers. */
void net_set_player_info(const char *name, uint8_t r, uint8_t g, uint8_t b);

/* Set ready state and send to host. */
void net_set_ready(int ready);

/* Host only: start the game with the given number of bots. */
void net_start_game(int bot_count);

/* Host only: kick a player. */
void net_kick_player(int player_id);

/* Returns 1 if we're in the lobby (not yet playing). */
int net_get_lobby_state(void);

/* Returns 1 if all connected human players have ready=1. */
int net_all_humans_ready(void);

#endif /* SKIFREE_NET_H */
