/* ========================================================================
   $File: C:\work\handmade\code\handmade_import.cpp $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright by Molly Rocket, Inc., All Rights Reserved. $
   ======================================================================== */

#if HANDMADE_INTERNAL

internal u32x
GetTagCount(import_tag_array Array)
{
    u32x Result = (Array.OnePastLastTagIndex - Array.FirstTagIndex);
    return(Result);
}

internal b32x
TagsAreEqual(hha_tag A, hha_tag B)
{
    b32x Result = ((A.ID == B.ID) &&
                   (A.Value == B.Value));
    return(Result);
}

internal u32
ReserveTags(game_assets *Assets, u32 TagCount)
{
    u32 Result = 0;
    
    if((Assets->TagCount + TagCount) <= Assets->MaxTagCount)
    {
        Result = Assets->TagCount;
        Assets->TagCount += TagCount;
    }
    
    return(Result);
}

internal u32
ReserveAsset(game_assets *Assets, u32 AssetCount = 1)
{
    u32 Result = 0;
    
    if((Assets->AssetCount + AssetCount) <= Assets->MaxAssetCount)
    {
        Result = Assets->AssetCount;
        Assets->AssetCount += AssetCount;
    }
    
    return(Result);
}

internal u64
ReserveData(game_assets *Assets, asset_file *AssetFile, u32 AssetDataSize)
{
    AssetFile->Modified = true;
    u64 Result = AssetFile->HighWaterMark;
    AssetFile->HighWaterMark += AssetDataSize;
    
    return(Result);
}

internal void
WriteAssetData(asset_file *File, u64 DataOffset, u32 DataSize, void *Data)
{
    File->Modified = true;
    Platform.WriteDataToFile(&File->Handle, DataOffset, DataSize, Data);
}

internal void
UpdateAssetString(game_assets *Assets, asset_file *File, string Source, u32 *Count, u64 *Offset)
{
    // TODO(casey): Don't write this if the string is already the same in the file.
    
    if(Source.Count > *Count)
    {
        *Offset = ReserveData(Assets, File, (u32)Source.Count);
    }
    
    *Count = (u32)Source.Count;
    WriteAssetData(File, *Offset, *Count, Source.Data);
}

internal void
WriteModificationsToHHA(game_assets *Assets, u32 FileIndex, memory_arena *TempArena)
{
    asset_file *File = Assets->Files + FileIndex;
    
    File->Modified = false;
    
    u32 AssetCount = 1; // NOTE(casey): First asset entry is skipped as the null asset!
    u32 TagCount = 1; // NOTE(casey): First tag entry is skipped as the null tag!
    for(u32 AssetIndex = 0;
        AssetIndex < Assets->AssetCount;
        ++AssetIndex)
    {
        asset *Asset = Assets->Assets + AssetIndex;
        if(Asset->FileIndex == FileIndex)
        {
            ++AssetCount;
            TagCount += (Asset->HHA.OnePastLastTagIndex - Asset->HHA.FirstTagIndex);
        }
    }
    
    u32 TagArraySize = TagCount * sizeof(hha_tag);
    u32 AssetsArraySize = AssetCount * sizeof(hha_asset);
    u32 AnnotationArraySize = AssetCount * sizeof(hha_annotation);
    
    File->Header.TagCount = TagCount;
    File->Header.AssetCount = AssetCount;
    
    File->Header.Tags = File->HighWaterMark;
    File->Header.Assets = File->Header.Tags + TagArraySize;
    File->Header.Annotations = File->Header.Assets + AssetsArraySize;
    
    hha_tag *Tags = PushArray(TempArena, TagCount, hha_tag);
    hha_asset *AssetArray = PushArray(TempArena, AssetCount, hha_asset);
    hha_annotation *AnnotationArray = PushArray(TempArena, AssetCount, hha_annotation);
    
    u32 TagIndexInFile = 1;
    u32 AssetIndexInFile = 1;
    
    for(u32 GlobalAssetIndex = 1;
        GlobalAssetIndex < Assets->AssetCount;
        ++GlobalAssetIndex)
    {
        asset *Source = Assets->Assets + GlobalAssetIndex;
        if(Source->FileIndex == FileIndex)
        {
            hha_asset *Dest = AssetArray + AssetIndexInFile;
            hha_annotation *DestAnnotation = AnnotationArray + AssetIndexInFile;
            Source->AssetIndexInFile = AssetIndexInFile;
            
            *DestAnnotation = Source->Annotation;
            
            *Dest = Source->HHA;
            Dest->FirstTagIndex = TagIndexInFile;
            for(u32 TagIndex = Source->HHA.FirstTagIndex;
                TagIndex < Source->HHA.OnePastLastTagIndex;
                ++TagIndex)
            {
                Tags[TagIndexInFile++] = Assets->Tags[TagIndex];
            }
            Dest->OnePastLastTagIndex = TagIndexInFile;
            
            ++AssetIndexInFile;
        }
    }
    
    Assert(TagIndexInFile == TagCount);
    Assert(AssetIndexInFile == AssetCount);
    
    Platform.WriteDataToFile(&File->Handle, 0, sizeof(File->Header), &File->Header);
    Platform.WriteDataToFile(&File->Handle, File->Header.Tags, TagArraySize, Tags);
    Platform.WriteDataToFile(&File->Handle, File->Header.Assets, AssetsArraySize, AssetArray);
    Platform.WriteDataToFile(&File->Handle, File->Header.Annotations, AnnotationArraySize, AnnotationArray);
}

internal void
WriteAssetStream(asset_file *File, u64 DataOffset, stream *Stream)
{
    for(stream_chunk *Chunk = Stream->First;
        Chunk;
        Chunk = Chunk->Next)
    {
        WriteAssetData(File, DataOffset, (u32)Chunk->Contents.Count, Chunk->Contents.Data);
        DataOffset += Chunk->Contents.Count;
    }
}

internal void
WriteImageToHHA(game_assets *Assets,
                asset_source_file *File, v2u OrigDim, image_u32 SourceImage,
                memory_arena *TempArena, u32 TileXIndex, u32 TileYIndex)
{
    hha_asset HHAAsset = {};
    
    u32 AssetIndex = File->AssetIndices[TileYIndex][TileXIndex];
    if(AssetIndex)
    {
        asset *Asset = Assets->Assets + AssetIndex;
        HHAAsset = Asset->HHA;
    }
    else
    {
        // NOTE(casey): Assign a new asset index here
        AssetIndex = ReserveAsset(Assets);
    }
    
    if(AssetIndex)
    {
        u32 AssetDataSize = GetTotalImageSize(SourceImage);
        asset *Asset = Assets->Assets + AssetIndex;
        
        // NOTE(casey): Make sure no one tries to use the _old_ image
        bitmap_id BitmapID = {AssetIndex};
        UnloadBitmap(Assets, BitmapID);
        
        Asset->FileIndex = File->DestFileIndex;
        Assert(Asset->FileIndex);
        
        asset_file *AssetFile = Assets->Files + Asset->FileIndex;
        if((HHAAsset.DataOffset == 0) ||
           (HHAAsset.DataSize < AssetDataSize))
        {
            HHAAsset.DataOffset = ReserveData(Assets, AssetFile, AssetDataSize);
        }
        HHAAsset.DataSize = AssetDataSize;
        
        // TODO(casey): Maybe we check here and record an error if it's larger than 64k?
        HHAAsset.Bitmap.Dim[0] = (u16)SourceImage.Width;
        HHAAsset.Bitmap.Dim[1] = (u16)SourceImage.Height;
        HHAAsset.Bitmap.OrigDim[0] = (u16)OrigDim.Width;
        HHAAsset.Bitmap.OrigDim[1] = (u16)OrigDim.Height;
        HHAAsset.Type = HHAAsset_Bitmap;
        
        Asset->HHA = HHAAsset;
        Asset->Annotation.SourceFileDate = File->FileDate;
        Asset->Annotation.SourceFileChecksum = File->FileCheckSum;
        Asset->Annotation.SpriteSheetX = TileXIndex;
        Asset->Annotation.SpriteSheetY = TileYIndex;
        
        File->AssetIndices[TileYIndex][TileXIndex] = AssetIndex;
        
        WriteAssetData(AssetFile, HHAAsset.DataOffset, AssetDataSize,
                       SourceImage.Pixels);
    }
    else
    {
        Outf(&File->Errors, "Out of asset memory - please restart Handmade Hero!\n");
    }
}

internal void
ProcessAudioImport(game_assets *Assets, asset_source_file *SourceFile,
                   sound_s16 Sound, memory_arena *TempArena)
{
    u32 MaxChannelCount = ASSET_IMPORT_GRID_MAX*ASSET_IMPORT_GRID_MAX;
    if(Sound.ChannelCount > MaxChannelCount)
    {
        Sound.ChannelCount = MaxChannelCount;
    }
    
    for(u32 ChannelIndex = 0;
        ChannelIndex < Sound.ChannelCount;
        ++ChannelIndex)
    {
        b32 ReuseAsset = true;
        u32 MaxSampleCount = 128*1024; // TODO(casey): What should this actually be?
        u32 TotalChunkCount = (Sound.SampleCount + MaxSampleCount - 1) / MaxSampleCount;
        
        u32 XIndex = ChannelIndex % 8;
        u32 YIndex = ChannelIndex / 8;
        u32 *AssetIndexSlot = &SourceFile->AssetIndices[YIndex][XIndex];
        u32 AssetIndex = *AssetIndexSlot;
        if(AssetIndex)
        {
            for(u32 ChunkIndex = 0;
                ChunkIndex < (TotalChunkCount - 1);
                ++ChunkIndex)
            {
                asset *Asset = Assets->Assets + AssetIndex + ChunkIndex;
                if(Asset->HHA.Sound.Chain != HHASoundChain_Loop)
                {
                    AssetIndex = 0;
                    break;
                }
            }
        }
        
        if(!AssetIndex)
        {
            // NOTE(casey): Assign a new asset index here
            AssetIndex = ReserveAsset(Assets, TotalChunkCount);
            ReuseAsset = false;
        }
        
        if(AssetIndex)
        {
            *AssetIndexSlot = AssetIndex;
            
            s16 *Samples = GetChannelSamples(Sound, ChannelIndex);
            u32 SampleCountRemaining = Sound.SampleCount;
            while(SampleCountRemaining)
            {
                hha_sound_chain Chain = HHASoundChain_None;
                u32 SampleCount = SampleCountRemaining;
                if(SampleCount > MaxSampleCount)
                {
                    Chain = HHASoundChain_Advance;
                    SampleCount = MaxSampleCount;
                }
                
                hha_asset HHAAsset = {};
                if(ReuseAsset)
                {
                    asset *Asset = Assets->Assets + AssetIndex;
                    HHAAsset = Asset->HHA;
                }
                
                if(AssetIndex)
                {
                    u32 AssetDataSize = SampleCount*sizeof(s16);
                    asset *Asset = Assets->Assets + AssetIndex;
                    
                    // NOTE(casey): Make sure no one tries to use the _old_ image
                    sound_id AudioID = {AssetIndex};
                    UnloadAudio(Assets, AudioID);
                    
                    Asset->FileIndex = SourceFile->DestFileIndex;
                    Assert(Asset->FileIndex);
                    
                    asset_file *AssetFile = Assets->Files + Asset->FileIndex;
                    if((HHAAsset.DataOffset == 0) ||
                       (HHAAsset.DataSize < AssetDataSize))
                    {
                        HHAAsset.DataOffset = ReserveData(Assets, AssetFile, AssetDataSize);
                    }
                    HHAAsset.DataSize = AssetDataSize;
                    
                    HHAAsset.Sound.SampleCount = Sound.SampleCount;
                    HHAAsset.Sound.ChannelCount = Sound.ChannelCount;
                    HHAAsset.Sound.Chain = Chain;
                    HHAAsset.Type = HHAAsset_Sound;
                    
                    Asset->HHA = HHAAsset;
                    Asset->Annotation.SourceFileDate = SourceFile->FileDate;
                    Asset->Annotation.SourceFileChecksum = SourceFile->FileCheckSum;
                    Asset->Annotation.SpriteSheetX = 0;
                    Asset->Annotation.SpriteSheetY = 0;
                    
                    WriteAssetData(AssetFile, HHAAsset.DataOffset, AssetDataSize, Samples);
                }
                
                SampleCountRemaining -= SampleCount;
                Samples += SampleCount;
                ++AssetIndex;
            }
        }
        else
        {
            Outf(&SourceFile->Errors, "Out of asset memory - please restart Handmade Hero!\n");
        }
    }
}

internal u32
GetDownsampleCountForFit(image_u32 Source, u32 MaxWidth, u32 MaxHeight)
{
    // NOTE(casey): This could be done closed-form if we cared, but we don't.
    u32 Result = 0;
    u32 Width = Source.Width;
    u32 Height = Source.Height;
    while((Width > MaxWidth) ||
          (Height > MaxHeight))
    {
        Width /= 2;
        Height /= 2;
        ++Result;
    }
    return(Result);
}

internal image_u32
Downsample(image_u32 Source, u32 DownsampleCount)
{
    image_u32 Result = Source;
    for(u32 DownsampleIndex = 0;
        DownsampleIndex < DownsampleCount;
        ++DownsampleIndex)
    {
        image_u32 Prev = Result;
        Result.Width /= 2;
        Result.Height /= 2;
        Downsample2x(Prev, Result);
    }
    
    return(Result);
}

internal image_u32
ExtractImage(image_u32 SourceImage,
             u32 MinX, u32 MinY, u32 OnePastMaxX, u32 OnePastMaxY,
             memory_arena *TempArena)
{
    image_u32 Result = PushImage(TempArena, OnePastMaxX - MinX, OnePastMaxY - MinY);
    
    u32 *DestPixel = Result.Pixels;
    u32 *SourceRow = (SourceImage.Pixels + (OnePastMaxY - 1)*SourceImage.Width + MinX);
    for(u32 Y = 0;
        Y < Result.Height;
        ++Y)
    {
        u32 *SourcePixel = SourceRow;
        for(u32 X = 0;
            X < Result.Width;
            ++X)
        {
            // TODO(casey): Should we actually take this opportunity to properly
            // use RGBA ordering instead of BGRA ordering?
            
            u32 SourceC = *SourcePixel++;
            v4 C = RGBAUnpack4x8(SourceC);
            C = SRGB255ToLinear1(C);
            C.rgb *= C.a;
            C = Linear1ToSRGB255(C);
            *DestPixel++ = BGRAPack4x8(C);
        }
        
        SourceRow -= SourceImage.Width;
    }
    
    return(Result);
}

internal void
ProcessSingleTileImport(game_assets *Assets,
                        asset_source_file *File, image_u32 SourceImage,
                        memory_arena *TempArena, u32 MaximumDimension)
{
    v2u OrigDim =
    {
        SourceImage.Width,
        SourceImage.Height,
    };
    image_u32 PrepImage = ExtractImage(SourceImage,
                                       0, 0, SourceImage.Width, SourceImage.Height,
                                       TempArena);
    
    u32 DownsampleCount = GetDownsampleCountForFit(PrepImage, MaximumDimension, MaximumDimension);
    image_u32 DestImage = Downsample(PrepImage, DownsampleCount);
    WriteImageToHHA(Assets, File, OrigDim, DestImage, TempArena, 0, 0);
}

internal void
ProcessMultiTileImport(game_assets *Assets,
                       asset_source_file *File, image_u32 Image,
                       memory_arena *TempArena)
{
    u32 BorderDim = 8;
    u32 TileDim = 1024;
    
    u32 XCountMax = ArrayCount(File->AssetIndices[0]);
    u32 YCountMax = ArrayCount(File->AssetIndices);
    
    u32 XCount = Image.Width / TileDim;
    if(XCount > XCountMax)
    {
        Outf(&File->Errors, "Tile column count of %u exceeds maximum of %u columns\n",
             XCount, XCountMax);
        
        XCount = XCountMax;
    }
    
    u32 YCount = Image.Height / TileDim;
    if(YCount > YCountMax)
    {
        Outf(&File->Errors, "Tile row count of %u exceeds maximum of %u rows\n",
             YCount, YCountMax);
        
        YCount = YCountMax;
    }
    
    for(u32 YIndex = 0;
        YIndex < YCount;
        ++YIndex)
    {
        for(u32 XIndex = 0;
            XIndex < XCount;
            ++XIndex)
        {
            u32 MinX = U32Max;
            u32 MaxX = U32Min;
            u32 MinY = U32Max;
            u32 MaxY = U32Min;
            
            u32 ValidPixelCount = 0;
            u32 SolidPixelCount = 0;
            
            {
                u32 *SourceRow = Image.Pixels + YIndex*TileDim*Image.Width + XIndex*TileDim;
                for(u32 Y = 0;
                    Y < TileDim;
                    ++Y)
                {
                    u32 *SourcePixel = SourceRow;
                    for(u32 X = 0;
                        X < TileDim;
                        ++X)
                    {
                        u32 SourceC = *SourcePixel++;
                        u32 Alpha = (SourceC >> 24);
                        if(Alpha)
                        {
                            MinX = Minimum(MinX, X);
                            MaxX = Maximum(MaxX, X);
                            MinY = Minimum(MinY, Y);
                            MaxY = Maximum(MaxY, Y);
                            if(Alpha > 16)
                            {
                                ++SolidPixelCount;
                            }
                            ++ValidPixelCount;
                        }
                    }
                    
                    SourceRow += Image.Width;
                }
            }
            
            f32 SolidPixelsPerArea = 0.0f;
            if(ValidPixelCount > 16)
            {
                f32 Area = (f32)(((MaxX - MinX) * (MaxY - MinY)));
                SolidPixelsPerArea = (f32)SolidPixelCount / Area;
            }
            
            if(SolidPixelsPerArea > 0.1f)
            {
                // NOTE(casey): There was something in this tile
                
                if(MinX >= BorderDim)
                {
                    MinX -= BorderDim;
                }
                else
                {
                    MinX = 0;
                    
                    Outf(&File->Errors, "Tile %u,%u extends into left %u-pixel border\n",
                         XIndex, YIndex, BorderDim);
                }
                
                if(MaxX < (TileDim - BorderDim))
                {
                    MaxX += BorderDim;
                }
                else
                {
                    MaxX = TileDim - 1;
                    
                    Outf(&File->Errors, "Tile %u,%u extends into right %u-pixel border\n",
                         XIndex, YIndex, BorderDim);
                }
                
                if(MinY >= BorderDim)
                {
                    MinY -= BorderDim;
                }
                else
                {
                    MinY = 0;
                    
                    Outf(&File->Errors, "Tile %u,%u extends into top %u-pixel border\n",
                         XIndex, YIndex, BorderDim);
                }
                
                if(MaxY < (TileDim - BorderDim))
                {
                    MaxY += BorderDim;
                }
                else
                {
                    MaxY = TileDim - 1;
                    
                    Outf(&File->Errors, "Tile %u,%u extends into bottom %u-pixel border\n",
                         XIndex, YIndex, BorderDim);
                }
                
                image_u32 TileImage = ExtractImage(Image,
                                                   XIndex*TileDim + MinX,
                                                   YIndex*TileDim + MinY,
                                                   XIndex*TileDim + MaxX + 1,
                                                   YIndex*TileDim + MaxY + 1,
                                                   TempArena);
                v2u OrigDim =
                {
                    TileImage.Width,
                    TileImage.Height,
                };
                
                u32 DownsampleCount = GetDownsampleCountForFit(TileImage, ASSET_MAX_SPRITE_DIM, ASSET_MAX_SPRITE_DIM);
                image_u32 DestImage = Downsample(TileImage, DownsampleCount);
                WriteImageToHHA(Assets, File, OrigDim, DestImage, TempArena, XIndex, YIndex);
            }
        }
    }
}

internal token
PopToken(string *Source)
{
    token Result = {};
    Result.Text = *Source;
    Result.F32 = 1.0f;
    
    u32 Skip = 0;
    for(u32 Index = 0;
        Index < Source->Count;
        ++Index)
    {
        if(Source->Data[Index] == '_')
        {
            Result.Text.Count = Index;
            Result.Text.Data = Source->Data;
            Skip = 1;
            break;
        }
    }
    
    if(Result.Text.Count)
    {
        Source->Count -= (Result.Text.Count + Skip);
        Source->Data += (Result.Text.Count + Skip);
    }
    
    return(Result);
}

struct tag_builder
{
    game_assets *Assets;
    u32 FirstTagIndex;
    b32 Error;
};

internal void
AddTag(tag_builder *Builder, asset_tag_id ID, f32 Value)
{
    if(Builder->Assets->TagCount < Builder->Assets->MaxTagCount)
    {
        hha_tag *Tag = Builder->Assets->Tags + Builder->Assets->TagCount++;
        Tag->ID = ID;
        Tag->Value = Value;
    }
    else
    {
        Builder->Error = true;
    }
}

internal tag_builder
BeginTags(game_assets *Assets)
{
    tag_builder Builder = {};
    Builder.Assets = Assets;
    Builder.FirstTagIndex = Assets->TagCount;
    
    return(Builder);
}

internal import_grid_tag
EndTags(tag_builder *Builder, asset_basic_category Category)
{
    import_grid_tag Result = {};
    
    // TODO(casey): Make it explict, instead of auto-adding the category?
    if(Category != Asset_None)
    {
        AddTag(Builder, Tag_BasicCategory, (f32)Category);
    }
    
    Result.TypeID = Category;
    Result.Tags.FirstTagIndex = Builder->FirstTagIndex;
    Result.Tags.OnePastLastTagIndex = Builder->Assets->TagCount;
    
    return(Result);
}

internal import_grid_tag
ImportBody(game_assets *Assets, u32 XIndex, u32 YIndex)
{
    import_grid_tag Result = {};
    
    if(XIndex <= 6)
    {
        tag_builder Builder = BeginTags(Assets);
        AddTag(&Builder, Tag_FacingDirection, (f32)(YIndex % 4) / 4.0f);
        
        switch(XIndex)
        {
            // case 0: {AddTag(&Builder, Tag_Idle, 1.0f);} break;
            case 1: {AddTag(&Builder, Tag_Dodge, -1.0f);} break;
            case 2: {AddTag(&Builder, Tag_Dodge, 1.0f);} break;
            case 3: {AddTag(&Builder, Tag_Move, 1.0f);} break;
            case 4: {AddTag(&Builder, Tag_Hit, 1.0f);} break;
            case 5: {AddTag(&Builder, Tag_Attack1, 1.0f);} break;
            case 6: {AddTag(&Builder, Tag_Attack2, 1.0f);} break;
            // case 7: {AddTag(&Builder, Tag_, 1.0f);} break;
        }
        
        Result = EndTags(&Builder, Asset_Body);
    }
    
    return(Result);
}

internal import_grid_tag
ImportHead(game_assets *Assets, u32 XIndex, u32 YIndex)
{
    import_grid_tag Result = {};
    
    if(XIndex <= 2)
    {
        tag_builder Builder = BeginTags(Assets);
        AddTag(&Builder, Tag_FacingDirection, (f32)(YIndex % 4) / 4.0f);
        
        switch(XIndex)
        {
            // case 0: {AddTag(&Builder, Tag_Idle, 1.0f);} break;
            case 1: {AddTag(&Builder, Tag_Surprise, 1.0f);} break;
            case 2: {AddTag(&Builder, Tag_Anger, 1.0f);} break;
        }
        
        Result = EndTags(&Builder, Asset_Head);
    }
    
    return(Result);
}

internal void
CreateAudioChannelTagGrid(game_assets *Assets, import_grid_tags *Tags)
{
    for(u32 YIndex = 0;
        YIndex < ASSET_IMPORT_GRID_MAX;
        ++YIndex)
    {
        for(u32 XIndex = 0;
            XIndex < ASSET_IMPORT_GRID_MAX;
            ++XIndex)
        {
            u32 ChannelIndex = (YIndex*ASSET_IMPORT_GRID_MAX + XIndex);
            tag_builder Builder = BeginTags(Assets);
            AddTag(&Builder, Tag_ChannelIndex, (f32)ChannelIndex);
            Tags->Tags[YIndex][XIndex] = EndTags(&Builder, Asset_Audio);
        }
    }
}

internal void
CreateArtBlockTagGrid(game_assets *Assets, import_grid_tags *Tags)
{
    import_grid_tag *Tag = &Tags->Tags[0][0];
    tag_builder Builder = BeginTags(Assets);
    *Tag = EndTags(&Builder, Asset_Block);
}

internal void
CreateArtHeadTagGrid(game_assets *Assets, import_grid_tags *Tags)
{
    for(u32 YIndex = 0;
        YIndex < ASSET_IMPORT_GRID_MAX;
        ++YIndex)
    {
        for(u32 XIndex = 0;
            XIndex < ASSET_IMPORT_GRID_MAX;
            ++XIndex)
        {
            Tags->Tags[YIndex][XIndex] = ImportHead(Assets, XIndex, YIndex);
        }
    }
}

internal void
CreateArtBodyTagGrid(game_assets *Assets, import_grid_tags *Tags)
{
    for(u32 YIndex = 0;
        YIndex < ASSET_IMPORT_GRID_MAX;
        ++YIndex)
    {
        for(u32 XIndex = 0;
            XIndex < ASSET_IMPORT_GRID_MAX;
            ++XIndex)
        {
            Tags->Tags[YIndex][XIndex] = ImportBody(Assets, XIndex, YIndex);
        }
    }
}

internal void
CreateArtCharacterTagGrid(game_assets *Assets, import_grid_tags *Tags)
{
    for(u32 YIndex = 0;
        YIndex < ASSET_IMPORT_GRID_MAX;
        ++YIndex)
    {
        for(u32 XIndex = 0;
            XIndex < ASSET_IMPORT_GRID_MAX;
            ++XIndex)
        {
            import_grid_tag *Tag = &Tags->Tags[YIndex][XIndex];
            if(YIndex <= 3)
            {
                *Tag = ImportBody(Assets, XIndex, YIndex);
            }
            else
            {
                *Tag = ImportHead(Assets, XIndex, YIndex - 4);
            }
        }
    }
}

internal void
CreateArtParticleTagGrid(game_assets *Assets, import_grid_tags *Tags)
{
    for(u32 YIndex = 0;
        YIndex < 4;
        ++YIndex)
    {
        for(u32 XIndex = 0;
            XIndex < 4;
            ++XIndex)
        {
            u32 Variant = 4*YIndex + XIndex;
            import_grid_tag *Tag = &Tags->Tags[YIndex][XIndex];
            tag_builder Builder = BeginTags(Assets);
            AddTag(&Builder, Tag_Variant, (1.0f / 32.0f) + ((f32)Variant / 16.0f));
            *Tag = EndTags(&Builder, Asset_Particle);
            Tag->VariantGroup = 1;
        }
    }
    Tags->VariantGroupCount = 2;
}

internal void
CreateArtHandTagGrid(game_assets *Assets, import_grid_tags *Tags)
{
    for(u32 YIndex = 0;
        YIndex < ASSET_IMPORT_GRID_MAX;
        ++YIndex)
    {
        for(u32 XIndex = 0;
            XIndex < ASSET_IMPORT_GRID_MAX;
            ++XIndex)
        {
            import_grid_tag *Tag = &Tags->Tags[YIndex][XIndex];
            if((XIndex == 0) && (YIndex < 4))
            {
                tag_builder Builder = BeginTags(Assets);
                AddTag(&Builder, Tag_FacingDirection, (f32)YIndex / 4.0f);
                *Tag = EndTags(&Builder, Asset_Hand);
            }
        }
    }
}

internal void
CreateArtItemTagGrid(game_assets *Assets, import_grid_tags *Tags)
{
    for(u32 YIndex = 0;
        YIndex < 4;
        ++YIndex)
    {
        for(u32 XIndex = 0;
            XIndex < 4;
            ++XIndex)
        {
            u32 VariantGroup = 0;
            import_grid_tag *Tag = &Tags->Tags[YIndex][XIndex];
            tag_builder Builder = BeginTags(Assets);
            switch(XIndex)
            {
                case 0:
                {
                    AddTag(&Builder, Tag_Variant, 0.25f);
                    AddTag(&Builder, Tag_Floor, 1.0f);
                    VariantGroup = YIndex + 1;
                } break;
                
                case 1:
                {
                    AddTag(&Builder, Tag_Variant, 0.75f);
                    AddTag(&Builder, Tag_Floor, 1.0f);
                    VariantGroup = YIndex + 1;
                } break;
                
                case 2:
                {
                    AddTag(&Builder, Tag_Worn, 1.0f);
                } break;
            }
            
            AddTag(&Builder, Tag_FacingDirection, (f32)YIndex / 4.0f);
            *Tag = EndTags(&Builder, Asset_Item);
            Tag->VariantGroup = VariantGroup;
        }
    }
    
    Tags->VariantGroupCount = 5;
}

internal void
CreateArtSceneryTagGrid(game_assets *Assets, import_grid_tags *Tags)
{
    for(u32 YIndex = 0;
        YIndex < 4;
        ++YIndex)
    {
        for(u32 XIndex = 0;
            XIndex < 4;
            ++XIndex)
        {
            import_grid_tag *Tag = &Tags->Tags[YIndex][XIndex];
            tag_builder Builder = BeginTags(Assets);
            AddTag(&Builder, Tag_Variant, ((f32)XIndex) / 4.0f);
            AddTag(&Builder, Tag_FacingDirection, (f32)YIndex / 4.0f);
            *Tag = EndTags(&Builder, Asset_Scenery);
            Tag->VariantGroup = YIndex + 1;
        }
    }
    
    Tags->VariantGroupCount = 5;
}

internal void
CreateArtPlateTagGrid(game_assets *Assets, import_grid_tags *Tags)
{
    import_grid_tag *Tag = &Tags->Tags[0][0];
    
    tag_builder Builder = BeginTags(Assets);
    *Tag = EndTags(&Builder, Asset_Plate);
}

internal import_tag_array
ParseTagList(game_assets *Assets, tokenizer *Tokenizer)
{
    tag_builder Builder = BeginTags(Assets);
    
    while(Parsing(Tokenizer))
    {
        token Token = PeekToken(Tokenizer);
        if(Token.Type == Token_Semicolon)
        {
            break;
        }
        else if(Token.Type == Token_Identifier)
        {
            Token = GetToken(Tokenizer);
            f32 TagValue = 1.0f;
            
            token Check = PeekToken(Tokenizer);
            if(Check.Type == Token_OpenParen)
            {
                RequireToken(Tokenizer, Token_OpenParen);
                token Value = RequireToken(Tokenizer, Token_Number);
                TagValue = Value.F32;
                RequireToken(Tokenizer, Token_CloseParen);
                Check = PeekToken(Tokenizer);
            }
            
            asset_tag_id TagID = TagIDFromName(Token.Text);
            if(TagID != Tag_None)
            {
                AddTag(&Builder, TagID, TagValue);
            }
            else
            {
                Error(Tokenizer, Token, "Unrecognized tag name");
            }
            
            if(Check.Type == Token_Semicolon)
            {
                break;
            }
            else if(Check.Type == Token_Comma)
            {
                RequireToken(Tokenizer, Token_Comma);
            }
            else
            {
                Error(Tokenizer, Check, "Expected comma or semicolon");
            }
        }
        else
        {
            Error(Tokenizer, Token, "Expected a tag name");
        }
    }
    
    import_tag_array Result = EndTags(&Builder, Asset_None).Tags;
    if(Builder.Error)
    {
        Error(Tokenizer, "Out of tag space.");
    }
    
    return(Result);
}

internal u32
GetOrCreateHHAByStem(game_assets *Assets, string Stem, b32x CreateIfNotFound)
{
    u32 Result = 0;
    
    for(u32 HHAIndex = 1;
        HHAIndex < Assets->FileCount;
        ++HHAIndex)
    {
        asset_file *File = Assets->Files + HHAIndex;
        if(StringsAreEqual(File->Stem, Stem))
        {
            Result = HHAIndex;
            break;
        }
    }
    
    if(CreateIfNotFound && (Result == 0))
    {
        platform_file_group FileGroup = Platform.GetAllFilesOfTypeBegin(PlatformFileType_AssetFile);
        
        char Path[4096];
        FormatString(sizeof(Path), Path, "data/%S.hha", Stem);
        
        platform_file_info *FileInfo = Platform.GetFileByPath(&FileGroup, Path, OpenFile_Read|OpenFile_Write);
        if(FileInfo)
        {
            platform_file_handle Handle = Platform.OpenFile(&FileGroup, FileInfo, OpenFile_Write);
            if(PlatformNoFileErrors(&Handle))
            {
                hha_header Header = {};
                Header.MagicValue = HHA_MAGIC_VALUE;
                Header.Version = HHA_VERSION;
                Platform.WriteDataToFile(&Handle, 0, sizeof(Header), &Header);
            }
            Platform.CloseFile(&Handle);
            
            if(PlatformNoFileErrors(&Handle))
            {
                // TODO(casey): It would be much nicer if we actually had a way to refresh the file info here :(
                FileInfo = Platform.GetFileByPath(&FileGroup, Path, OpenFile_Read|OpenFile_Write);
                Result = InitSourceHHA(Assets, &FileGroup, FileInfo);
            }
        }
        
        Platform.GetAllFilesOfTypeEnd(&FileGroup);
    }
    
    return(Result);
}

internal void
CopyAllInputUpToButNotIncluding(hht_context *Context, token Token, umm Extra = 0)
{
    if(Context->HHTOut)
    {
        u8 *Start = Context->HHTCopyPoint;
        u8 *End = Token.Text.Data + Extra;
        if(Start < End)
        {
            AppendChunk(Context->HHTOut, End - Start, Start);
            Context->HHTCopyPoint = End;
        }
    }
}

internal void
CopyAllInputUpToAndIncluding(hht_context *Context, token Token)
{
    CopyAllInputUpToButNotIncluding(Context, Token, Token.Text.Count);
}

internal void
IgnoreAllInputUpToAndIncluding(hht_context *Context, token Token)
{
    if(Context->HHTOut)
    {
        Context->HHTCopyPoint = Token.Text.Data + Token.Text.Count;
    }
}

internal void
UpdateSingleAssetMetadata(game_assets *Assets, asset_file *AssetFile,
                          string FileBaseName, stream *Errors,
                          hht_fields *Fields, import_tag_array AppendTags,
                          u32 AssetIndex, import_grid_tag Tags)
{
    asset *Asset = Assets->Assets + AssetIndex;
    RemoveAssetFromHash(Assets, AssetIndex);
    
    if(!Tags.TypeID)
    {
        Outf(Errors, "Sprite found in what is required to be a blank tile.\n");
    }
    
    b32x TagsDiffer = false;
    u32x TotalTagCount = GetTagCount(Tags.Tags) + GetTagCount(AppendTags);
    if(TotalTagCount == (Asset->HHA.OnePastLastTagIndex - Asset->HHA.FirstTagIndex))
    {
        u32 TestTagIndex = Asset->HHA.FirstTagIndex;
        for(u32 TagIndex = Tags.Tags.FirstTagIndex;
            TagIndex < Tags.Tags.OnePastLastTagIndex;
            ++TagIndex)
        {
            hha_tag *SourceTag = Assets->Tags + TagIndex;
            if(!TagsAreEqual(Assets->Tags[TestTagIndex++], *SourceTag))
            {
                TagsDiffer = true;
            }
        }
        
        for(u32 TagIndex = AppendTags.FirstTagIndex;
            TagIndex < AppendTags.OnePastLastTagIndex;
            ++TagIndex)
        {
            hha_tag *SourceTag = Assets->Tags + TagIndex;
            if(!TagsAreEqual(Assets->Tags[TestTagIndex++], *SourceTag))
            {
                TagsDiffer = true;
            }
        }
    }
    else
    {
        TagsDiffer = true;
    }
    
    if(TagsDiffer)
    {
        tag_builder Builder = BeginTags(Assets);
        for(u32 TagIndex = Tags.Tags.FirstTagIndex;
            TagIndex < Tags.Tags.OnePastLastTagIndex;
            ++TagIndex)
        {
            hha_tag *SourceTag = Assets->Tags + TagIndex;
            AddTag(&Builder, (asset_tag_id)SourceTag->ID, SourceTag->Value);
        }
        for(u32 TagIndex = AppendTags.FirstTagIndex;
            TagIndex < AppendTags.OnePastLastTagIndex;
            ++TagIndex)
        {
            hha_tag *SourceTag = Assets->Tags + TagIndex;
            AddTag(&Builder, (asset_tag_id)SourceTag->ID, SourceTag->Value);
        }
        import_grid_tag CombinedTags = EndTags(&Builder, Asset_None);
        if(Builder.Error)
        {
            Outf(Errors, "Out of tag space.\n");
        }
        
        Asset->HHA.FirstTagIndex = CombinedTags.Tags.FirstTagIndex;
        Asset->HHA.OnePastLastTagIndex = CombinedTags.Tags.OnePastLastTagIndex;
        
        AssetFile->Modified = true;
    }
    
    // TODO(casey): We write strings out for every asset on every
    // import because we don't want to complicate the code path
    // to do string compaction and potentially introduce bugs.
    // We will do string compaction as an hhaedit pass if we
    // really want it.
    UpdateAssetString(Assets, AssetFile, Fields->Name,
                      &Asset->Annotation.AssetNameCount,
                      &Asset->Annotation.AssetNameOffset);
    UpdateAssetString(Assets, AssetFile, Fields->Description,
                      &Asset->Annotation.AssetDescriptionCount,
                      &Asset->Annotation.AssetDescriptionOffset);
    UpdateAssetString(Assets, AssetFile, Fields->Author,
                      &Asset->Annotation.AuthorCount,
                      &Asset->Annotation.AuthorOffset);
    UpdateAssetString(Assets, AssetFile, FileBaseName,
                      &Asset->Annotation.SourceFileBaseNameCount,
                      &Asset->Annotation.SourceFileBaseNameOffset);
    
    stream AssetErrors = {};
    u32 FileErrorStreamSize = (u32)GetTotalSize(Errors);
    Asset->Annotation.ErrorStreamCount = FileErrorStreamSize + (u32)GetTotalSize(&AssetErrors);
    Asset->Annotation.ErrorStreamOffset =
        ReserveData(Assets, AssetFile, Asset->Annotation.ErrorStreamCount);
    WriteAssetStream(AssetFile, Asset->Annotation.ErrorStreamOffset, Errors);
    WriteAssetStream(AssetFile, Asset->Annotation.ErrorStreamOffset + FileErrorStreamSize, &AssetErrors);

    if(!AddAssetToHash(Assets, AssetIndex))
    {
        Outf(Errors, "Invalid tag set.\n");
    }
}

internal void
UpdateAssetVariants(game_assets *Assets, asset_source_file *File,
                    memory_arena *TempMemory, import_grid_tags *Grid)
{
    if(Grid->VariantGroupCount)
    {
        asset_file *AssetFile = GetFile(Assets, File->DestFileIndex);
        
        u32 XCount = ArrayCount(File->AssetIndices[0]);
        u32 YCount = ArrayCount(File->AssetIndices);
        
        u32 *VariantCount = PushArray(TempMemory, Grid->VariantGroupCount, u32);
        for(u32 YIndex = 0;
            YIndex < YCount;
            ++YIndex)
        {
            for(u32 XIndex = 0;
                XIndex < XCount;
                ++XIndex)
            {
                u32 AssetIndex = File->AssetIndices[YIndex][XIndex];
                if(AssetIndex)
                {
                    import_grid_tag Tags = Grid->Tags[YIndex][XIndex];
                    if(Tags.VariantGroup)
                    {
                        Assert(Tags.VariantGroup < Grid->VariantGroupCount);
                        ++VariantCount[Tags.VariantGroup];
                    }
                }
            }
        }
        
        f32 *BucketSize = PushArray(TempMemory, Grid->VariantGroupCount, f32);
        f32 *AtPoint = PushArray(TempMemory, Grid->VariantGroupCount, f32);
        for(u32 VariantGroupIndex = 0;
            VariantGroupIndex < Grid->VariantGroupCount;
            ++VariantGroupIndex)
        {
            u32 Count = VariantCount[VariantGroupIndex];
            if(Count)
            {
                f32 Size = (1.0f / (f32)Count);
                BucketSize[VariantGroupIndex] = Size;
                AtPoint[VariantGroupIndex] = 0.5f*Size;
            }
        }
        
        for(u32 YIndex = 0;
            YIndex < YCount;
            ++YIndex)
        {
            for(u32 XIndex = 0;
                XIndex < XCount;
                ++XIndex)
            {
                u32 AssetIndex = File->AssetIndices[YIndex][XIndex];
                if(AssetIndex)
                {
                    asset *Asset = Assets->Assets + AssetIndex;
                    import_grid_tag Tags = Grid->Tags[YIndex][XIndex];
                    u32 VariantGroupIndex = Tags.VariantGroup;
                    if(VariantGroupIndex)
                    {
                        Assert(VariantGroupIndex < Grid->VariantGroupCount);
                        
                        f32 Point = AtPoint[VariantGroupIndex];
                        AtPoint[VariantGroupIndex] += BucketSize[VariantGroupIndex];
                        
                        u32 TestTagIndex = Asset->HHA.FirstTagIndex;
                        for(u32 TagIndex = Tags.Tags.FirstTagIndex;
                            TagIndex < Tags.Tags.OnePastLastTagIndex;
                            ++TagIndex)
                        {
                            hha_tag *SourceTag = Assets->Tags + TagIndex;
                            if(SourceTag->ID == Tag_Variant)
                            {
                                // TODO(casey): This would break multi-threaded importing,
                                // so we should probably clean up the tags pipeline
                                // so we're not just writing to the same grid all the time.
                                // We should really change the way we're doing tags now,
                                // it's realllllllly antiquated at this point.
                                SourceTag->Value = Point;
                            }
                        }
                    }
                }
            }
        }
    }
}

internal void
UpdateAssetMetadata(game_assets *Assets, asset_source_file *File,
                    hht_fields *Fields, import_grid_tags *Grid, import_tag_array AppendTags)
{
    // TODO(casey): Really, what we should do here is very different from this I think.
    // We should just produce the entire HHA header information, and then _diff_ that
    // in binary with the one that is currently in the file, and if they are not the
    // same, we rewrite.  This would be less error-prone.  The only tough part about it
    // is we would have to make sure that the edit process doesn't "rearrange" things
    // in the header if no edits have occurred, but that seems pretty easy to ensure.
    
    asset_file *AssetFile = GetFile(Assets, File->DestFileIndex);
    
    u32 XCount = ArrayCount(File->AssetIndices[0]);
    u32 YCount = ArrayCount(File->AssetIndices);
    
    for(u32 YIndex = 0;
        YIndex < YCount;
        ++YIndex)
    {
        for(u32 XIndex = 0;
            XIndex < XCount;
            ++XIndex)
        {
            u32 AssetIndex = File->AssetIndices[YIndex][XIndex];
            if(AssetIndex)
            {
                import_grid_tag Tags = Grid->Tags[YIndex][XIndex];
                
                UpdateSingleAssetMetadata(Assets, AssetFile, File->BaseName, &File->Errors, Fields, AppendTags, AssetIndex, Tags);
            }
        }
    }
}

internal void
OutPoint(hht_context *Context, u32 GridX, u32 GridY, u32 PointIndex, hha_align_point Point)
{
    Outf(Context->HHTOut, "Align[%u,%u][%u] = %u, %u, %u, %S%s;",
         GridX, GridY, PointIndex,
         Point.PPercent[0], Point.PPercent[1], Point.Size,
         AlignPointNameFromType(GetType(Point)),
         IsToParent(Point) ? " | ToParent" : "");
}

internal b32x
HandleCommonFields(tokenizer *Tokenizer, hht_context *Context, token Token,
                   hht_fields *Fields, import_tag_array *AppendTags)
{
    b32x Handled = true;
    
    if(TokenEquals(Token, "Author"))
    {
        RequireToken(Tokenizer, Token_Equals);
        Fields->Author = RequireToken(Tokenizer, Token_String).Text;
    }
    else if(TokenEquals(Token, "Description"))
    {
        RequireToken(Tokenizer, Token_Equals);
        Fields->Description = RequireToken(Tokenizer, Token_String).Text;
    }
    else if(TokenEquals(Token, "Name"))
    {
        RequireToken(Tokenizer, Token_Equals);
        Fields->Name = RequireToken(Tokenizer, Token_String).Text;
    }
    else if(AppendTags && TokenEquals(Token, "Tags"))
    {
        RequireToken(Tokenizer, Token_Equals);
        *AppendTags = ParseTagList(Context->Assets, Tokenizer);
    }
    else
    {
        Handled = false;
    }
    
    return(Handled);
}

internal void
ParseDefaultBlock(tokenizer *Tokenizer, hht_context *Context, token BlockToken)
{
    hht_fields Fields = Context->DefaultFields;
    
    token OpenBrace = RequireToken(Tokenizer, Token_OpenBrace);
    
    while(Parsing(Tokenizer))
    {
        token Token = GetToken(Tokenizer);
        if(Token.Type == Token_CloseBrace)
        {
            break;
        }
        else if(!HandleCommonFields(Tokenizer, Context, Token, &Fields, 0))
        {
            Error(Tokenizer, Token, "Expected field name");
        }
        
        RequireToken(Tokenizer, Token_Semicolon);
    }
    
    if(Parsing(Tokenizer))
    {
        Context->DefaultFields = Fields;
    }
    
    token Semicolon = RequireToken(Tokenizer, Token_Semicolon);
    CopyAllInputUpToAndIncluding(Context, Semicolon);
}

internal token
RequireField(tokenizer *Tokenizer, token_type Type, char *Name)
{
    RequireIdentifier(Tokenizer, Name);
    RequireToken(Tokenizer, Token_Equals);
    token Result = RequireToken(Tokenizer, Type);
    RequireToken(Tokenizer, Token_Semicolon);
    return(Result);
}

internal b32x
BlockDiffers(memory_arena *TempMem, platform_file_handle *FileHandle, u64 DataOffset, u32 Size, void *TestValue)
{
    temporary_memory Temp = BeginTemporaryMemory(TempMem);
    
    b32x Result = false;
    
    void *FileValue = PushSize(TempMem, Size, NoClear());
    Platform.ReadDataFromFile(FileHandle, DataOffset, Size, FileValue);
    if(PlatformNoFileErrors(FileHandle))
    {
        Result = MemoryIsEqual(Size, FileValue, TestValue);
    }
    
    EndTemporaryMemory(Temp);
    return(Result);
}

internal b32x
UpdateAssetDataFromFile(hht_context *Context, import_file_match Match,
                        import_type ImportType, hht_fields *Fields,
                        import_grid_tags *TemplateTags,
                        import_tag_array AppendTags,
                        hha_align_point (*AlignPoints)[ASSET_IMPORT_GRID_MAX][ASSET_IMPORT_GRID_MAX][HHA_BITMAP_ALIGN_POINT_COUNT])
{
    b32x Result = false;
    memory_arena *TempArena = Context->TempArena;
    
    if(Match.Valid)
    {
        if(Match.Source->DestFileIndex != Context->HHAIndex)
        {
            Match.Source->DestFileIndex = Context->HHAIndex;
            Match.NeedsFullRebuild = true;
        }
        
        if(Match.NeedsFullRebuild)
        {
            temporary_memory TempMarker = BeginTemporaryMemory(TempArena);
            
            platform_file_handle Handle = Platform.OpenFile(&Context->FileGroup, Match.FileInfo, OpenFile_Read);
            buffer FileBuffer = {};
            FileBuffer.Count = Match.FileInfo->FileSize;
            FileBuffer.Data = (u8 *)PushSize(TempArena, FileBuffer.Count);
            Platform.ReadDataFromFile(&Handle, 0, FileBuffer.Count, FileBuffer.Data);
            Platform.CloseFile(&Handle);
            
            // NOTE(casey): We update this _first_, because assets that
            // get packed from here on out need to be able to stamp themselves
            // with the right data!
            Match.Source->FileDate = Match.FileInfo->FileDate;
            Match.Source->FileCheckSum = CheckSumOf(FileBuffer);
            
            stream ContentsStream = MakeReadStream(FileBuffer, &Match.Source->Errors);
            image_u32 Image = {};
            sound_s16 Sound = {};
            
            switch(ImportType)
            {
                case ImportType_None:
                {
                } break;
                
                case ImportType_MultiTile:
                {
                    Image = ParsePNG(TempArena, ContentsStream);
                    ProcessMultiTileImport(Context->Assets, Match.Source, Image, TempArena);
                } break;
                
                case ImportType_SingleTile:
                {
                    Image = ParsePNG(TempArena, ContentsStream);
                    ProcessSingleTileImport(Context->Assets, Match.Source, Image, TempArena,
                                            ASSET_MAX_SPRITE_DIM);
                } break;
                
                case ImportType_Plate:
                {
                    Image = ParsePNG(TempArena, ContentsStream);
                    ProcessSingleTileImport(Context->Assets, Match.Source, Image, TempArena,
                                            ASSET_MAX_PLATE_DIM);
                } break;
                
                case ImportType_Audio:
                {
                    Sound = ParseWAV(TempArena, FileBuffer, &Match.Source->Errors);
                    ProcessAudioImport(Context->Assets, Match.Source, Sound, TempArena);
                } break;
                
                InvalidDefaultCase;
            }
            
            EndTemporaryMemory(TempMarker);
        }
        
        UpdateAssetVariants(Context->Assets, Match.Source, TempArena, TemplateTags);
        UpdateAssetMetadata(Context->Assets, Match.Source, Fields, TemplateTags, AppendTags);
        
        if(AlignPoints && !Context->HHTOut)
        {
            //
            // TODO(casey): Shouldn't this be marking the assets as having been changed
            // any time a change is detected in the alignment points?
            //
            
            for(u32 GridY = 0;
                GridY < ASSET_IMPORT_GRID_MAX;
                ++GridY)
            {
                for(u32 GridX = 0;
                    GridX < ASSET_IMPORT_GRID_MAX;
                    ++GridX)
                {
                    u32 AssetIndex = Match.Source->AssetIndices[GridY][GridX];
                    if(AssetIndex)
                    {
                        asset *Asset = GetAsset(Context->Assets, AssetIndex);
                        Assert(Asset);
                        Assert(Asset->HHA.Type == HHAAsset_Bitmap);
                        
                        hha_bitmap *Bitmap = &Asset->HHA.Bitmap;
                        for(u32 PointIndex = 0;
                            PointIndex < HHA_BITMAP_ALIGN_POINT_COUNT;
                            ++PointIndex)
                        {
                            Bitmap->AlignPoints[PointIndex] = (*AlignPoints)[GridY][GridX][PointIndex];
                        }
                    }
                }
            }
        }
        
        Result = true;
    }
    
    return(Result);
}

internal import_file_match
FindMatchingFileFor(hht_context *Context, tokenizer *Tokenizer, token SourceToken, char *SubDir, string BaseName)
{
    import_file_match Result = {};
    
    char Path[4096];
    FormatString(sizeof(Path), Path, "sources/%S/%s/%S", Context->HHAStem, SubDir, BaseName);
    
    Result.FileInfo = Platform.GetFileByPath(&Context->FileGroup, Path, OpenFile_Read);
    if(Result.FileInfo)
    {
        Result.Source = GetOrCreateAssetSourceFile(Context->Assets, Path);
        Assert(Result.Source);
        if(Result.Source->FileDate != Result.FileInfo->FileDate)
        {
            Result.NeedsFullRebuild = true;
        }
        
        Result.Valid = true;
    }
    else
    {
        Error(Tokenizer, SourceToken, "File not found (looked in %s)", Path);
    }
    
    return(Result);
}

internal void
ParseFontBlock(tokenizer *Tokenizer, hht_context *Context, token BlockToken)
{
    game_assets *Assets = Context->Assets;
    hht_fields Fields = Context->DefaultFields;
    import_tag_array AppendTags = {};
    import_grid_tags TemplateTags = {};
    TemplateTags.Tags[0][0].TypeID = Asset_FontGlyph;
    hha_align_point AlignPoints[ASSET_IMPORT_GRID_MAX][ASSET_IMPORT_GRID_MAX][HHA_BITMAP_ALIGN_POINT_COUNT] = {};
    
    token FontName = RequireToken(Tokenizer, Token_String);
    token OpenBrace = RequireToken(Tokenizer, Token_OpenBrace);
    token GlyphCountToken = RequireField(Tokenizer, Token_Number, "GlyphCount");
    if(Parsing(Tokenizer) && (GlyphCountToken.S32 > 0))
    {
        asset_source_file *FontSourceFile = GetOrCreateAssetSourceFile(Assets, FontName.Text);
        u32 GlyphCount = (u32)GlyphCountToken.S32;
        
        hha_font_glyph *CodePoints = PushArray(Context->TempArena, GlyphCount, hha_font_glyph);
        f32 *HorizontalAdvance = PushArray(Context->TempArena, GlyphCount*GlyphCount, f32);
        
        f32 AscenderHeight = 0;
        f32 DescenderHeight = 0;
        f32 ExternalLeading = 0;
        u32 OnePastHighestCodepoint = 0;
        
        while(Parsing(Tokenizer))
        {
            token Token = GetToken(Tokenizer);
            if(Token.Type == Token_CloseBrace)
            {
                break;
            }
            else if(TokenEquals(Token, "AscenderHeight"))
            {
                RequireToken(Tokenizer, Token_Equals);
                AscenderHeight = RequireToken(Tokenizer, Token_Number).F32;
            }
            else if(TokenEquals(Token, "DescenderHeight"))
            {
                RequireToken(Tokenizer, Token_Equals);
                DescenderHeight = RequireToken(Tokenizer, Token_Number).F32;
            }
            else if(TokenEquals(Token, "ExternalLeading"))
            {
                RequireToken(Tokenizer, Token_Equals);
                ExternalLeading = RequireToken(Tokenizer, Token_Number).F32;
            }
            else if(TokenEquals(Token, "Glyph"))
            {
                RequireToken(Tokenizer, Token_OpenBracket);
                token GlyphToken = RequireToken(Tokenizer, Token_Number);
                RequireToken(Tokenizer, Token_CloseBracket);
                u32 GlyphIndex = GlyphToken.S32;
                
                RequireToken(Tokenizer, Token_Equals);
                
                token FileName = RequireToken(Tokenizer, Token_String);
                RequireToken(Tokenizer, Token_Comma);
                u32 Codepoint = (u32)RequireToken(Tokenizer, Token_Number).S32;
                RequireToken(Tokenizer, Token_Comma);
                RequireToken(Tokenizer, Token_OpenBrace);
                u32 AlignX = (u32)RequireToken(Tokenizer, Token_Number).S32;
                RequireToken(Tokenizer, Token_Comma);
                u32 AlignY = (u32)RequireToken(Tokenizer, Token_Number).S32;
                RequireToken(Tokenizer, Token_CloseBrace);
                
                SetAlignPoint(&AlignPoints[0][0][0], HHAAlign_Default, true,
                              1.0f, V2(0.5f, 0.5f));
                
                if(GlyphIndex < GlyphCount)
                {
                    if((AlignX <= U16Max) && (AlignY <= U16Max))
                    {
                        CodePoints[GlyphIndex].UnicodeCodePoint = Codepoint;
                        if(OnePastHighestCodepoint <= Codepoint)
                        {
                            OnePastHighestCodepoint = Codepoint + 1;
                        }
                        
                        AlignPoints[0][0][0].PPercent[0] = (u16)AlignX;
                        AlignPoints[0][0][0].PPercent[1] = (u16)AlignY;
                    }
                    else
                    {
                        Error(Tokenizer, GlyphToken, "Glyph has bad alignment values");
                    }
                }
                else
                {
                    Error(Tokenizer, GlyphToken, "Glyph index too high for font (%u exceeds limit of %u)",
                          GlyphIndex, GlyphCount);
                }
                
                if(Parsing(Tokenizer))
                {
                    import_file_match Match = FindMatchingFileFor(Context, Tokenizer, GlyphToken, "fonts", FileName.Text);
                    if(Match.Valid)
                    {
                        import_tag_array NullTags = {};
                        UpdateAssetDataFromFile(Context, Match, ImportType_SingleTile,
                                                &Fields, &TemplateTags, NullTags, &AlignPoints);
                        CodePoints[GlyphIndex].BitmapID = Match.Source->AssetIndices[0][0];
                    }
                    else
                    {
                        Error(Tokenizer, GlyphToken, "Unable to load font glyph");
                    }
                }
            }
            else if(TokenEquals(Token, "HorizontalAdvance"))
            {
                RequireToken(Tokenizer, Token_Equals);
                for(u32 ReadIndex = 0;
                    ReadIndex < (GlyphCount*GlyphCount);
                    ++ReadIndex)
                {
                    if(ReadIndex > 0)
                    {
                        RequireToken(Tokenizer, Token_Comma);
                    }
                    HorizontalAdvance[ReadIndex] = RequireToken(Tokenizer, Token_Number).F32;
                }
            }
            else if(!HandleCommonFields(Tokenizer, Context, Token, &Fields, &AppendTags))
            {
                Error(Tokenizer, Token, "Expected field name");
            }
            
            RequireToken(Tokenizer, Token_Semicolon);
        }
        
        if(Parsing(Tokenizer))
        {
            FontSourceFile->DestFileIndex = Context->HHAIndex;
            asset_file *AssetFile = GetFile(Assets, FontSourceFile->DestFileIndex);
            for(u32 GlyphIndex = 0;
                GlyphIndex < GlyphCount;
                ++GlyphIndex)
            {
                CodePoints[GlyphIndex].BitmapID -= AssetFile->AssetBase;
            }
            
            b32x NeedsFullRebuild = false;
            
            u32 AssetIndex = FontSourceFile->AssetIndices[0][0];
            if(!AssetIndex)
            {
                NeedsFullRebuild = true;
                AssetIndex = ReserveAsset(Assets);
            }
            
            tag_builder Builder = BeginTags(Assets);
            import_grid_tag ExtraTags = EndTags(&Builder, Asset_Font);
            
            asset *Asset = Assets->Assets + AssetIndex;
            Asset->HHA.Type = HHAAsset_Font;
            Asset->FileIndex = FontSourceFile->DestFileIndex;
            
            hha_font *Font = &Asset->HHA.Font;
            u32 CodePointSize = SafeTruncateToU32(GlyphCount*sizeof(hha_font_glyph));
            u32 HorizontalAdvanceSize = SafeTruncateToU32(GlyphCount*GlyphCount*sizeof(f32));
            u32 DataSize = (CodePointSize + HorizontalAdvanceSize);
            NeedsFullRebuild = (NeedsFullRebuild ||
                                (Asset->HHA.DataSize != DataSize) ||
                                (Font->OnePastHighestCodepoint != OnePastHighestCodepoint) ||
                                (Font->GlyphCount != GlyphCount) ||
                                (Font->AscenderHeight != AscenderHeight) ||
                                (Font->DescenderHeight != DescenderHeight) ||
                                (Font->ExternalLeading != ExternalLeading) ||
                                BlockDiffers(Context->TempArena, &AssetFile->Handle, Asset->HHA.DataOffset, CodePointSize, CodePoints) ||
                                BlockDiffers(Context->TempArena, &AssetFile->Handle, Asset->HHA.DataOffset + CodePointSize, HorizontalAdvanceSize, HorizontalAdvance));
            
            Font->OnePastHighestCodepoint = OnePastHighestCodepoint;
            Font->GlyphCount = GlyphCount;
            Font->AscenderHeight = AscenderHeight;
            Font->DescenderHeight = DescenderHeight;
            Font->ExternalLeading = ExternalLeading;
            
            UpdateSingleAssetMetadata(Assets, AssetFile,
                                      Context->HHAStem, Tokenizer->ErrorStream,
                                      &Fields, AppendTags, AssetIndex, ExtraTags);
            
            if(NeedsFullRebuild)
            {
                Asset->HHA.DataOffset = ReserveData(Assets, AssetFile, DataSize);
                WriteAssetData(AssetFile, Asset->HHA.DataOffset, CodePointSize, CodePoints);
                WriteAssetData(AssetFile, Asset->HHA.DataOffset + CodePointSize, HorizontalAdvanceSize, HorizontalAdvance);
            }
        }
    }
    
    token Semicolon = RequireToken(Tokenizer, Token_Semicolon);
    CopyAllInputUpToAndIncluding(Context, Semicolon);
}

internal void
ParseTopLevelBlock(tokenizer *Tokenizer, hht_context *Context, token BlockToken)
{
    memory_arena *TempArena = Context->TempArena;
    
    hht_fields Fields = Context->DefaultFields;
    
    //
    // NOTE(casey): Determine the import type from the block type
    //
    
    b32x IsArt = false;
    
    import_grid_tags *TemplateTags = 0;
    import_type ImportType = ImportType_None;
    char *SubDir = "";
    
    if(TokenEquals(BlockToken, "music"))
    {
        ImportType = ImportType_Audio;
        TemplateTags = &Context->Assets->AudioChannelTags;
        
        SubDir = "music";
    }
    else if(TokenEquals(BlockToken, "sound"))
    {
        ImportType = ImportType_Audio;
        TemplateTags = &Context->Assets->AudioChannelTags;
        
        SubDir = "sound";
    }
    else
    {
        IsArt = true;
        SubDir = "art";
        
        if(TokenEquals(BlockToken, "block"))
        {
            TemplateTags = &Context->Assets->ArtBlockTags;
            ImportType = ImportType_SingleTile;
        }
        else if(TokenEquals(BlockToken, "head"))
        {
            TemplateTags = &Context->Assets->ArtHeadTags;
            ImportType = ImportType_MultiTile;
        }
        else if(TokenEquals(BlockToken, "body"))
        {
            TemplateTags = &Context->Assets->ArtBodyTags;
            ImportType = ImportType_MultiTile;
        }
        else if(TokenEquals(BlockToken, "character"))
        {
            TemplateTags = &Context->Assets->ArtCharacterTags;
            ImportType = ImportType_MultiTile;
        }
        else if(TokenEquals(BlockToken, "particle"))
        {
            TemplateTags = &Context->Assets->ArtParticleTags;
            ImportType = ImportType_MultiTile;
        }
        else if(TokenEquals(BlockToken, "hand"))
        {
            TemplateTags = &Context->Assets->ArtHandTags;
            ImportType = ImportType_MultiTile;
        }
        else if(TokenEquals(BlockToken, "item"))
        {
            TemplateTags = &Context->Assets->ArtItemTags;
            ImportType = ImportType_MultiTile;
        }
        else if(TokenEquals(BlockToken, "scenery"))
        {
            TemplateTags = &Context->Assets->ArtSceneryTags;
            ImportType = ImportType_MultiTile;
        }
        else if(TokenEquals(BlockToken, "plate"))
        {
            TemplateTags = &Context->Assets->ArtPlateTags;
            ImportType = ImportType_Plate;
        }
        else
        {
            Error(Tokenizer, BlockToken, "Unrecognized block type");
        }
    }
    
    //
    // NOTE(casey): Locate the matching asset file
    //
    
    b32x NeedsFullRebuild = false;
    
    token FileName = RequireToken(Tokenizer, Token_String);
    import_file_match Match = FindMatchingFileFor(Context, Tokenizer, FileName, SubDir, FileName.Text);
    
    token OpenBrace = RequireToken(Tokenizer, Token_OpenBrace);
    CopyAllInputUpToAndIncluding(Context, OpenBrace);
    
    //
    // NOTE(casey): Prepare the existing tag grid
    //
    
    import_tag_array AppendTags = {};
    b32x AlignPointUnprocessed[ASSET_IMPORT_GRID_MAX][ASSET_IMPORT_GRID_MAX][HHA_BITMAP_ALIGN_POINT_COUNT] = {};
    hha_align_point AlignPoints[ASSET_IMPORT_GRID_MAX][ASSET_IMPORT_GRID_MAX][HHA_BITMAP_ALIGN_POINT_COUNT] = {};
    
    if(IsArt)
    {
        if(Context->HHTOut && Match.Valid)
        {
            for(u32 GridY = 0;
                GridY < ASSET_IMPORT_GRID_MAX;
                ++GridY)
            {
                for(u32 GridX = 0;
                    GridX < ASSET_IMPORT_GRID_MAX;
                    ++GridX)
                {
                    u32 AssetIndex = Match.Source->AssetIndices[GridY][GridX];
                    if(AssetIndex)
                    {
                        asset *Asset = GetAsset(Context->Assets, AssetIndex);
                        Assert(Asset);
                        Assert(Asset->HHA.Type == HHAAsset_Bitmap); // TODO(casey): May need to relax when we do sound.
                        
                        hha_bitmap *Bitmap = &Asset->HHA.Bitmap;
                        for(u32 PointIndex = 0;
                            PointIndex < HHA_BITMAP_ALIGN_POINT_COUNT;
                            ++PointIndex)
                        {
                            if(Bitmap->AlignPoints[PointIndex].Type != HHAAlign_None)
                            {
                                AlignPointUnprocessed[GridY][GridX][PointIndex] = true;
                                AlignPoints[GridY][GridX][PointIndex] = Bitmap->AlignPoints[PointIndex];
                            }
                        }
                    }
                }
            }
        }
    }
    
    //
    // NOTE(casey): Parse the contents of the HHT block
    //
    
    while(Parsing(Tokenizer))
    {
        token Semicolon = {};
        token Token = GetToken(Tokenizer);
        if(Token.Type == Token_CloseBrace)
        {
            if(IsArt && (Context->HHTOut))
            {
                CopyAllInputUpToButNotIncluding(Context, Token);
                for(u32 GridY = 0;
                    GridY < ASSET_IMPORT_GRID_MAX;
                    ++GridY)
                {
                    for(u32 GridX = 0;
                        GridX < ASSET_IMPORT_GRID_MAX;
                        ++GridX)
                    {
                        for(u32 PointIndex = 0;
                            PointIndex < HHA_BITMAP_ALIGN_POINT_COUNT;
                            ++PointIndex)
                        {
                            if(AlignPointUnprocessed[GridY][GridX][PointIndex])
                            {
                                hha_align_point Point = AlignPoints[GridY][GridX][PointIndex];
                                Outf(Context->HHTOut, "    ");
                                OutPoint(Context, GridX, GridY, PointIndex, Point);
                                Outf(Context->HHTOut, "\n");
                            }
                        }
                    }
                }
                CopyAllInputUpToAndIncluding(Context, Token);
            }
            
            break;
        }
        else if(TokenEquals(Token, "Align"))
        {
            if(!IsArt)
            {
                Error(Tokenizer, Token, "Alignment points not allowed on audio assets");
            }
            
            CopyAllInputUpToButNotIncluding(Context, Token);
            
            RequireToken(Tokenizer, Token_OpenBracket);
            s32 GridX = RequireIntegerRange(Tokenizer, 0, ASSET_IMPORT_GRID_MAX - 1).S32;
            RequireToken(Tokenizer, Token_Comma);
            s32 GridY = RequireIntegerRange(Tokenizer, 0, ASSET_IMPORT_GRID_MAX - 1).S32;
            RequireToken(Tokenizer, Token_CloseBracket);
            RequireToken(Tokenizer, Token_OpenBracket);
            s32 Index = RequireIntegerRange(Tokenizer, 0, HHA_BITMAP_ALIGN_POINT_COUNT - 1).S32;
            RequireToken(Tokenizer, Token_CloseBracket);
            RequireToken(Tokenizer, Token_Equals);
            
            s32 PPercent0 = RequireIntegerRange(Tokenizer, 0, U16Max).S32;
            RequireToken(Tokenizer, Token_Comma);
            s32 PPercent1 = RequireIntegerRange(Tokenizer, 0, U16Max).S32;
            RequireToken(Tokenizer, Token_Comma);
            s32 Size = RequireIntegerRange(Tokenizer, 0, U16Max).S32;
            RequireToken(Tokenizer, Token_Comma);
            token Type0 = RequireToken(Tokenizer, Token_Identifier);
            u16 Type = (u16)AlignPointTypeFromName(Type0.Text);
            if(Type != HHAAlign_None)
            {
                token Type1 = {};
                if(OptionalToken(Tokenizer, Token_Or))
                {
                    Type1 = RequireToken(Tokenizer, Token_Identifier);
                    if(TokenEquals(Type1, "ToParent"))
                    {
                        Type |= HHAAlign_ToParent;
                    }
                    else
                    {
                        Error(Tokenizer, Type0, "Expected \"ToParent\"");
                    }
                }
                
                Semicolon = RequireToken(Tokenizer, Token_Semicolon);
                if(Parsing(Tokenizer))
                {
                    hha_align_point Point = {};
                    Point.PPercent[0] = (u16)PPercent0;
                    Point.PPercent[1] = (u16)PPercent1;
                    Point.Size = (u16)Size;
                    Point.Type = Type;
                    
                    if(Context->HHTOut)
                    {
                        if(MemoryIsEqual(sizeof(Point), &Point, &AlignPoints[GridY][GridX][Index]))
                        {
                            CopyAllInputUpToAndIncluding(Context, Semicolon);
                        }
                        else
                        {
                            IgnoreAllInputUpToAndIncluding(Context, Semicolon);
                            
                            Point = AlignPoints[GridY][GridX][Index];
                            if(Point.Type)
                            {
                                OutPoint(Context, GridX, GridY, Index, Point);
                            }
                        }
                        
                        AlignPointUnprocessed[GridY][GridX][Index] = false;
                    }
                    else
                    {
                        AlignPoints[GridY][GridX][Index] = Point;
                    }
                }
            }
            else
            {
                Error(Tokenizer, Type0, "Unrecognized alignment point type");
            }
        }
        else if(!HandleCommonFields(Tokenizer, Context, Token, &Fields, &AppendTags))
        {
            Error(Tokenizer, Token, "Expected field name");
        }
        
        if(!IsValid(Semicolon))
        {
            Semicolon = RequireToken(Tokenizer, Token_Semicolon);
            CopyAllInputUpToAndIncluding(Context, Semicolon);
        }
    }
    
    RequireToken(Tokenizer, Token_Semicolon);
    
    //
    // NOTE(casey): Import the asset contents
    //
    
    if(Parsing(Tokenizer))
    {
        UpdateAssetDataFromFile(Context, Match, ImportType, &Fields, TemplateTags, AppendTags,
                                IsArt ? &AlignPoints : 0);
    }
}

internal void
ParseHHT(hht_context *Context, platform_file_info *FileInfo, b32x SaveChangesToHHTs)
{
    platform_file_handle Handle = Platform.OpenFile(&Context->FileGroup, FileInfo, OpenFile_Read);
    buffer FileBuffer = {};
    FileBuffer.Count = FileInfo->FileSize;
    FileBuffer.Data = (u8 *)PushSize(Context->TempArena, FileBuffer.Count);
    Platform.ReadDataFromFile(&Handle, 0, FileBuffer.Count, FileBuffer.Data);
    Platform.CloseFile(&Handle);
    
    tokenizer Tokenizer_ = Tokenize(FileBuffer, WrapZ(FileInfo->BaseName));
    tokenizer *Tokenizer = &Tokenizer_;
    Tokenizer->ErrorStream = Context->ErrorStream;
    
    stream HHTStreamStore = {};
    if(SaveChangesToHHTs)
    {
        HHTStreamStore = OnDemandMemoryStream(Context->TempArena);
        Context->HHTOut = &HHTStreamStore;
        Context->HHTCopyPoint = PeekTokenRaw(Tokenizer).Text.Data;
    }
    
    while(Parsing(Tokenizer))
    {
        token Token = GetToken(Tokenizer);
        if(Token.Type == Token_Pound)
        {
            token Directive = GetToken(Tokenizer);
            if(TokenEquals(Directive, "include"))
            {
                if(Context->IncludeDepth++ < 16)
                {
                    token FileName = RequireToken(Tokenizer, Token_String);
                    
                    char Path[4096];
                    FormatString(sizeof(Path), Path, "tags/%S", FileName.Text);
                    platform_file_info *IncludedFileInfo = Platform.GetFileByPath(&Context->FileGroup, Path, OpenFile_Read);
                    if(IncludedFileInfo)
                    {
                        stream *HHTOut = Context->HHTOut;
                        u8 *HHTCopyPoint = Context->HHTCopyPoint;
                        Context->HHTOut = 0;
                        Context->HHTCopyPoint = 0;
                        ParseHHT(Context, IncludedFileInfo, SaveChangesToHHTs);
                        Context->HHTOut = HHTOut;
                        Context->HHTCopyPoint = HHTCopyPoint;
                    }
                    else
                    {
                        Error(Tokenizer, FileName, "Unable to include file");
                    }
                }
                else
                {
                    Error(Tokenizer, Directive, "Maximum include depth exceeded");
                }
            }
            else if(TokenEquals(Directive, "hha"))
            {
                token HHAStem = RequireToken(Tokenizer, Token_String);
                u32 HHAIndex = GetOrCreateHHAByStem(Context->Assets, HHAStem.Text, true);
                if(HHAIndex)
                {
                    Context->HHAStem = HHAStem.Text;
                    Context->HHAIndex = HHAIndex;
                }
                else
                {
                    Error(Tokenizer, "Couldn't open HHA %S for writing", HHAStem.Text);
                }
            }
            else
            {
                Error(Tokenizer, Directive, "Unrecognized directive");
            }
        }
        else if(TokenEquals(Token, "default"))
        {
            ParseDefaultBlock(Tokenizer, Context, Token);
        }
        else if(TokenEquals(Token, "font"))
        {
            ParseFontBlock(Tokenizer, Context, Token);
        }
        else if(Token.Type == Token_Identifier)
        {
            if(Context->HHAIndex)
            {
                ParseTopLevelBlock(Tokenizer, Context, Token);
            }
            else
            {
                Error(Tokenizer, Token, "Import blocks are not allowed to appear before at least one hha directive");
            }
        }
        else if(Token.Type == Token_EndOfStream)
        {
            CopyAllInputUpToAndIncluding(Context, Token);
            break;
        }
        else
        {
            Error(Tokenizer, Token, "Unexpected top-level token");
        }
    }
    
    if(SaveChangesToHHTs && Parsing(Tokenizer))
    {
        buffer HHTContents = PushBuffer(Context->TempArena, GetTotalSize(Context->HHTOut));
        CopyStreamToBuffer(Context->HHTOut, HHTContents);
        if(MemoryIsEqual(FileBuffer, HHTContents))
        {
            Outf(Context->ErrorStream, "%s: No changes.\n", FileInfo->BaseName);
        }
        else
        {
            if(Platform.AtomicReplaceFileContents(FileInfo, HHTContents.Count, HHTContents.Data))
            {
                Outf(Context->ErrorStream, "%s: \\#0f0Replaced\\#fff.\n", FileInfo->BaseName);
            }
            else
            {
                Outf(Context->ErrorStream, "%s: Replacement \\#f00FAILED\\#fff.\n", FileInfo->BaseName);
            }
        }
    }
    
    Context->HHTOut = 0;
    Context->HHTCopyPoint = 0;
}
#endif

internal void
SynchronizeAssetFileChanges(game_assets *Assets, b32x SaveChangesToHHTs)
{
#if HANDMADE_INTERNAL
    memory_arena TempArena = {};
    
    hht_context Context = {};
    Context.FileGroup = Platform.GetAllFilesOfTypeBegin(PlatformFileType_HHT);
    Context.Assets = Assets;
    Context.TempArena = &TempArena;
    
    Clear(&Assets->ErrorStreamMemory);
    Assets->ErrorStream = OnDemandMemoryStream(&Assets->ErrorStreamMemory);
    Context.ErrorStream = &Assets->ErrorStream;
    Outf(Context.ErrorStream, "%s #%u:\n", SaveChangesToHHTs ? "Save" : "Load",
         Assets->SaveNumber++);
    DEBUG_ARENA_NAME(Assets->ErrorStream.Memory, "SynchronizeAssetFileChanges stdout");
    
    for(platform_file_info *FileInfo = Context.FileGroup.FirstFileInfo;
        FileInfo;
        FileInfo = FileInfo->Next)
    {
        ParseHHT(&Context, FileInfo, SaveChangesToHHTs);
    }
    
    Platform.GetAllFilesOfTypeEnd(&Context.FileGroup);
    
    for(u32 FileIndex = 1;
        FileIndex < Assets->FileCount;
        ++FileIndex)
    {
        asset_file *File = Assets->Files + FileIndex;
        // TODO(casey): It looks like "modified" is getting marked when it shouldn't be
        if(File->Modified)
        {
            temporary_memory TempMem = BeginTemporaryMemory(&TempArena);
            WriteModificationsToHHA(Assets, FileIndex, &TempArena);
            EndTemporaryMemory(TempMem);
        }
    }
    
    Clear(&TempArena);
#endif
}
