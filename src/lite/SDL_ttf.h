/*
 * The lite build's SDL_ttf: just what the status window uses, drawn with a
 * public-domain 8x8 font, so a console needs no SDL_ttf (or FreeType).
 * On the include path ahead of the real one only when SKIFREE_LITE is on.
 */
#ifndef SKIFREE_LITE_SDL_TTF_H
#define SKIFREE_LITE_SDL_TTF_H

#include <SDL.h>
#include "font8x8_basic.h"

#define LITE_LINE_H 10   /* 8x8 glyphs in an 8x10 cell, close to vgaoem's 8x12 */

typedef struct { int unused; } TTF_Font;

static inline int TTF_Init(void) { return 0; }

static inline TTF_Font *TTF_OpenFontRW(SDL_RWops *src, int freesrc, int ptsize) {
    static TTF_Font font;
    (void)ptsize;
    if (src && freesrc) SDL_RWclose(src);
    return &font;
}

/* Widest line and number of lines */
static inline void lite_ttf_measure(const char *text, int *w, int *lines) {
    int cur = 0;
    *w = 0; *lines = 1;
    for (; *text; text++) {
        if (*text == '\n') { (*lines)++; cur = 0; continue; }
        if (++cur > *w) *w = cur;
    }
}

static inline int TTF_SizeUTF8(TTF_Font *font, const char *text, int *w, int *h) {
    int cols, lines;
    (void)font;
    lite_ttf_measure(text, &cols, &lines);
    if (w) *w = cols * 8;
    if (h) *h = lines * LITE_LINE_H;
    return 0;
}

static inline SDL_Surface *TTF_RenderUTF8_Blended_Wrapped(TTF_Font *font, const char *text, SDL_Color fg, Uint32 wrap) {
    int cols, lines, x = 0, y = 0, gx, gy;
    SDL_Surface *s;
    Uint32 ink;
    (void)font; (void)wrap;
    lite_ttf_measure(text, &cols, &lines);
    s = SDL_CreateRGBSurfaceWithFormat(0, cols ? cols * 8 : 1, lines * LITE_LINE_H, 32, SDL_PIXELFORMAT_RGBA32);
    if (!s) return NULL;
    SDL_FillRect(s, NULL, SDL_MapRGBA(s->format, 0, 0, 0, 0));
    ink = SDL_MapRGBA(s->format, fg.r, fg.g, fg.b, 255);
    SDL_SetSurfaceBlendMode(s, SDL_BLENDMODE_BLEND);
    for (; *text; text++) {
        unsigned char c = (unsigned char)*text;
        if (c == '\n') { x = 0; y += LITE_LINE_H; continue; }
        if (c < 128) {
            for (gy = 0; gy < 8; gy++)
                for (gx = 0; gx < 8; gx++)
                    if (font8x8_basic[c][gy] & (1 << gx))   /* bit 0 is the leftmost pixel */
                        ((Uint32 *)((Uint8 *)s->pixels + (y + 1 + gy) * s->pitch))[x + gx] = ink;
        }
        x += 8;
    }
    return s;
}

#endif
