#define _USE_MATH_DEFINES
#include <math.h>
#include "game.h"
#include "player.h"
#include "world.h"
#include "utility.h"

/**
 * Gets the X and Y pixel coordinates of a certain part of the player
 *
 * @param PlayerPositionData    positionData        The PlayerPositionData object to write the position data into
 * @param xyuint             playerPixelPos      The X and Y pixel coords to base the calculation on. This is the top left point
 * @param PLAYER_POINT_POS      pointPos            The offset from the playerPixelPos to apply
 * @param Player                player              The player object containing the height/width etc
 * @param World                 world               The world object
*/
void getPositionDataForPlayer(PlayerPositionData *positionData,
                                xyuint playerPixelPos,
                                PLAYER_POINT_POS pointPos,
                                Player player,
                                World world)
{
    float32 pixelInset = 0.0f;
    float32 x = 0.0f;
    float32 y = 0.0f;

    // Apply point the offsets...
    switch (pointPos)
    {
    case PLAYER_POINT_POS::RAW:
        x = (float32)playerPixelPos.x;
        y = (float32)playerPixelPos.y;
        break;
    case PLAYER_POINT_POS::TOP_LEFT:
        x = (float32)playerPixelPos.x;
        y = (float32)playerPixelPos.y;
        break;
    case PLAYER_POINT_POS::TOP_MIDDLE:
        x = ((float32)playerPixelPos.x + (float32)player.widthPx / 2.0f);
        y = ((float32)playerPixelPos.y + (float32)player.heightPx - pixelInset);
        break;
    case PLAYER_POINT_POS::MIDDLE_LEFT:
        x = (float32)playerPixelPos.x;
        y = ((float32)playerPixelPos.y + ((float32)player.heightPx / 2.0f) - pixelInset);
        break;
    case PLAYER_POINT_POS::MIDDLE:
        x = (float32)playerPixelPos.x + ((float32)player.widthPx / 2.0f);
        y = (float32)playerPixelPos.y + ((float32)player.heightPx / 2.0f);
        break;
    case PLAYER_POINT_POS::MIDDLE_RIGHT:
        x = ((float32)playerPixelPos.x + (float32)player.widthPx - pixelInset);
        y = ((float32)playerPixelPos.y + ((float32)player.heightPx / 2.0f) - pixelInset);
        break;
    case PLAYER_POINT_POS::BOTTOM_MIDDLE:
        x = ((float32)playerPixelPos.x + (float32)player.widthPx / 2.0f);
        y = ((float32)playerPixelPos.y + pixelInset);
        break;
    case PLAYER_POINT_POS::BOTTOM_RIGHT:
        x = ((float32)playerPixelPos.x + (float32)player.widthPx - pixelInset);
        y = ((float32)playerPixelPos.y + pixelInset);
        break;
    case PLAYER_POINT_POS::BOTTOM_LEFT:
        x = (float32)playerPixelPos.x;
        y = ((float32)playerPixelPos.y + pixelInset);
        break;
    default:
        assert(!"Tile point position not yet supported");
        break;
    }

    positionData->pointPosition = pointPos;

    // The absolute pixel position of the point (in relation to the world)
    positionData->activeTile.pixelCoordinates.x = (uint32)x;
    positionData->activeTile.pixelCoordinates.y = (uint32)y;

    // X and Y position coordinates of the tile (relative to the world)
    positionData->activeTile.tileIndex.x = (int32)floorf(x / (float32)world.tileWidthPx);
    positionData->activeTile.tileIndex.y = (int32)floorf(y / (float32)world.tileHeightPx);

    positionData->activeTile.tileIndex.x = (modulo(positionData->activeTile.tileIndex.x, world.totalTileDimensions));
    positionData->activeTile.tileIndex.y = (modulo(positionData->activeTile.tileIndex.y, world.totalTileDimensions));

    // Currently active tile chunk
    positionData->activeTile.chunkIndex.x = (int32)floorf((float32)x / (float32)(world.tileWidthPx * world.tileChunkDimensions));
    positionData->activeTile.chunkIndex.y = (int32)floorf((float32)y / (float32)(world.tileHeightPx * world.tileChunkDimensions));

    // @TODO(JM)
    positionData->activeTile.chunkRelativePixelCoordinates.x = 0;
    positionData->activeTile.chunkRelativePixelCoordinates.y = 0;

    // The tile relative pixel position of the point (in relation to the tile itself)
    // @TODO(JM)
    positionData->activeTile.tileRelativePixelCoordinates.x = 0;
    positionData->activeTile.tileRelativePixelCoordinates.y = 0;
    //positionData->activeTile.tileRelativePixelCoordinates.x = (positionData->activeTile.tileRelativePixelCoordinates.x - (positionData->activeTile.tileIndex.x * world.tileWidthPx));
    //positionData->activeTile.tileRelativePixelCoordinates.y = (positionData->activeTile.tileRelativePixelCoordinates.y - (positionData->activeTile.tileIndex.y * world.tileHeightPx));
}