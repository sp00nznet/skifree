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

/* Sound state */
static int  snd_enabled = 1;
static int  snd_volume = 128;
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

/* Keyboard bindings */
static const char *action_names[] = {
    "Left", "Right", "Up", "Down", "Jump", "Turbo", "Pause", "Reset"
};
static const char *key_names[512]; /* filled on init */

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
    ImGui_ImplSDL2_ProcessEvent(event);
}

int menu_gui_wants_input(void) {
    ImGuiIO &io = ImGui::GetIO();
    return (io.WantCaptureMouse || io.WantCaptureKeyboard) ? 1 : 0;
}

int menu_gui_is_active(void) {
    return (show_sound_settings || show_keyboard_config || show_gamepad_config ||
            show_host_dialog || show_join_dialog || show_resolution) ? 1 : 0;
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
            if (ImGui::SliderInt("Volume", &snd_volume, 0, 128)) {
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
        ImGui::SetNextWindowSize(ImVec2(350, 300), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(145, 80), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Keyboard Controls", &show_keyboard_config, 0)) {
            ImGui::Text("Keyboard bindings:");
            ImGui::Separator();
            ImGui::Columns(2);
            ImGui::Text("Action"); ImGui::NextColumn();
            ImGui::Text("Key"); ImGui::NextColumn();
            ImGui::Separator();

            const char *default_keys[] = {
                "Left Arrow", "Right Arrow", "Up Arrow", "Down Arrow",
                "Space", "F", "F3", "F2"
            };

            for (int i = 0; i < 8; i++) {
                ImGui::Text("%s", action_names[i]); ImGui::NextColumn();
                ImGui::Text("%s", default_keys[i]); ImGui::NextColumn();
            }
            ImGui::Columns(1);
            ImGui::Separator();
            ImGui::TextWrapped("Key remapping coming soon. Use skifree.ini [controls] section for now.");
        }
        ImGui::End();
    }

    /* Gamepad Config */
    if (show_gamepad_config) {
        ImGui::SetNextWindowSize(ImVec2(350, 300), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(145, 80), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Gamepad Controls", &show_gamepad_config, 0)) {
            ImGui::Text("Xbox Controller bindings:");
            ImGui::Separator();
            ImGui::Columns(2);
            ImGui::Text("Action"); ImGui::NextColumn();
            ImGui::Text("Button"); ImGui::NextColumn();
            ImGui::Separator();

            const char *pad_defaults[] = {
                "Left Stick", "Left Stick", "Left Stick", "Left Stick",
                "A", "RB", "Start", "Back"
            };

            for (int i = 0; i < 8; i++) {
                ImGui::Text("%s", action_names[i]); ImGui::NextColumn();
                ImGui::Text("%s", pad_defaults[i]); ImGui::NextColumn();
            }
            ImGui::Columns(1);
            ImGui::Separator();
            ImGui::TextWrapped("Button remapping coming soon. Use skifree.ini [controls] section for now.");
        }
        ImGui::End();
    }

    /* Host Dialog */
    if (show_host_dialog) {
        ImGui::SetNextWindowSize(ImVec2(320, 280), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Host Multiplayer", &show_host_dialog, 0)) {
            ImGui::InputInt("Port", &host_port);
            ImGui::SliderInt("Max Players", &max_players, 2, 4);
            ImGui::Separator();
            ImGui::Text("Fun Modifiers:");
            ImGui::Checkbox("Extra Yetis", &extra_yetis);
            ImGui::Checkbox("Super Speed (beyond turbo!)", &super_speed);
            ImGui::Checkbox("Star Power Pickups", &star_power);
            ImGui::Separator();
            if (ImGui::Button("Start Hosting", ImVec2(-1, 30))) {
                host_requested = true;
                show_host_dialog = false;
            }
        }
        ImGui::End();
    }

    /* Join Dialog */
    if (show_join_dialog) {
        ImGui::SetNextWindowSize(ImVec2(300, 160), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Join Multiplayer", &show_join_dialog, 0)) {
            ImGui::InputText("IP Address", join_ip, sizeof(join_ip));
            ImGui::InputInt("Port", &join_port);
            ImGui::Separator();
            if (ImGui::Button("Connect", ImVec2(-1, 30))) {
                join_requested = true;
                show_join_dialog = false;
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

/* Sound getters */
int  menu_gui_get_sound_enabled(void) { return snd_enabled; }
int  menu_gui_get_volume(void) { return snd_volume; }
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
