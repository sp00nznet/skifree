/*
 * SkiFree Save/Load System
 * Serializes complete game state to/from JSON using cJSON.
 */

#include "saveload.h"
#include "skifree.h"
#include "consts.h"
#include <cjson/cJSON.h>
#include <stdio.h>
#include <string.h>

/* Extern declarations for globals from data.h */
extern int playerX;
extern short playerY;
extern Actor *playerActor;
extern Actor *playerActorPtrMaybe_1;
extern Actor *actorListPtr;
extern int stylePoints;
extern int elapsedTime;
extern BOOL isTurboMode;
extern BOOL isSsGameMode;
extern BOOL isFsGameMode;
extern BOOL isGsGameMode;
extern int skierScreenXOffset;
extern int skierScreenYOffset;
extern PermObject *permObjects;
extern uint16_t permObjectCount;
extern DWORD currentTickCount;
extern DWORD prevTickCount;
extern DWORD timerFrameDurationInMillis;
extern BOOL isPaused;

int save_game_state(const char *filepath) {
    cJSON *root, *player_obj, *game_obj, *perms_arr, *actors_arr;
    Actor *actor;
    int i;
    char *json_str;
    FILE *f;

    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "version", 1);

    /* Player state */
    player_obj = cJSON_CreateObject();
    if (playerActor) {
        cJSON_AddNumberToObject(player_obj, "x", playerActor->xPosMaybe);
        cJSON_AddNumberToObject(player_obj, "y", playerActor->yPosMaybe);
        cJSON_AddNumberToObject(player_obj, "frameNo", playerActor->frameNo);
        cJSON_AddNumberToObject(player_obj, "isInAir", playerActor->isInAir);
        cJSON_AddNumberToObject(player_obj, "hVel", playerActor->HorizontalVelMaybe);
        cJSON_AddNumberToObject(player_obj, "vVel", playerActor->verticalVelocityMaybe);
        cJSON_AddNumberToObject(player_obj, "spriteIdx", playerActor->spriteIdx2);
        cJSON_AddNumberToObject(player_obj, "inAirCounter", playerActor->inAirCounter);
    }
    cJSON_AddItemToObject(root, "player", player_obj);

    /* Game state */
    game_obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(game_obj, "playerX", playerX);
    cJSON_AddNumberToObject(game_obj, "playerY", playerY);
    cJSON_AddNumberToObject(game_obj, "stylePoints", stylePoints);
    cJSON_AddNumberToObject(game_obj, "elapsedTime", elapsedTime);
    cJSON_AddBoolToObject(game_obj, "turbo", isTurboMode);
    cJSON_AddBoolToObject(game_obj, "ssMode", isSsGameMode);
    cJSON_AddBoolToObject(game_obj, "fsMode", isFsGameMode);
    cJSON_AddBoolToObject(game_obj, "gsMode", isGsGameMode);
    cJSON_AddNumberToObject(game_obj, "screenOffX", skierScreenXOffset);
    cJSON_AddNumberToObject(game_obj, "screenOffY", skierScreenYOffset);
    cJSON_AddItemToObject(root, "game", game_obj);

    /* Perm objects */
    perms_arr = cJSON_CreateArray();
    for (i = 0; i < permObjectCount; i++) {
        cJSON *po = cJSON_CreateObject();
        cJSON_AddNumberToObject(po, "type", permObjects[i].actorTypeMaybe);
        cJSON_AddNumberToObject(po, "frame", permObjects[i].actorFrameNo);
        cJSON_AddNumberToObject(po, "x", permObjects[i].maybeX);
        cJSON_AddNumberToObject(po, "y", permObjects[i].maybeY);
        cJSON_AddNumberToObject(po, "sprite", permObjects[i].spriteIdx);
        cJSON_AddNumberToObject(po, "vx", permObjects[i].xVelocity);
        cJSON_AddNumberToObject(po, "vy", permObjects[i].yVelocity);
        cJSON_AddNumberToObject(po, "air", permObjects[i].unk_0x18);
        cJSON_AddItemToArray(perms_arr, po);
    }
    cJSON_AddItemToObject(root, "permObjects", perms_arr);

    /* Active actors */
    actors_arr = cJSON_CreateArray();
    for (actor = actorListPtr; actor != NULL; actor = actor->next) {
        cJSON *ao = cJSON_CreateObject();
        cJSON_AddNumberToObject(ao, "type", actor->typeMaybe);
        cJSON_AddNumberToObject(ao, "frame", actor->frameNo);
        cJSON_AddNumberToObject(ao, "x", actor->xPosMaybe);
        cJSON_AddNumberToObject(ao, "y", actor->yPosMaybe);
        cJSON_AddNumberToObject(ao, "air", actor->isInAir);
        cJSON_AddNumberToObject(ao, "hVel", actor->HorizontalVelMaybe);
        cJSON_AddNumberToObject(ao, "vVel", actor->verticalVelocityMaybe);
        cJSON_AddNumberToObject(ao, "sprite", actor->spriteIdx2);
        cJSON_AddNumberToObject(ao, "flags", actor->flags);
        cJSON_AddItemToArray(actors_arr, ao);
    }
    cJSON_AddItemToObject(root, "actors", actors_arr);

    json_str = cJSON_Print(root);
    f = fopen(filepath, "w");
    if (!f) {
        cJSON_Delete(root);
        free(json_str);
        return 0;
    }
    fputs(json_str, f);
    fclose(f);

    printf("[save] Saved game state to %s\n", filepath);
    cJSON_Delete(root);
    free(json_str);
    return 1;
}

int load_game_state(const char *filepath) {
    FILE *f;
    long fsize;
    char *json_str;
    cJSON *root, *player_obj, *game_obj, *perms_arr, *actors_arr, *item;
    int i;

    f = fopen(filepath, "r");
    if (!f) {
        printf("[save] Could not open %s\n", filepath);
        return 0;
    }

    fseek(f, 0, SEEK_END);
    fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    json_str = (char *)malloc(fsize + 1);
    fread(json_str, 1, fsize, f);
    json_str[fsize] = '\0';
    fclose(f);

    root = cJSON_Parse(json_str);
    free(json_str);
    if (!root) {
        printf("[save] Failed to parse JSON\n");
        return 0;
    }

    /* Reset the game first */
    resetGame();

    /* Restore game state */
    game_obj = cJSON_GetObjectItem(root, "game");
    if (game_obj) {
        playerX = cJSON_GetObjectItem(game_obj, "playerX")->valueint;
        playerY = (short)cJSON_GetObjectItem(game_obj, "playerY")->valueint;
        stylePoints = cJSON_GetObjectItem(game_obj, "stylePoints")->valueint;
        elapsedTime = cJSON_GetObjectItem(game_obj, "elapsedTime")->valueint;
        isTurboMode = cJSON_IsTrue(cJSON_GetObjectItem(game_obj, "turbo"));
        isSsGameMode = cJSON_IsTrue(cJSON_GetObjectItem(game_obj, "ssMode"));
        isFsGameMode = cJSON_IsTrue(cJSON_GetObjectItem(game_obj, "fsMode"));
        isGsGameMode = cJSON_IsTrue(cJSON_GetObjectItem(game_obj, "gsMode"));
        skierScreenXOffset = cJSON_GetObjectItem(game_obj, "screenOffX")->valueint;
        skierScreenYOffset = cJSON_GetObjectItem(game_obj, "screenOffY")->valueint;
    }

    /* Restore perm objects */
    perms_arr = cJSON_GetObjectItem(root, "permObjects");
    if (perms_arr) {
        i = 0;
        cJSON_ArrayForEach(item, perms_arr) {
            if (i >= NUM_PERM_OBJECTS) break;
            permObjects[i].actorTypeMaybe = cJSON_GetObjectItem(item, "type")->valueint;
            permObjects[i].actorFrameNo = cJSON_GetObjectItem(item, "frame")->valueint;
            permObjects[i].maybeX = (short)cJSON_GetObjectItem(item, "x")->valueint;
            permObjects[i].maybeY = (short)cJSON_GetObjectItem(item, "y")->valueint;
            permObjects[i].spriteIdx = (short)cJSON_GetObjectItem(item, "sprite")->valueint;
            permObjects[i].xVelocity = (short)cJSON_GetObjectItem(item, "vx")->valueint;
            permObjects[i].yVelocity = (short)cJSON_GetObjectItem(item, "vy")->valueint;
            permObjects[i].unk_0x18 = (short)cJSON_GetObjectItem(item, "air")->valueint;
            permObjects[i].actor = NULL;
            i++;
        }
        permObjectCount = (uint16_t)i;
    }

    /* Setup game with loaded state then restore player */
    setupGame();

    /* Restore player position */
    player_obj = cJSON_GetObjectItem(root, "player");
    if (player_obj && playerActor) {
        short px = (short)cJSON_GetObjectItem(player_obj, "x")->valueint;
        short py = (short)cJSON_GetObjectItem(player_obj, "y")->valueint;
        short air = (short)cJSON_GetObjectItem(player_obj, "air")->valueint;
        updateActorPositionMaybe(playerActor, px, py, air);
        setActorFrameNo(playerActor, cJSON_GetObjectItem(player_obj, "frameNo")->valueint);
        playerActor->HorizontalVelMaybe = (short)cJSON_GetObjectItem(player_obj, "hVel")->valueint;
        playerActor->verticalVelocityMaybe = (short)cJSON_GetObjectItem(player_obj, "vVel")->valueint;
    }

    cJSON_Delete(root);
    printf("[save] Loaded game state from %s\n", filepath);
    return 1;
}
