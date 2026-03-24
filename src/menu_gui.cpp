/**
 * SkiFree ImGui Menu System
 * Win32 menu bar + ImGui config dialogs.
 * Pattern from sp00nznet/gb-recompiled.
 */

#include "menu_gui.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

#include <stdio.h>
#include <string.h>

extern "C" {
#include "input_bind.h"
#include "net.h"
}

/* Cached renderer pointer for RenderDrawData */
static SDL_Renderer *g_renderer = NULL;

/* Cached window pointer for resolution changes */
static SDL_Window *g_window = NULL;

/* Dialog visibility flags */
static bool show_sound_settings = false;
static bool show_keyboard_config = false;
static bool show_gamepad_config = false;
static bool show_host_dialog = false;
static bool show_join_dialog = false;
static bool show_resolution = false;
static bool show_mp_settings = false;

/* Sound state */
static int  snd_enabled = 1;
static int  snd_volume = 100;
static bool snd_changed = false;

/* Resolution state */
static int current_res_idx = 0;

typedef struct { int w; int h; const char *label; } Resolution;

static const Resolution res_original[] = {
    { 640,  480,  "640 x 480" },
    { 800,  600,  "800 x 600" },
    { 1024, 768,  "1024 x 768" },
    { 1280, 960,  "1280 x 960" },
    { 1600, 1200, "1600 x 1200" },
};
static const int num_res_original = 5;

static const Resolution res_widescreen[] = {
    { 854,  480,  "854 x 480" },
    { 1280, 720,  "1280 x 720 (720p)" },
    { 1600, 900,  "1600 x 900" },
    { 1920, 1080, "1920 x 1080 (1080p)" },
    { 2560, 1440, "2560 x 1440 (1440p)" },
    { 3840, 2160, "3840 x 2160 (4K)" },
};
static const int num_res_widescreen = 6;

/* Multiplayer state */
static bool host_requested = false;
static int  host_port = 27015;
static int  max_players = 4;
static bool extra_yetis = false;
static bool super_speed = false;
static bool star_power = false;

static bool join_requested = false;
static char join_ip[64] = "127.0.0.1";
static int  join_port = 27015;

/* Player identity settings */
static char player_name[16] = "Player";
static int  player_color_r = 255;
static int  player_color_g = 255;
static int  player_color_b = 255;

/* Keyboard bindings */
static const char *action_names[] = {
    "Left", "Right", "Up", "Down", "Jump", "Turbo", "Pause", "Reset"
};

/* Key rebinding state: -1 = not listening, 0-7 = action index waiting for key */
static int listening_for_key = -1;
static int listening_for_pad = -1;

extern "C" {

int menu_gui_init(SDL_Window *window, SDL_Renderer *renderer) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = NULL; /* don't save imgui.ini */

    ImGui::StyleColorsDark();
    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowRounding = 4.0f;
    style.FrameRounding = 2.0f;
    style.Alpha = 0.95f;

    g_renderer = renderer;
    g_window = window;
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    printf("[gui] ImGui initialized\n");
    return 1;
}

void menu_gui_shutdown(void) {
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void menu_gui_process_event(SDL_Event *event) {
    /* Intercept key/button presses when rebinding */
    if (listening_for_key >= 0 && event->type == SDL_KEYDOWN) {
        InputBindings *bind = input_bind_get();
        SDL_Keycode key = event->key.keysym.sym;
        switch (listening_for_key) {
        case 0: bind->key_left  = key; break;
        case 1: bind->key_right = key; break;
        case 2: bind->key_up    = key; break;
        case 3: bind->key_down  = key; break;
        case 4: bind->key_jump  = key; break;
        case 5: bind->key_turbo = key; break;
        case 6: bind->key_pause = key; break;
        case 7: bind->key_reset = key; break;
        }
        listening_for_key = -1;
        return; /* consume the event */
    }
    if (listening_for_pad >= 0 && event->type == SDL_CONTROLLERBUTTONDOWN) {
        InputBindings *bind = input_bind_get();
        int btn = event->cbutton.button;
        switch (listening_for_pad) {
        case 4: bind->pad_jump  = btn; break;
        case 5: bind->pad_turbo = btn; break;
        case 6: bind->pad_pause = btn; break;
        case 7: bind->pad_reset = btn; break;
        }
        listening_for_pad = -1;
        return;
    }
    ImGui_ImplSDL2_ProcessEvent(event);
}

int menu_gui_wants_input(void) {
    ImGuiIO &io = ImGui::GetIO();
    return (io.WantCaptureMouse || io.WantCaptureKeyboard) ? 1 : 0;
}

int menu_gui_is_active(void) {
    return (show_sound_settings || show_keyboard_config || show_gamepad_config ||
            show_host_dialog || show_join_dialog || show_resolution ||
            show_mp_settings) ? 1 : 0;
}

void menu_gui_new_frame(void) {
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    /* Draw active dialogs */

    /* Sound Settings */
    if (show_sound_settings) {
        ImGui::SetNextWindowSize(ImVec2(300, 160), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(170, 100), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Sound Settings", &show_sound_settings, 0)) {
            bool enabled = (snd_enabled != 0);
            if (ImGui::Checkbox("Enable Sound", &enabled)) {
                snd_enabled = enabled ? 1 : 0;
                snd_changed = true;
            }
            ImGui::Spacing();
            if (ImGui::SliderInt("Volume", &snd_volume, 0, 100)) {
                snd_changed = true;
            }
        }
        ImGui::End();
    }

    /* Resolution Picker */
    if (show_resolution) {
        ImGui::SetNextWindowSize(ImVec2(380, 360), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(130, 60), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Resolution", &show_resolution, 0)) {
            ImGui::Text("Original (4:3)");
            ImGui::Separator();
            for (int i = 0; i < num_res_original; i++) {
                if (ImGui::Selectable(res_original[i].label, false)) {
                    SDL_SetWindowSize(g_window, res_original[i].w, res_original[i].h);
                    SDL_SetWindowPosition(g_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
                }
            }

            ImGui::Spacing();
            ImGui::Text("Widescreen (16:9)");
            ImGui::Separator();
            for (int i = 0; i < num_res_widescreen; i++) {
                if (ImGui::Selectable(res_widescreen[i].label, false)) {
                    SDL_SetWindowSize(g_window, res_widescreen[i].w, res_widescreen[i].h);
                    SDL_SetWindowPosition(g_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
                }
            }
        }
        ImGui::End();
    }

    /* Keyboard Config */
    if (show_keyboard_config) {
        ImGui::SetNextWindowSize(ImVec2(380, 340), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(145, 80), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Keyboard Controls", &show_keyboard_config, 0)) {
            InputBindings *bind = input_bind_get();
            SDL_Keycode keys[8];
            keys[0] = bind->key_left;  keys[1] = bind->key_right;
            keys[2] = bind->key_up;    keys[3] = bind->key_down;
            keys[4] = bind->key_jump;  keys[5] = bind->key_turbo;
            keys[6] = bind->key_pause; keys[7] = bind->key_reset;

            ImGui::Text("Click a button then press a key to rebind.");
            ImGui::Separator();
            ImGui::Columns(3);
            ImGui::Text("Action"); ImGui::NextColumn();
            ImGui::Text("Key"); ImGui::NextColumn();
            ImGui::Text(""); ImGui::NextColumn();
            ImGui::Separator();

            for (int i = 0; i < 8; i++) {
                ImGui::Text("%s", action_names[i]); ImGui::NextColumn();
                ImGui::Text("%s", SDL_GetKeyName(keys[i])); ImGui::NextColumn();
                char btn_label[32];
                if (listening_for_key == i) {
                    snprintf(btn_label, sizeof(btn_label), "Press...##kb%d", i);
                } else {
                    snprintf(btn_label, sizeof(btn_label), "Rebind##kb%d", i);
                }
                if (ImGui::SmallButton(btn_label)) {
                    listening_for_key = i;
                }
                ImGui::NextColumn();
            }
            ImGui::Columns(1);
            ImGui::Separator();
            if (ImGui::Button("Reset Defaults")) {
                input_bind_init();
                listening_for_key = -1;
            }
        }
        ImGui::End();
    }

    /* Gamepad Config */
    if (show_gamepad_config) {
        ImGui::SetNextWindowSize(ImVec2(380, 340), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(145, 80), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Gamepad Controls", &show_gamepad_config, 0)) {
            InputBindings *bind = input_bind_get();
            /* Directions are stick-based, only buttons 4-7 are rebindable */
            const char *pad_dir_labels[] = { "Left Stick", "Left Stick", "Left Stick", "Left Stick" };
            int pad_btns[4];
            pad_btns[0] = bind->pad_jump;  pad_btns[1] = bind->pad_turbo;
            pad_btns[2] = bind->pad_pause; pad_btns[3] = bind->pad_reset;

            ImGui::Text("Press a button on your controller to rebind.");
            ImGui::Separator();
            ImGui::Columns(3);
            ImGui::Text("Action"); ImGui::NextColumn();
            ImGui::Text("Button"); ImGui::NextColumn();
            ImGui::Text(""); ImGui::NextColumn();
            ImGui::Separator();

            /* Direction actions (not rebindable) */
            for (int i = 0; i < 4; i++) {
                ImGui::Text("%s", action_names[i]); ImGui::NextColumn();
                ImGui::Text("%s", pad_dir_labels[i]); ImGui::NextColumn();
                ImGui::Text("(stick)"); ImGui::NextColumn();
            }
            /* Button actions (rebindable) */
            const char *btn_names[] = {
                "A", "B", "X", "Y", "Back", "Guide", "Start",
                "LS", "RS", "LB", "RB", "Up", "Down", "Left", "Right"
            };
            for (int i = 0; i < 4; i++) {
                int action_idx = i + 4; /* Jump, Turbo, Pause, Reset */
                ImGui::Text("%s", action_names[action_idx]); ImGui::NextColumn();
                const char *bname = (pad_btns[i] >= 0 && pad_btns[i] < 15) ? btn_names[pad_btns[i]] : "?";
                ImGui::Text("%s", bname); ImGui::NextColumn();
                char btn_label[32];
                if (listening_for_pad == action_idx) {
                    snprintf(btn_label, sizeof(btn_label), "Press...##gp%d", action_idx);
                } else {
                    snprintf(btn_label, sizeof(btn_label), "Rebind##gp%d", action_idx);
                }
                if (ImGui::SmallButton(btn_label)) {
                    listening_for_pad = action_idx;
                }
                ImGui::NextColumn();
            }
            ImGui::Columns(1);
            ImGui::Separator();
            if (ImGui::Button("Reset Defaults")) {
                input_bind_init();
                listening_for_pad = -1;
            }
        }
        ImGui::End();
    }

    /* Multiplayer Settings Dialog */
    if (show_mp_settings) {
        ImGui::SetNextWindowSize(ImVec2(340, 280), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(150, 100), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Multiplayer Settings", &show_mp_settings, 0)) {
            ImGui::Text("Player Identity");
            ImGui::Separator();
            ImGui::InputText("Username", player_name, sizeof(player_name));
            ImGui::Spacing();
            ImGui::Text("Player Color");
            ImGui::SliderInt("Red",   &player_color_r, 0, 255);
            ImGui::SliderInt("Green", &player_color_g, 0, 255);
            ImGui::SliderInt("Blue",  &player_color_b, 0, 255);
            /* Color preview */
            ImVec2 p = ImGui::GetCursorScreenPos();
            ImDrawList *draw_list = ImGui::GetWindowDrawList();
            draw_list->AddRectFilled(p, ImVec2(p.x + 60, p.y + 30),
                IM_COL32(player_color_r, player_color_g, player_color_b, 255));
            draw_list->AddRect(p, ImVec2(p.x + 60, p.y + 30),
                IM_COL32(255, 255, 255, 255));
            ImGui::Dummy(ImVec2(60, 35));

            ImGui::Spacing();
            if (ImGui::Button("Apply", ImVec2(-1, 28))) {
                if (net_is_active()) {
                    net_set_player_info(player_name,
                        (uint8_t)player_color_r,
                        (uint8_t)player_color_g,
                        (uint8_t)player_color_b);
                }
            }
        }
        ImGui::End();
    }

    /* Host Lobby Dialog */
    if (show_host_dialog) {
        ImGui::SetNextWindowSize(ImVec2(460, 440), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Host Multiplayer", &show_host_dialog, 0)) {
            if (!net_is_active()) {
                /* Pre-hosting: port + start listening */
                ImGui::InputInt("Port", &host_port);
                if (ImGui::Button("Start Listening", ImVec2(-1, 30))) {
                    host_requested = true;
                }
            } else if (net_is_host()) {
                /* Lobby: player list */
                ImGui::Text("Lobby — waiting for players...");
                ImGui::Separator();

                /* Player list table */
                ImGui::Columns(5, "playerlist");
                ImGui::Text("#"); ImGui::NextColumn();
                ImGui::Text("Name"); ImGui::NextColumn();
                ImGui::Text("Color"); ImGui::NextColumn();
                ImGui::Text("Ready"); ImGui::NextColumn();
                ImGui::Text(""); ImGui::NextColumn();
                ImGui::Separator();

                {
                    int pcount = 0;
                    NetPlayer *plist = net_get_players(&pcount);
                    for (int i = 0; i < pcount; i++) {
                        if (!plist[i].connected) continue;
                        ImGui::Text("%d", plist[i].player_id); ImGui::NextColumn();
                        ImGui::Text("%s", plist[i].name); ImGui::NextColumn();
                        /* Color square */
                        ImVec2 cp = ImGui::GetCursorScreenPos();
                        ImGui::GetWindowDrawList()->AddRectFilled(cp,
                            ImVec2(cp.x + 16, cp.y + 16),
                            IM_COL32(plist[i].r, plist[i].g, plist[i].b, 255));
                        ImGui::Dummy(ImVec2(16, 16)); ImGui::NextColumn();
                        ImGui::Text("%s", plist[i].ready ? "Yes" : "No"); ImGui::NextColumn();
                        /* Kick button (not for self) */
                        if (i != 0) {
                            char kick_lbl[32];
                            snprintf(kick_lbl, sizeof(kick_lbl), "Kick##k%d", i);
                            if (ImGui::SmallButton(kick_lbl)) {
                                net_kick_player(i);
                            }
                        } else {
                            ImGui::Text("(host)");
                        }
                        ImGui::NextColumn();
                    }
                }
                ImGui::Columns(1);
                ImGui::Separator();

                /* Bot count */
                static int bot_count = 0;
                ImGui::SliderInt("Bot Count", &bot_count, 0, 4);
                ImGui::Separator();

                /* Game settings */
                ImGui::Text("Game Settings:");
                ImGui::Checkbox("Extra Yetis", &extra_yetis);
                ImGui::Checkbox("Super Speed", &super_speed);
                ImGui::Checkbox("Star Power", &star_power);
                ImGui::Separator();

                /* Start Game button - enabled only when all humans ready */
                bool can_start = net_all_humans_ready() ? true : false;
                if (!can_start) {
                    ImGui::BeginDisabled();
                }
                if (ImGui::Button("Start Game", ImVec2(-1, 30))) {
                    net_start_game(bot_count);
                    show_host_dialog = false;
                }
                if (!can_start) {
                    ImGui::EndDisabled();
                    ImGui::Text("Waiting for all players to ready up...");
                }
            }
        }
        ImGui::End();
    }

    /* Join/Client Lobby Dialog */
    if (show_join_dialog) {
        ImGui::SetNextWindowSize(ImVec2(420, 360), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Join Multiplayer", &show_join_dialog, 0)) {
            if (!net_is_active()) {
                /* Pre-connect: IP/port + connect */
                ImGui::InputText("IP Address", join_ip, sizeof(join_ip));
                ImGui::InputInt("Port", &join_port);
                ImGui::Separator();
                if (ImGui::Button("Connect", ImVec2(-1, 30))) {
                    join_requested = true;
                }
            } else if (!net_is_host()) {
                /* Client lobby: player list + ready toggle */
                ImGui::Text("Connected to server — lobby");
                ImGui::Separator();

                ImGui::Columns(4, "cplayerlist");
                ImGui::Text("#"); ImGui::NextColumn();
                ImGui::Text("Name"); ImGui::NextColumn();
                ImGui::Text("Color"); ImGui::NextColumn();
                ImGui::Text("Ready"); ImGui::NextColumn();
                ImGui::Separator();

                {
                    int pcount = 0;
                    NetPlayer *plist = net_get_players(&pcount);
                    for (int i = 0; i < pcount; i++) {
                        if (!plist[i].connected) continue;
                        ImGui::Text("%d", plist[i].player_id); ImGui::NextColumn();
                        ImGui::Text("%s", plist[i].name); ImGui::NextColumn();
                        ImVec2 cp = ImGui::GetCursorScreenPos();
                        ImGui::GetWindowDrawList()->AddRectFilled(cp,
                            ImVec2(cp.x + 16, cp.y + 16),
                            IM_COL32(plist[i].r, plist[i].g, plist[i].b, 255));
                        ImGui::Dummy(ImVec2(16, 16)); ImGui::NextColumn();
                        ImGui::Text("%s", plist[i].ready ? "Yes" : "No"); ImGui::NextColumn();
                    }
                }
                ImGui::Columns(1);
                ImGui::Separator();

                /* Ready toggle */
                static bool client_ready = false;
                if (ImGui::Checkbox("Ready", &client_ready)) {
                    net_set_ready(client_ready ? 1 : 0);
                }

                if (net_get_lobby_state()) {
                    ImGui::Text("Waiting for host to start...");
                }
            }
        }
        ImGui::End();
    }
}

void menu_gui_render(void) {
    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), g_renderer);
}

/* Show/hide toggles */
void menu_gui_show_sound_settings(void) { show_sound_settings = true; }
void menu_gui_show_resolution(void) { show_resolution = true; }
void menu_gui_show_keyboard_config(void) { show_keyboard_config = true; }
void menu_gui_show_gamepad_config(void) { show_gamepad_config = true; }
void menu_gui_show_host_dialog(void) { show_host_dialog = true; }
void menu_gui_show_join_dialog(void) { show_join_dialog = true; }
void menu_gui_show_mp_settings(void) { show_mp_settings = true; }

/* Player identity getters */
const char *menu_gui_get_player_name(void) { return player_name; }
uint8_t menu_gui_get_player_r(void) { return (uint8_t)player_color_r; }
uint8_t menu_gui_get_player_g(void) { return (uint8_t)player_color_g; }
uint8_t menu_gui_get_player_b(void) { return (uint8_t)player_color_b; }

/* Sound getters */
int  menu_gui_get_sound_enabled(void) { return snd_enabled; }
int  menu_gui_get_volume(void) { return (snd_volume * 128) / 100; }
int  menu_gui_sound_changed(void) { return snd_changed ? 1 : 0; }
void menu_gui_clear_sound_changed(void) { snd_changed = false; }

/* Host getters */
int  menu_gui_host_requested(void) { return host_requested ? 1 : 0; }
int  menu_gui_get_host_port(void) { return host_port; }
int  menu_gui_get_max_players(void) { return max_players; }
int  menu_gui_get_extra_yetis(void) { return extra_yetis ? 1 : 0; }
int  menu_gui_get_super_speed(void) { return super_speed ? 1 : 0; }
int  menu_gui_get_star_power(void) { return star_power ? 1 : 0; }
void menu_gui_clear_host_request(void) { host_requested = false; }

/* Join getters */
int  menu_gui_join_requested(void) { return join_requested ? 1 : 0; }
const char *menu_gui_get_join_ip(void) { return join_ip; }
int  menu_gui_get_join_port(void) { return join_port; }
void menu_gui_clear_join_request(void) { join_requested = false; }

} /* extern "C" */
