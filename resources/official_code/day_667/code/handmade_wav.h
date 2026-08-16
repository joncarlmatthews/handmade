/* ========================================================================
   $File: C:\work\handmade\code\handmade_wav.h $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright by Molly Rocket, Inc., All Rights Reserved. $
   ======================================================================== */

struct sound_s16
{
    u32 SampleCount;
    u32 ChannelCount;
    s16 *Samples;
};
internal u32
GetTotalSoundSize(sound_s16 Sound)
{
    u32 Result = Sound.ChannelCount*Sound.SampleCount*sizeof(s16);
    return(Result);
}
internal sound_s16
PushSound(memory_arena *Arena, u32 SampleCount, u32 ChannelCount)
{
    sound_s16 Result;
    
    Result.SampleCount = SampleCount;
    Result.ChannelCount = ChannelCount;
    Result.Samples = (s16 *)PushSize(Arena, GetTotalSoundSize(Result));
    
    return(Result);
}

internal s16 *
GetChannelSamples(sound_s16 Sound, u32 ChannelIndex)
{
    Assert(ChannelIndex < Sound.ChannelCount);
    s16 *Result = Sound.Samples + ChannelIndex*Sound.SampleCount;
    return(Result);
}

#pragma pack(push, 1)

enum
{
    WAVE_ChunkID_fmt = RIFF_CODE('f', 'm', 't', ' '),
    WAVE_ChunkID_data = RIFF_CODE('d', 'a', 't', 'a'),
    WAVE_ChunkID_RIFF = RIFF_CODE('R', 'I', 'F', 'F'),
    WAVE_ChunkID_WAVE = RIFF_CODE('W', 'A', 'V', 'E'),
};

struct wave_fmt
{
    u16 wFormatTag;
    u16 nChannels;
    u32 nSamplesPerSec;
    u32 nAvgBytesPerSec;
    u16 nBlockAlign;
    u16 wBitsPerSample;
    u16 cbSize;
    u16 wValidBitsPerSample;
    u32 dwChannelMask;
    u8 SubFormat[16];
};

#pragma pack(pop)

internal sound_s16  ParseWAV(memory_arena *Memory, buffer Contents, stream *Errors);
