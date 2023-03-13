#include "game.h"

EXTERN_DLL_EXPORT GAME_UPDATE(gameUpdate)
{
    /**
     * Game state initialisation
     */
    assert(sizeof(GameState) <= memory->permanentStorage.sizeInBytes);

    GameState *gameState = (GameState *)memory->permanentStorage.bytes;

    if (!memory->initialised) {

        BitmapFile bitmapFile = {0};
        const char *filename = "C:\\Users\\jonca\\Documents\\clones\\handmade\\data\\test\\test_hero_front_head.bmp";
        DEBUGReadBMP(thread, memory->DEBUG_platformReadEntireFile, filename, &bitmapFile);

        gameState->tempBitmapFile = bitmapFile;

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
                    TILE_DIMENSIONS_METERS,
                    frameBuffer);

        // Init the World
        gameState->world.pixelsPerMeter  = (uint16)WORLD_PIXELS_PER_METER;
        gameState->world.worldHeightPx   = (gameState->world.tilemap.tileHeightPx * gameState->world.tilemap.tileDimensions);
        gameState->world.worldWidthPx    = gameState->world.worldHeightPx;

        // The total pixel size of the world should be at least as big as one screen size
        assert(gameState->world.worldHeightPx > frameBuffer->heightPx);
        assert(gameState->world.worldWidthPx > frameBuffer->widthPx);

        // Character attributes
        gameState->player1.heightMeters  = PLAYER_HEIGHT_METERS;
        gameState->player1.widthMeters   = ((float32)gameState->player1.heightMeters * 0.65f);
        gameState->player1.heightPx  = (int16)metersToPixels(gameState->world, gameState->player1.heightMeters);
        gameState->player1.widthPx   = (int16)metersToPixels(gameState->world, gameState->player1.widthMeters);

        // A single tile should be bigger than the player.
        assert(gameState->world.tilemap.tileHeightPx > gameState->player1.heightPx);
        assert(gameState->world.tilemap.tileWidthPx > gameState->player1.widthPx);

        // Movement speed (assume always running)
        gameState->player1.movementSpeedMPS = PLAYER_SPEED;
        gameState->player1.totalJumpMovement = 15.0f;

        // Initial character starting position. Start the player in the middle
        // of screen
        gameState->player1.absolutePosition.x = 280;
        gameState->player1.absolutePosition.y = 280;
        gameState->player1.zIndex = 0;
        setPlayerGamePosition(gameState, frameBuffer);

        // Calculate the currently active tile based on player1's position and
        // write it to the World Position data
        setWorldPosition(gameState, frameBuffer);

        // Spam some tile values into the tilemap...      
        World world         = gameState->world;
        Tilemap tilemap     = world.tilemap;
        uint32 randomNumberIndex = 0;

        uint32 rooms = 20;
        uint32 roomTileDims = 18;

        uint32 absTileX = 0;
        uint32 absTileY = 0;
        uint32 absTileZ = 0;

        uint32 roomStartTileX = 0;
        uint32 roomStartTileY = 0;

        bool shiftTop       = false;
        bool shiftRight     = false;
        bool shiftUp        = false;
        bool shiftDown      = false;

        uint32 doorTop        = false;
        uint32 doorLeft       = false;
        uint32 doorBottom     = false;
        uint32 doorRight      = false;
        uint32 doorUp         = false;
        uint32 doorDown       = false;

        for (uint32 room = 0; room < rooms; room++){

            // Where should the starting X and Y be for this room..?

            // What was the state of the last room?
            if (shiftRight) { // Did the last room have a door right?
                roomStartTileX = roomStartTileX + roomTileDims;
                doorLeft = true;
            }else if (shiftTop) { // Did the last room have a door top?
                roomStartTileY = roomStartTileY + roomTileDims;
                doorBottom = true;
            }else if (shiftUp) { // Did the last room have a door up?
                doorDown = true;
            }else if (shiftDown) { // Did the last room have a door up?
                doorUp = true;
            }

            absTileX = roomStartTileX;
            absTileY = roomStartTileY;

            uint32 randomNumber = (randomNumbers[randomNumberIndex] % 4);

            // Was the last move up or down? If so, next move can only be lateral
            if (shiftUp || shiftDown){
                randomNumber = (randomNumbers[randomNumberIndex] % 2);
            }

            switch (randomNumber)
            {
            case 0:
                doorRight = true;
                break;

            case 1:
                doorTop = true;
                break;

            case 2: // up
                doorUp = true;

                // Already at the highest level?
                if (absTileZ == (TILEMAP_Z_PLANES-1)){
                    doorUp      = false;
                    doorDown    = true;
                }
                break;

            case 3: // down
                doorDown = true;

                // Already at the lowest level?
                if (absTileZ == 0){
                    doorDown    = false;
                    doorUp      = true;
                }
                break;
            }

            // Final room?
            if ((rooms -1) == room){
                doorTop         = false;
                doorRight       = false;
            }

            for (uint32 y = 0; y < roomTileDims; y++) {
                for (uint32 x = 0; x < roomTileDims; x++) {

                    // Floor
                    uint32 tileValue = 1;

                    // Middle?
                    if ( ((roomTileDims / 2) == x) && ((roomTileDims / 2) == y) ){
                        // Stairwell
                        if (doorUp){
                            tileValue = 5;
                        }else if(doorDown) {
                            tileValue = 6;
                        }
                    }

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

                    setTileValue(memory->permanentStorage,
                                    gameState,
                                    absTileX,
                                    absTileY,
                                    absTileZ,
                                    tileValue);
                    absTileX++;

                } // x

                absTileX = roomStartTileX;
                absTileY++;

            } // y

            switch (randomNumber)
            {
                case 0:
                    shiftRight  = true;
                    shiftTop    = false;
                    shiftUp     = false;
                    shiftDown   = false;
                    break;
                case 1:
                    shiftRight  = false;
                    shiftTop    = true;
                    shiftUp     = false;
                    shiftDown   = false;
                    break;
                case 2: // up
                    shiftRight  = false;
                    shiftTop    = false;

                    // Already at the highest level?
                    if (absTileZ == (TILEMAP_Z_PLANES-1)){
                        --absTileZ;
                        shiftUp     = false;
                        shiftDown   = true;
                    }else{
                        ++absTileZ;
                        shiftUp     = true;
                        shiftDown   = false;
                    }
                    break;

                case 3: // down
                    shiftRight  = false;
                    shiftTop    = false;

                    // Already at the lowest level?
                    if (absTileZ == 0){
                        ++absTileZ;
                        shiftUp     = true;
                        shiftDown   = false;
                    }else{
                        --absTileZ;
                        shiftUp     = false;
                        shiftDown   = true;
                    }
                    break;
            }

            doorTop         = false;
            doorLeft        = false;
            doorBottom      = false;
            doorRight       = false;
            doorUp          = false;
            doorDown        = false;

            randomNumberIndex++;
        }        

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
    // 0 = keyboard, 1 = first controller
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
    Tilemap tilemap = gameState->world.tilemap;

    uint32 absTileIndexZ = gameState->player1.zIndex;

    xyzuint centerTileIndex = gameState->worldPosition.tileIndex;

    xyuint tileRelPos = gameState->worldPosition.tileRelativePixelPos;

    // Center tile start x and y
    xyuint centerTileStart = {};
    centerTileStart.x = ((frameBuffer->widthPx / 2) - tileRelPos.x);
    centerTileStart.y = ((frameBuffer->heightPx / 2) - tileRelPos.y);

    // Draw the tile map...
    for (int32 row = ((int32)gameState->world.tilemap.tilesPerHalfScreenY * -1); row <= (int32)gameState->world.tilemap.tilesPerHalfScreenY; row++) {
        for (int32 column = ((int32)gameState->world.tilemap.tilesPerHalfScreenX *-1); column <= (int32)gameState->world.tilemap.tilesPerHalfScreenX; column++){

            xyuint tileIndex = {
                (centerTileIndex.x + column),
                (centerTileIndex.y + row)
            };

            xyint startPixelPos = {
                ((int32)centerTileStart.x + (column * (int32)tilemap.tileWidthPx)),
                ((int32)centerTileStart.y + (row * (int32)tilemap.tileHeightPx))
            };

            // Get the tile chunk index based off of the absolute tile indexes
            xyzuint tileChunkIndex = getTileChunkIndexForAbsTile(tileIndex.x,
                                                                    tileIndex.y,
                                                                    absTileIndexZ,
                                                                    tilemap);

            // Is this tile chunk out of the sparse storage memory bounds?
            if ((tileChunkIndex.x > (tilemap.tileChunkDimensions - 1))
                    || (tileChunkIndex.y > (tilemap.tileChunkDimensions - 1))) {
                writeRectangle(frameBuffer,
                            startPixelPos.x,
                            startPixelPos.y,
                            tilemap.tileWidthPx,
                            tilemap.tileHeightPx,
                            getOutOfTileChunkMemoryBoundsColour()); // blue
                continue;
            }

            // Get the relevant tile chunk
            TileChunk *tileChunk = getTileChunkFromTileChunkIndex(tileChunkIndex,
                                                                tilemap);

            // Calculate the tile chunk relative tile indexes
            xyuint chunkRelTileIndex = getChunkRelativeTileIndex(tileIndex.x,
                                                                    tileIndex.y,
                                                                    tilemap);

            // Get the individual tile
            uint32 *tileValue = (tileChunk->tiles + ((chunkRelTileIndex.y * tilemap.tileChunkTileDimensions) + chunkRelTileIndex.x));

            // Is this tile out of the sparse storage memory bounds?
            if ((gameState->tilesMemoryBlock.bytesUsed <= 0) ||
                    ((uint8*)tileValue > gameState->tilesMemoryBlock.lastAddressReserved
                        || (uint8*)tileValue < gameState->tilesMemoryBlock.startingAddress)) {
                writeRectangle(frameBuffer,
                            startPixelPos.x,
                            startPixelPos.y,
                            tilemap.tileWidthPx,
                            tilemap.tileHeightPx,
                            getUninitialisedTileChunkTilesColour()); // red
                continue;
            }

            // Null pointer check
            if (!tileValue) {
                continue;
            }

            Colour pixelColour = {};
            setTileColour(&pixelColour, *tileValue);

            if (tileIndex.x == gameState->worldPosition.tileIndex.x
                && tileIndex.y == gameState->worldPosition.tileIndex.y) {
                pixelColour.r = 0.0f;
                pixelColour.g = 1.0f;
                pixelColour.b = 0.0f;
            }

            writeRectangle(frameBuffer,
                            startPixelPos.x,
                            startPixelPos.y,
                            tilemap.tileWidthPx,
                            tilemap.tileHeightPx,
                            pixelColour);
        }
    }

    // Draw player
    writeRectangle(frameBuffer,
                    gameState->player1.fixedPosition.x,
                    gameState->player1.fixedPosition.y,
                    gameState->player1.widthPx,
                    gameState->player1.heightPx,
                    { 0.301f, 0.156f, 0.0f });
    writeBitmap(frameBuffer,
                50,
                200,
                gameState->tempBitmapFile.widthPx,
                gameState->tempBitmapFile.heightPx,
                gameState->tempBitmapFile);

#if 0
    // Mouse input testing
    if (inputInstances->mouse.leftClick.endedDown) {
        writeRectangle(frameBuffer,
                        inputInstances->mouse.position.x,
                        inputInstances->mouse.position.y,
                        50,
                        50,
                        { 0.5f, 0.0f, 0.5f });
    }
#endif

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