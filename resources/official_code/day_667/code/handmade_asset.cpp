/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

struct load_asset_work
{
    task_with_memory *Task;
    asset *Asset;

    platform_file_handle *Handle;
    u64 Offset;
    u64 Size;
    void *Destination;

    u32 FinalState;

    texture_op *TextureOp;
    renderer_texture_queue *TextureOpQueue;
};

internal asset_tag_hash
GetTagHash(asset_basic_category Cat,
           asset_tag_id E0 = Tag_None,
           asset_tag_id E1 = Tag_None,
           asset_tag_id E2 = Tag_None,
           asset_tag_id E3 = Tag_None)
{
    asset_tag_hash Result =
    {
        ((u64)Cat <<  0) |
            ((u64)E0  <<  8) |
            ((u64)E1  << 22) |
            ((u64)E2  << 36) |
            ((u64)E3  << 50)
    };
    
    return(Result);
}

internal asset_tag_hash
GetTagHash(asset_basic_category Cat, asset_tag_hash Tags)
{
    asset_tag_hash Result =
    {
        ((u64)Cat <<  0) | (Tags.Value & ~0xff)
    };
    
    return(Result);
}

internal asset_basic_category
GetCategoryFrom(asset_tag_hash Hash)
{
    asset_basic_category Result = (asset_basic_category)(Hash.Value & 0xff);
    return(Result);
}

internal
PLATFORM_WORK_QUEUE_CALLBACK(LoadAssetWork)
{
    TIMED_FUNCTION();

    asset_state ResultingState = AssetState_Unloaded;
    load_asset_work *Work = (load_asset_work *)Data;

    Platform.ReadDataFromFile(Work->Handle, Work->Offset, Work->Size, Work->Destination);
    if(PlatformNoFileErrors(Work->Handle))
    {
        ResultingState = AssetState_Loaded;
    }
    else
    {
        ZeroSize(Work->Size, Work->Destination);
    }

    if(Work->TextureOp && Work->TextureOp->GenMipMaps)
    {
        GenerateSequentialMIPs(Work->TextureOp->Texture.Width,
                               Work->TextureOp->Texture.Height,
                               Work->TextureOp->Data);
    }


    CompletePreviousWritesBeforeFutureWrites;
    if(Work->TextureOp)
    {
        CompleteTextureOp(Work->TextureOpQueue, Work->TextureOp);
    }

    Work->Asset->State = ResultingState;
    EndTaskWithMemory(Work->Task);
}

internal asset_file *
GetFile(game_assets *Assets, u32 FileIndex)
{
    Assert(FileIndex < Assets->FileCount);
    asset_file *Result = Assets->Files + FileIndex;

    return(Result);
}

internal platform_file_handle *
GetFileHandleFor(game_assets *Assets, u32 FileIndex)
{
    platform_file_handle *Result = &GetFile(Assets, FileIndex)->Handle;

    return(Result);
}

internal b32x
DimensionsRequireSpecialTexture(game_assets *Assets, u32 Width, u32 Height)
{
    // TODO(casey): In the future, probably make this dynamic!
    b32x Result = ((Width > TEXTURE_ARRAY_DIM) ||
                   (Height > TEXTURE_ARRAY_DIM));
    return(Result);
}

internal v2
GetUVScaleForBitmap(game_assets *Assets, u32 Width, u32 Height)
{
    v2 Result = {1, 1};
    if(!DimensionsRequireSpecialTexture(Assets, Width, Height))
    {
        Result.x = (f32)Width / (f32)TEXTURE_ARRAY_DIM;
        Result.y = (f32)Height / (f32)TEXTURE_ARRAY_DIM;
    }

    return(Result);
}

internal void
UnloadBitmap(game_assets *Assets, bitmap_id ID)
{
    asset *Asset = Assets->Assets + ID.Value;
    if(ID.Value)
    {
        if(AtomicCompareExchangeUInt32((uint32 *)&Asset->State, AssetState_Queued, AssetState_Loaded) ==
           AssetState_Loaded)
        {
            Clear(&Asset->TextureHandle);

            CompletePreviousWritesBeforeFutureWrites;

            Asset->State = AssetState_Unloaded;
        }
    }
}

internal void
UnloadAudio(game_assets *Assets, sound_id ID)
{
    asset *Asset = Assets->Assets + ID.Value;
    if(ID.Value)
    {
        if(AtomicCompareExchangeUInt32((uint32 *)&Asset->State, AssetState_Queued, AssetState_Loaded) ==
           AssetState_Loaded)
        {
            CompletePreviousWritesBeforeFutureWrites;

            Asset->State = AssetState_Unloaded;
        }
    }
}

internal u32
AcquireTextureHandle(game_assets *Assets, b32x Special)
{
    asset_lru_link *ReplaceSentinel = 0;

    u32 Result = 0;
    if(Special)
    {
        if(Assets->NextSpecialTextureHandle < Assets->SpecialTextureHandleCount)
        {
            Result = SpecialTextureIndexFrom(Assets->NextSpecialTextureHandle++);
        }
        else
        {
            ReplaceSentinel = &Assets->SpecialTextureLRUSentinel;
        }
    }
    else
    {
        if(Assets->NextFreeTextureHandle < Assets->NormalTextureHandleCount)
        {
            Result = Assets->NextFreeTextureHandle++;
        }
        else
        {
            ReplaceSentinel = &Assets->RegularTextureLRUSentinel;
        }
    }

    if(ReplaceSentinel)
    {
        Assert(!DLIST_IS_EMPTY(ReplaceSentinel));

        asset_lru_link *First = ReplaceSentinel->Next;
        DLIST_REMOVE(First);
        asset *ReplaceAsset = (asset *)First;
        ReplaceAsset->State = AssetState_Unloaded;
        Result = ReplaceAsset->TextureHandle.Index;

        Clear(&ReplaceAsset->TextureHandle);
    }

    return(Result);
}

internal void
LoadBitmap(game_assets *Assets, bitmap_id ID)
{
    TIMED_FUNCTION();

    asset *Asset = Assets->Assets + ID.Value;
    if(ID.Value)
    {
        if(AtomicCompareExchangeUInt32((uint32 *)&Asset->State, AssetState_Queued, AssetState_Unloaded) ==
           AssetState_Unloaded)
        {
            hha_bitmap *Info = &Asset->HHA.Bitmap;
            u32 Width = Info->Dim[0];
            u32 Height = Info->Dim[1];

            b32 Special = DimensionsRequireSpecialTexture(Assets, Width, Height);
            b32 GenMipMaps = !Special;

            u32 TopLevelSize = Width*Height*4;
            u32 SizeRequested = TopLevelSize;
            if(GenMipMaps)
            {
                SizeRequested = GetTotalSizeForMIPs(Width, Height);
            }

            texture_op *TextureOp = BeginTextureOp(Assets->TextureOpQueue, SizeRequested);
            if(TextureOp)
            {
                task_with_memory *Task = BeginTaskWithMemory(Assets->GameState, false);
                if(Task)
                {
                    Clear(&Asset->TextureHandle);

                    u32 TextureHandle = AcquireTextureHandle(Assets, Special);
                    Asset->TextureHandle = ReferToTexture(TextureHandle, Width, Height);
                    TextureOp->Texture = Asset->TextureHandle;
                    TextureOp->GenMipMaps = GenMipMaps;

                    load_asset_work *Work = PushStruct(&Task->Arena, load_asset_work, NoClear());
                    Work->Task = Task;
                    Work->Asset = Asset;
                    Work->Handle = GetFileHandleFor(Assets, Asset->FileIndex);
                    Work->Offset = Asset->HHA.DataOffset;
                    Work->Size = TopLevelSize;
                    Work->Destination = TextureOp->Data;
                    Work->FinalState = AssetState_Loaded;
                    Work->TextureOp = TextureOp;
                    Work->TextureOpQueue = Assets->TextureOpQueue;

                    Platform.AddEntry(Assets->GameState->LowPriorityQueue, LoadAssetWork, Work);
                }
                else
                {
                    CancelTextureOp(Assets->TextureOpQueue, TextureOp);
                    Asset->State = AssetState_Unloaded;
                }
            }
            else
            {
                Asset->State = AssetState_Unloaded;
            }
        }
    }
}

internal asset_sound_buffer_ranges
GetSoundBufferRanges(game_assets *Assets)
{
    asset_sound_buffer_ranges Result;
    Result.SoundBufferBaseIndex = Assets->SampleBufferTopIndex - Assets->SampleBufferSize;
    Result.SoundBufferLRUIndex = Result.SoundBufferBaseIndex + Assets->SampleBufferLRURange;
    return(Result);
};

internal void *
GetSoundBufferMemory(game_assets *Assets, u64 LoadedAt)
{
    u64 Mask = Assets->SampleBufferMappingMask;
    void *Result = Assets->SampleBuffer + (LoadedAt & Mask);
    return(Result);
}

internal sound_buffer_memory
ReserveSoundMemory(game_assets *Assets, u32 DataSize)
{
    u64 Mask = Assets->SampleBufferMappingMask;
    u32 SampleBufferIndex = (u32)(Assets->SampleBufferTopIndex & Mask);
    if((SampleBufferIndex + DataSize) > Assets->SampleBufferSize)
    {
        SampleBufferIndex = 0;
        Assets->SampleBufferTopIndex = (Assets->SampleBufferTopIndex + DataSize) & ~Mask;
    }

    sound_buffer_memory Result;
    Result.BufferIndex = Assets->SampleBufferTopIndex;
    Result.Pointer = (Assets->SampleBuffer + SampleBufferIndex);
    Assert(Result.Pointer == GetSoundBufferMemory(Assets, Result.BufferIndex));
    Assert(((u8 *)Result.Pointer + DataSize) <= (Assets->SampleBuffer + Assets->SampleBufferSize));

    Assets->SampleBufferTopIndex += DataSize;

    return(Result);
}

internal void
InitSoundMemory(game_assets *Assets, memory_arena *Arena)
{
    u32 SampleBufferSize = 256*1024*1024;
    Assets->SampleBufferSize = SampleBufferSize;
    Assets->SampleBuffer = (u8 *)PushSize(Arena, SampleBufferSize);

    Assets->SampleBufferMappingMask = (u64)SampleBufferSize - 1ULL;
    Assets->SampleBufferTopIndex = 2*SampleBufferSize;
    Assets->SampleBufferLRURange = 16*1024*1024;
}

internal void
LoadSound(game_assets *Assets, sound_id ID)
{
    TIMED_FUNCTION();

    asset *Asset = Assets->Assets + ID.Value;
    if(ID.Value &&
       (AtomicCompareExchangeUInt32((uint32 *)&Asset->State, AssetState_Queued, AssetState_Unloaded) ==
        AssetState_Unloaded))
    {
        task_with_memory *Task = BeginTaskWithMemory(Assets->GameState, false);
        if(Task)
        {
            sound_buffer_memory Memory = ReserveSoundMemory(Assets, Asset->HHA.DataSize);
            Asset->LoadedAtSoundBufferIndex = Memory.BufferIndex;

            load_asset_work *Work = PushStruct(&Task->Arena, load_asset_work);
            Work->Task = Task;
            Work->Asset = Assets->Assets + ID.Value;
            Work->Handle = GetFileHandleFor(Assets, Asset->FileIndex);
            Work->Offset = Asset->HHA.DataOffset;
            Work->Size = Asset->HHA.DataSize;
            Work->Destination = Memory.Pointer;
            Work->FinalState = (AssetState_Loaded);

            Platform.AddEntry(Assets->GameState->LowPriorityQueue, LoadAssetWork, Work);
        }
        else
        {
            Assets->Assets[ID.Value].State = AssetState_Unloaded;
        }
    }
}

internal void
LoadFont(game_assets *Assets, font_id ID)
{
    if(ID.Value)
    {
        asset *Asset = Assets->Assets + ID.Value;
        Assert(Asset->State == AssetState_Unloaded);
        
        hha_font *Info = &Asset->HHA.Font;
        
        u32 HorizontalAdvanceSize = sizeof(r32)*Info->GlyphCount*Info->GlyphCount;
        u32 GlyphsSize = Info->GlyphCount*sizeof(hha_font_glyph);
        u32 UnicodeMapSize = sizeof(u16)*Info->OnePastHighestCodepoint;
        u32 SizeData = GlyphsSize + HorizontalAdvanceSize;
        u32 SizeTotal = SizeData + UnicodeMapSize;
        
        temporary_memory MemoryPoint = BeginTemporaryMemory(&Assets->NonRestoredMemory);
        u8 *Memory = (u8 *)PushSize(&Assets->NonRestoredMemory, SizeTotal);
        
        loaded_font *Font = &Asset->Font;
        Font->BitmapIDOffset = GetFile(Assets, Asset->FileIndex)->AssetBase;
        Font->Glyphs = (hha_font_glyph *)(Memory);
        Font->HorizontalAdvance = (r32 *)((u8 *)Font->Glyphs + GlyphsSize);
        Font->UnicodeMap = (u16 *)((u8 *)Font->HorizontalAdvance + HorizontalAdvanceSize);
        
        ZeroSize(UnicodeMapSize, Font->UnicodeMap);
        
        platform_file_handle *FileHandle = GetFileHandleFor(Assets, Asset->FileIndex);
        Platform.ReadDataFromFile(FileHandle, Asset->HHA.DataOffset, SizeData, Font->Glyphs);
        if(PlatformNoFileErrors(FileHandle))
        {
            hha_font *HHA = &Asset->HHA.Font;
            for(u32 GlyphIndex = 1;
                GlyphIndex < HHA->GlyphCount;
                ++GlyphIndex)
            {
                hha_font_glyph *Glyph = Font->Glyphs + GlyphIndex;
                
                Assert(Glyph->UnicodeCodePoint < HHA->OnePastHighestCodepoint);
                Assert((u32)(u16)GlyphIndex == GlyphIndex);
                Font->UnicodeMap[Glyph->UnicodeCodePoint] = (u16)GlyphIndex;
            }
            
            Asset->State = AssetState_Loaded;
            KeepTemporaryMemory(MemoryPoint);
        }
        else
        {
            EndTemporaryMemory(MemoryPoint);
        }
    }
}

internal u32
GetAssetFrom(game_assets *Assets, asset_tag_hash TagHash, asset_match_vector MatchVector)
{
    u32 HashSlot = GetAssetHashSlot(Assets, TagHash);

    // TODO(casey): The matching test could be done potentially quite efficiently
    // in SIMD.
    f32 BestMatch = 0.0f;
    u32 Result = 0;
    u32 CheckIndex = Assets->AssetsByTag[HashSlot];
    while(CheckIndex)
    {
        asset *Check = GetAsset(Assets, CheckIndex);
        if(TagHash.Value == Check->TagHash.Value)
        {
            f32 TotalMatch = 0.0f;
            for(u32 EIndex = 0; EIndex < ArrayCount(MatchVector.E); ++EIndex)
            {
                f32 A = MatchVector.E[EIndex];
                f32 B = Check->MatchVector.E[EIndex];
                f32 D0 = AbsoluteValue(A - B);
                f32 Difference = 1.0f - D0;
                TotalMatch += Difference;
            }

            if(BestMatch < TotalMatch)
            {
                BestMatch = TotalMatch;
                Result = CheckIndex;
            }
        }

        CheckIndex = Check->NextInTagHash;
    }

    return(Result);
}

inline bitmap_id
GetBestMatchBitmapFrom(game_assets *Assets, asset_tag_hash TagHash, asset_match_vector MatchVector = {})
{
    bitmap_id Result = {GetAssetFrom(Assets, TagHash, MatchVector)};
    return(Result);
}

inline sound_id
GetBestMatchSoundFrom(game_assets *Assets, asset_tag_hash TagHash, asset_match_vector MatchVector = {})
{
    sound_id Result = {GetAssetFrom(Assets, TagHash, MatchVector)};
    return(Result);
}

internal font_id
GetBestMatchFontFrom(game_assets *Assets, asset_tag_hash TagHash, asset_match_vector MatchVector = {})
{
    font_id Result = {GetAssetFrom(Assets, TagHash, MatchVector)};
    return(Result);
}

internal asset *
GetAsset(game_assets *Assets, u32 ID)
{
    Assert(ID <= Assets->AssetCount);
    asset *Asset = Assets->Assets + ID;

    return(Asset);
}

internal hha_tag *
GetTag(game_assets *Assets, u32 ID)
{
    Assert(ID <= Assets->TagCount);
    hha_tag *Tag = Assets->Tags + ID;

    return(Tag);
}

internal renderer_texture
GetBitmap(game_assets *Assets, bitmap_id ID)
{
    asset *Asset = GetAsset(Assets, ID.Value);

    renderer_texture Result = {};
    if(Asset)
    {
        Assert((ID.Value == 0) || (Asset->HHA.Type == HHAAsset_Bitmap));
        Result = Asset->TextureHandle;
        if(IsValid(Result))
        {
            // TODO(casey): This check can eventually be elided with the check
            // done to determine if we need to switch to special texture mode,
            // so I _think_ this is OK in the long run!
            DLIST_REMOVE(&Asset->LRU);
            hha_bitmap *Info = &Asset->HHA.Bitmap;
            if(DimensionsRequireSpecialTexture(Assets, Info->Dim[0], Info->Dim[1]))
            {
                DLIST_INSERT_AS_LAST(&Assets->SpecialTextureLRUSentinel, &Asset->LRU);
            }
            else
            {
                DLIST_INSERT_AS_LAST(&Assets->RegularTextureLRUSentinel, &Asset->LRU);
            }
            // TODO(casey): This writes way more than it would have to to just keep
            // a generation number (6 pointer writes at minimum for this, whereas)
            // just a single 64-bit write for the generation number...
        }
    }

    return(Result);
}

internal hha_bitmap *
GetBitmapInfo(game_assets *Assets, bitmap_id ID)
{
    asset *Asset = GetAsset(Assets, ID.Value);
    hha_bitmap *Result = &Assets->NullHHABitmap;
    if(Asset)
    {
        Assert((ID.Value == 0) || (Asset->HHA.Type == HHAAsset_Bitmap));
        Result = &Asset->HHA.Bitmap;
    }

    return(Result);
}

internal s16 *
GetSoundSamples(game_assets *Assets, sound_id ID)
{
    asset *Asset = GetAsset(Assets, ID.Value);

    s16 *Result = 0;
    if(Asset && (Asset->State == AssetState_Loaded))
    {
        Assert((ID.Value == 0) || (Asset->HHA.Type == HHAAsset_Sound));

        asset_sound_buffer_ranges Ranges = GetSoundBufferRanges(Assets);
        u64 BufferIndex = Asset->LoadedAtSoundBufferIndex;
        if(BufferIndex >= Ranges.SoundBufferLRUIndex)
        {
            Result = (s16 *)GetSoundBufferMemory(Assets, BufferIndex);
        }
        else if(BufferIndex >= Ranges.SoundBufferBaseIndex)
        {
            u32 DataSize = Asset->HHA.DataSize;
            sound_buffer_memory Memory = ReserveSoundMemory(Assets, DataSize);
            void *Source = GetSoundBufferMemory(Assets, BufferIndex);
            Result = (s16 *)Memory.Pointer;
            Copy(DataSize, Source, Result);
            Asset->LoadedAtSoundBufferIndex = Memory.BufferIndex;
        }
    }

    return(Result);
}

internal hha_asset *
GetSoundInfo(game_assets *Assets, sound_id ID)
{
    asset *Asset = GetAsset(Assets, ID.Value);

    hha_asset *Result = &Assets->NullHHAAsset;
    if(Asset)
    {
        Result = &Asset->HHA;
        Assert((ID.Value == 0) || (Asset->HHA.Type == HHAAsset_Sound));
    }


    return(Result);
}

internal loaded_font *
GetFont(game_assets *Assets, font_id ID)
{
    asset *Asset = GetAsset(Assets, ID.Value);
    loaded_font *Result = 0;
    if(Asset)
    {
        Assert((ID.Value == 0) || (Asset->HHA.Type == HHAAsset_Font));

        Result = (Asset->State == AssetState_Loaded) ? &Asset->Font : 0;
    }

    return(Result);
}

internal hha_font *
GetFontInfo(game_assets *Assets, font_id ID)
{
    asset *Asset = GetAsset(Assets, ID.Value);

    hha_font *Result = &Assets->NullHHAFont;
    if(Asset)
    {
        Assert((Asset->HHA.Type == HHAAsset_Font) ||
               (Asset->HHA.Type == HHAAsset_None));
        Result = &Asset->HHA.Font;
    }


    return(Result);
}

internal void
PrefetchBitmap(game_assets *Assets, bitmap_id ID)
{
    LoadBitmap(Assets, ID);
}

internal void
PrefetchSound(game_assets *Assets, sound_id ID)
{
    LoadSound(Assets, ID);

    // TODO(casey): I _think_ we want to force a sample pull here, because
    // that way it will do a copy out of the LRU region as necessary to ensure
    // that the prefetched sound is not about to be evicted... but we could
    // do some more instrumentation to see if this actually helps or not!
    GetSoundSamples(Assets, ID);
}

internal void
PrefetchFont(game_assets *Assets, font_id ID)
{
    LoadFont(Assets, ID);
}

internal sound_id
GetNextSoundInChain(game_assets *Assets, sound_id ID)
{
    sound_id Result = {};

    hha_sound *Info = &GetSoundInfo(Assets, ID)->Sound;
    switch(Info->Chain)
    {
        case HHASoundChain_None:
        {
            // NOTE(casey): Nothing to do.
        } break;

        case HHASoundChain_Loop:
        {
            Result = ID;
        } break;

        case HHASoundChain_Advance:
        {
            Result.Value = ID.Value + 1;
        } break;

        default:
        {
            InvalidCodePath;
        } break;
    }

    return(Result);
}

internal b32x
IsValid(bitmap_id ID)
{
    bool32 Result = (ID.Value != 0);

    return(Result);
}

internal b32x
IsValid(sound_id ID)
{
    b32x Result = (ID.Value != 0);

    return(Result);
}

internal b32x
RetractWaterMark(asset_file *File, u64 Count, u64 Offset, u64 Size)
{
    b32x Result = false;

    if(Offset == (File->HighWaterMark - (Count*Size)))
    {
        File->HighWaterMark = Offset;
        Result = true;
    }

    return(Result);
}

internal u32
GetAssetHashSlot(game_assets *Assets, asset_tag_hash TagHash)
{
    u32 Result = Hash32(TagHash.Value) % ArrayCount(Assets->AssetsByTag);
    return(Result);
}

internal b32
AddAssetToHash(game_assets *Assets, u32 AssetIndex)
{
    b32 Result = true;
    
    Assert(AssetIndex != 0);
    
    asset_match_vector MatchVector = {};
        
    asset *Asset = GetAsset(Assets, AssetIndex);
    
    u32 TagCount = 0;
    asset_tag_id Tags[4] = {};
    asset_basic_category TypeID = Asset_None;
    for(u32 AssetTagIndex = Asset->HHA.FirstTagIndex;
        AssetTagIndex < Asset->HHA.OnePastLastTagIndex;
        ++AssetTagIndex)
    {
        hha_tag *Tag = Assets->Tags + AssetTagIndex;
        switch(Tag->ID)
        {
            case Tag_BasicCategory:
            {
                TypeID = (asset_basic_category)RoundReal32ToInt32(Tag->Value);
            } break;
            
            case Tag_Variant:
            {
                MatchVector.E[MatchElement_Variant] = Tag->Value;
            } break;
            
            case Tag_FacingDirection:
            {
                MatchVector.E[MatchElement_FacingDirection] = Tag->Value;
            } break;
            
            case Tag_ChannelIndex:
            {
                MatchVector.E[MatchElement_Channel] = Tag->Value;
            } break;
            
            default:
            {
                if(TagCount < ArrayCount(Tags))
                {
                    Tags[TagCount++] = (asset_tag_id)Tag->ID;
                }
                else
                {
                    Result = false;
                }
            } break;
        }
    }
    
    if(TypeID == 0)
    {
        Result = false;
    }
    
    Asset->TagHash = GetTagHash(TypeID, Tags[0], Tags[1], Tags[2], Tags[3]);
    Asset->MatchVector = MatchVector;
    
    u32 HashSlot = GetAssetHashSlot(Assets, Asset->TagHash);
    
    Asset->NextInTagHash = Assets->AssetsByTag[HashSlot];
    Assets->AssetsByTag[HashSlot] = AssetIndex;
    
    return(Result);
}

internal void
RemoveAssetFromHash(game_assets *Assets, u32 AssetIndex)
{
    asset *Asset = GetAsset(Assets, AssetIndex);
    u32 HashSlot = GetAssetHashSlot(Assets, Asset->TagHash);
    
    u32 *PrevSlot = &Assets->AssetsByTag[HashSlot];
    while(*PrevSlot && (*PrevSlot != AssetIndex))
    {
        asset *Check = GetAsset(Assets, *PrevSlot);
        PrevSlot = &Check->NextInTagHash;
    }
    
    if(*PrevSlot)
    {
        Assert(*PrevSlot == AssetIndex);
        *PrevSlot = Asset->NextInTagHash;
    }
}

internal string
ReadAssetString(asset_file *File, memory_arena *Memory, u32 Count, u64 Offset)
{
    string Result = {};
    Result.Count = Count;
    Result.Data = (u8 *)PushSize(Memory, Count);
    Platform.ReadDataFromFile(&File->Handle, Offset, Result.Count, Result.Data);
    return(Result);
}

internal asset_source_file *
GetOrCreateAssetSourceFile(game_assets *Assets, string BaseName)
{
    // TODO(casey): Check to see if the compiler is smart enough to change
    // a power-of-two modulus into an and with the value minus 1!
    u32 HashValue = (StringHashOf(BaseName) % ArrayCount(Assets->SourceFileHash));

    asset_source_file *Match = 0;
    for(asset_source_file *SourceFile = Assets->SourceFileHash[HashValue];
        SourceFile;
        SourceFile = SourceFile->NextInHash)
    {
        if(StringsAreEqual(SourceFile->BaseName, BaseName))
        {
            Match = SourceFile;
            break;
        }
    }

    if(!Match)
    {
        Match = PushStruct(&Assets->NonRestoredMemory, asset_source_file);
        Match->BaseName = PushString(&Assets->NonRestoredMemory, BaseName);
        Match->NextInHash = Assets->SourceFileHash[HashValue];
        Assets->SourceFileHash[HashValue] = Match;

        Match->Errors = OnDemandMemoryStream(&Assets->NonRestoredMemory, 0);
    }

    return(Match);
}

internal asset_source_file *
GetOrCreateAssetSourceFile(game_assets *Assets, char *BaseName)
{
    asset_source_file *Result = GetOrCreateAssetSourceFile(Assets, WrapZ(BaseName));
    return(Result);
}

internal asset_source_file *
GetOrCreateAssetSourceFile(game_assets *Assets, char *BaseName, u64 FileDate, u64 FileCheckSum)
{
    asset_source_file *Result = GetOrCreateAssetSourceFile(Assets, BaseName);
    if((Result->FileDate == 0) ||
       (Result->FileDate > FileDate))
    {
        Result->FileDate = FileDate;
        Result->FileCheckSum = FileCheckSum;
    }

    return(Result);
}

internal u32
InitSourceHHA(game_assets *Assets, platform_file_group *FileGroup, platform_file_info *FileInfo)
{
    u32 FileIndex = 0;

    if(Assets->FileCount < Assets->MaxFileCount)
    {
        FileIndex = Assets->FileCount++;

        memory_arena *Arena = &Assets->NonRestoredMemory;

        u32 OpenFlags = OpenFile_Read;
#if HANDMADE_INTERNAL
        OpenFlags |= OpenFile_Write;
#endif

        Assert(FileIndex < Assets->FileCount);
        asset_file *File = Assets->Files + FileIndex;

        string Stem = RemovePath(RemoveExtension(WrapZ(FileInfo->BaseName)));
        File->Stem = PushString(Arena, Stem);
        File->AssetBase = Assets->AssetCount - 1;
        File->TagBase = Assets->TagCount;

        ZeroStruct(File->Header);
        File->Handle = Platform.OpenFile(FileGroup, FileInfo, OpenFlags);
        Platform.ReadDataFromFile(&File->Handle, 0, sizeof(File->Header), &File->Header);

        if(File->Header.MagicValue != HHA_MAGIC_VALUE)
        {
            Platform.FileError(&File->Handle, "HHA file has an invalid magic value.");
        }

        if(File->Header.Version > HHA_VERSION)
        {
            Platform.FileError(&File->Handle, "HHA file is of a later version.");
        }

        if(PlatformNoFileErrors(&File->Handle))
        {
            // NOTE(casey): The first asset and tag slot in every
            // HHA is a null (reserved) so we don't count it as
            // something we will need space for!
            if(File->Header.TagCount)
            {
                Assets->TagCount += (File->Header.TagCount - 1);
            }

            if(File->Header.AssetCount)
            {
                Assets->AssetCount += (File->Header.AssetCount - 1);
            }
        }
        else
        {
            // TODO(casey): Eventually, have some way of notifying users of bogus files?
            InvalidCodePath;
        }

        File->HighWaterMark = FileInfo->FileSize;
        while(RetractWaterMark(File, File->Header.TagCount, File->Header.Tags, sizeof(hha_tag)) ||
              RetractWaterMark(File, File->Header.AssetCount, File->Header.Assets, sizeof(hha_asset)) ||
              RetractWaterMark(File, File->Header.AssetCount, File->Header.Annotations, sizeof(hha_annotation)))
        {
        }
    }

    return(FileIndex);
}

internal game_assets *
AllocateGameAssets(memory_index Size, game_state *GameState,
                   renderer_texture_queue *TextureOpQueue)
{
    TIMED_FUNCTION();

    game_assets *Assets = BootstrapPushStruct(game_assets, NonRestoredMemory, NonRestoredArena());
    memory_arena *Arena = &Assets->NonRestoredMemory;

    InitSoundMemory(Assets, Arena);

    Assets->GameState = GameState;
    Assets->TextureOpQueue = TextureOpQueue;
    Assets->NormalTextureHandleCount = HANDMADE_NORMAL_TEXTURE_COUNT;
    Assets->SpecialTextureHandleCount = HANDMADE_SPECIAL_TEXTURE_COUNT;

    DLIST_INIT(&Assets->SpecialTextureLRUSentinel);
    DLIST_INIT(&Assets->RegularTextureLRUSentinel);

    texture_op *Op = BeginTextureOp(TextureOpQueue, 1*1*4);
    Op->Texture = ReferToTexture(0, 1, 1);
    Assert(Op);
    *(u32 *)Op->Data = 0xFFFFFFFF;
    CompleteTextureOp(TextureOpQueue, Op);

    Assets->NextFreeTextureHandle = 1;

    Assets->TagCount = 1;
    Assets->AssetCount = 1;
    Assets->FileCount = 1;

    // NOTE(casey): This code was written using Snuffleupagus-Oriented Programming (SOP)
    {
        platform_file_group FileGroup = Platform.GetAllFilesOfTypeBegin(PlatformFileType_AssetFile);

        Assets->MaxFileCount = FileGroup.FileCount + 1;
#if HANDMADE_INTERNAL
        Assets->MaxFileCount += 256;
#endif

        Assets->Files = PushArray(Arena, Assets->MaxFileCount, asset_file);
        for(platform_file_info *FileInfo = FileGroup.FirstFileInfo;
            FileInfo;
            FileInfo = FileInfo->Next)
        {
            InitSourceHHA(Assets, &FileGroup, FileInfo);
        }
        Platform.GetAllFilesOfTypeEnd(&FileGroup);
    }

    // NOTE(casey): Allocate all metadata space
    Assets->MaxAssetCount = Assets->AssetCount;
    Assets->MaxTagCount = Assets->TagCount;
#if HANDMADE_INTERNAL
    Assets->MaxAssetCount += 65536;
    Assets->MaxTagCount += 65536;
#endif
    Assets->Assets = PushArray(Arena, Assets->MaxAssetCount, asset);
    Assets->Tags = PushArray(Arena, Assets->MaxTagCount, hha_tag);

    // NOTE(casey): Reserve one null tag at the beginning
    ZeroStruct(Assets->Tags[0]);

    // NOTE(casey): Reserve one null asset at the beginning
    u32 AssetCount = 0;
    ZeroStruct(*(Assets->Assets + AssetCount));
    ++AssetCount;

    hha_annotation NullAnnotation = {};

    // TODO(casey): Exercise for the reader - how would you do this in a way
    // that scaled gracefully to hundreds of asset pack files?  (or more!)
    for(u32 FileIndex = 1;
        FileIndex < Assets->FileCount;
        ++FileIndex)
    {
        asset_file *File = Assets->Files + FileIndex;
        if(PlatformNoFileErrors(&File->Handle))
        {
            if(File->Header.TagCount)
            {
                // NOTE(casey): Skip the first tag, since it's null
                u32 TagArraySize = sizeof(hha_tag)*(File->Header.TagCount - 1);
                Platform.ReadDataFromFile(&File->Handle, File->Header.Tags + sizeof(hha_tag),
                                          TagArraySize, Assets->Tags + File->TagBase);
            }

            if(File->Header.AssetCount)
            {
                u32 FileAssetCount = File->Header.AssetCount - 1;

                temporary_memory TempMem = BeginTemporaryMemory(GameState->FrameArena);
                hha_asset *HHAAssetArray = PushArray(GameState->FrameArena,
                                                     FileAssetCount, hha_asset);
                hha_annotation *HHAAnnotationArray = PushArray(GameState->FrameArena,
                                                               FileAssetCount, hha_annotation);
                Platform.ReadDataFromFile(&File->Handle,
                                          File->Header.Assets + 1*sizeof(hha_asset),
                                          FileAssetCount*sizeof(hha_asset),
                                          HHAAssetArray);

                if(File->Header.Annotations)
                {
                    Platform.ReadDataFromFile(&File->Handle,
                                              File->Header.Annotations + 1*sizeof(hha_annotation),
                                              FileAssetCount*sizeof(hha_annotation),
                                              HHAAnnotationArray);
                }
                else
                {
                    HHAAnnotationArray = 0;
                }

                for(u32 AssetIndex = 0;
                    AssetIndex < FileAssetCount;
                    ++AssetIndex)
                {
                    hha_asset *HHAAsset = HHAAssetArray + AssetIndex;
                    hha_annotation *HHAAnnotation = &NullAnnotation;
                    if(HHAAnnotationArray)
                    {
                        HHAAnnotation = HHAAnnotationArray + AssetIndex;
                    }

                    Assert(AssetCount < Assets->AssetCount);
                    u32 GlobalAssetIndex = AssetCount++;
                    asset *Asset = Assets->Assets + GlobalAssetIndex;

                    Asset->FileIndex = FileIndex;
                    Asset->AssetIndexInFile = GlobalAssetIndex - File->AssetBase;
                    Asset->HHA = *HHAAsset;
                    Asset->Annotation = *HHAAnnotation;

                    if(Asset->HHA.FirstTagIndex == 0)
                    {
                        Asset->HHA.FirstTagIndex = Asset->HHA.OnePastLastTagIndex = 0;
                    }
                    else
                    {
                        Asset->HHA.FirstTagIndex += (File->TagBase - 1);
                        Asset->HHA.OnePastLastTagIndex += (File->TagBase - 1);
                    }

#if HANDMADE_INTERNAL
                    // TODO(casey): This is very inefficient, and we could modify
                    // the file format to keep a separate array of filenames (or
                    // we could hash filenames based on their location in the
                    // file as well, and only read them once).  But at the moment,
                    // we just read the source name directly, because we don't
                    // care how long it takes in "editting" mode of the game anyway.
                    u32 SourceFileNameCount = HHAAnnotation->SourceFileBaseNameCount;
                    char *SourceFileName = PushArray(GameState->FrameArena,
                                                     SourceFileNameCount + 1, char);
                    Platform.ReadDataFromFile(&File->Handle, HHAAnnotation->SourceFileBaseNameOffset,
                                              SourceFileNameCount, SourceFileName);
                    SourceFileName[SourceFileNameCount] = 0;

                    u32 GridX = HHAAnnotation->SpriteSheetX;
                    u32 GridY = HHAAnnotation->SpriteSheetY;
                    asset_source_file *SourceFile =
                        GetOrCreateAssetSourceFile(Assets, SourceFileName,
                                                   HHAAnnotation->SourceFileDate,
                                                   HHAAnnotation->SourceFileChecksum);
                    SourceFile->DestFileIndex = FileIndex;

                    u32 *GridAssetIndex = &SourceFile->AssetIndices[GridY][GridX];
                    if(*GridAssetIndex == 0)
                    {
                        *GridAssetIndex = GlobalAssetIndex;
                    }
                    else
                    {
                        asset *Conflict = Assets->Assets + *GridAssetIndex;
                        if(Conflict->HHA.Type == HHAAsset_Sound)
                        {
                            if(*GridAssetIndex > GlobalAssetIndex)
                            {
                                *GridAssetIndex = GlobalAssetIndex;
                            }
                        }
                        else
                        {
                            Outf(&SourceFile->Errors, "%s(%u,%u): Asset %u and %u occupy same slot in spritesheet and cannot be edited properly.\n",
                                 SourceFileName, GridX, GridY,
                                 Asset->AssetIndexInFile,
                                 Conflict->AssetIndexInFile);
                        }
                    }
#endif
                    if(!AddAssetToHash(Assets, GlobalAssetIndex))
                    {
#if HANDMADE_INTERNAL
                        Outf(&SourceFile->Errors, "%s(%u,%u): Invalid tag set for asset.\n",
                             SourceFileName, GridX, GridY,
                             Asset->AssetIndexInFile);
#endif
                    }
                }

                EndTemporaryMemory(TempMem);
            }
        }
    }

    Assert(AssetCount == Assets->AssetCount);

#if HANDMADE_INTERNAL
    CreateAudioChannelTagGrid(Assets, &Assets->AudioChannelTags);
    CreateArtBlockTagGrid(Assets, &Assets->ArtBlockTags);
    CreateArtHeadTagGrid(Assets, &Assets->ArtHeadTags);
    CreateArtBodyTagGrid(Assets, &Assets->ArtBodyTags);
    CreateArtCharacterTagGrid(Assets, &Assets->ArtCharacterTags);
    CreateArtParticleTagGrid(Assets, &Assets->ArtParticleTags);
    CreateArtHandTagGrid(Assets, &Assets->ArtHandTags);
    CreateArtItemTagGrid(Assets, &Assets->ArtItemTags);
    CreateArtSceneryTagGrid(Assets, &Assets->ArtSceneryTags);
    CreateArtPlateTagGrid(Assets, &Assets->ArtPlateTags);

    SynchronizeAssetFileChanges(Assets, false);
#endif

    if(Assets->AssetCount == 1)
    {
        // NOTE(casey): We found literally no assets - this is a fatal error, probably?
        Platform.ErrorMessage(PlatformError_Nonfatal, "WARNING: No assets found. Ensure the starting directory is correct.");
    }

    return(Assets);
}

inline u32
GetGlyphFromCodePoint(hha_font *Info, loaded_font *Font, u32 CodePoint)
{
    u32 Result = 0;
    if(CodePoint < Info->OnePastHighestCodepoint)
    {
        Result = Font->UnicodeMap[CodePoint];
        Assert(Result < Info->GlyphCount);
    }

    return(Result);
}

internal r32
GetHorizontalAdvanceForPair(hha_font *Info, loaded_font *Font, u32 DesiredPrevCodePoint, u32 DesiredCodePoint)
{
    u32 PrevGlyph = GetGlyphFromCodePoint(Info, Font, DesiredPrevCodePoint);
    u32 Glyph = GetGlyphFromCodePoint(Info, Font, DesiredCodePoint);

    r32 Result = Font->HorizontalAdvance[PrevGlyph*Info->GlyphCount + Glyph];

    return(Result);
}

internal bitmap_id
GetBitmapForGlyph(game_assets *Assets, hha_font *Info, loaded_font *Font, u32 DesiredCodePoint)
{
    u32 Glyph = GetGlyphFromCodePoint(Info, Font, DesiredCodePoint);
    bitmap_id Result;
    Result.Value = Font->Glyphs[Glyph].BitmapID;
    Result.Value += Font->BitmapIDOffset;

    return(Result);
}

internal r32
GetLineAdvanceFor(hha_font *Info)
{
    r32 Result = Info->AscenderHeight + Info->DescenderHeight + Info->ExternalLeading;

    return(Result);
}

internal r32
GetStartingBaselineY(hha_font *Info)
{
    r32 Result = Info->AscenderHeight;

    return(Result);
}
