/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

internal f32
GetCameraOffsetZForCloseup(void)
{
    f32 Result = 6.0f;
    return(Result);
}

internal f32
GetCameraOffsetZForDim(v2s Dim, u32 *CameraBehavior)
{
    s32 XCount = Dim.x;
    s32 YCount = Dim.y;

    f32 XDist = 13.0f;
    if(XCount == 12)
    {
        XDist = 14.0f;
    }
    else if(XCount == 13)
    {
        XDist = 15.0f;
    }
    else if(XCount == 14)
    {
        XDist = 16.0f;
        *CameraBehavior |= Camera_ViewPlayerX;
    }
    else if(XCount >= 15)
    {
        XDist = 17.0f;
        *CameraBehavior |= Camera_ViewPlayerX;
    }

    f32 YDist = 13.0f;
    if(YCount == 10)
    {
        YDist = 15.0f;
    }
    else if(YCount == 11)
    {
        YDist = 17.0f;
    }
    else if(YCount == 12)
    {
        YDist = 19.0f;
        *CameraBehavior |= Camera_ViewPlayerY;
    }
    else if(YCount >= 13)
    {
        YDist = 21.0f;
        *CameraBehavior |= Camera_ViewPlayerY;
    }

    f32 Result = Maximum(XDist, YDist);

    return(Result);
}

internal rectangle3 GetTileVolume(world_generator *Gen)
{
    rectangle3 Result = RectMinDim(-0.5f * Gen->TileDim, Gen->TileDim);
    return Result;
}

internal v3 GetMinZCenterP(world_generator *Gen)
{
    v3 Result = GetMinZCenterP(GetTileVolume(Gen));
    return Result;
}

internal rectangle3 GetVolumeFromMinZ(world_generator *Gen, f32 Height)
{
    rectangle3 Result = GetTileVolume(Gen);
    Result.Max.z = (Result.Min.z + Height);
    return Result;
}

#if 0
GEN_CREATE_ENTITY_PATTERN(TileSwitchPattern)
{
    random_series *Series = Gen->Entropy;
    f32 D = Pattern->Difficulty;
    
    tile_pattern Tiles = {};
    
    if(D < 0.25f)
    {
        // Tiles.MaxCount = RandomBetween(Series, 1, 4);
        Tiles.MaxCount = 16;
        
        Tiles.Type = TilePattern_Scatter;
        Tiles.RequiredEmptyNeighborCount = 4;
    }
    else if(D < 0.75f)
    {
        Tiles.MaxCount = RandomBetween(Series, 4, 12);
    }
    else
    {
        Tiles.MaxCount = RandomBetween(Series, 8, 16);
    }
    
    for(u32 AttemptIndex = 0;
        (AttemptIndex < 100) && (Tiles.Count < Tiles.MaxCount);
        ++AttemptIndex)
    {
        tile_result TileAt = {};
        
        v2s Dir[] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};
        
        if(Tiles.Count)
        {
            TileAt.Valid = true;
            Tiles.Index = Tiles.AtI[Tiles.Count - 1];
            switch(Tiles.Type)
            {
                case TilePattern_Horizontal:
                {
                    ++TileAt.Index.x;
                } break;
                
                case TilePattern_Vertical:
                {
                    ++TileAt.Index.y;
                } break;
                
                case TilePattern_Snake:
                {
                    TileAt.Index += Dir[RandomChoice(Series, ArrayCount(Dir))];
                } break;
                
                case TilePattern_Branching:
                {
                    TileAt.Index = Tiles.AtI[RandomChoice(Series, Tiles.Count)];
                    TileAt.Index += Dir[RandomChoice(Series, ArrayCount(Dir))];
                } break;
                
                case TilePattern_Scatter:
                {
                    TileAt.Index = Tiles.AtI[RandomChoice(Series, Tiles.Count)];
                    TileAt.Index.x += RandomBetween(Series, -3, 3);
                    TileAt.Index.y += RandomBetween(Series, -3, 3);
                } break;
            }
        }
        else
        {
            TileAt = FindRandomOpenTile(Gen->SimRegion, V2(RandomUnilateral(Series), RandomUnilateral(Series)));
        }
        
        if(TileAt.Valid && TileIsOpen(Gen->SimRegion, TileAt.Index) &&
           !TileAlreadyExists(&Tiles, TileAt.Index))
        {
            u32 EmptyNeighborCount = 0;
            for(u32 DirIndex = 0;
                DirIndex < ArrayCount(Dir);
                ++DirIndex)
            {
                if(!TileAlreadyExists(&Tiles, TileAt.Index + Dir[DirIndex]))
                {
                    ++EmptyNeighborCount;
                }
            }
            
            if(EmptyNeighborCount >= Tiles.RequiredEmptyNeighborCount)
            {
                AppendTile(&Tiles, TileAt.Index);
            }
        }
    }
    
    for(u32 TileIndex = 0;
        TileIndex < Tiles.Count;
        ++TileIndex)
    {
        edit_tile_contents *Contents = GetTile(Grid, Tiles.AtI[TileIndex]);
        Assert(Contents);
        
        entity *Tile = Contents->Structural;
        Tile->BrainSlot = IndexedBrainSlotFor(brain_switches, Tiles, TileIndex);
        Tile->BrainID = Pattern->BrainID;
        Tile->Pieces[0].Asset = GetTagHash(Asset_Block, Tag_Floor, Tag_Stone);
        Tile->Pieces[0].Color = {0.1f, 0.1f, 0.1f, 1.0f};
    }
}
#endif

internal void GenerateBlock(world_generator *Gen, rectangle2i BlockArea)
{
    memory_arena *Memory = &Gen->TempMemory;
    temporary_memory TempMem = BeginTemporaryMemory(Memory);
    sim_region *SimRegion = BeginWorldChange(Gen->Assets, Memory, Gen->World, Grid->BaseP,
                                          GetCenter(BlockArea), BlockArea, BlockArea, 0);
    Gen->SimRegion = SimRegion;
    
    for(s32 Y = BlockArea.Min.y; Y < BlockArea.Max.y; ++Y)
    {
        for(s32 X = BlockArea.Min.x; X < BlockArea.Max.x; ++X)
        {
            v2s TileIndex = V2S(X, Y);
            v3 P = GetMinZCenterP(Gen);
            rectangle3 Vol = GetVolumeFromMinZ(Gen, 0.5f);
            
            b32 IsWalkable = false;
            
            for(u32 LocAIndex = 0; LocAIndex < Gen->LocationCount; ++LocAIndex)
            {
                world_gen_location *LocA = Gen->Locations + LocAIndex;
                
                if(LengthSq(LocA->P - AbsP) < Square(10.0f))
                {
                    IsWalkable = true;
                }
            }
            
            for(u32 ConIndex = 0; ConIndex < Gen->World->ConnectionCount; ++ConIndex)
            {
                world_room_connection *Con = Gen->World->Connections + ConIndex;
                world_gen_location *A = Gen->Locations + Con->RoomA;
                world_gen_location *B = Gen->Locations + Con->RoomB;
                
                if(DistanceBetweenLineSegmentAndPointSq(A->P, B->P, AbsP) < Square(5.0f))
                {
                    IsWalkable = true;
                }
            }
            
            entity *Entity = AddEntity(Grid->Region);
            Entity->P = P;
            Entity->CollisionVolume = Vol;
            
            v4 Color = sRGBLinearize(0.8f, 0.8f, 0.8f, 1.0f);
            entity_visible_piece *Piece =
                AddPiece(Entity, GetTagHash(Asset_Block, Tag_Floor, Tag_Grass), GetRadius(Vol), GetCenter(Vol), Color, PieceType_Cube);
            Piece->Variant = RandomUnilateral(Gen->Entropy);
            Piece->Cube.UVLayout = EncodeCubeUVLayout(0, 0, 0, 0, 0, 0,
                                                      RandomChoice(Gen->Entropy, 4),
                                                      RandomChoice(Gen->Entropy, 4));
            
            Entity->GroundCoverSpecs[0].Type = CoverType_ThickGrass;
            Entity->GroundCoverSpecs[0].Density = 64;
            
            Entity->TraversableCount = 1;
            Entity->Traversables[0].P = GetMaxZCenterP(Vol);
            Clear(&Entity->Traversables[0].Occupier);
            
            if(!IsWalkable)
            {
                traversable_reference Ref = {};
                Ref.Entity = Entity->ID;
                
                entity *Tree = AddTree(Gen, Grid->Region);
                PlaceEntityOnTraversable(Grid->Region, Tree, Ref);
                
                rectangle3 TreeVol = MakeRelative(GetTotalVolume(&Tile), P);
                TreeVol = AddRadiusTo(TreeVol, -V3(0.25f, 0.25f, 1.0f));
                AddPieceOccluder(Tree, GetCenter(TreeVol), GetRadius(TreeVol), Color.rgb);
            }
        }
    }
    
    entity *CamRoom = AddEntity(Grid->Region);
    CamRoom->P = GetCenter(GetRoomVolume(Grid));
    CamRoom->CollisionVolume = MakeRelative(GetRoomVolume(Grid), CamRoom->P);
    CamRoom->BrainSlot = SpecialBrainSlot(Type_brain_room);
    CamRoom->CameraOffset.z = GetCameraOffsetZForDim(Grid->TileCount, &CamRoom->CameraBehavior);
    
    EndWorldChange(SimRegion);
    Gen->SimRegion = 0;
    EndTemporaryMemory(TempMem);
}

#if 0
internal void
GenerateRoom(world_generator *Gen, gen_room_spec Spec, gen_volume RoomVol)
{
    f32 FullWallHeight = 2.0f;

    edit_grid *Grid = BeginGridEdit(Gen, RoomVol);
    for(edit_tile Tile = IterateAsPlanarTiles(Grid);
        IsValid(Tile);
        Advance(&Tile))
    {
        gen_v3 AbsIndex = GetAbsoluteTileIndex(&Tile);
        v3 P = GetMinZCenterP(&Tile);

        edit_tile_contents *Contents = GetTile(&Tile);
        Assert(Contents);

        b32x Traversable = true;
        b32x OnEdge = IsOnEdge(&Tile);
        b32 OnBoundary = OnEdge;
        f32 tStair = 0.0f;
        b32 OnConnection = false;
        b32 Stairwell = false;
        if(Spec.Outdoors)
        {
            OnBoundary = false;
        }

        OnEdge = false;

#if 0
        for(gen_room_connection *RoomCon = Room->FirstConnection;
            RoomCon;
            RoomCon = RoomCon->Next)
        {
            gen_connection *Con = RoomCon->Connection;
            if(IsInVolume(&Con->Vol, AbsIndex))
            {
                OnConnection = true;
            }
        }
#endif

        entity *Entity = AddEntity(Grid->Region);

        v4 Color = sRGBLinearize(0.31f, 0.49f, 0.32f, 1.0f);
        if(OnConnection)
        {
            Color = sRGBLinearize(0.21f, 0.29f, 0.42f, 1.0f);
        }

        asset_tag_id TileType = Tag_Floor;
        asset_tag_id TileMaterial = Tag_Wood;

        f32 WallHeight = 0.5f;

        b32x OnWall = (OnBoundary && !OnConnection);
        b32x PlaceTree = (Spec.Outdoors && !OnConnection && OnEdge);
        b32x RandomizeTop = false;
        if(OnWall)
        {
            WallHeight = FullWallHeight;
            Color = sRGBLinearize(0.5f, 0.2f, 0.2f, 1.0f);
            TileType = Tag_Wall;
            TileMaterial = Tag_Wood;
            Traversable = false;
        }
        else
        {
            TileType = Tag_Floor;
            if(Spec.Outdoors)
            {
                TileMaterial = Tag_Grass;
                Entity->GroundCoverSpecs[0].Type = CoverType_ThickGrass;
                Entity->GroundCoverSpecs[0].Density = 64;
            }
            else
            {
                TileMaterial = Spec.StoneFloor ? Tag_Stone : Tag_Wood;
            }
            RandomizeTop = true;
        }

        rectangle3 Vol = MakeRelative(GetVolumeFromMinZ(&Tile, WallHeight), P);

        if(Traversable)
        {
            Entity->TraversableCount = 1;
            Entity->Traversables[0].P = GetMaxZCenterP(Vol);
            Clear(&Entity->Traversables[0].Occupier);
        }

        if(Stairwell)
        {
            P.z -= tStair*Grid->TileDim.z;
        }
        v3 BasisP = P;

        P.x += 0.0f;
        P.y += 0.0f;
        //P.z += 0.5f*RandomUnilateral(Grid->Series);

        Color = sRGBLinearize(0.8f, 0.8f, 0.8f, 1.0f);
        entity_visible_piece *Piece =
            AddPiece(Entity, GetTagHash(Asset_Block, TileType, TileMaterial), GetRadius(Vol), GetCenter(Vol), Color, PieceType_Cube);
        Piece->Variant = RandomUnilateral(Gen->Entropy);

        if(RandomizeTop)
        {
            Piece->Cube.UVLayout = EncodeCubeUVLayout(0, 0, 0, 0, 0, 0,
                                                      RandomChoice(Gen->Entropy, 4),
                                                      RandomChoice(Gen->Entropy, 4));
        }

        rectangle3 OccluderVol = OnWall ? MakeRelative(GetTotalVolume(&Tile), P) : Vol;
        AddPieceOccluder(Entity, GetCenter(OccluderVol), GetRadius(OccluderVol), Color.rgb);
        if(!OnWall)
        {
            rectangle3 CeilingVol = MakeRelative(GetTotalVolume(&Tile), P);
            CeilingVol.Min.z = CeilingVol.Max.z - 0.1f;
            if(Spec.Outdoors)
            {
                AddPiece(Entity, {}, GetRadius(CeilingVol), GetCenter(CeilingVol),
                         V4(0.0f, 0.1f, 0.2f, 0.01f), PieceType_Light);
            }
            else
            {
                AddPieceOccluder(Entity, GetCenter(CeilingVol), GetRadius(CeilingVol), Color.rgb);
            }
        }

        Entity->P = P;
        Entity->CollisionVolume = Vol;
        if(OnWall)
        {
            Entity->CollisionVolume.Max.z += FullWallHeight;;
        }

        Contents->Structural = Entity;
        b32x Open = (!Stairwell && !OnConnection &&
                         (Entity->TraversableCount == 1));

        if(Open)
        {
            traversable_reference Ref = {};
            Ref.Entity = Contents->Structural->ID;
            v3 GroundP = GetSimSpaceTraversable(Grid->Region, Ref).P;
            if(PlaceTree)
            {
                entity *Tree = AddTree(Gen, Grid->Region);
                PlaceEntityOnTraversable(Grid->Region, Tree, Ref);

                rectangle3 TreeVol = MakeRelative(GetTotalVolume(&Tile), P);
                TreeVol = AddRadiusTo(TreeVol, -V3(0.25f, 0.25f, 1.0f));
                AddPieceOccluder(Tree, GetCenter(TreeVol), GetRadius(TreeVol), Color.rgb);
            }
        }
    }

#if 0
    u32 DoorIndex = 0;
    for(gen_room_connection *RoomCon = Room->FirstConnection;
        RoomCon;
        RoomCon = RoomCon->Next)
    {
        gen_connection *Con = RoomCon->Connection;
        if(IsValid(RoomCon->DoorBrainID))
        {
            Assert(DoorIndex < ArrayCount(brain_switches::Unlocks));

            gen_volume DoorVol = Intersect(&Con->Vol, &Room->Vol);
            edit_tile_contents *DoorTile = GetTileAbs(Grid, DoorVol.Min);
            v3 DoorDim = Grid->TileDim;
            DoorDim.z = FullWallHeight;

            traversable_reference Ref = {};
            Ref.Entity = DoorTile->Structural->ID;
            entity *Door = AddBlock(Grid->Region, GetTagHash(Asset_Block, Tag_Floor, Tag_Stone), DoorDim, V4(1, 1, 1, 1));
            PlaceEntityOnTraversable(Grid->Region, Door, Ref);
            Door->BrainSlot = IndexedBrainSlotFor(brain_switches, Unlocks, DoorIndex);
            Door->BrainID = RoomCon->DoorBrainID;

            ++DoorIndex;
        }
    }

    for(gen_entity_pattern *Pattern = Room->FirstPattern;
            Pattern;
            Pattern = Pattern->Next)
    {
        Pattern->Creator(Grid, Pattern);
    }
#endif

    entity *CamRoom = AddEntity(Grid->Region);
    CamRoom->P = GetCenter(GetRoomVolume(Grid));
    CamRoom->CollisionVolume = MakeRelative(GetRoomVolume(Grid), CamRoom->P);
    CamRoom->BrainSlot = SpecialBrainSlot(Type_brain_room);
    CamRoom->CameraOffset.z = GetCameraOffsetZForDim(Grid->TileCount, &CamRoom->CameraBehavior);

#if 0
    world_room *WorldRoom = AddWorldRoom(Gen->World, GetRoomMinWorldP(Grid), GetRoomMaxWorldP(Grid),
                                         V3(0, 0, 1));
#endif

    EndGridEdit(Grid);

#if 0
    if(Spec->Apron)
    {
        gen_apron *Apron = GenApron(Gen, Spec->Apron);
        Apron->Vol = AddRadiusTo(&RoomVol, GenV3(8, 8, 0));
    }
#endif
}

internal void
GenerateApron(world_generator *Gen,
              gen_apron *Apron)
{
    gen_apron_spec *Spec = Apron->Spec;

    edit_grid *Grid = BeginGridEdit(Gen, Apron->Vol);
    for(edit_tile Tile = IterateAsPlanarTiles(Grid);
        IsValid(Tile);
        Advance(&Tile))
    {
        f32 Eps = 0.001f;
        b32 Overlapped = OverlappingEntitiesExist(Grid->Region, AddRadiusTo(GetTotalVolume(&Tile), V3(-Eps, -Eps, -Eps)));
        if(!Overlapped)
        {
            v3 P = GetMinZCenterP(&Tile);
            f32 Height = RandomBetween(Grid->Series, 0.5f, 1.0f);
            rectangle3 Vol = MakeRelative(GetVolumeFromMinZ(&Tile, Height), P);

            entity *Entity = AddEntity(Grid->Region);
            Entity->P = P;
            Entity->CollisionVolume = Vol;

            v4 Color = sRGBLinearize(0.5f, 0.5f, 0.5f, 1.0f);

            entity_visible_piece *Piece = AddPiece(Entity, GetTagHash(Asset_Block, Tag_Floor, Tag_Grass), GetRadius(Vol), GetCenter(Vol), Color, PieceType_Cube);

            AddPieceOccluder(Entity, GetCenter(Vol), GetRadius(Vol), Color.rgb);
            rectangle3 CeilingVol = MakeRelative(GetTotalVolume(&Tile), P);
            CeilingVol.Min.z = CeilingVol.Max.z - 0.1f;
            AddPiece(Entity, {}, GetRadius(CeilingVol), GetCenter(CeilingVol),
                     V4(0.0f, 0.1f, 0.2f, 0.005f), PieceType_Light);

            Entity->GroundCoverSpecs[0].Type = CoverType_ThickGrass;
            Entity->GroundCoverSpecs[0].Density = 64;

            v3 GroundP = P + GetMaxZCenterP(Vol);
            if(RandomChoice(Grid->Series, 3))
            {
                entity *Tree = AddTree(Gen, Grid->Region);
                PlaceEntityAtP(Tree, GroundP);
            }
        }
    }
    EndGridEdit(Grid);
}
#endif