#ifndef HEADER_HH_PLAYER
#define HEADER_HH_PLAYER

#include "types.h"
#include "world.h"

//
// Player
//====================================================

// Average male height
#define PLAYER_HEIGHT_METERS 1.7f

// Average male running speed
// https://www.calculateme.com/speed/miles-per-hour/to-meters-per-second/8
//#define PLAYER_SPEED 3.57632f
#define PLAYER_SPEED 15.0f

typedef struct Player {
    // Pixel position in relation to the entire tilemap.
    // This is the starting position that the player is drawn from (bottom left).
    xyuint absolutePosition;

    // The pixel position of where we consider the player to be. Which isnt
    // necessarily the bottom left.
    xyuint gamePosition;

    // Which tilemap z-plane is the player on?
    uint32 zIndex;

    // The fixed position that we draw the player from. @NOTE(JM) Should we move
    // this to some place else other than the player?
    xyuint fixedPosition;

    // Last direction the player moved in (up, down, left, right)
    uint32 lastMoveDirections;

    float32 heightMeters;
    float32 widthMeters;
    uint16 heightPx;
    uint16 widthPx;
    float32 movementSpeedMPS; // Metres per second

    // @NOTE(JM) old, temp jump code
    bool8 jumping;
    float32 jumpDuration;
    float32 totalJumpMovement;
    float32 jumpRunningFrameCtr;
    float32 jumpDirection;
    int32 jumpStartPos;

} Player;

enum class PLAYER_POINT_POS {
    TOP_LEFT,
    TOP_MIDDLE,
    TOP_RIGHT,
    MIDDLE_LEFT,
    MIDDLE,
    MIDDLE_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_MIDDLE,
    BOTTOM_RIGHT,
    RAW,
};

enum class PlayerMovementDirection {
    NONE = 0,
    UP = 1,
    DOWN = 2,
    LEFT = 4,
    RIGHT = 8
};

enum jumpDirection {
    JUMP_UP,
    JUMP_DOWN
};

typedef struct PlayerPositionData {
    PLAYER_POINT_POS pointPosition;
    TilemapPosition activeTile;
} PlayerPositionData;

void getPositionDataForPlayer(PlayerPositionData *positionData,
                                xyuint playerPixelPos,
                                uint32 zIndex,
                                PLAYER_POINT_POS pointPos,
                                GameState *gameState);


typedef struct GameState GameState;
typedef struct GameFrameBuffer GameFrameBuffer;
typedef struct GameAudioBuffer GameAudioBuffer;
void setPlayerGamePosition(GameState *gameState, GameFrameBuffer *frameBuffer);

typedef struct GameMemory GameMemory;
typedef struct GameInput GameInput;
void playerHandleMovement(GameState *gameState,
                            GameMemory *memory,
                            GameFrameBuffer *frameBuffer,
                            GameAudioBuffer *audioBuffer,
                            GameInput *gameInput,
                            uint8 selectedController);

#endif