/*
 * SkiFree Win32 Native Menu Bar
 * Based on sp00nznet/LinksAwakening pattern.
 */

#include "win32_menu.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <SDL.h>
#include <SDL_syswm.h>

static HMENU h_menu = NULL;
static HMENU h_gfx_menu = NULL;
static HMENU h_sound_menu = NULL;
static WNDPROC original_wndproc = NULL;
static menu_action_t pending_action = MENU_NONE;

static LRESULT CALLBACK menu_wndproc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_COMMAND) {
        switch (LOWORD(wParam)) {
        case IDM_FILE_SAVE:     pending_action = MENU_FILE_SAVE;    break;
        case IDM_FILE_SAVE_AS:  pending_action = MENU_FILE_SAVE_AS; break;
        case IDM_FILE_LOAD:     pending_action = MENU_FILE_LOAD;    break;
        case IDM_FILE_EXIT:     pending_action = MENU_FILE_EXIT;    break;
        case IDM_GFX_SCALE1:    pending_action = MENU_GFX_SCALE1;   break;
        case IDM_GFX_SCALE2:    pending_action = MENU_GFX_SCALE2;   break;
        case IDM_GFX_SCALE3:    pending_action = MENU_GFX_SCALE3;   break;
        case IDM_GFX_SCALE4:    pending_action = MENU_GFX_SCALE4;   break;
        case IDM_GFX_SCALE5:    pending_action = MENU_GFX_SCALE5;   break;
        case IDM_SOUND_TOGGLE:  pending_action = MENU_SOUND_TOGGLE; break;
        case IDM_SOUND_VOLUME:  pending_action = MENU_SOUND_VOLUME; break;
        case IDM_CTRL_KEYBOARD: pending_action = MENU_CTRL_KEYBOARD;break;
        case IDM_CTRL_GAMEPAD:  pending_action = MENU_CTRL_GAMEPAD; break;
        case IDM_MP_HOST:       pending_action = MENU_MP_HOST;      break;
        case IDM_MP_JOIN:       pending_action = MENU_MP_JOIN;      break;
        case IDM_DEBUG_OVERLAY: pending_action = MENU_DEBUG_OVERLAY; break;
        case IDM_DEBUG_ASSETS:  pending_action = MENU_DEBUG_ASSETS;  break;
        case IDM_ABOUT:         pending_action = MENU_ABOUT;        break;
        }
        return 0;
    }
    return CallWindowProcW(original_wndproc, hwnd, msg, wParam, lParam);
}

bool menu_init(void *sdl_window) {
    SDL_SysWMinfo wminfo;
    HWND hwnd;
    HMENU file_menu, gfx_menu, sound_menu, ctrl_menu, mp_menu, debug_menu, about_menu;

    SDL_VERSION(&wminfo.version);
    if (!SDL_GetWindowWMInfo((SDL_Window *)sdl_window, &wminfo)) {
        return false;
    }

    hwnd = wminfo.info.win.window;

    h_menu = CreateMenu();

    /* File */
    file_menu = CreatePopupMenu();
    AppendMenuW(file_menu, MF_STRING, IDM_FILE_SAVE,    L"&Save\tCtrl+S");
    AppendMenuW(file_menu, MF_STRING, IDM_FILE_SAVE_AS, L"Save &As...\tCtrl+Shift+S");
    AppendMenuW(file_menu, MF_STRING, IDM_FILE_LOAD,    L"&Load...\tCtrl+O");
    AppendMenuW(file_menu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(file_menu, MF_STRING, IDM_FILE_EXIT,    L"E&xit\tAlt+F4");
    AppendMenuW(h_menu, MF_POPUP, (UINT_PTR)file_menu, L"&File");

    /* Graphics */
    gfx_menu = CreatePopupMenu();
    AppendMenuW(gfx_menu, MF_STRING, IDM_GFX_SCALE1, L"1x Scale");
    AppendMenuW(gfx_menu, MF_STRING, IDM_GFX_SCALE2, L"2x Scale");
    AppendMenuW(gfx_menu, MF_STRING, IDM_GFX_SCALE3, L"3x Scale");
    AppendMenuW(gfx_menu, MF_STRING, IDM_GFX_SCALE4, L"4x Scale");
    AppendMenuW(gfx_menu, MF_STRING, IDM_GFX_SCALE5, L"5x Scale");
    h_gfx_menu = gfx_menu;
    AppendMenuW(h_menu, MF_POPUP, (UINT_PTR)gfx_menu, L"&Graphics");

    /* Sound */
    sound_menu = CreatePopupMenu();
    AppendMenuW(sound_menu, MF_STRING | MF_CHECKED, IDM_SOUND_TOGGLE, L"&Enable Sound");
    AppendMenuW(sound_menu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(sound_menu, MF_STRING, IDM_SOUND_VOLUME, L"&Volume...");
    h_sound_menu = sound_menu;
    AppendMenuW(h_menu, MF_POPUP, (UINT_PTR)sound_menu, L"&Sound");

    /* Controls */
    ctrl_menu = CreatePopupMenu();
    AppendMenuW(ctrl_menu, MF_STRING, IDM_CTRL_KEYBOARD, L"&Keyboard...");
    AppendMenuW(ctrl_menu, MF_STRING, IDM_CTRL_GAMEPAD,  L"&Gamepad...");
    AppendMenuW(h_menu, MF_POPUP, (UINT_PTR)ctrl_menu, L"&Controls");

    /* Multiplayer */
    mp_menu = CreatePopupMenu();
    AppendMenuW(mp_menu, MF_STRING, IDM_MP_HOST, L"&Host...");
    AppendMenuW(mp_menu, MF_STRING, IDM_MP_JOIN, L"&Join...");
    AppendMenuW(h_menu, MF_POPUP, (UINT_PTR)mp_menu, L"&Multiplayer");

    /* Debug */
    debug_menu = CreatePopupMenu();
    AppendMenuW(debug_menu, MF_STRING, IDM_DEBUG_OVERLAY, L"&Overlay\tF9");
    AppendMenuW(debug_menu, MF_STRING, IDM_DEBUG_ASSETS,  L"&Asset Viewer");
    AppendMenuW(h_menu, MF_POPUP, (UINT_PTR)debug_menu, L"&Debug");

    /* About */
    about_menu = CreatePopupMenu();
    AppendMenuW(about_menu, MF_STRING, IDM_ABOUT, L"&About SkiFree...");
    AppendMenuW(h_menu, MF_POPUP, (UINT_PTR)about_menu, L"&About");

    SetMenu(hwnd, h_menu);

    original_wndproc = (WNDPROC)SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)menu_wndproc);

    return true;
}

menu_action_t menu_poll_action(void) {
    menu_action_t action = pending_action;
    pending_action = MENU_NONE;
    return action;
}

void menu_set_scale_check(int scale) {
    if (!h_gfx_menu) return;
    CheckMenuRadioItem(h_gfx_menu, IDM_GFX_SCALE1, IDM_GFX_SCALE5,
                       IDM_GFX_SCALE1 + scale - 1, MF_BYCOMMAND);
}

void menu_set_sound_check(int enabled) {
    if (!h_sound_menu) return;
    CheckMenuItem(h_sound_menu, IDM_SOUND_TOGGLE,
                  MF_BYCOMMAND | (enabled ? MF_CHECKED : MF_UNCHECKED));
}

#else
/* Stubs for non-Windows platforms */
bool menu_init(void *sdl_window) { (void)sdl_window; return false; }
menu_action_t menu_poll_action(void) { return MENU_NONE; }
void menu_set_scale_check(int scale) { (void)scale; }
void menu_set_sound_check(int enabled) { (void)enabled; }
#endif
