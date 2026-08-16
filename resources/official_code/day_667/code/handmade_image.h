/* ========================================================================
   $File: C:\work\handmade\code\handmade_image.h $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright by Molly Rocket, Inc., All Rights Reserved. $
   ======================================================================== */

struct image_u32
{
    u32 Width;
    u32 Height;
    u32 *Pixels;
};

struct mip_iterator
{
    u32 Level;
    image_u32 Image;
};

internal void Downsample2x(image_u32 Source, image_u32 Dest);

internal mip_iterator IterateMIPs(u32 Width, u32 Height, void *Data);
internal b32x IsValid(mip_iterator *MIP);
internal void Advance(mip_iterator *MIP);

internal void GenerateSequentialMIPs(u32 Width, u32 Height, void *Data);


