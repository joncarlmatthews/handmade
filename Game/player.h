#ifndef HEADER_HH_PLAYER
#define HEADER_HH_PLAYER

#include "types.h"
#include "macros.h"
#include "utility.h"
#include "world.h"
#include "filesystem.h"
#include "math.h"

//
// Player
//====================================================

// Average male height
#define PLAYER_HEIGHT_METERS 1.7f

// Average male running speed
// https://www.calculateme.com/speed/miles-per-hour/to-meters-per-second/8
//#define PLAYER_SPEED 3.57632f
#define PLAYER_SPEED_METERS_PER_SECOND 15.57632f

typedef struct PlayerBitmap {
    BitmapFile torso;
    BitmapFile head;
    BitmapFile cape;
} PlayerBitmap;

typedef struct Player {
    // Pixel position in relation to the entire tilemap.
    // This is the starting position that the player is drawn from (bottom left).
    struct Vec2 absolutePosition;

    // The pixel position of where we consider the player to be. Which isnt
    // necessarily the bottom left (which is where we start drawing from).
    struct Vec2 gamePosition;

    // The absolute position normalised so it's relative to the screen.
    struct Vec2 canonicalAbsolutePosition;

    // Which tilemap z-plane is the player on?
    uint32 zIndex;

    // The fixed position that we draw the player from if the game scroll type
    // is set to screen scrolling
    struct Vec2 fixedPosition;

    // Player bitmaps
    PlayerBitmap bitmaps[4];
    uint32 currentBitmapIndex;

    float32 heightMeters;
    float32 widthMeters;
    uint32 heightPx;
    uint32 widthPx;
    float32 movementSpeedMPS; // Metres per second

    // @NOTE(JM) old, temp jump code
    bool8 jumping;
    float32 jumpDuration;
    float32 totalJumpMovement;
    float32 jumpRunningFrameCtr;
    float32 jumpDirection;
    int32 jumpStartPos;

} Player;

typedef enum {
    PLAYER_POINT_POS_TOP_LEFT,
    PLAYER_POINT_POS_TOP_MIDDLE,
    PLAYER_POINT_POS_TOP_RIGHT,

    PLAYER_POINT_POS_MIDDLE_LEFT,
    PLAYER_POINT_POS_MIDDLE,
    PLAYER_POINT_POS_MIDDLE_RIGHT,

    PLAYER_POINT_POS_BOTTOM_LEFT,
    PLAYER_POINT_POS_BOTTOM_MIDDLE,
    PLAYER_POINT_POS_BOTTOM_RIGHT,

    PLAYER_POINT_POS_RAW
} PlayerPointPosition;

typedef enum {
    PLAYER_JUMP_UP,
    PLAYER_JUMP_DOWN
} PlayerJumpDirection;

typedef struct PlayerPositionData {
    PlayerPointPosition pointPosition;
    TilemapPosition tilemapPosition;
} PlayerPositionData;

void getPositionDataForPlayer(PlayerPositionData *positionData,
                                struct Vec2 playerPixelPos,
                                uint32 zIndex,
                                PlayerPointPosition pointPos,
                                GameState *gameState);


typedef struct GameState GameState;
typedef struct GameFrameBuffer GameFrameBuffer;
typedef struct GameAudioBuffer GameAudioBuffer;
void setPlayerPosition(float32 absX,
                        float32 absY,
                        uint32 zIndex,
                        GameState* gameState,
                        GameFrameBuffer* frameBuffer);

typedef struct GameMemory GameMemory;
typedef struct GameInput GameInput;
void playerHandleMovement(GameState *gameState,
                            GameMemory *memory,
                            GameFrameBuffer *frameBuffer,
                            GameAudioBuffer *audioBuffer,
                            GameInput *gameInput,
                            uint8 selectedController);

internal bool playerHasSwitchedActiveTile(GameState *gameState);

#endif