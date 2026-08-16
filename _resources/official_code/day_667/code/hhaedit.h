/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include "handmade_platform.h"
#include "handmade_file_formats.h"
#include "handmade_file_formats_v0.h"
#include "handmade_intrinsics.h"
#include "handmade_math.h"
#include "handmade_shared.h"

struct loaded_hha
{
    b32x Valid;
    b32x HadAnnotations;
    
    char *SourceFileName;
    
    u32 MagicValue;
    u32 SourceVersion;
    
    u32 TagCount;
    hha_tag *Tags;
    
    u32 AssetCount;
    hha_asset *Assets;
    loaded_hha_annotation *Annotations;
    
    // TODO(casey): Make DataStore more flexible, so we can do things like merge
    u8 *DataStore;
};
