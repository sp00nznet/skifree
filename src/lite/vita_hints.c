/* PS Vita: SDL's default renderer is GLES2 on vitaGL, which compiles shaders at
 * run time and needs libshacccg.suprx (absent on many consoles and in Vita3K:
 * it crashes). The native GXM renderer needs nothing extra. Set before main. */
#include <SDL.h>

__attribute__((constructor)) static void vita_hints(void)
{
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "VITA gxm");
}
