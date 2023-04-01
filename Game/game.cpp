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

#ifdef HANDMADE_LOCAL_BUILD

        // Load the player's bitmaps...
        gameState->player1.currentBitmapIndex = 0;
        
        // Back
        DEBUGReadBMP(thread,
                        memory->DEBUG_platformReadEntireFile,
                        memory->platformAbsPath,
                        "data\\test\\test_hero_back_head.bmp",
                        &gameState->player1.bitmaps[0].head);

        DEBUGReadBMP(thread,
                        memory->DEBUG_platformReadEntireFile,
                        memory->platformAbsPath,
                        "data\\test\\test_hero_back_cape.bmp",
                        &gameState->player1.bitmaps[0].cape);

        DEBUGReadBMP(thread,
                        memory->DEBUG_platformReadEntireFile,
                        memory->platformAbsPath,
                        "data\\test\\test_hero_front_torso.bmp",
                        &gameState->player1.bitmaps[0].torso);

        // Right
        DEBUGReadBMP(thread,
                        memory->DEBUG_platformReadEntireFile,
                        memory->platformAbsPath,
                        "data\\test\\test_hero_right_head.bmp",
                        &gameState->player1.bitmaps[1].head);

        DEBUGReadBMP(thread,
                        memory->DEBUG_platformReadEntireFile,
                        memory->platformAbsPath,
                        "data\\test\\test_hero_right_cape.bmp",
                        &gameState->player1.bitmaps[1].cape);

        DEBUGReadBMP(thread,
                        memory->DEBUG_platformReadEntireFile,
                        memory->platformAbsPath,
                        "data\\test\\test_hero_right_torso.bmp",
                        &gameState->player1.bitmaps[1].torso);

        // Front
        DEBUGReadBMP(thread,
                        memory->DEBUG_platformReadEntireFile,
                        memory->platformAbsPath,
                        "data\\test\\test_hero_front_head.bmp",
                        &gameState->player1.bitmaps[2].head);

        DEBUGReadBMP(thread,
                        memory->DEBUG_platformReadEntireFile,
                        memory->platformAbsPath,
                        "data\\test\\test_hero_front_cape.bmp",
                        &gameState->player1.bitmaps[2].cape);

        DEBUGReadBMP(thread,
                        memory->DEBUG_platformReadEntireFile,
                        memory->platformAbsPath,
                        "data\\test\\test_hero_front_torso.bmp",
                        &gameState->player1.bitmaps[2].torso);

        // Left
        DEBUGReadBMP(thread,
                        memory->DEBUG_platformReadEntireFile,
                        memory->platformAbsPath,
                        "data\\test\\test_hero_left_head.bmp",
                        &gameState->player1.bitmaps[3].head);

        DEBUGReadBMP(thread,
                        memory->DEBUG_platformReadEntireFile,
                        memory->platformAbsPath,
                        "data\\test\\test_hero_left_cape.bmp",
                        &gameState->player1.bitmaps[3].cape);

        DEBUGReadBMP(thread,
                        memory->DEBUG_platformReadEntireFile,
                        memory->platformAbsPath,
                        "data\\test\\test_hero_left_torso.bmp",
                        &gameState->player1.bitmaps[3].torso);

#endif

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

    PlayerBitmap playerBitmap = gameState->player1.bitmaps[gameState->player1.currentBitmapIndex];

    writeBitmap(frameBuffer,
                gameState->player1.fixedPosition.x,
                gameState->player1.fixedPosition.y,
                playerBitmap.torso.widthPx,
                playerBitmap.torso.heightPx,
                playerBitmap.torso);

    writeBitmap(frameBuffer,
                gameState->player1.fixedPosition.x,
                gameState->player1.fixedPosition.y,
                playerBitmap.cape.widthPx,
                playerBitmap.cape.heightPx,
                playerBitmap.cape);

    writeBitmap(frameBuffer,
                gameState->player1.fixedPosition.x,
                gameState->player1.fixedPosition.y,
                playerBitmap.head.widthPx,
                playerBitmap.head.heightPx,
                playerBitmap.head);

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