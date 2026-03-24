#ifndef SKIFREE_WIN32_DIALOGS_H
#define SKIFREE_WIN32_DIALOGS_H

#include <stdbool.h>

/* File dialogs — return true if user selected a file */
bool dialog_save_file(char *out_path, int max_len);
bool dialog_open_file(char *out_path, int max_len);

/* Volume dialog — returns selected volume (0-128), or -1 if cancelled */
int dialog_volume(int current_volume);

/* Multiplayer host dialog — fills in settings, returns true if user clicked Start */
typedef struct {
    int port;
    int max_players;
    int extra_yetis;
    int super_speed;
    int star_power;
} HostSettings;
bool dialog_mp_host(HostSettings *out);

/* Multiplayer join dialog — fills in ip/port, returns true if user clicked Connect */
bool dialog_mp_join(char *out_ip, int ip_len, int *out_port);

/* About dialog */
void dialog_about(void *sdl_window);

#endif
