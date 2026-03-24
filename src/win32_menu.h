#ifndef SKIFREE_WIN32_MENU_H
#define SKIFREE_WIN32_MENU_H

#include <stdbool.h>

/* Menu command IDs */
#define IDM_FILE_SAVE       1001
#define IDM_FILE_SAVE_AS    1002
#define IDM_FILE_LOAD       1003
#define IDM_FILE_EXIT       1004

#define IDM_GFX_SCALE1      2001
#define IDM_GFX_SCALE2      2002
#define IDM_GFX_SCALE3      2003
#define IDM_GFX_SCALE4      2004
#define IDM_GFX_SCALE5      2005

#define IDM_SOUND_TOGGLE    3001
#define IDM_SOUND_VOLUME    3002

#define IDM_CTRL_KEYBOARD   4001
#define IDM_CTRL_GAMEPAD    4002

#define IDM_MP_HOST         5001
#define IDM_MP_JOIN         5002

#define IDM_DEBUG_OVERLAY   6001
#define IDM_DEBUG_ASSETS    6002

#define IDM_ABOUT           7001

typedef enum {
    MENU_NONE = 0,
    MENU_FILE_SAVE, MENU_FILE_SAVE_AS, MENU_FILE_LOAD, MENU_FILE_EXIT,
    MENU_GFX_SCALE1, MENU_GFX_SCALE2, MENU_GFX_SCALE3, MENU_GFX_SCALE4, MENU_GFX_SCALE5,
    MENU_SOUND_TOGGLE, MENU_SOUND_VOLUME,
    MENU_CTRL_KEYBOARD, MENU_CTRL_GAMEPAD,
    MENU_MP_HOST, MENU_MP_JOIN,
    MENU_DEBUG_OVERLAY, MENU_DEBUG_ASSETS,
    MENU_ABOUT
} menu_action_t;

bool menu_init(void *sdl_window);
menu_action_t menu_poll_action(void);
void menu_set_scale_check(int scale);
void menu_set_sound_check(int enabled);

#endif
