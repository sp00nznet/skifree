/*
 * The lite build (consoles, Android, the browser): no ImGui menus and no
 * network game, so these stand in for menu_gui.cpp and net.c. Everything is
 * off and nothing is ever requested.
 */
#include "../menu_gui.h"
#include "../net.h"

int menu_gui_init(SDL_Window *window, SDL_Renderer *renderer) { (void)window; (void)renderer; return 1; }
void menu_gui_shutdown(void) {}
void menu_gui_process_event(SDL_Event *event) { (void)event; }
int menu_gui_wants_input(void) { return 0; }
int menu_gui_is_active(void) { return 0; }
void menu_gui_new_frame(void) {}
void menu_gui_render(void) {}
void menu_gui_show_sound_settings(void) {}
void menu_gui_show_resolution(void) {}
void menu_gui_show_keyboard_config(void) {}
void menu_gui_show_gamepad_config(void) {}
void menu_gui_show_host_dialog(void) {}
void menu_gui_show_join_dialog(void) {}
void menu_gui_show_mp_settings(void) {}
const char *menu_gui_get_player_name(void) { return "Player"; }
uint8_t menu_gui_get_player_r(void) { return 255; }
uint8_t menu_gui_get_player_g(void) { return 255; }
uint8_t menu_gui_get_player_b(void) { return 255; }
int menu_gui_get_sound_enabled(void) { return 1; }
int menu_gui_get_volume(void) { return 128; }
int menu_gui_sound_changed(void) { return 0; }
void menu_gui_clear_sound_changed(void) {}
int menu_gui_host_requested(void) { return 0; }
int menu_gui_get_host_port(void) { return 0; }
int menu_gui_get_max_players(void) { return 0; }
int menu_gui_get_extra_yetis(void) { return 0; }
int menu_gui_get_super_speed(void) { return 0; }
int menu_gui_get_star_power(void) { return 0; }
void menu_gui_clear_host_request(void) {}
int menu_gui_join_requested(void) { return 0; }
const char *menu_gui_get_join_ip(void) { return ""; }
int menu_gui_get_join_port(void) { return 0; }
void menu_gui_clear_join_request(void) {}

int net_init(void) { return 0; }
int net_host(int port) { (void)port; return 0; }
int net_connect(const char *host, int port) { (void)host; (void)port; return 0; }
void net_disconnect(void) {}
void net_shutdown(void) {}
void net_update(void) {}
void net_update_actors(void) {}
int net_is_active(void) { return 0; }
int net_is_host(void) { return 0; }
NetPlayer *net_get_players(int *count) { if (count) *count = 0; return 0; }
void net_set_player_info(const char *name, uint8_t r, uint8_t g, uint8_t b) { (void)name; (void)r; (void)g; (void)b; }
void net_set_ready(int ready) { (void)ready; }
void net_start_game(int bot_count) { (void)bot_count; }
void net_kick_player(int player_id) { (void)player_id; }
int net_get_lobby_state(void) { return 0; }
int net_all_humans_ready(void) { return 0; }
int net_get_spawn_offset(void) { return 0; }
int net_get_mp_extra_yetis(void) { return 0; }
int net_get_mp_super_speed(void) { return 0; }
int net_get_mp_star_power(void) { return 0; }
void net_set_mp_settings(int extra_yetis, int super_speed, int star_power) { (void)extra_yetis; (void)super_speed; (void)star_power; }
void net_send_player_state(short x, short y, uint16_t sprite_idx, short is_in_air, short vel_x, short vel_y) {
    (void)x; (void)y; (void)sprite_idx; (void)is_in_air; (void)vel_x; (void)vel_y;
}
