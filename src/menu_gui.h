/**
 * SkiFree ImGui Menu System
 * Settings dialogs triggered from Win32 menu bar.
 * Pattern from sp00nznet/gb-recompiled.
 */

#ifndef SKIFREE_MENU_GUI_H
#define SKIFREE_MENU_GUI_H

#include <SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize ImGui (call after SDL_CreateRenderer) */
int menu_gui_init(SDL_Window *window, SDL_Renderer *renderer);

/* Shut down ImGui */
void menu_gui_shutdown(void);

/* Process SDL event for ImGui */
void menu_gui_process_event(SDL_Event *event);

/* Returns 1 if ImGui wants mouse/keyboard (block game input) */
int menu_gui_wants_input(void);

/* Begin frame — call before any ImGui drawing */
void menu_gui_new_frame(void);

/* Render ImGui draw data — call before SDL_RenderPresent */
void menu_gui_render(void);

/* Show specific dialogs (called from Win32 menu actions) */
void menu_gui_show_sound_settings(void);
void menu_gui_show_resolution(void);
void menu_gui_show_keyboard_config(void);
void menu_gui_show_gamepad_config(void);
void menu_gui_show_host_dialog(void);
void menu_gui_show_join_dialog(void);

/* Query results from dialogs */

/* Sound */
int   menu_gui_get_sound_enabled(void);
int   menu_gui_get_volume(void);
int   menu_gui_sound_changed(void);
void  menu_gui_clear_sound_changed(void);

/* Multiplayer */
int   menu_gui_host_requested(void);
int   menu_gui_get_host_port(void);
int   menu_gui_get_max_players(void);
int   menu_gui_get_extra_yetis(void);
int   menu_gui_get_super_speed(void);
int   menu_gui_get_star_power(void);
void  menu_gui_clear_host_request(void);

int   menu_gui_join_requested(void);
const char *menu_gui_get_join_ip(void);
int   menu_gui_get_join_port(void);
void  menu_gui_clear_join_request(void);

/* Returns 1 if any dialog is open */
int   menu_gui_is_active(void);

#ifdef __cplusplus
}
#endif

#endif /* SKIFREE_MENU_GUI_H */
