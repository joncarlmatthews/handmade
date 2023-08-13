#ifndef HEADER_HH_AUDIO
#define HEADER_HH_AUDIO

#include "game_types.h"
#include "intrinsics.h"

typedef struct GameState GameState;
typedef struct GameFrameBuffer GameFrameBuffer;
typedef struct GameAudioBuffer GameAudioBuffer;

void audioBufferWriteSineWave(GameState *gameState, GameAudioBuffer *audioBuffer);

void frameBufferWriteAudioDebug(GameState *gameState,
                                GameFrameBuffer *buffer,
                                GameAudioBuffer *audioBuffer);

#endif