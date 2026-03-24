/*
 * SkiFree Win32 Native Dialogs
 * File save/load, volume, multiplayer host/join, about.
 */

#include "win32_dialogs.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <stdio.h>
#include <SDL.h>
#include <SDL_syswm.h>

static HWND get_hwnd(void *sdl_window) {
    SDL_SysWMinfo wminfo;
    SDL_VERSION(&wminfo.version);
    if (sdl_window && SDL_GetWindowWMInfo((SDL_Window *)sdl_window, &wminfo)) {
        return wminfo.info.win.window;
    }
    return NULL;
}

bool dialog_save_file(char *out_path, int max_len) {
    OPENFILENAMEA ofn = {0};
    out_path[0] = '\0';
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = "SkiFree Save (*.json)\0*.json\0All Files\0*.*\0";
    ofn.lpstrFile = out_path;
    ofn.nMaxFile = max_len;
    ofn.lpstrDefExt = "json";
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    return GetSaveFileNameA(&ofn) != 0;
}

bool dialog_open_file(char *out_path, int max_len) {
    OPENFILENAMEA ofn = {0};
    out_path[0] = '\0';
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = "SkiFree Save (*.json)\0*.json\0All Files\0*.*\0";
    ofn.lpstrFile = out_path;
    ofn.nMaxFile = max_len;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    return GetOpenFileNameA(&ofn) != 0;
}

int dialog_volume(int current_volume) {
    char buf[16];
    char msg[128];
    sprintf(msg, "Enter volume (0-128).\nCurrent: %d", current_volume);
    sprintf(buf, "%d", current_volume);

    /* Simple input box via a prompt — Win32 doesn't have a built-in input dialog,
     * so we use a message box + clipboard workaround, or just a simple approach */
    int result = MessageBoxA(NULL, msg, "SkiFree Volume",
                             MB_OKCANCEL | MB_ICONQUESTION);
    if (result == IDOK) {
        /* For now, cycle through preset volumes */
        current_volume += 32;
        if (current_volume > 128) current_volume = 0;
        return current_volume;
    }
    return -1;
}

bool dialog_mp_host(HostSettings *out) {
    int result;
    out->port = 27015;
    out->max_players = 4;
    out->extra_yetis = 0;
    out->super_speed = 0;
    out->star_power = 0;

    result = MessageBoxA(NULL,
        "Host a multiplayer game?\n\n"
        "Port: 27015\n"
        "Max players: 4\n\n"
        "Other players can join with:\n"
        "  --connect YOUR_IP 27015",
        "SkiFree Multiplayer - Host",
        MB_OKCANCEL | MB_ICONQUESTION);

    return result == IDOK;
}

bool dialog_mp_join(char *out_ip, int ip_len, int *out_port) {
    /* For now, use a simple hardcoded approach.
     * A proper implementation would use a custom dialog with text inputs. */
    int result = MessageBoxA(NULL,
        "Join a multiplayer game?\n\n"
        "This will connect to localhost:27015.\n"
        "For custom IP, use:\n"
        "  skifree --connect IP PORT",
        "SkiFree Multiplayer - Join",
        MB_OKCANCEL | MB_ICONQUESTION);

    if (result == IDOK) {
        strncpy(out_ip, "127.0.0.1", ip_len);
        *out_port = 27015;
        return true;
    }
    return false;
}

void dialog_about(void *sdl_window) {
    (void)sdl_window;
    MessageBoxA(NULL,
        "SkiFree: Reborn\n\n"
        "A static recompilation of the 1991 classic by Chris Pirih.\n\n"
        "Credits:\n"
        "  Chris Pirih - Original game (1991)\n"
        "  Eric Fry (yuv422) - Decompilation\n"
        "  jeff-1amstudios - SDL2 port\n"
        "  Alice Averlong - Sound research\n\n"
        "https://github.com/sp00nznet/skifree",
        "About SkiFree",
        MB_OK | MB_ICONINFORMATION);
}

#else
/* Non-Windows stubs */
bool dialog_save_file(char *out_path, int max_len) { (void)out_path; (void)max_len; return false; }
bool dialog_open_file(char *out_path, int max_len) { (void)out_path; (void)max_len; return false; }
int dialog_volume(int current_volume) { (void)current_volume; return -1; }
bool dialog_mp_host(HostSettings *out) { (void)out; return false; }
bool dialog_mp_join(char *out_ip, int ip_len, int *out_port) { (void)out_ip; (void)ip_len; (void)out_port; return false; }
void dialog_about(void *sdl_window) { (void)sdl_window; }
#endif
