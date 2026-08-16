/* ========================================================================
   $File: C:\work\handmade\code\handmade_wav.cpp $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright by Molly Rocket, Inc., All Rights Reserved. $
   ======================================================================== */

internal sound_s16 
ParseWAV(memory_arena *Memory, buffer Contents, stream *Errors)
{
    sound_s16 Result = {};
    
    riff_header Header;
    riff_iterator Iter = IterateRIFF(Contents, &Header);
    if((Header.RIFFID == RIFFID_RIFF) &&
       (Header.FileTypeID == WAVE_ChunkID_WAVE))
    {
        u32 ChannelCount = 0;
        u32 SampleDataSize = 0;
        s16 *SampleData = 0;
        while(IsValid(Iter))
        {
            switch(GetType(Iter))
            {
                case WAVE_ChunkID_fmt:
                {
                    wave_fmt *fmt = (wave_fmt *)GetChunkData(Iter);
                    if((fmt->wFormatTag == 1) && // NOTE(casey): 1 == PCM
                       (fmt->nSamplesPerSec == 48000) &&
                       (fmt->wBitsPerSample == 16) &&
                       (fmt->nBlockAlign == (sizeof(int16)*fmt->nChannels)))
                    {
                        ChannelCount = fmt->nChannels;
                    }
                    else
                    {
                        Outf(Errors, "Unsupported WAV layout: format %u, %uhz, %ubps, %u align\n",
                             fmt->wFormatTag,
                             fmt->nSamplesPerSec,
                             fmt->wBitsPerSample,
                             fmt->nBlockAlign);
                    }
                } break;
                
                case WAVE_ChunkID_data:
                {
                    SampleData = (s16 *)GetChunkData(Iter);
                    SampleDataSize = GetChunkDataSize(Iter);
                } break;
            }
            
            Iter = NextChunk(Iter);
        }
        
        if(ChannelCount && SampleData)
        {
            u32 SampleCount = SampleDataSize / (ChannelCount*sizeof(s16));
            Result = PushSound(Memory, SampleCount, ChannelCount);
            
            s16 *SourceSample = SampleData;
            for(u32 SampleIndex = 0;
                SampleIndex < SampleCount;
                ++SampleIndex)
            {
                for(u32 ChannelIndex = 0;
                    ChannelIndex < ChannelCount;
                    ++ChannelIndex)
                {
                    GetChannelSamples(Result, ChannelIndex)[SampleIndex] =
                        *SourceSample++;
                }
            }
        }
        else
        {
            Outf(Errors, "ERROR: Unrecognized WAVE data layout\n");
        }
    }
    else
    {
        Outf(Errors, "ERROR: Unable to parse WAVE header\n");
    }
    
    return(Result);
}

