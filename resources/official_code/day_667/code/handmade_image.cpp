/* ========================================================================
   $File: C:\work\handmade\code\handmade_image.cpp $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright by Molly Rocket, Inc., All Rights Reserved. $
   ======================================================================== */

internal void
FillImage(image_u32 Dest, u32 Color)
{
    u32 *DestPixel = Dest.Pixels;
    u32 Count = Dest.Width*Dest.Height;
    while(Count--)
    {
        *DestPixel++ = Color;
    }
}

internal void
Downsample2x(image_u32 Source, image_u32 Dest)
{
    // TODO(casey): This routine probably wants to be optimized,
    // since it _is_ used during MIP generation at runtime.
    
    // TODO(casey): Probably shouldn't duplicate, probably should use zero instead...
    
    u32 *DestPixel = Dest.Pixels;
    u32 *SourceRow = Source.Pixels;
    for(u32 Y = 0;
        Y < Dest.Height;
        ++Y)
    {
        u32 *SourcePixel0 = SourceRow;
        u32 *SourcePixel1 = SourceRow;
        if((Y+1) < Source.Height)
        {
            SourcePixel1 += Source.Width;
        }
        
        for(u32 X = 0;
            X < Dest.Width;
            ++X)
        {
            v4 P00 = BGRAUnpack4x8(*SourcePixel0++);
            v4 P01 = BGRAUnpack4x8(*SourcePixel1++);
            
            v4 P10 = P00;
            v4 P11 = P01;
            if((X+1) < Source.Width)
            {
                P10 = BGRAUnpack4x8(*SourcePixel0++);
                P11 = BGRAUnpack4x8(*SourcePixel1++);
            }
            
            P00 = SRGB255ToLinear1(P00);
            P10 = SRGB255ToLinear1(P10);
            P01 = SRGB255ToLinear1(P01);
            P11 = SRGB255ToLinear1(P11);
            
            v4 C = 0.25f*(P00 + P10 + P01 + P11);
            C = Linear1ToSRGB255(C);
            
            *DestPixel++ = BGRAPack4x8(C);
        }
        
        SourceRow += 2*Source.Width;
    }
}

internal mip_iterator
IterateMIPs(u32 Width, u32 Height, void *Data)
{
    mip_iterator MIP;
    
    MIP.Level = 0;
    MIP.Image.Width = Width;
    MIP.Image.Height = Height;
    MIP.Image.Pixels = (u32 *)Data;
    
    return(MIP);
}

internal b32x
IsValid(mip_iterator *MIP)
{
    b32x Result = (MIP->Image.Width && MIP->Image.Height);
    return(Result);
}

internal void
Advance(mip_iterator *MIP)
{
    MIP->Image.Pixels += (MIP->Image.Width*MIP->Image.Height);
    if((MIP->Image.Width == 1) &&
       (MIP->Image.Height == 1))
    {
        MIP->Image.Width = MIP->Image.Height = 0;
    }
    else
    {
        ++MIP->Level;
        if(MIP->Image.Width > 1)
        {
            MIP->Image.Width = (MIP->Image.Width + 1) / 2;
        }
        
        if(MIP->Image.Height > 1)
        {
            MIP->Image.Height = (MIP->Image.Height + 1) / 2;
        }
    }
}

internal u32
GetTotalSizeForMIPs(u32 Width, u32 Height)
{
    u32 Result = 0;
    
    for(mip_iterator MIP = IterateMIPs(Width, Height, 0);
        IsValid(&MIP);
        Advance(&MIP))
    {
        Result += MIP.Image.Width * MIP.Image.Height * 4;
    }
    
    return(Result);
}

internal void
GenerateSequentialMIPs(u32 Width, u32 Height, void *Data)
{
    mip_iterator MIP = IterateMIPs(Width, Height, Data);
    image_u32 Source = MIP.Image;
    Advance(&MIP);
    while(IsValid(&MIP))
    {
        Assert(((u8 *)Source.Pixels + Source.Width*Source.Height*4) ==
               (u8 *)MIP.Image.Pixels);
#if 1
        Downsample2x(Source, MIP.Image);
#else
        FillImage(MIP.Image, RGBAPack4x8(GetDebugColor4(MIP.Level)));
#endif
        Source = MIP.Image;
        Advance(&MIP);
    }
    
    Assert(GetTotalSizeForMIPs(Width, Height) == ((u8 *)MIP.Image.Pixels - (u8 *)Data));
}
