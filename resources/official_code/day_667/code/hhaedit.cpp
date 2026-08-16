/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

#include "hhaedit.h"

#include "handmade_file_formats.cpp"

internal b32x
FileExists(char *FileName)
{
    b32x Result = false;
    
    // NOTE(casey): This is not a good way to do this, but we are staying "CRT"-compliant
    // here, so we can't call the OS to actually find out if the file exists :(
    FILE *Test = fopen(FileName, "rb");
    if(Test)
    {
        fclose(Test);
        Result = true;
    }
    
    return(Result);
}

internal string
ReadEntireFile(FILE *In)
{
    string Result = {};
    
    fseek(In, 0, SEEK_END);
    Result.Count = ftell(In);
    fseek(In, 0, SEEK_SET);
    
    Result.Data = (u8 *)malloc(Result.Count);
    fread(Result.Data, Result.Count, 1, In);
    
    return(Result);
}

internal void
ReadHHA_V0(FILE *SourceFile, loaded_hha *HHA)
{
    hha_header_v0 *Header = (hha_header_v0 *)HHA->DataStore;
    hha_asset_v0 *SourceAssets = (hha_asset_v0 *)(HHA->DataStore + Header->Assets);
    hha_tag *SourceTags = (hha_tag *)(HHA->DataStore + Header->Tags);
    hha_asset_type_v0 *SourceAssetTypes = (hha_asset_type_v0 *)(HHA->DataStore + Header->AssetTypes);
    
    HHA->TagCount = Header->TagCount + Header->AssetCount - 1;
    HHA->Tags = (hha_tag *)malloc(HHA->TagCount*sizeof(hha_tag));
    
    HHA->AssetCount = Header->AssetCount;
    HHA->Assets = (hha_asset *)malloc(HHA->AssetCount*sizeof(hha_asset));
    HHA->Annotations = (loaded_hha_annotation *)malloc(HHA->AssetCount*sizeof(loaded_hha_annotation));
    
    loaded_hha_annotation DefaultAnnotation = {};
    
    DefaultAnnotation.SourceFileBaseName = RemovePath(RemoveExtension(WrapZ(HHA->SourceFileName)));
    DefaultAnnotation.AssetName = WrapZ("UNKNOWN");
    DefaultAnnotation.AssetDescription = WrapZ("Imported by ReadHHA_V0");
    DefaultAnnotation.Author = WrapZ("hhaedit.exe");
    
    loaded_hha_annotation NullAnnotation = {};
    hha_asset NullAsset = {};
    hha_tag NullTag = {};
    
    HHA->Assets[0] = NullAsset;
    HHA->Tags[0] = NullTag;
    HHA->Annotations[0] = NullAnnotation;
    
    u32 DestTagIndex = 1;
    for(u32 AssetTypeIndex = 0;
        AssetTypeIndex < Header->AssetTypeCount;
        ++AssetTypeIndex)
    {
        hha_asset_type_v0 AssetType = SourceAssetTypes[AssetTypeIndex];
        type_from_id_v0 *TypeInfo = TypeFromIDV0;
        if(AssetType.TypeID < ArrayCount(TypeFromIDV0))
        {
            TypeInfo = TypeFromIDV0 + AssetType.TypeID;
        }
        
        for(u32 AssetIndex = AssetType.FirstAssetIndex;
            AssetIndex < AssetType.OnePastLastAssetIndex;
            ++AssetIndex)
        {
            hha_asset_v0 *SourceAsset = SourceAssets + AssetIndex;
            hha_asset *DestAsset = HHA->Assets + AssetIndex;
            *DestAsset = NullAsset;
            
            if(AssetIndex < HHA->AssetCount)
            {
                DestAsset->FirstTagIndex = DestTagIndex;
                for(u32 TagIndex = SourceAsset->FirstTagIndex;
                    TagIndex < SourceAsset->OnePastLastTagIndex;
                    ++TagIndex)
                {
                    HHA->Tags[DestTagIndex++] = SourceTags[TagIndex];
                }
                HHA->Tags[DestTagIndex].ID = Tag_BasicCategory;
                HHA->Tags[DestTagIndex].Value = (f32)AssetType.TypeID;
                ++DestTagIndex;
                DestAsset->OnePastLastTagIndex = DestTagIndex;
                
                DestAsset->DataOffset = SourceAsset->DataOffset;
                DestAsset->Type = TypeInfo->Type;
                
                switch(DestAsset->Type)
                {
                    case HHAAsset_Bitmap:
                    {
                        hha_bitmap_v0 *Bitmap = &SourceAsset->Bitmap;
                        
                        v2 AlignPercentage =
                        {
                            Bitmap->AlignPercentage[0],
                            Bitmap->AlignPercentage[1],
                        };
                        
                        DestAsset->Bitmap.Dim[0] = (u16)Bitmap->Dim[0];
                        DestAsset->Bitmap.Dim[1] = (u16)Bitmap->Dim[1];
                        DestAsset->Bitmap.OrigDim[0] = DestAsset->Bitmap.Dim[0];
                        DestAsset->Bitmap.OrigDim[1] = DestAsset->Bitmap.Dim[1];
                        
                        SetAlignPoint(DestAsset->Bitmap.AlignPoints + 0,
                                      HHAAlign_Default, true,
                                      1.0f, AlignPercentage);
                        
                        DestAsset->DataSize = 4*Bitmap->Dim[0]*Bitmap->Dim[1];
                    } break;
                    
                    case HHAAsset_Sound:
                    {
                        hha_sound *Sound = &SourceAsset->Sound;
                        
                        DestAsset->Sound = *Sound;
                        DestAsset->DataSize = Sound->SampleCount*Sound->ChannelCount*sizeof(int16);
                    } break;
                    
                    case HHAAsset_Font:
                    {
                        hha_font *Font = &SourceAsset->Font;
                        u32 HorizontalAdvanceSize = sizeof(r32)*Font->GlyphCount*Font->GlyphCount;
                        u32 GlyphsSize = Font->GlyphCount*sizeof(hha_font_glyph);
                        u32 UnicodeMapSize = sizeof(u16)*Font->OnePastHighestCodepoint;
                        
                        DestAsset->Font = *Font;
                        DestAsset->DataSize = GlyphsSize + HorizontalAdvanceSize;
                    } break;
                    
                    default:
                    {
                        fprintf(stderr, "ERROR: Asset %u has illegal type.", AssetIndex);
                    } break;
                }
            }
            else
            {
                fprintf(stderr, "ERROR: Asset index %u out of range", AssetIndex);
            }
            
            HHA->Annotations[AssetIndex] = DefaultAnnotation;
            HHA->Annotations[AssetIndex].AssetName = WrapZ(TypeInfo->Name);
        }
    }
}

internal string
RefString(u8 *D, u32 Count, u64 Offset)
{
    string Result;
    
    Result.Count = Count;
    Result.Data = D + Offset;
    
    return(Result);
}

internal void
ReadHHA_V2(FILE *SourceFile, loaded_hha *HHA)
{
    hha_header *Header = (hha_header *)HHA->DataStore;
    u8 *D = HHA->DataStore;
    
    HHA->TagCount = Header->TagCount;
    HHA->Tags = (hha_tag *)(D + Header->Tags);
    
    HHA->AssetCount = Header->AssetCount;
    HHA->Assets = (hha_asset *)(D + Header->Assets);
    
    if(Header->Annotations)
    {
        HHA->Annotations = (loaded_hha_annotation *)
            malloc(HHA->AssetCount*sizeof(loaded_hha_annotation));
        
        for(u32 AnnotationIndex = 0;
            AnnotationIndex < HHA->AssetCount;
            ++AnnotationIndex)
        {
            hha_annotation *SourceAnnotation =
                (hha_annotation *)(D + Header->Annotations) + AnnotationIndex;
            loaded_hha_annotation *DestAnnotation = HHA->Annotations + AnnotationIndex;
            
            DestAnnotation->SourceFileDate = SourceAnnotation->SourceFileDate;
            DestAnnotation->SourceFileChecksum = SourceAnnotation->SourceFileChecksum;
            DestAnnotation->HHTBlockChecksum = SourceAnnotation->HHTBlockChecksum;
            DestAnnotation->SpriteSheetX = SourceAnnotation->SpriteSheetX;
            DestAnnotation->SpriteSheetY = SourceAnnotation->SpriteSheetY;
            
            DestAnnotation->SourceFileBaseName =
                RefString(D, SourceAnnotation->SourceFileBaseNameCount,
                          SourceAnnotation->SourceFileBaseNameOffset);
            DestAnnotation->AssetName =
                RefString(D, SourceAnnotation->AssetNameCount,
                          SourceAnnotation->AssetNameOffset);
            DestAnnotation->AssetDescription =
                RefString(D, SourceAnnotation->AssetDescriptionCount,
                          SourceAnnotation->AssetDescriptionOffset);
            DestAnnotation->Author =
                RefString(D, SourceAnnotation->AuthorCount,
                          SourceAnnotation->AuthorOffset);
            DestAnnotation->ErrorStream =
                RefString(D, SourceAnnotation->ErrorStreamCount,
                          SourceAnnotation->ErrorStreamOffset);
        }
        
        HHA->HadAnnotations = true;
    }
}

internal loaded_hha *
ReadHHA(char *SourceFileName)
{
    loaded_hha *Result = (loaded_hha *)malloc(sizeof(loaded_hha));
    loaded_hha Null = {};
    *Result = Null;
    Result->SourceFileName = SourceFileName;
    
    FILE *SourceFile = fopen(SourceFileName, "rb");
    if(SourceFile)
    {
        Result->DataStore = ReadEntireFile(SourceFile).Data;
        
        Result->MagicValue = ((u32 *)Result->DataStore)[0];
        Result->SourceVersion = ((u32 *)Result->DataStore)[1];
        
        if(Result->MagicValue == HHA_MAGIC_VALUE)
        {
            if(Result->SourceVersion == 0)
            {
                ReadHHA_V0(SourceFile, Result);
                Result->Valid = true;
            }
            else if(Result->SourceVersion == 2)
            {
                ReadHHA_V2(SourceFile, Result);
                Result->Valid = true;
            }
            else
            {
                fprintf(stderr, "Unrecognized HHA version.\n");
            }
        }
        else
        {
            fprintf(stderr, "Magic value is not HHAF.\n");
        }
    }
    else
    {
        fprintf(stderr, "Unable to open %s for reading.\n", SourceFileName);
    }
    
    return(Result);
}

internal u64
WriteBlock(u32 Size, void *Data, FILE *DestFile)
{
    u64 Result = (u64)ftell(DestFile);
    fwrite(Data, Size, 1, DestFile);
    return(Result);
}

internal u64
WriteString(string String, u32 *DestCount, FILE *DestFile)
{
    *DestCount = (u32)String.Count;
    u64 Result = WriteBlock((u32)String.Count, String.Data, DestFile);
    return(Result);
}

internal void
WriteHHA_V2(loaded_hha *Source, FILE *DestFile, b32x IncludeAnnotations = true)
{
    hha_header Header = {};
    
    Header.MagicValue = HHA_MAGIC_VALUE;
    Header.Version = HHA_VERSION;
    
    Header.TagCount = Source->TagCount;
    Header.AssetCount = Source->AssetCount;
    
    u32 DestTagsSize = Source->TagCount*sizeof(hha_tag);
    u32 DestAssetsSize = Source->AssetCount*sizeof(hha_asset);
    u32 DestAnnotationsSize = Source->AssetCount*sizeof(hha_annotation);
    
    u32 HeaderSize = sizeof(Header);
    fseek(DestFile, HeaderSize, SEEK_SET);
    
    hha_tag *DestTags = (hha_tag *)malloc(DestTagsSize);
    hha_asset *DestAssets = (hha_asset *)malloc(DestAssetsSize);
    hha_annotation *DestAnnotations = (hha_annotation *)malloc(DestAnnotationsSize);
    
    for(u32 TagIndex = 0;
        TagIndex < Source->TagCount;
        ++TagIndex)
    {
        hha_tag *SourceTag = Source->Tags + TagIndex;
        hha_tag *DestTag = DestTags + TagIndex;
        
        *DestTag = *SourceTag;
    }
    
    hha_annotation NullAnnotation = {};
    for(u32 AssetIndex = 0;
        AssetIndex < Source->AssetCount;
        ++AssetIndex)
    {
        hha_asset *SourceAsset = Source->Assets + AssetIndex;
        loaded_hha_annotation *SourceAnnotation = Source->Annotations + AssetIndex;
        
        hha_asset *DestAsset = DestAssets + AssetIndex;
        hha_annotation *DestAnnotation = DestAnnotations + AssetIndex;
        
        *DestAsset = *SourceAsset;
        DestAsset->DataOffset = WriteBlock(DestAsset->DataSize,
                                           Source->DataStore + SourceAsset->DataOffset,
                                           DestFile);
        
        *DestAnnotation = NullAnnotation;
        DestAnnotation->SourceFileDate = SourceAnnotation->SourceFileDate;
        DestAnnotation->SourceFileChecksum = SourceAnnotation->SourceFileChecksum;
        DestAnnotation->HHTBlockChecksum = SourceAnnotation->HHTBlockChecksum;
        DestAnnotation->SpriteSheetX = SourceAnnotation->SpriteSheetX;
        DestAnnotation->SpriteSheetY = SourceAnnotation->SpriteSheetY;
        
        DestAnnotation->SourceFileBaseNameOffset =
            WriteString(SourceAnnotation->SourceFileBaseName,
                        &DestAnnotation->SourceFileBaseNameCount,
                        DestFile);
        
        DestAnnotation->AssetNameOffset =
            WriteString(SourceAnnotation->AssetName,
                        &DestAnnotation->AssetNameCount,
                        DestFile);
        
        DestAnnotation->AssetDescriptionOffset =
            WriteString(SourceAnnotation->AssetDescription,
                        &DestAnnotation->AssetDescriptionCount,
                        DestFile);
        
        DestAnnotation->AuthorOffset =
            WriteString(SourceAnnotation->Author,
                        &DestAnnotation->AuthorCount,
                        DestFile);
        
        DestAnnotation->ErrorStreamOffset =
            WriteString(SourceAnnotation->ErrorStream,
                        &DestAnnotation->ErrorStreamCount,
                        DestFile);
    }
    
    Header.Tags = WriteBlock(DestTagsSize, DestTags, DestFile);
    Header.Assets = WriteBlock(DestAssetsSize, DestAssets, DestFile);
    if(IncludeAnnotations)
    {
        Header.Annotations = WriteBlock(DestAnnotationsSize, DestAnnotations, DestFile);
    }
    
    fseek(DestFile, 0, SEEK_SET);
    u64 CheckHeaderLocation = WriteBlock(HeaderSize, &Header, DestFile);
    Assert(CheckHeaderLocation == 0);
}

internal void
WriteHHA(loaded_hha *Source, char *DestFileName)
{
    if(!FileExists(DestFileName))
    {
        if(Source->Valid)
        {
            FILE *DestFile = fopen(DestFileName, "wb");
            if(DestFile)
            {
                WriteHHA_V2(Source, DestFile);
            }
            else
            {
                fprintf(stderr, "Unable to open %s for writing.\n", DestFileName);
            }
            fclose(DestFile);
        }
        else
        {
            fprintf(stderr, "(Source HHA was not valid, so not writing to %s)\n", DestFileName);
        }
    }
    else
    {
        fprintf(stderr, "%s must not exist.\n", DestFileName);
    }
}

internal void
PrintHeaderInfo(loaded_hha *HHA)
{
    fprintf(stdout, "    Header:\n");
    fprintf(stdout, "        MagicValue: %.*s\n", 4, (char *)&HHA->MagicValue);
    fprintf(stdout, "        Version: %u\n", HHA->SourceVersion);
    fprintf(stdout, "        Assets: %u\n", HHA->AssetCount);
    fprintf(stdout, "        Tags: %u\n", HHA->TagCount);
    fprintf(stdout, "        Annotations: %s\n", HHA->HadAnnotations ? "yes" : "no");
}

internal void
PrintTag(loaded_hha *HHA, u32 TagIndex)
{
    if(TagIndex < HHA->TagCount)
    {
        hha_tag *Tag = HHA->Tags + TagIndex;
        char *TagName = (char *)TagNameFromID((asset_tag_id)Tag->ID).Data;
        fprintf(stdout, "%s = %f", TagName, Tag->Value);
    }
    else
    {
        fprintf(stdout, "TAG INDEX OVERFLOW");
    }
}

internal void
PrintContents(loaded_hha *HHA)
{
    fprintf(stdout, "    Assets:\n");
    for(u32 AssetIndex = 1;
        AssetIndex < HHA->AssetCount;
        ++AssetIndex)
    {
        hha_asset *Asset = HHA->Assets + AssetIndex;
        loaded_hha_annotation *An = HHA->Annotations + AssetIndex;
        string Category = CategoryNameFromID(Asset_None);
        for(u32 TagIndex = Asset->FirstTagIndex;
            TagIndex < Asset->OnePastLastTagIndex;
            ++TagIndex)
        {
            if(TagIndex < HHA->TagCount)
            {
                hha_tag *Tag = HHA->Tags + TagIndex;
                if(Tag->ID == Tag_BasicCategory)
                {
                    Category = CategoryNameFromID((asset_basic_category)(u32)Tag->Value);
                }
            }
        }
        fprintf(stdout, "        [%u - %.*s] %.*s %.*s %u,%u\n",
                AssetIndex,
                (int)Category.Count, (char *)Category.Data,
                (int)An->AssetName.Count, (char *)An->AssetName.Data,
                (int)An->SourceFileBaseName.Count, (char *)An->SourceFileBaseName.Data,
                An->SpriteSheetX, An->SpriteSheetY);
        
        if(An->AssetDescription.Count)
        {
            fprintf(stdout, "            Description: %.*s\n",
                    (int)An->AssetDescription.Count, (char *)An->AssetDescription.Data);
        }
        
        if(An->Author.Count)
        {
            fprintf(stdout, "            Author: %.*s\n",
                    (int)An->Author.Count, (char *)An->Author.Data);
        }
        
        if(An->ErrorStream.Count)
        {
            fprintf(stdout, "            Errors:\n");
            char *First = (char *)An->ErrorStream.Data;
            int At = 0;
            int Base = 0;
            while(At <= An->ErrorStream.Count)
            {
                if((At == An->ErrorStream.Count) ||
                   (An->ErrorStream.Data[At] == '\n'))
                {
                    if(Base != At)
                    {
                        fprintf(stdout, "                %.*s\n",
                                (int)(At - Base), (char *)(First + Base));
                        ++At;
                        Base = At;
                    }
                    else
                    {
                        ++At;
                    }
                }
                else
                {
                    ++At;
                }
            }
        }
        
        fprintf(stdout, "            From: %.*s %u,%u (date: %zu, checksum: %zu, hht: %zu)\n",
                (int)An->SourceFileBaseName.Count, (char *)An->SourceFileBaseName.Data,
                An->SpriteSheetX, An->SpriteSheetY,
                (size_t)An->SourceFileDate, (size_t)An->SourceFileChecksum,
                (size_t)An->HHTBlockChecksum);
        
        fprintf(stdout, "            Data: %u bytes at %zu\n", Asset->DataSize, (size_t)Asset->DataOffset);
        u32 TagCount = (Asset->OnePastLastTagIndex - Asset->FirstTagIndex);
        if(TagCount)
        {
            fprintf(stdout, "            Tags: %u at %u\n", TagCount, Asset->FirstTagIndex);
            for(u32 TagIndex = Asset->FirstTagIndex;
                TagIndex < Asset->OnePastLastTagIndex;
                ++TagIndex)
            {
                fprintf(stdout, "                [%u] ", TagIndex);
                PrintTag(HHA, TagIndex);
                fprintf(stdout, "\n");
            }
        }
        
        switch(Asset->Type)
        {
            case HHAAsset_Bitmap:
            {
                hha_bitmap *Bitmap = &Asset->Bitmap;
                fprintf(stdout, "            Type: %ux%u Bitmap (%u)\n", Bitmap->Dim[0], Bitmap->Dim[1], Asset->Type);
                b32 First = true;
                for(u32 PointIndex = 0;
                    PointIndex < ArrayCount(Bitmap->AlignPoints);
                    ++PointIndex)
                {
                    hha_align_point Point = Bitmap->AlignPoints[PointIndex];
                    if(Point.Type)
                    {
                        if(First)
                        {
                            fprintf(stdout, "            Alignment:\n");
                            First = false;
                        }
                        
                        v2 PPercent = GetPPercent(Point);
                        f32 Size = GetSize(Point);
                        hha_align_point_type AlignType = GetType(Point);
                        b32 ToParent = IsToParent(Point);
                        string TypeName = AlignPointNameFromType(AlignType);
                        
                        fprintf(stdout, "                [%u]: %s%.*s%s {%f,%f} %f\n",
                                PointIndex,
                                ToParent ? "<-" : "",
                                (int)TypeName.Count, (char *)TypeName.Data,
                                ToParent ? "" : "->",
                                PPercent.x, PPercent.y, Size);
                    }
                }
            } break;
            
            case HHAAsset_Sound:
            {
                hha_sound *Sound = &Asset->Sound;
                char *Mode = "UNKNOWN";
                switch(Sound->Chain)
                {
                    case HHASoundChain_None:
                    {
                        Mode = "";
                    } break;
                    
                    case HHASoundChain_Loop:
                    {
                        Mode = "Looped";
                    } break;
                    
                    case HHASoundChain_Advance:
                    {
                        Mode = "Chained";
                    } break;
                }
                fprintf(stdout, "            Type: %ux%u %s Sound (%u)\n",
                        Sound->SampleCount, Sound->ChannelCount, Mode, Asset->Type);
            } break;
            
            case HHAAsset_Font:
            {
                hha_font *Font = &Asset->Font;
                fprintf(stdout, "            Type: Font (%u)\n", Asset->Type);
                fprintf(stdout, "            Glyphs: %u (one past last codepoint: %u)\n",
                        Font->GlyphCount,
                        Font->OnePastHighestCodepoint);
                fprintf(stdout, "            Ascender: %f\n", Font->AscenderHeight);
                fprintf(stdout, "            Descender: %f\n", Font->DescenderHeight);
                fprintf(stdout, "            ExternalLeading: %f\n", Font->ExternalLeading);
            } break;
            
            default:
            {
                fprintf(stdout, "            Type: UNKNOWN (%u)\n", Asset->Type);
            } break;
        }
    }
    
    fprintf(stdout, "    Tags:\n");
    for(u32 TagIndex = 1;
        TagIndex < HHA->TagCount;
        ++TagIndex)
    {
        fprintf(stdout, "        [%u] ", TagIndex);
        PrintTag(HHA, TagIndex);
        fprintf(stdout, "\n");
    }
}

// TODO(casey): This should probably be moved to a utility because it is duplicated
// across a bunch of files at this point...
// {
enum pixel_op
{
    PixelOp_SwapR = 0x1,
    PixelOp_ReplA = 0x2,
    PixelOp_MulA = 0x4,
    PixelOp_Invert = 0x8,
};

#pragma pack(push, 1)
struct bitmap_header
{
    u16 FileType;
    u32 FileSize;
    u16 Reserved1;
    u16 Reserved2;
    u32 BitmapOffset;
    u32 Size;
    s32 Width;
    s32 Height;
    u16 Planes;
    u16 BitsPerPixel;
    u32 Compression;
    u32 SizeOfBitmap;
    s32 HorzResolution;
    s32 VertResolution;
    u32 ColorsUsed;
    u32 ColorsImportant;
};
#pragma pack(pop)

internal void
WriteImageTopDownRGBA(u32 Width, u32 Height, u8 *Pixels, char *OutputFileName, u32 PixelOps)
{
    u32 OutputPixelSize = 4*Width*Height;
    
    b32x ReplA = (PixelOps & PixelOp_ReplA);
    b32x SwapR = (PixelOps & PixelOp_SwapR);
    b32x MulA = (PixelOps & PixelOp_MulA);
    b32x Invert = (PixelOps & PixelOp_Invert);
    
    bitmap_header Header = {};
    Header.FileType = 0x4D42;
    Header.FileSize = sizeof(Header) + OutputPixelSize;
    Header.BitmapOffset = sizeof(Header);
    Header.Size = sizeof(Header) - 14;
    Header.Width = Width;
    Header.Height = Height;
    Header.Planes = 1;
    Header.BitsPerPixel = 32;
    Header.Compression = 0;
    Header.SizeOfBitmap = OutputPixelSize;
    Header.HorzResolution = 0;
    Header.VertResolution = 0;
    Header.ColorsUsed = 0;
    Header.ColorsImportant = 0;
    
    u32 MidPointY = ((Header.Height + 1) / 2);
    u32 *Row0 = (u32 *)Pixels;
    u32 *Row1 = Row0 + ((Height - 1)*Width);
    for(u32 Y = 0;
        Y < MidPointY;
        ++Y)
    {
        u32 *Pix0 = Row0;
        u32 *Pix1 = Row1;
        for(u32 X = 0;
            X < Width;
            ++X)
        {
            u32 C0 = *Pix0;
            u32 C1 = *Pix1;
            
            if(SwapR)
            {
                C0 = SwapRAndB(C0);
                C1 = SwapRAndB(C1);
            }
            
            if(MulA)
            {
                C0 = MulAlpha(C0);
                C1 = MulAlpha(C1);
            }
            
            if(ReplA)
            {
                C0 = ReplAlpha(C0);
                C1 = ReplAlpha(C1);
            }
            
            if(Invert)
            {
                *Pix0++ = C1;
                *Pix1++ = C0;
            }
            else
            {
                *Pix0++ = C0;
                *Pix1++ = C1;
            }
        }
        
        Row0 += Width;
        Row1 -= Width;
    }
    
    FILE *OutFile = fopen(OutputFileName, "wb");
    if(OutFile)
    {
        fwrite(&Header, sizeof(Header), 1, OutFile);
        fwrite(Pixels, OutputPixelSize, 1, OutFile);
        fclose(OutFile);
    }
    else
    {
        fprintf(stderr, "[ERROR] Unable to write output file %s.\n", OutputFileName);
    }
}
// }

internal void
ExtractBitmapAsset(loaded_hha *HHA, hha_asset *Asset, char *DestFileName)
{
    u8 *Pixels = HHA->DataStore + Asset->DataOffset;
    WriteImageTopDownRGBA(Asset->Bitmap.Dim[0], Asset->Bitmap.Dim[1], Pixels, DestFileName, 0);
}

int
main(int ArgCount, char **Args)
{
    // TODO(casey): Put a little switch parser in here to make it tidier!
    b32x PrintUsage = false;
    
    if(ArgCount == 4)
    {
        if(strcmp(Args[1], "-rewrite") == 0)
        {
            char *SourceFileName = Args[2];
            char *DestFileName = Args[3];
            
            loaded_hha *HHA = ReadHHA(SourceFileName);
            WriteHHA(HHA, DestFileName);
        }
        else if(strcmp(Args[1], "-extract") == 0)
        {
            char *SourceFileName = Args[2];
            char *DestFileStem = Args[3];
            
            loaded_hha *HHA = ReadHHA(SourceFileName);
            for(u32 AssetIndex = 1;
                AssetIndex < HHA->AssetCount;
                ++AssetIndex)
            {
                hha_asset *Asset = HHA->Assets + AssetIndex;
                if(Asset->Type == HHAAsset_Bitmap)
                {
                    char DestFileName[1024];
                    FormatString(sizeof(DestFileName), DestFileName, "%s_%04u.bmp",
                                 DestFileStem, AssetIndex);
                    ExtractBitmapAsset(HHA, Asset, DestFileName);
                }
            }
        }
        else
        {
            PrintUsage = true;
        }
    }
    else if(ArgCount == 3)
    {
        if(strcmp(Args[1], "-info") == 0)
        {
            char *FileName = Args[2];
            loaded_hha *HHA = ReadHHA(FileName);
            fprintf(stdout, "%s:\n", HHA->SourceFileName);
            PrintHeaderInfo(HHA);
        }
        else if(strcmp(Args[1], "-dump") == 0)
        {
            char *FileName = Args[2];
            loaded_hha *HHA = ReadHHA(FileName);
            fprintf(stdout, "%s:\n", HHA->SourceFileName);
            PrintHeaderInfo(HHA);
            PrintContents(HHA);
        }
        else if(strcmp(Args[1], "-create") == 0)
        {
            char *FileName = Args[2];
            if(!FileExists(FileName))
            {
                FILE *Dest = fopen(FileName, "wb");
                if(Dest)
                {
                    hha_header Header = {};
                    Header.MagicValue = HHA_MAGIC_VALUE;
                    Header.Version = HHA_VERSION;
                    fwrite(&Header, sizeof(Header), 1, Dest);
                    fclose(Dest);
                }
                else
                {
                    fprintf(stderr, "Unable to open file %s for writing.\n", FileName);
                }
            }
            else
            {
                fprintf(stderr, "File %s already exists.\n", FileName);
            }
        }
        else
        {
            PrintUsage = true;
        }
    }
    else
    {
        PrintUsage = true;
    }
    
    if(PrintUsage)
    {
        fprintf(stderr, "Usage: %s -create (dest.hha)\n", Args[0]);
        fprintf(stderr, "       %s -rewrite (source.hha) (dest.hha)\n", Args[0]);
        fprintf(stderr, "       %s -info (source.hha)\n", Args[0]);
        fprintf(stderr, "       %s -dump (source.hha)\n", Args[0]);
        fprintf(stderr, "       %s -extract (source.hha) (dest filename stem)\n", Args[0]);
    }
}
