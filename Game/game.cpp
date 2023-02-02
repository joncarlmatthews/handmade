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

#include "game.h"

EXTERN_DLL_EXPORT GAME_UPDATE(gameUpdate)
{
    /**
     * Game state initialisation
     */
    assert(sizeof(GameState) <= memory->permanentStorage.sizeInBytes);

    GameState *gameState = (GameState *)memory->permanentStorage.bytes;

    if (!memory->initialised) {

        memory->permanentStorage.bytesUsed = sizeof(GameState);
        memory->permanentStorage.bytesFree = (memory->permanentStorage.sizeInBytes - sizeof(GameState));

        // Reserve a block of the memory region for the tile chunks
        memoryRegionReserveBlock(memory->permanentStorage,
                                    &gameState->tileChunksMemoryBlock,
                                    (uint8 *)(gameState + 1),
                                     (sizet)utilMebibytesToBytes(5));

        // Reserve a block of the memory region for the tiles (used within the tile chunks)
        memoryRegionReserveBlock(memory->permanentStorage,
                                    &gameState->tilesMemoryBlock,
                                    (gameState->tileChunksMemoryBlock.endingAddress +1),
                                    (sizet)utilMebibytesToBytes(10));

        // Init the World's Tilemap
        initTilemap(&memory->permanentStorage,
                    gameState,
                    &gameState->tileChunksMemoryBlock,
                    WORLD_PIXELS_PER_METER,
                    TILE_DIMENSIONS_BIT_SHIFT,
                    TILE_CHUNK_DIMENSIONS_BIT_SHIFT,
                    TILEMAP_Z_PLANES,
                    TILE_CHUNK_TILE_DIMENSIONS_BIT_SHIFT,
                    TILE_DIMENSIONS_METERS);

        // Init the World
        gameState->world.pixelsPerMeter  = (uint16)WORLD_PIXELS_PER_METER;
        gameState->world.worldHeightPx   = (gameState->world.tilemap.tileHeightPx * gameState->world.tilemap.tileDimensions);
        gameState->world.worldWidthPx    = gameState->world.worldHeightPx;

        // The total pixel size of the world should be at least as big as one screen size
        assert(gameState->world.worldHeightPx > frameBuffer->heightPx);
        assert(gameState->world.worldWidthPx > frameBuffer->widthPx);

        // Spam some tile values into the tilemap...      
        World world         = gameState->world;
        Tilemap tilemap     = world.tilemap;
        uint randomNumberIndex = 0;

        // z-plane 0
        {
            uint32 rooms = 20;
            uint32 roomTileDims = 10;

            uint32 absTileX = 0;
            uint32 absTileY = 0;
            uint32 absTileZ = 0;

            uint32 roomStartTileX = 0;
            uint32 roomStartTileY = 0;

            bool shiftUp        = false;
            bool shiftRight     = false;

            uint doorTop        = false;
            uint doorLeft       = false;
            uint doorBottom     = false;
            uint doorRight      = false;

            for (uint32 room = 0; room < rooms; room++){

                uint32 randomNumber = (randomNumbers[randomNumberIndex] % 2);

                // Where should the starting X and Y be for this room?
                if (shiftRight) {
                    roomStartTileX = roomStartTileX + roomTileDims;
                    doorLeft = true;
                }else if (shiftUp) {
                    roomStartTileY = roomStartTileY + roomTileDims;
                    doorBottom = true;
                }

                absTileX = roomStartTileX;
                absTileY = roomStartTileY;

                switch (randomNumber)
                {
                case 0:
                default:
                    doorRight = true;
                    break;
                case 1:
                    doorTop = true;
                    break;
                }

                for (uint32 y = 0; y < roomTileDims; y++) {
                    for (uint32 x = 0; x < roomTileDims; x++) {

                        // Floor
                        uint32 tileValue = 1;

                        // Edge?
                        if (0 == x || y == 0 || x == (roomTileDims -1) || y == (roomTileDims -1)) {

                            // Wall
                            tileValue = 2;

                            // Left edge, up
                            if (0 == x) {
                                if (doorLeft){
                                    // Half way up?
                                    if ((y == (roomTileDims / 2))){
                                        tileValue = 3; // Passageway
                                    }
                                }
                            }

                            // Right edge, up
                            if (x == (roomTileDims -1)) {
                                if (doorRight){
                                    // Half way up?
                                    if ((y == (roomTileDims / 2))){
                                        tileValue = 3; // Passageway
                                    }
                                }
                            }

                            // Bottom edge, along
                            if (0 == y) {
                                if (doorBottom){
                                    // Half way along?
                                    if ((x == (roomTileDims / 2))){
                                        tileValue = 3; // Passageway
                                    }
                                }
                            }

                            // Top edge, along
                            if (y == (roomTileDims -1)) {
                                if (doorTop){
                                    // Half way along?
                                    if ((x == (roomTileDims / 2))){
                                        tileValue = 3; // Passageway
                                    }
                                }
                            }
                        }

                        setTileValue(memory->permanentStorage, gameState, absTileX, absTileY, absTileZ, tileValue);
                        absTileX++;
                    }
                    absTileX = roomStartTileX;
                    absTileY++;
                }

                switch (randomNumber)
                {
                case 0:
                default:
                    shiftRight = true;
                    shiftUp = false;
                    break;
                case 1:
                    shiftRight = false;
                    shiftUp = true;
                    break;
                }

                doorTop        = false;
                doorLeft       = false;
                doorBottom     = false;
                doorRight      = false;

                randomNumberIndex++;
            }
        }

        // z-plane 1
        {
            uint32 rooms = 8;
            uint32 roomTileDims = 10;

            uint32 absTileX = 0;
            uint32 absTileY = 0;
            uint32 absTileZ = 1;

            uint32 roomStartTileX = 0;
            uint32 roomStartTileY = 0;

            bool shiftUp        = false;
            bool shiftRight     = false;

            uint doorTop        = false;
            uint doorLeft       = false;
            uint doorBottom     = false;
            uint doorRight      = false;

            for (uint32 room = 0; room < rooms; room++){

                uint32 randomNumber = (randomNumbers[randomNumberIndex] % 2);

                // Where should the starting X and Y be for this room?
                if (shiftRight) {
                    roomStartTileX = roomStartTileX + roomTileDims;
                    doorLeft = true;
                }else if (shiftUp) {
                    roomStartTileY = roomStartTileY + roomTileDims;
                    doorBottom = true;
                }

                absTileX = roomStartTileX;
                absTileY = roomStartTileY;

                switch (randomNumber)
                {
                case 0:
                default:
                    doorRight = true;
                    break;
                case 1:
                    doorTop = true;
                    break;
                }

                for (uint32 y = 0; y < roomTileDims; y++) {
                    for (uint32 x = 0; x < roomTileDims; x++) {

                        // Floor
                        uint32 tileValue = 1;

                        // Edge?
                        if (0 == x || y == 0 || x == (roomTileDims -1) || y == (roomTileDims -1)) {

                            // Wall
                            tileValue = 2;

                            // Left edge, up
                            if (0 == x) {
                                if (doorLeft){
                                    // Half way up?
                                    if ((y == (roomTileDims / 2))){
                                        tileValue = 3; // Passageway
                                    }
                                }
                            }

                            // Right edge, up
                            if (x == (roomTileDims -1)) {
                                if (doorRight){
                                    // Half way up?
                                    if ((y == (roomTileDims / 2))){
                                        tileValue = 3; // Passageway
                                    }
                                }
                            }

                            // Bottom edge, along
                            if (0 == y) {
                                if (doorBottom){
                                    // Half way along?
                                    if ((x == (roomTileDims / 2))){
                                        tileValue = 3; // Passageway
                                    }
                                }
                            }

                            // Top edge, along
                            if (y == (roomTileDims -1)) {
                                if (doorTop){
                                    // Half way along?
                                    if ((x == (roomTileDims / 2))){
                                        tileValue = 3; // Passageway
                                    }
                                }
                            }
                        }

                        setTileValue(memory->permanentStorage, gameState, absTileX, absTileY, absTileZ, tileValue);
                        absTileX++;
                    }
                    absTileX = roomStartTileX;
                    absTileY++;
                }

                switch (randomNumber)
                {
                case 0:
                default:
                    shiftRight = true;
                    shiftUp = false;
                    break;
                case 1:
                    shiftRight = false;
                    shiftUp = true;
                    break;
                }

                doorTop        = false;
                doorLeft       = false;
                doorBottom     = false;
                doorRight      = false;

                randomNumberIndex++;
            }
        }

        // Character attributes
        gameState->player1.heightMeters  = PLAYER_HEIGHT_METERS;
        gameState->player1.widthMeters   = ((float32)gameState->player1.heightMeters * 0.65f);
        gameState->player1.heightPx  = (int16)metersToPixels(gameState->world, gameState->player1.heightMeters);
        gameState->player1.widthPx   = (int16)metersToPixels(gameState->world, gameState->player1.widthMeters);

        // Tiles should be bigger than the player.
        assert(gameState->world.tilemap.tileHeightPx > gameState->player1.heightPx);
        assert(gameState->world.tilemap.tileWidthPx > gameState->player1.widthPx);

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
    } // initialisation
    
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
    Tilemap tilemap = gameState->world.tilemap;

    for (uint32 y = 0; y < frameBuffer->heightPx; y++) {
        for (uint32 x = 0; x < frameBuffer->widthPx; x++) {

            // Calculate the x and y absolute tile index for this pixel
            uint32 absTileIndexX = (((x + gameState->cameraPositionPx.x) / tilemap.tileWidthPx) % tilemap.tileDimensions);
            uint32 absTileIndexY = (((y + gameState->cameraPositionPx.y) / tilemap.tileHeightPx) % tilemap.tileDimensions);
            uint32 absTileIndexZ = 1;

            // Get the tile chunk index based off of the absolute tile indexes
            xyzuint tileChunkIndex = getTileChunkIndexForAbsTile(absTileIndexX, absTileIndexY, absTileIndexZ, tilemap);

            // Is this tile chunk out of the sparse storage memory bounds?
            if ( (tileChunkIndex.x > (tilemap.tileChunkDimensions -1))
                || (tileChunkIndex.y > (tilemap.tileChunkDimensions -1))){
                writeRectangle(frameBuffer, x, y, 1, 1, getOutOfTileChunkMemoryBoundsColour()); // blue
                continue;
            }

            // Get the relevant tile chunk
            TileChunk* tileChunk = getTileChunkForAbsTile(absTileIndexX, absTileIndexY, absTileIndexZ, tilemap);

            // Calculate the tile chunk relative tile indexes
            xyuint chunkRelTileIndex = getChunkRelativeTileIndex(absTileIndexX, absTileIndexY, tilemap);

            // Get the individual tile
            uint32 *tileValue = (tileChunk->tiles + ((chunkRelTileIndex.y * tilemap.tileChunkTileDimensions) + chunkRelTileIndex.x));

            // Is this tile out of the sparse storage memory bounds?
            if ((gameState->tilesMemoryBlock.bytesUsed <= 0) ||
                    ( (uint8 *)tileValue > gameState->tilesMemoryBlock.lastAddressReserved
                        || (uint8 *)tileValue < gameState->tilesMemoryBlock.startingAddress) ){
                writeRectangle(frameBuffer, x, y, 1, 1, getUninitialisedTileChunkTilesColour()); // red
                continue;
            }

            // Null pointer check
            if (!tileValue) {
                continue;
            }

            Colour pixelColour = {};
            setTileColour(&pixelColour, *tileValue);

            if ( absTileIndexX == gameState->worldPosition.tileIndex.x
                && absTileIndexY == gameState->worldPosition.tileIndex.y ){
                pixelColour.r = 0.0f;
                pixelColour.g = 1.0f;
                pixelColour.b = 0.0f;
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