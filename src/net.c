/*
 * SkiFree Network Multiplayer
 *
 * Architecture:
 * - UDP for position updates (lossy is fine, positions update every frame)
 * - Peer-to-peer with host as authority for session management
 * - Remote players rendered as real actors using the existing actor system
 * - Player-to-player collision via handleActorCollision()
 *
 * Cross-platform socket abstraction handles Windows (winsock2) vs POSIX.
 */

/* winsock2.h MUST come before any of our headers on Windows
 * to avoid type redefinition conflicts with windows.h */
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
typedef SOCKET socket_t;
#define SOCKET_INVALID INVALID_SOCKET
#define sock_close closesocket
/* Prevent our types.h from redefining Windows types */
#define SKIFREE_NO_WIN_TYPEDEFS
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
typedef int socket_t;
#define SOCKET_INVALID -1
#define sock_close close
#endif

#include "net.h"
#include "consts.h"
#include "skifree.h"
#include <stdio.h>
#include <string.h>

/* Extern declarations for globals we need from data.h
 * (data.h can't be included in multiple .c files — it defines, not declares) */
extern Actor *actorListPtr;

static int net_initialized = 0;
static int is_hosting = 0;
static int is_connected = 0;
static int local_player_id = 0;
static int tick_counter = 0;

static socket_t udp_socket = SOCKET_INVALID;
static struct sockaddr_in peer_addrs[NET_MAX_PLAYERS];
static int num_peers = 0;

static NetPlayer players[NET_MAX_PLAYERS];

static void set_nonblocking(socket_t sock) {
#ifdef _WIN32
    u_long mode = 1;
    ioctlsocket(sock, FIONBIO, &mode);
#else
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
#endif
}

int net_init(void) {
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("[net] WSAStartup failed\n");
        return 0;
    }
#endif
    memset(players, 0, sizeof(players));
    net_initialized = 1;
    printf("[net] Initialized\n");
    return 1;
}

int net_host(int port) {
    struct sockaddr_in addr;

    if (!net_initialized && !net_init()) return 0;

    udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udp_socket == SOCKET_INVALID) {
        printf("[net] Failed to create socket\n");
        return 0;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons((uint16_t)port);

    if (bind(udp_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        printf("[net] Failed to bind port %d\n", port);
        sock_close(udp_socket);
        udp_socket = SOCKET_INVALID;
        return 0;
    }

    set_nonblocking(udp_socket);
    is_hosting = 1;
    is_connected = 1;
    local_player_id = 0;

    players[0].connected = 1;
    players[0].player_id = 0;
    strncpy(players[0].name, "Host", sizeof(players[0].name));

    printf("[net] Hosting on port %d\n", port);
    return 1;
}

int net_connect(const char *host, int port) {
    struct sockaddr_in addr;

    if (!net_initialized && !net_init()) return 0;

    udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udp_socket == SOCKET_INVALID) {
        printf("[net] Failed to create socket\n");
        return 0;
    }

    set_nonblocking(udp_socket);

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
    inet_pton(AF_INET, host, &addr.sin_addr);

    peer_addrs[0] = addr;
    num_peers = 1;

    /* Send join packet */
    {
        NetPlayerJoin join_pkt;
        join_pkt.type = NET_PKT_PLAYER_JOIN;
        join_pkt.player_id = 1;
        strncpy(join_pkt.name, "Player", sizeof(join_pkt.name));
        sendto(udp_socket, (char *)&join_pkt, sizeof(join_pkt), 0,
               (struct sockaddr *)&addr, sizeof(addr));
    }

    is_hosting = 0;
    is_connected = 1;
    local_player_id = 1;

    players[1].connected = 1;
    players[1].player_id = 1;
    strncpy(players[1].name, "Player", sizeof(players[1].name));

    printf("[net] Connected to %s:%d\n", host, port);
    return 1;
}

void net_disconnect(void) {
    int i;
    if (udp_socket != SOCKET_INVALID) {
        sock_close(udp_socket);
        udp_socket = SOCKET_INVALID;
    }
    for (i = 0; i < NET_MAX_PLAYERS; i++) {
        if (players[i].actor) {
            actorSetFlag8IfFlag1IsUnset(players[i].actor);
            players[i].actor = NULL;
        }
        players[i].connected = 0;
    }
    is_connected = 0;
    is_hosting = 0;
    num_peers = 0;
    printf("[net] Disconnected\n");
}

void net_shutdown(void) {
    net_disconnect();
#ifdef _WIN32
    WSACleanup();
#endif
    net_initialized = 0;
}

void net_send_player_state(short x, short y, uint16_t sprite_idx,
                           short is_in_air, short vel_x, short vel_y) {
    NetPlayerState pkt;
    int i;

    if (!is_connected || udp_socket == SOCKET_INVALID) return;

    tick_counter++;
    if (tick_counter % NET_TICK_RATE != 0) return;

    pkt.type = NET_PKT_PLAYER_STATE;
    pkt.player_id = (uint8_t)local_player_id;
    pkt.x = x;
    pkt.y = y;
    pkt.sprite_idx = sprite_idx;
    pkt.is_in_air = is_in_air;
    pkt.vel_x = vel_x;
    pkt.vel_y = vel_y;
    pkt.tick = SDL_GetTicks();

    for (i = 0; i < num_peers; i++) {
        sendto(udp_socket, (char *)&pkt, sizeof(pkt), 0,
               (struct sockaddr *)&peer_addrs[i], sizeof(peer_addrs[i]));
    }
}

void net_update(void) {
    char buf[256];
    struct sockaddr_in from_addr;
    int from_len = sizeof(from_addr);
    int recv_len;
    uint8_t pkt_type;

    if (!is_connected || udp_socket == SOCKET_INVALID) return;

    /* Process all pending packets */
    while (1) {
        recv_len = recvfrom(udp_socket, buf, sizeof(buf), 0,
                            (struct sockaddr *)&from_addr, &from_len);
        if (recv_len <= 0) break;

        pkt_type = (uint8_t)buf[0];

        switch (pkt_type) {
        case NET_PKT_PLAYER_STATE: {
            NetPlayerState *state = (NetPlayerState *)buf;
            int pid = state->player_id;
            if (pid >= 0 && pid < NET_MAX_PLAYERS && pid != local_player_id) {
                players[pid].x = state->x;
                players[pid].y = state->y;
                players[pid].sprite_idx = state->sprite_idx;
                players[pid].is_in_air = state->is_in_air;
                players[pid].last_update_tick = SDL_GetTicks();
                players[pid].connected = 1;
            }
            break;
        }
        case NET_PKT_PLAYER_JOIN: {
            NetPlayerJoin *join = (NetPlayerJoin *)buf;
            int pid = join->player_id;
            if (pid >= 0 && pid < NET_MAX_PLAYERS) {
                players[pid].connected = 1;
                players[pid].player_id = pid;
                strncpy(players[pid].name, join->name, sizeof(players[pid].name));

                /* If we're host, track this peer */
                if (is_hosting && num_peers < NET_MAX_PLAYERS) {
                    peer_addrs[num_peers++] = from_addr;
                }
                printf("[net] Player %d (%s) joined\n", pid, join->name);
            }
            break;
        }
        case NET_PKT_PLAYER_LEAVE: {
            int pid = (uint8_t)buf[1];
            if (pid >= 0 && pid < NET_MAX_PLAYERS) {
                if (players[pid].actor) {
                    actorSetFlag8IfFlag1IsUnset(players[pid].actor);
                    players[pid].actor = NULL;
                }
                players[pid].connected = 0;
                printf("[net] Player %d left\n", pid);
            }
            break;
        }
        }
    }
}

void net_update_actors(void) {
    int i;
    for (i = 0; i < NET_MAX_PLAYERS; i++) {
        if (i == local_player_id) continue;
        if (!players[i].connected) continue;

        /* Check for stale player (no update in 5 seconds) */
        if (SDL_GetTicks() - players[i].last_update_tick > 5000) {
            if (players[i].actor) {
                actorSetFlag8IfFlag1IsUnset(players[i].actor);
                players[i].actor = NULL;
            }
            players[i].connected = 0;
            continue;
        }

        /* Create actor if needed */
        if (!players[i].actor) {
            players[i].actor = addActorOfTypeWithSpriteIdx(
                ACTOR_TYPE_0_PLAYER, players[i].sprite_idx);
            if (!players[i].actor) continue;
        }

        /* Update actor position */
        updateActorPositionMaybe(players[i].actor,
                                players[i].x, players[i].y,
                                players[i].is_in_air);
        if (players[i].sprite_idx != players[i].actor->spriteIdx2) {
            actorSetSpriteIdx(players[i].actor, players[i].sprite_idx);
        }
    }
}

int net_is_active(void) { return is_connected; }
int net_is_host(void) { return is_hosting; }

NetPlayer *net_get_players(int *count) {
    if (count) *count = NET_MAX_PLAYERS;
    return players;
}
