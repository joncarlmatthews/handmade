// Wide-strings.
// Defines wchar_t
// @see https://www.cplusplus.com/reference/cwchar/
#include <wchar.h>

// Unicode characters.
// Defines char16_t, char32_t
// @see https://www.cplusplus.com/reference/cuchar/
#include <uchar.h>

// Common mathematical operations and transformations.
// Defines floor, floorf, M_PI
// @see https://www.cplusplus.com/reference/cmath/
#define _USE_MATH_DEFINES
#include <math.h>

// Common input/output operations.
// Defines: sprintf_s
// @see https://www.cplusplus.com/reference/cstdio/
#include <stdio.h>

#include "types.h"
#include "utility.h"
#include "game.h"
#include "world.h"
#include "tilemap.h"
#include "player.h"

EXTERN_DLL_EXPORT GAME_UPDATE(gameUpdate)
{
    /**
     * Game state initialisation
     */
    assert(sizeof(GameState) <= memory->permanentStorageSizeInBytes);

    GameState *gameState = (GameState *)memory->permanentStorage;

    if (!memory->initialised) {

        // Init the World
        initGameMemoryBlock(&gameState->worldMemoryBlock,
                            (uint8 *)(gameState + 1),
                            (sizet)(memory->permanentStorageSizeInBytes - sizeof(GameState)));

        gameState->world = (World *)GameMemoryBlockReserveStruct(&gameState->worldMemoryBlock, sizeof(World));

        // Init the Tilemap
        initTilemap(&gameState->worldMemoryBlock,
                    gameState->world,
                    WORLD_PIXELS_PER_METER,
                    TILE_DIMENSIONS_BIT_SHIFT,
                    TILE_CHUNK_DIMENSIONS_BIT_SHIFT,
                    TILE_CHUNK_TILE_DIMENSIONS_BIT_SHIFT,
                    TILE_DIMENSIONS_METERS);

        (*gameState->world).pixelsPerMeter  = (uint16)WORLD_PIXELS_PER_METER;
        (*gameState->world).worldHeightPx   = ((*gameState->world).tilemap.tileHeightPx * (*gameState->world).tilemap.tileDimensions);
        (*gameState->world).worldWidthPx    = (*gameState->world).worldHeightPx;

        // The total pixel size of the world should be at least as big as one screen size
        assert((*gameState->world).worldHeightPx > frameBuffer->heightPx);
        assert((*gameState->world).worldWidthPx > frameBuffer->widthPx);

        // Spam some tile values into the tilemap...
        World world         = *gameState->world;
        Tilemap tilemap     = world.tilemap;
        uint32 *startTile   = tilemap.tileChunks->tiles;

        uint32 rooms = 10;
        uint32 roomTileDims = 10;

        uint32 absTileX = 0;
        uint32 absTileY = 0;

        for (size_t room = 0; room < rooms; room++){

            uint32 *roomTile = startTile + (((absTileY * roomTileDims) * (tilemap.tileDimensions)) + (absTileX * roomTileDims));

            for (size_t y = 0; y < roomTileDims; y++) {
                for (size_t x = 0; x < roomTileDims; x++) {
                    if (0 == x || y == 0 || x == (roomTileDims -1) || y == (roomTileDims -1)) {
                        if ( (x == (roomTileDims / 2)) || (y == (roomTileDims / 2)) ) {
                            setTileValue(&tilemap, roomTile, 3);
                        }else {
                            setTileValue(&tilemap, roomTile, 2);
                        }
                    }else{
                        setTileValue(&tilemap, roomTile, 1);
                    }
                    roomTile += 1;
                }
                roomTile += (tilemap.tileDimensions - roomTileDims);
            }
            if (0 == (room % 3)){
                absTileY += 1;
            }else {
                absTileX += 1;

            }
        }

        // Character attributes
        gameState->player1.heightMeters  = PLAYER_HEIGHT_METERS;
        gameState->player1.widthMeters   = ((float32)gameState->player1.heightMeters * 0.65f);
        gameState->player1.heightPx  = (int16)metersToPixels((*gameState->world), gameState->player1.heightMeters);
        gameState->player1.widthPx   = (int16)metersToPixels((*gameState->world), gameState->player1.widthMeters);

        // Tiles should be bigger than the player.
        assert(gameState->world->tilemap.tileHeightPx > gameState->player1.heightPx);
        assert(gameState->world->tilemap.tileWidthPx > gameState->player1.widthPx);

        // Movement speed (assume always running)
        gameState->player1.movementSpeedMPS = PLAYER_SPEED;
        gameState->player1.totalJumpMovement = 15.0f;

        // Initial character starting position. Start the player in the middle
        // of screen
        gameState->player1.fixedPosition.x = (frameBuffer->widthPx / 2);
        gameState->player1.fixedPosition.y = (frameBuffer->heightPx / 2);
        gameState->player1.absolutePosition.x = gameState->player1.fixedPosition.x;
        gameState->player1.absolutePosition.y = gameState->player1.fixedPosition.y;
       
        // Calculate the currently active tile based on player1's position and
        // write it to the World Position data
        setWorldPosition(gameState, frameBuffer);

        memory->initialised = true;
    }

    /**
    * Handle controller input...
    */

    // Main controller loop
    // @TODO(JM) Support for multiple controllers.See below for single controller support.
    /*
    for (uint8 i = 0; i < controllerCounts->connectedControllers; i++){

        if (!inputInstances->controllers[i].isConnected) {
            continue;
        }
    }
    */

    // Which controller has the user selected as the main controller?
    uint8 userSelectedMainController = 0; // @TODO(JM) make this selectable through a UI

    playerHandleMovement(gameState,
                            memory,
                            frameBuffer,
                            audioBuffer,
                            &inputInstances[0],
                            userSelectedMainController);

    /**
     * Audio stuff...
     */

#ifdef HANDMADE_DEBUG_AUDIO
    audioBufferWriteSineWave(gameState, audioBuffer);
#endif // HANDMADE_DEBUG_AUDIO
   

    /**
     * Write the frame buffer...
     * 
     */


    // Draw the tile map.
    // @NOTE(JM) Drawing this pixel by pixel, this is incapable of hitting 60fps
    // @TODO(JM) Optimise this!!!
    Tilemap tilemap = (*gameState->world).tilemap;
    for (uint32 y = 0; y < frameBuffer->heightPx; y++) {
        for (uint32 x = 0; x < frameBuffer->widthPx; x++) {

            uint32 tilePosY = modulo(((y + gameState->cameraPositionPx.y) / tilemap.tileHeightPx), tilemap.tileDimensions);
            uint32 tilePosX = modulo(((x + gameState->cameraPositionPx.x) / tilemap.tileWidthPx), tilemap.tileDimensions);

            // Out of sparse storage memory bounds?
            if ((tilePosX > tilemap.tilesStoredDimensions) || (tilePosY > tilemap.tilesStoredDimensions)) {
                writeRectangle(frameBuffer, x, y, 1, 1, { (230.0f/255.0f), 0.f, 0.f });
                continue;
            }

            uint32 *tileValue = (tilemap.tileChunks->tiles + ((tilePosY * tilemap.tileDimensions) + tilePosX));

            // Null pointer check
            if (!tileValue) {
                continue;
            }

            Colour pixelColour = {};
            setTileColour(&pixelColour, *tileValue);

            if ( tilePosX == gameState->worldPosition.tileIndex.x
                && tilePosY == gameState->worldPosition.tileIndex.y ){
                pixelColour.r = 0.0f;
                pixelColour.g = 0.6f;
                pixelColour.b = 0.2f;
            }

            writeRectangle(frameBuffer, x, y, 1, 1, pixelColour);
        }
    }

    // Draw player
    writeRectangle(frameBuffer,
                    gameState->player1.fixedPosition.x,
                    gameState->player1.fixedPosition.y,
                    gameState->player1.widthPx,
                    gameState->player1.heightPx,
                    { 0.301f, 0.156f, 0.0f });

    // Mouse input testing
    if (inputInstances->mouse.leftClick.endedDown) {
        writeRectangle(frameBuffer,
                        inputInstances->mouse.position.x,
                        inputInstances->mouse.position.y,
                        50,
                        50,
                        { 0.5f, 0.0f, 0.5f });
    }

#if defined(HANDMADE_DEBUG_AUDIO)
    frameBufferWriteAudioDebug(gameState, frameBuffer, audioBuffer);
#endif

}

#if defined(HANDMADE_DEBUG_AUDIO)

internal_func void frameBufferWriteAudioDebug(GameState *gameState, GameFrameBuffer *buffer, GameAudioBuffer *audioBuffer)
{
    float32 coefficient = ((float32)buffer->widthPx / (float32)audioBuffer->platformBufferSizeInBytes);

    // Audio buffer box
    {
        uint16 height = 100;
        uint16 width = (uint16)((float32)audioBuffer->platformBufferSizeInBytes * coefficient);
        uint32 yOffset = 100;
        writeRectangle(buffer, 0x3333ff, height, width, yOffset, 0);
    }

    // Play cursor (green)
    {
        uint16 height = 100;
        uint16 width = 10;
        uint32 yOffset = 100;
        uint32 xOffset = (uint32)((float32)audioBuffer->playCursorPosition * coefficient);
        writeRectangle(buffer, 0x669900, height, width, yOffset, xOffset);
    }

    // Write cursor + lock size (amount written) (red)
    {
        uint16 height = 100;
        uint32 width = (uint32)((float32)audioBuffer->lockSizeInBytes * coefficient);
        uint32 yOffset = 100;
        uint32 xOffset = (uint32)((float32)audioBuffer->writeCursorPosition * coefficient);
        writeRectangle(buffer, 0xcc0000, height, width, yOffset, xOffset);
    }
}

#endif

/**
 * Simple pixel loop.
 *
 * x = along the corridoor, y = up the stairs.
 *
 * Therefore x is concerned with the screen buffer's width,
 * and y is concerned with the screen buffer's height.
 * 
 */
internal_func
void writeRectangle(GameFrameBuffer *buffer,
                    int64 xOffset,
                    int64 yOffset,
                    int64 width,
                    int64 height,
                    Colour colour)
{
    // Bounds checking
    if (xOffset >= buffer->widthPx) {
        return;
    }

    if (yOffset >= buffer->heightPx) {
        return;
    }

    // Min x
    if (xOffset < 0) {
        width = (width - (xOffset * -1));
        if (width <= 0) {
            return;
        }
        xOffset = 0;
    }

    // Min y
    if (yOffset < 0) {
        height = (height - (yOffset * -1));
        if (height <= 0) {
            return;
        }
        yOffset = 0;
    }

    // Max x
    int64 maxX = (xOffset + width);

    if (maxX > buffer->widthPx) {
        maxX = (buffer->widthPx - xOffset);
        if (width > maxX) {
            width = maxX;
        }
    }

    // Max y
    int64 maxY = (yOffset + height);

    if (maxY > buffer->heightPx) {
        maxY = (buffer->heightPx - yOffset);
        if (height > maxY) {
            height = maxY;
        }
    }

    // Set the colour
    uint32 alpha    = ((uint32)(255.0f * colour.a) << 24);
    uint32 red      = ((uint32)(255.0f * colour.r) << 16);
    uint32 green    = ((uint32)(255.0f * colour.g) << 8);
    uint32 blue     = ((uint32)(255.0f * colour.b) << 0);

    uint32 hexColour = (alpha | red | green | blue);

    // Write the memory
    uint32 *row = (uint32*)buffer->memory;

    // Move to last row as starting position (bottom left of axis)
    row = (row + ((buffer->widthPx * buffer->heightPx) - buffer->widthPx));

    // Move up to starting row
    row = (row - (buffer->widthPx * yOffset));

    // Move in from left to starting absolutePosition
    row = (row + xOffset);

    // Up (rows)
    for (int64 i = 0; i < height; i++) {

        // Accross (columns)
        uint32 *pixel = (uint32*)row;
        for (int64 x = 0; x < width; x++) {

            *pixel = hexColour;
            pixel = (pixel + 1);
        }

        // Move up one entire row
        row = (row - buffer->widthPx);
    }
}

internal_func
void audioBufferWriteSineWave(GameState* gameState, GameAudioBuffer* audioBuffer)
{
    gameState->sineWave.hertz = 100;
    gameState->sineWave.sizeOfWave = 1000; // Volume

    // Calculate the total number of 4-byte audio sample groups that we will have per complete cycle.
    uint64 audioSampleGroupsPerCycle = ((audioBuffer->platformBufferSizeInBytes / audioBuffer->bytesPerSample) / gameState->sineWave.hertz);

    // At the start of which 4 byte group index we are starting our write from?
    // @TODO(JM) assert that this is a 4 byte boundry
    uint32 byteGroupIndex = 0;

    float32 percentageOfAngle = 0.0f;
    float32 angle = 0.0f;
    float32 radians = 0.0f;
    float64 sine = 0.0f;

    uint16 *audioSample = (uint16*)audioBuffer->memory;

    // Iterate over each 2 - bytes and write the same data for both...
    for (uint32 i = 0; i < audioBuffer->noOfSamplesToWrite; i++) {

        percentageOfAngle = percentageOfAnotherf((float32)byteGroupIndex, (float32)audioSampleGroupsPerCycle);
        angle = (360.0f * (percentageOfAngle / 100.0f));
        radians = (angle * ((float32)M_PI / 180.0f));
        sine = sin(radians);

        int16 audioSampleValue = (int16)(sine * gameState->sineWave.sizeOfWave);

        // Left channel (16-bits)
        *audioSample = audioSampleValue;

        // Move to the right sample (16-bits)
        audioSample++;

        // Right channel (16-bits)
        *audioSample = audioSampleValue;

        // Move cursor to the start of the next sample grouping.
        audioSample++;

        // Write another 4 to the running byte group index.
        byteGroupIndex = (uint32)((uint64)(byteGroupIndex + audioBuffer->bytesPerSample) % audioSampleGroupsPerCycle);
    }
}

void initGameMemoryBlock(GameMemoryBlock *memoryBlock,
                            uint8 *startingAddress,
                            sizet maximumSizeInBytes)
{
    memoryBlock->startingAddress    = startingAddress;
    memoryBlock->totalSizeInBytes   = maximumSizeInBytes;
    memoryBlock->bytesUsed          = 0;
    memoryBlock->bytesFree          = maximumSizeInBytes;
}

void *GameMemoryBlockReserveStruct(GameMemoryBlock *memoryBlock, sizet structSize)
{
    void *startingAddress = memoryBlock->startingAddress + memoryBlock->bytesUsed;

    sizet bytesToReserve = structSize;

    assert(bytesToReserve <= memoryBlock->bytesFree);

    memoryBlock->bytesUsed = (memoryBlock->bytesUsed + structSize);
    memoryBlock->bytesFree = (memoryBlock->bytesFree - structSize);

    assert(memoryBlock->bytesUsed <= memoryBlock->totalSizeInBytes);

    return startingAddress;
}
void *GameMemoryBlockReserveArray(GameMemoryBlock *memoryBlock, sizet typeSize, sizet noOfElements)
{
    void *startingAddress = memoryBlock->startingAddress + memoryBlock->bytesUsed;

    sizet bytesToReserve = (typeSize * noOfElements);

    assert(bytesToReserve <= memoryBlock->bytesFree);

    memoryBlock->bytesUsed = (memoryBlock->bytesUsed + bytesToReserve);
    memoryBlock->bytesFree = (memoryBlock->bytesFree - bytesToReserve);

    assert(memoryBlock->bytesUsed <= memoryBlock->totalSizeInBytes);

    return startingAddress;
}

EXTERN_DLL_EXPORT GAME_INIT_FRAME_BUFFER(gameInitFrameBuffer)
{
    frameBuffer->heightPx = height;
    frameBuffer->widthPx = width;
    frameBuffer->bytesPerPixel = bytesPerPixel;
    frameBuffer->byteWidthPerRow = byteWidthPerRow;
    frameBuffer->memory = memory;

    return frameBuffer;
}

EXTERN_DLL_EXPORT GAME_INIT_AUDIO_BUFFER(gameInitAudioBuffer)
{
    if ((noOfBytesToWrite <= 0) || (bytesPerSample <= 0)) {
        return audioBuffer;
    }

    uint32 noOfSamplesToWrite = (noOfBytesToWrite / bytesPerSample);

    if (audioBuffer->noOfSamplesToWrite != noOfSamplesToWrite) {

        // @TODO(JM) move the audio memory to the GameMemory object
        if (!audioBuffer->initialised) {
            audioBuffer->initialised = 1;
            audioBuffer->memory = memory->platformAllocateMemory(thread, 0, noOfBytesToWrite);
        } else {
            memory->platformFreeMemory(thread, audioBuffer->memory);
            audioBuffer->memory = memory->platformAllocateMemory(thread, 0, noOfBytesToWrite);
        }
    }
    audioBuffer->bytesPerSample = bytesPerSample;
    audioBuffer->noOfSamplesToWrite = noOfSamplesToWrite;
    audioBuffer->platformBufferSizeInBytes = platformBufferSizeInBytes;

    return audioBuffer;
}