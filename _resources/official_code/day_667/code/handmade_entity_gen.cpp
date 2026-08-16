/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

inline rectangle3
MakeSimpleGroundedCollision(f32 DimX, f32 DimY, f32 DimZ, f32 OffsetZ)
{
    rectangle3 Result = RectCenterDim(V3(0, 0, 0.5f*DimZ + OffsetZ),
                                      V3(DimX, DimY, DimZ));
    return(Result);
}

internal entity_visible_piece *
AddPiece(entity *Entity, asset_tag_hash Asset, v3 Dim, v3 Offset, v4 Color, u32 Flags)
{
    Assert(Entity->PieceCount < ArrayCount(Entity->Pieces));
    entity_visible_piece *Piece = Entity->Pieces + Entity->PieceCount++;
    Piece->Asset = Asset;
    Piece->Dim = Dim;
    Piece->Offset = Offset;
    Piece->Color = Color;
    Piece->Flags = Flags;

    return(Piece);
}

internal entity_visible_piece *
AddPiece(entity *Entity, asset_tag_hash Asset, f32 Height, v3 Offset, v4 Color, u32 Flags)
{
    entity_visible_piece *Result =
        AddPiece(Entity, Asset, V3(0, Height, 0), Offset, Color, Flags);
    return(Result);
}

internal void
ConnectPiece(entity *Entity,
             entity_visible_piece *Parent, hha_align_point_type ParentType,
             entity_visible_piece *Child, hha_align_point_type ChildType)
{
    Assert(IsBitmap(Child));
    bitmap_piece *Bitmap = &Child->Bitmap;
    Bitmap->ParentPiece = SafeTruncateToU8(Parent - Entity->Pieces);
    Bitmap->ParentAlignType = SafeTruncateToU8(ParentType);
    Bitmap->ChildAlignType = SafeTruncateToU8(ChildType);

    Assert(Bitmap->ParentPiece < (Child - Entity->Pieces));
}

internal void
ConnectPieceToWorld(entity *Entity, entity_visible_piece *Child, hha_align_point_type ChildType)
{
    Assert(IsBitmap(Child));
    bitmap_piece *Bitmap = &Child->Bitmap;
    Bitmap->ParentPiece = 0;
    Bitmap->ParentAlignType = SafeTruncateToU8(HHAAlign_None);
    Bitmap->ChildAlignType = SafeTruncateToU8(ChildType);
}

internal entity_visible_piece *
AddPieceLight(entity *Entity, f32 Radius, v3 Offset, f32 Emission, v3 Color)
{
    entity_visible_piece *Result =
        AddPiece(Entity, {}, V3(Radius, Radius, Radius), Offset, V4(Color, Emission), PieceType_Light);
    return(Result);
}

internal entity_visible_piece *
AddPieceOccluder(entity *Entity, v3 Offset, v3 Dim, v3 Color)
{
    entity_visible_piece *Result =
        AddPiece(Entity, {}, Dim, Offset, V4(Color, 1.0f), PieceType_Occluder);
    return(Result);
}

internal void
InitHitPoints(entity *Entity, uint32 HitPointCount)
{
    Assert(HitPointCount <= ArrayCount(Entity->HitPoint));
    Entity->HitPointMax = HitPointCount;
    for(uint32 HitPointIndex = 0;
        HitPointIndex < Entity->HitPointMax;
        ++HitPointIndex)
    {
        hit_point *HitPoint = Entity->HitPoint + HitPointIndex;
        HitPoint->Flags = 0;
        HitPoint->FilledAmount = HIT_POINT_SUBDIVISIONS;
    }
}

internal entity *
AddInanimate(sim_region *Region, asset_tag_hash BaseAsset, f32 Variant = 0)
{
    entity *Entity = AddEntity(Region);
    entity_visible_piece *Body = AddPiece(Entity, GetTagHash(Asset_Scenery, BaseAsset), 1.0f, V3(0, 0, 0), V4(1, 1, 1, 1));
    Body->Variant = Variant;
    ConnectPieceToWorld(Entity, Body, HHAAlign_Default);
    return(Entity);
}

internal entity *
AddObstacle(sim_region *Region, asset_tag_hash BaseAsset, f32 Variant = 0)
{
    entity *Entity = AddInanimate(Region, BaseAsset, Variant);
    return(Entity);
}

internal entity *
AddBlock(sim_region *Region, asset_tag_hash BaseAsset, v3 Dim, v4 Color, f32 Variant = 0)
{
    entity *Entity = AddEntity(Region);
    entity_visible_piece *Piece =
        AddPiece(Entity, GetTagHash(Asset_Block, BaseAsset), 0.5f*Dim, V3(0, 0, 0.5f*Dim.z), Color, PieceType_Cube);
    Piece->Variant = Variant;

    return(Entity);
}

internal entity *
AddConversation(sim_region *Region)
{
    entity *Entity = AddEntity(Region);
    v3 Dim = {0.5f, 0.5f, 0.5f};
    Entity->CollisionVolume = AddRadiusTo(RectCenterDim(V3(0, 0, 0.5f*Dim.z), Dim),
                                          V3(0.1f, 0.1f, 0.1f));
    Entity->CameraBehavior = Camera_Offset;
    Entity->CameraOffset.z = GetCameraOffsetZForCloseup();
    Entity->CameraOffset.y = 2.0f;

    return(Entity);
}

internal entity *
AddTree(world_generator *Gen, sim_region *Region)
{
    asset_tag_hash AssetHash = GetTagHash(Asset_None, Tag_Tree, RandomChoice(Gen->Entropy, 2) ? Tag_Winter : Tag_None);
    entity *Entity = AddObstacle(Region, AssetHash, RandomUnilateral(Gen->Entropy));
    return(Entity);
}

GEN_CREATE_ENTITY_PATTERN(SingleEnemyPattern)
{
    tile_result OnTile = FindRandomOpenTile(Gen->SimRegion, V2(0.5f, 0.5f));
    if(OnTile.Valid)
    {
        entity *Entity = AddEntity(Gen->SimRegion);

        Entity->BrainSlot = BrainSlotFor(brain_familiar, Head);
        Entity->BrainID = AddBrain(Gen->SimRegion);
        Entity->CollisionVolume = MakeSimpleGroundedCollision(1.0f, 1.0f, 1.0f, 0.25f);
        Entity->TileIndex = OnTile.Index;

        entity_visible_piece *Piece = AddPiece(Entity, GetTagHash(Asset_Head, Pattern->BaseAsset), 2.5f, V3(0, 0, 0), V4(1, 1, 1, 1), PieceMove_BobOffset);

        ConnectPieceToWorld(Entity, Piece, HHAAlign_Default);
    }
}

GEN_CREATE_ENTITY_PATTERN(SnakePattern)
{
    tile_result OnTile = FindRandomOpenTile(Gen->SimRegion, V2(0.5f, 0.5f));
    if(OnTile.Valid)
    {
        asset_tag_hash BaseAsset = Pattern->BaseAsset;

        entity *Head = AddEntity(Gen->SimRegion);
        Head->CollisionVolume = MakeSimpleGroundedCollision(0.75f, 0.75f, 0.75f, 0.0f);
        entity_visible_piece *HeadPiece = AddPiece(Head, GetTagHash(Asset_Head, BaseAsset), 1.5f, V3(0, 0, 0.5f), V4(1, 1, 1, 1));
        AddPieceLight(Head, 0.5f, V3(0, 0, 1.0f), 0.25f, V3(1, 1, 0));
        ConnectPieceToWorld(Head, HeadPiece, HHAAlign_Default);
        InitHitPoints(Head, 3);
        Head->BrainSlot = IndexedBrainSlotFor(brain_snake, Segments, 0);
        Head->BrainID = AddBrain(Gen->SimRegion);
        Head->TileIndex = OnTile.Index;
        AddFlags(Head, EntityFlag_ReceivesCollision);

        v2s PrevTile = Head->TileIndex;
        u32 SegmentCount = 4; // RandomBetween(Gen->Entropy, 1, 3);
        for(u32 SegmentIndex = 0;
            SegmentIndex < SegmentCount;
            ++SegmentIndex)
        {
            tile_result SegTile = FindAdjacentOpenTile(Gen->SimRegion, PrevTile, BoxMask_Planar);
            if(SegTile.Valid)
            {
                entity *Seg = AddEntity(Gen->SimRegion);
                Seg->CollisionVolume = Head->CollisionVolume;

                entity_visible_piece *SegPiece = AddPiece(Seg, GetTagHash(Asset_Body, BaseAsset), 1.5f, V3(0, 0, 0.5f), V4(1, 1, 1, 1));
//                AddPieceLight(Seg, 0.5f, V3(0, 0, 1.0f), 5.0f, V3(1, 1, 0));
                ConnectPieceToWorld(Seg, SegPiece, HHAAlign_Default);
                InitHitPoints(Seg, 3);

                Seg->BrainSlot = IndexedBrainSlotFor(brain_snake, Segments, SegmentIndex + 1);
                Seg->BrainID = Head->BrainID;
                AddFlags(Seg, EntityFlag_ReceivesCollision);
                Seg->TileIndex = SegTile.Index;

                PrevTile = SegTile.Index;
            }
            else
            {
                break;
            }
        }
    }
}

internal b32x TileAlreadyExists(tile_pattern *Tiles, v2s I)
{
    b32x Result = false;

    for(u32 TileIndex = 0;
        TileIndex < Tiles->Count;
        ++TileIndex)
    {
        if(AreEqual(Tiles->AtI[TileIndex], I))
        {
            Result = true;
            break;
        }
    }

    return Result;
}

internal u32 AppendTile(tile_pattern *Tiles, v2s I)
{
    Assert(Tiles->Count < Tiles->MaxCount);
    Assert(Tiles->Count < ArrayCount(Tiles->AtI));

    u32 Result = Tiles->Count++;
    Tiles->AtI[Result] = I;

    return(Result);
}

GEN_CREATE_ENTITY_PATTERN(NPCPattern)
{
    tile_result CharTile = FindRandomOpenTile(Gen->SimRegion, V2(0.5f, 0.75f));
    tile_result TalkTile = FindAdjacentOpenTile(Gen->SimRegion, CharTile.Index, BoxMask_South);
    if(CharTile.Valid && TalkTile.Valid)
    {
        entity *Entity = AddEntity(Gen->SimRegion);
        Entity->FacingDirection = 0.75f*Tau32;

        entity_visible_piece *Body = AddPiece(Entity, GetTagHash(Asset_Body, Pattern->BaseAsset), 1.0f, V3(0, 0, 0), V4(1, 1, 1, 1));
        entity_visible_piece *Head = AddPiece(Entity, GetTagHash(Asset_Head, Pattern->BaseAsset), 1.0f, V3(0, 0, 0.1f), V4(1, 1, 1, 1));

        ConnectPieceToWorld(Entity, Body, HHAAlign_Default);
        ConnectPiece(Entity, Body, HHAAlign_BaseOfNeck, Head, HHAAlign_Default);
        Entity->TileIndex = CharTile.Index;

        entity *Conversation = AddConversation(Gen->SimRegion);
        Conversation->TileIndex = TalkTile.Index;
    }
}

GEN_CREATE_ENTITY_PATTERN(StandardLightingPattern)
{
    random_series *Entropy = Gen->Entropy;

#if 0
    v2s LightSpacing = {5, 5};
    v2s BaseP = {1, 1}; // TODO(casey): We never did compute this :(
    for(edit_tile Tile = IterateAsPlanarTiles(Grid, BaseP);
        IsValid(Tile);
        Advance(&Tile, BaseP, LightSpacing))
    {
        entity *Light = AddEntity(Grid->Region);
        v3 LampLight = V3(RandomBetween(Entropy, 0.4f, 0.7f),
                          RandomBetween(Entropy, 0.4f, 0.7f),
                          0.5f);
        AddPieceLight(Light, 1.0f, V3(0, 0, 2.5f), 0.05f, LampLight);

        PlaceEntityAtP(Light, Tile);
    }
#endif

#if 0
        entity *Lamp = AddObstacle(Grid->Region, GetTagHash(Asset_Scenery, Tag_Orphan, Tag_Lamp));
        PlaceEntityOnTraversable(Grid->Region, Lamp, Ref);
        v3 LampLight = V3(RandomBetween(Gen->Entropy, 0.4f, 0.7f),
                          RandomBetween(Gen->Entropy, 0.4f, 0.7f),
                          0.5f);
        AddPieceLight(Lamp, 0.5f, V3(0, 0, 2.5f), 20.0f, LampLight);
#endif
}
