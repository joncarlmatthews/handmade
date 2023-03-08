#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "game.h"
#include "player.h"
#include "world.h"
#include "utility.h"

void playerHandleMovement(GameState *gameState,
                            GameMemory *memory,
                            GameFrameBuffer *frameBuffer,
                            GameAudioBuffer *audioBuffer,
                            GameInput *gameInput,
                            uint8 selectedController)
{
    GameControllerInput controller = gameInput->controllers[selectedController];

    // Basic player movement...

    bool playerAttemptingMove = false;

    // Normalise pixel movement regardless of framerate
    // @NOTE(JM) The truncated fractions cause issues with different framerates.
    // Not sure how to resolve at this point.
    float32 pixelsPerSecond = (gameState->world.pixelsPerMeter * gameState->player1.movementSpeedMPS);
    float32 pixelsPerFrame = (pixelsPerSecond / gameInput->fps);

    // Ensure the player can at least move!
    if (pixelsPerFrame < 1.0f){
        pixelsPerFrame = 1.0f;
    }

#if 0
    {
        char buff[400] = {};
        sprintf_s(buff, sizeof(buff),
            "MS per frame: %f. \
FPS: %f. \
Pixels per second: %f. \
Pixels per frame: %f\n",
gameInput->msPerFrame,
gameInput->fps,
pixelsPerSecond,
pixelsPerFrame);
        memory->DEBUG_platformLog(buff);
    }
#endif

    xyint playerNewPosTmp = {0};
    playerNewPosTmp.x = gameState->player1.absolutePosition.x;
    playerNewPosTmp.y = gameState->player1.absolutePosition.y;


    if (controller.dPadLeft.endedDown) {
        playerAttemptingMove = true;
        playerNewPosTmp.x += (int32)(pixelsPerFrame * -1.0f);
    }else if (controller.dPadRight.endedDown) {
        playerAttemptingMove = true;
        playerNewPosTmp.x += (int32)pixelsPerFrame;
    }else if (controller.dPadUp.endedDown) {
        playerAttemptingMove = true;
        playerNewPosTmp.y += (int32)pixelsPerFrame;
    }else if (controller.dPadDown.endedDown) {
        playerAttemptingMove = true;
        playerNewPosTmp.y += (int32)(pixelsPerFrame * -1.0f);
    }

    if (controller.up.endedDown) {
        gameState->player1.zIndex = 1;
    }else if (controller.down.endedDown) {
        gameState->player1.zIndex = 0;
    }

    if (controller.isAnalog) {

        if (controller.leftThumbstick.position.x) {
            playerAttemptingMove = true;
            if (controller.leftThumbstick.position.x >= 0.0f) {
                playerNewPosTmp.x += (int32)pixelsPerFrame;
            }else{
                playerNewPosTmp.x += (int32)(pixelsPerFrame * -1.0f);
            }
        }else if (controller.leftThumbstick.position.y) {
            playerAttemptingMove = true;
            if (controller.leftThumbstick.position.y >= 0.0f) {
                playerNewPosTmp.y += (int32)(pixelsPerFrame * -1.0f);
            }
            else {
                playerNewPosTmp.y += (int32)pixelsPerFrame;
            }
        }
    }

    if (playerAttemptingMove) {

        xyuint playerNewPos = { 0 };
        playerNewPos.x = (playerNewPosTmp.x % gameState->world.worldWidthPx);
        playerNewPos.y = (playerNewPosTmp.y % gameState->world.worldHeightPx);

        // Player movement direction
        uint32 movedUp = 0;
        uint32 movedDown = 0;
        uint32 movedLeft = 0;
        uint32 movedRight = 0;
        uint32 lastMoveDirections = 0;

        if (playerNewPos.y > gameState->player1.absolutePosition.y) {
            movedUp += (1 << 0); // UP
        }
        if (playerNewPos.y < gameState->player1.absolutePosition.y) {
            movedDown += (1 << 1); // DOWN
        }
        if (playerNewPos.x < gameState->player1.absolutePosition.x) {
            movedLeft += (1 << 2); // LEFT
        }
        if (playerNewPos.x > gameState->player1.absolutePosition.x) {
            movedRight += (1 << 3); // RIGHT
        }

        lastMoveDirections = (movedUp | movedDown | movedLeft | movedRight);

        // Tilemap collision detection
        PlayerPositionData middle;
        getPositionDataForPlayer(&middle,
            playerNewPos,
            gameState->player1.zIndex,
            PLAYER_POINT_POS::MIDDLE,
            gameState);

        PlayerPositionData bottomLeft;
        getPositionDataForPlayer(&bottomLeft,
            playerNewPos,
            gameState->player1.zIndex,
            PLAYER_POINT_POS::BOTTOM_LEFT,
            gameState);

        PlayerPositionData bottomRight;
        getPositionDataForPlayer(&bottomRight,
            playerNewPos,
            gameState->player1.zIndex,
            PLAYER_POINT_POS::BOTTOM_RIGHT,
            gameState);

#ifdef HANDMADE_DEBUG_TILE_POS
        // Visualisation
        PlayerPositionData debugPoint = middle;
#endif

        // Can the move to the new tile be taken?
        // @NOTE(JM) bug where rounding means player doesnt get a close as
        // possible to certain tiles when a move is invalid
        if ((false == (isTilemapTileFree(gameState, gameState->world.tilemap, &middle)))
            || (false == (isTilemapTileFree(gameState, gameState->world.tilemap, &bottomLeft)))
            || (false == (isTilemapTileFree(gameState, gameState->world.tilemap, &bottomRight)))) {

#if 0
#ifdef HANDMADE_DEBUG_TILE_POS
            char buff[400] = {};
            sprintf_s(buff, sizeof(buff),
                "CANNOT MOVE. \
Plr Proposed World Pos: x:%i y:%i. \
Plr Proposed World Tile: x:%i y:%i. \
Plr Actual World Pos: x:%i y:%i. \
playerNewPos.x,
playerNewPos.y,
debugPoint.activeTile.tileIndex.x,
debugPoint.activeTile.tileIndex.y,
gameState->player1.absolutePosition.x,
gameState->player1.absolutePosition.y);
            memory->DEBUG_platformLog(buff);
#endif
#endif

        } else {

            // ...yes, the move to the new tile can be taken

            // Sense check that the player actually moved
            if ( (gameState->player1.absolutePosition.x != playerNewPos.x)
                || (gameState->player1.absolutePosition.y != playerNewPos.y) ){

                gameState->player1.absolutePosition.x = playerNewPos.x;
                gameState->player1.absolutePosition.y = playerNewPos.y;
                gameState->player1.lastMoveDirections = lastMoveDirections;

                setPlayerGamePosition(gameState, frameBuffer);

                bool switchedTile = playerHasSwitchedActiveTile(gameState);

                setWorldPosition(gameState, frameBuffer);

                if (switchedTile){
                    if (5 == *gameState->worldPosition.activeTile) {
                        char buff[500] = {};
                        sprintf_s(buff,
                            sizeof(buff),
                            "STAIRWELL.\n");
                        memory->DEBUG_platformLog(buff);
                    }
                }

#ifdef HANDMADE_DEBUG_TILE_POS
                char buff[500] = {};
                sprintf_s(buff, sizeof(buff),
                    "MOVED. \
Plr World Pos x:%i y:%i. \
Abs Tile Index x:%i y:%i. \
Chunk Index x:%i y:%i. \
Chunk Rel Tile index x:%i y:%i. \
Tile Rel Pos x:%i y:%i. \
\n",
gameState->player1.absolutePosition.x, gameState->player1.absolutePosition.y,
gameState->worldPosition.tileIndex.x, gameState->worldPosition.tileIndex.y,
gameState->worldPosition.chunkIndex.x, gameState->worldPosition.chunkIndex.y,
gameState->worldPosition.chunkRelativeTileIndex.x, gameState->worldPosition.chunkRelativeTileIndex.y,
gameState->worldPosition.tileRelativePixelPos.x, gameState->worldPosition.tileRelativePixelPos.y
);
                memory->DEBUG_platformLog(buff);
#endif
            }

        }

    } // player attempting move

      // Temp jump code
#if 0
    if ((controller.down.endedDown) && (0 == gameState->player1.jumping)) {
        gameState->player1.jumping = 1;
        gameState->player1.jumpDuration = 20.0f;
        gameState->player1.jumpRunningFrameCtr = 0.0f;
        gameState->player1.jumpStartPos = gameState->player1.absolutePosition.y;
    }

    if (gameState->player1.jumping) {

        float32 jumpPercentageOfAngle = 0.0f;
        float32 jumpAngle = 0.0f;
        float32 jumpRadians = 0.0f;
        float32 jumpSine = 0.0f;

        gameState->player1.jumpRunningFrameCtr++;

        jumpPercentageOfAngle = percentageOfAnotherf(gameState->player1.jumpRunningFrameCtr, gameState->player1.jumpDuration);
        jumpAngle = (360.0f * (jumpPercentageOfAngle / 100.0f));
        jumpRadians = (jumpAngle * ((float32)M_PI / 180.0f));
        jumpSine = sinf(jumpRadians);

        if (jumpAngle >= 0.0f && jumpAngle <= 180.0f) {
            gameState->player1.jumpDirection = JUMP_UP;
        } else {
            gameState->player1.jumpDirection = JUMP_DOWN;
        }

        float32 amtToMove   = (gameState->player1.totalJumpMovement * jumpSine);
        int32 newPos        = (int32)(gameState->player1.absolutePosition.y - amtToMove);

        // Have we hit the top?
        if (newPos < 0) {
            gameState->player1.absolutePosition.y = 0;
            gameState->player1.jumpDuration = (gameState->player1.jumpRunningFrameCtr / 2);
        } else {
            gameState->player1.absolutePosition.y = newPos;
        }

        // Jump finished?
        if (jumpAngle >= 360.0f) {
            gameState->player1.jumping = 0;
            // Force reset the absolutePosition.
            // @NOTE(JM) This is a bug if you land on a new level.
            gameState->player1.absolutePosition.y = gameState->player1.jumpStartPos; 
        }
    }
#endif

}

/**
 * The absolute player position within the game state is the absolute
 * position where we start drawing the player from (bottom left). It's not
 * necessarily where we consider the player to "be" in terms of the game play.
 * This function is used when we want to get what the game considers the position
 * of the player to be.
 *
 * We also set the fixed position (for where to draw the player on screen) within
 * this function, as it needs to be relative to the game position offsets
 * 
 * @param gameState 
 * @return 
*/
void setPlayerGamePosition(GameState *gameState, GameFrameBuffer *frameBuffer)
{
    // Consider the game position as the bottom middle. With a small inset up
    // from the bottom
    uint32 offsetX = (gameState->player1.widthPx / 2);
    uint32 offsetY = (uint32)(gameState->player1.heightPx * 0.15);

    gameState->player1.gamePosition.x = (gameState->player1.absolutePosition.x + offsetX);
    gameState->player1.gamePosition.y = gameState->player1.absolutePosition.y + offsetY;

    gameState->player1.fixedPosition.x = ((frameBuffer->widthPx / 2) - offsetX);
    gameState->player1.fixedPosition.y = ((frameBuffer->heightPx / 2) - offsetY);

    return;
}

/**
* Gets the X and Y pixel coordinates of a certain part of the player
*
* @param PlayerPositionData    positionData        The PlayerPositionData object to write the position data into
* @param xyuint             playerPixelPos      The X and Y pixel coords to base the calculation on. This is the top left point
* @param PLAYER_POINT_POS      pointPos            The offset from the playerPixelPos to apply
*/
void getPositionDataForPlayer(PlayerPositionData *positionData,
                                xyuint playerPixelPos,
                                uint32 zIndex,
                                PLAYER_POINT_POS pointPos,
                                GameState *gameState)
{
    uint32 yInset = 0;
    uint32 x = 0;
    uint32 y = 0;

    // Apply point the offsets...
    switch (pointPos)
    {
    case PLAYER_POINT_POS::RAW:
        x = playerPixelPos.x;
        y = playerPixelPos.y;
        break;
    case PLAYER_POINT_POS::TOP_LEFT:
        x = playerPixelPos.x;
        y = playerPixelPos.y;
        break;
    case PLAYER_POINT_POS::TOP_MIDDLE:
        x = (playerPixelPos.x + gameState->player1.widthPx / 2);
        y = (playerPixelPos.y + gameState->player1.heightPx - yInset);
        break;
    case PLAYER_POINT_POS::MIDDLE_LEFT:
        x = playerPixelPos.x;
        y = (playerPixelPos.y + (gameState->player1.heightPx / 2) - yInset);
        break;
    case PLAYER_POINT_POS::MIDDLE:
        x = playerPixelPos.x + (gameState->player1.widthPx / 2);
        y = playerPixelPos.y + (gameState->player1.heightPx / 2);
        break;
    case PLAYER_POINT_POS::MIDDLE_RIGHT:
        x = (playerPixelPos.x + gameState->player1.widthPx - yInset);
        y = (playerPixelPos.y + (gameState->player1.heightPx / 2) - yInset);
        break;
    case PLAYER_POINT_POS::BOTTOM_MIDDLE:
        x = (playerPixelPos.x + gameState->player1.widthPx / 2);
        y = (playerPixelPos.y + yInset);
        break;
    case PLAYER_POINT_POS::BOTTOM_RIGHT:
        x = (playerPixelPos.x + gameState->player1.widthPx - yInset);
        y = (playerPixelPos.y + yInset);
        break;
    case PLAYER_POINT_POS::BOTTOM_LEFT:
        x = playerPixelPos.x;
        y = (playerPixelPos.y + yInset);
        break;
    default:
        assert(!"Tile point position not yet supported");
        break;
    }

    positionData->pointPosition = pointPos;

    setTilemapPositionData(&positionData->tilemapPosition,
                            x,
                            y,
                            zIndex,
                            gameState->world.tilemap);
}

bool playerHasSwitchedActiveTile(GameState *gameState)
{
    TilemapPosition posData = {0};
    setTilemapPositionData(&posData,
                            gameState->player1.gamePosition.x,
                            gameState->player1.gamePosition.y,
                            gameState->player1.zIndex,
                            gameState->world.tilemap);

    if ((posData.tileIndex.x != gameState->worldPosition.tileIndex.x) ||
        (posData.tileIndex.y != gameState->worldPosition.tileIndex.y)){
        return true;
    }

    return false;
}