/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

struct world_generator;
struct gen_entity_pattern;

#define GEN_CREATE_ENTITY_PATTERN(name) void name(world_generator *Gen, gen_entity_pattern *Pattern)
typedef GEN_CREATE_ENTITY_PATTERN(gen_create_pattern);

struct gen_entity_pattern
{
    gen_entity_pattern *Next;
    gen_create_pattern *Creator;

    asset_tag_hash BaseAsset;
    brain_id BrainID;

    f32 Difficulty;
};

enum tile_pattern_type
{
    TilePattern_Horizontal,
    TilePattern_Vertical,
    TilePattern_Snake,
    TilePattern_Branching,
    TilePattern_Scatter,
};
struct tile_pattern
{
    tile_pattern_type Type;
    u32 RequiredEmptyNeighborCount;

    u32 MaxCount;
    u32 Count;
    v2s AtI[MAX_TILES_PER_PATTERN];
};

internal void ConnectPiece(entity *Entity,
                           entity_visible_piece *Parent, hha_align_point_type ParentType,
                           entity_visible_piece *Child, hha_align_point_type ChildType);
internal void ConnectPieceToWorld(entity *Entity, entity_visible_piece *Child, hha_align_point_type ChildType);
