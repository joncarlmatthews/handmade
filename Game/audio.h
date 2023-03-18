#ifndef HEADER_HH_AUDIO
#define HEADER_HH_AUDIO

#include "types.h"

typedef struct GameState GameState;
typedef struct GameFrameBuffer GameFrameBuffer;
typedef struct GameAudioBuffer GameAudioBuffer;

internal_func
void audioBufferWriteSineWave(GameState *gameState, GameAudioBuffer *audioBuffer);

internal_func
void frameBufferWriteAudioDebug(GameState *gameState,
                                GameFrameBuffer *buffer,
                                GameAudioBuffer *audioBuffer);

#endif