/*
 * SkiFree Static Recompilation
 * Original game by Chris Pirih (1991)
 * Decompiled by Eric Fry (yuv422)
 * SDL2 port by jeff-1amstudios
 * Cleaned up and modernized for this project
 */

#ifndef SKIFREE_TYPES_H
#define SKIFREE_TYPES_H

#include <SDL.h>

typedef int BOOL;
typedef void *HDC;
typedef void *HWND;
typedef void *HGDIOBJ;
typedef unsigned long DWORD;
typedef char *LPCSTR;

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

typedef struct _RECT {
    long left;
    long top;
    long right;
    long bottom;
} RECT, *PRECT;

typedef struct {
    HDC sheetDC;
    HDC sheetDC_1bpp;
    SDL_Texture *sheet;
    short sheetYOffset;
    short width;
    short height;
    short totalPixels;
} Sprite;

typedef struct PermObject {
    struct Actor *actor;
    Sprite *spritePtr;
    short spriteIdx;
    int actorTypeMaybe;
    int actorFrameNo;
    short maybeX;
    short maybeY;
    short unk_0x18;
    short xVelocity;
    short yVelocity;
    short unk_0x1e;
    int unk_0x20;
} PermObject;

typedef struct PermObjectList {
    struct PermObject *startingObject;
    struct PermObject *nextObject;
    struct PermObject *currentObj;
} PermObjectList;

typedef struct Actor {
    struct Actor *next;
    struct Actor *linkedActor;
    struct Actor *actorPtr;
    struct PermObject *permObject;
    uint16_t spriteIdx2;
    Sprite *spritePtr;
    int typeMaybe;
    uint32_t frameNo;
    RECT someRect;
    RECT rect;
    short xPosMaybe;
    short yPosMaybe;
    short isInAir;
    short HorizontalVelMaybe;
    short verticalVelocityMaybe;
    short inAirCounter;
    uint32_t flags;
} Actor;

typedef struct {
    short unk_0;
    short unk_2;
    short unk_4;
    short unk_6;
    short xRelated;
    short unk_a;
    uint32_t frameNo;
} ActorVelStruct;

typedef struct {
    void *soundResource;
    void *soundData;
    void *effect; /* SoundEffect* from sound.h — kept as void* to avoid circular include */
} Sound;

typedef struct {
    int leftFrameNo;
    int rightFrameNo;
} PlayerTurnFrameNoLookupTbl;

typedef SDL_Surface *HBITMAP;

#endif /* SKIFREE_TYPES_H */
