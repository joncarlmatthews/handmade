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
// https://www.calculateme.com/speed/miles-per-hour/to-meters-per-second/19.52
#define PLAYER_SPEED 8.7262208f

typedef struct Player {
    // Position in relation to the world
    xyuint absolutePosition;

    // Position in relation to screen (for background scrolling)
    xyuint fixedPosition;

    // Position in relation to the tile chunk
    uint32 lastMoveDirections;

    float32 heightMeters;
    float32 widthMeters;
    uint16 heightPx;
    uint16 widthPx;
    float32 movementSpeedMPS; // Metre's per second

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
    TilemapCoordinates activeTile;
} PlayerPositionData;

void getPositionDataForPlayer(PlayerPositionData *positionData,
                                xyuint playerPixelPos,
                                PLAYER_POINT_POS pointPos,
                                GameState *gameState);


typedef struct GameState GameState;
typedef struct GameMemory GameMemory;
typedef struct GameFrameBuffer GameFrameBuffer;
typedef struct GameAudioBuffer GameAudioBuffer;
typedef struct GameInput GameInput;
void playerHandleMovement(GameState *gameState,
                            GameMemory *memory,
                            GameFrameBuffer *frameBuffer,
                            GameAudioBuffer *audioBuffer,
                            GameInput *gameInput,
                            uint8 selectedController);

#endif