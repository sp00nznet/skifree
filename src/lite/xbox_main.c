/* Original Xbox (nxdk): nxdk's SDL never sets a video mode, so set 640x480 before
 * the game starts. xbox_compat.h renames the game's main() to skifree_main().
 * If the game returns, say why on screen instead of leaving a black screen. */
#undef main
#include <hal/debug.h>
#include <hal/video.h>
#include <windows.h>
#include <SDL.h>

int skifree_main(int argc, char *argv[]);

int main(void)
{
    static char *argv[] = { "skifree", NULL };
    XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);
    int rc = skifree_main(1, argv);
    debugPrint("skifree exited (%d): %s\n", rc, SDL_GetError());
    for (;;) Sleep(1000);
}
