/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

internal u32x
GetDirMaskFromRoom(gen_connection *Connection, gen_room *From)
{
    u32x DirMask = Connection->DirMaskFromA;
    if(Connection->B == From)
    {
        DirMask = GetBoxMaskComplement(DirMask);
    }
    else
    {
        Assert(Connection->A == From);
    }

    return(DirMask);
}

internal b32x
CouldGoDirection(gen_connection *Connection, gen_room *From, u32x TestMask)
{
    u32x DirMask = GetDirMaskFromRoom(Connection, From);
    b32x Result = (DirMask & TestMask);
    return(Result);
}

internal b32x
CouldGoDirection(gen_connection *Connection, gen_room *From, u32x Dim, u32x Side)
{
    b32x Result = CouldGoDirection(Connection, From, GetSurfaceMask(Dim, Side));
    return(Result);
}

internal gen_room_connection *
AddRoomConnection(world_generator *Gen, gen_room *Room, gen_connection *Connection)
{
    gen_room_connection *RoomCon = PushStruct(&Gen->Memory, gen_room_connection);

    RoomCon->Connection = Connection;
    RoomCon->Next = Room->FirstConnection;

    Room->FirstConnection = RoomCon;

    return(RoomCon);
}

internal gen_connection *
Connect(world_generator *Gen, gen_room *A, gen_room *B, u32 DirMaskFromA = BoxMask_Planar)
{
    gen_connection *Connection = PushStruct(&Gen->Memory, gen_connection);

    Connection->DirMaskFromA = DirMaskFromA;
    Connection->A = A;
    Connection->B = B;

    Connection->GlobalNext = Gen->FirstConnection;
    Gen->FirstConnection = Connection;

    Connection->AToB = AddRoomConnection(Gen, A, Connection);
    Connection->BToA = AddRoomConnection(Gen, B, Connection);

    return(Connection);
}

internal gen_connection *
Connect(world_generator *Gen, gen_room *A, box_surface_index Direction, gen_room *B)
{
    gen_connection *Result = Connect(Gen, A, B, GetSurfaceMask(Direction));
    return(Result);
}

internal void
SetSize(world_generator *Gen, gen_room_spec *Spec, s32 DimX, s32 DimY)
{
    Spec->RequiredDim.x = DimX;
    Spec->RequiredDim.y = DimY;
}

internal gen_option_iterator
IterateOptions(world_generator *Gen, gen_option_type Type)
{
    gen_option_iterator Result = {};

    // TODO(casey): Not yet implemented for reals
    gen_option_array *Array = Gen->OptionArrays + Type;
    if(Array->OptionCount)
    {
        Result.Room = Array->Options[--Array->OptionCount].Room;
    }

    return(Result);
}

internal b32x
IsValid(gen_option_iterator *Iter)
{
    // TODO(casey): Not yet implemented for reals
    b32x Result = (Iter->Room != 0);
    return(Result);
}

internal void
Advance(gen_option_iterator *Iter)
{
    // TODO(casey): Not yet implemented for reals
    Iter->Room = 0;
}

internal void
Finish(gen_option_iterator *Iter)
{
    // TODO(casey): Not yet implemented for reals
    Iter->Room = 0;
}

internal gen_option *
AddOption(world_generator *Gen, gen_room *Room, gen_option_type OptionType)
{
    gen_option_array *Array = Gen->OptionArrays + OptionType;
    Assert(Array->OptionCount <= Array->MaxOptionCount);

    if(Array->OptionCount == Array->MaxOptionCount)
    {
        // TODO(casey): Very wasteful here (doesn't free the old option lock).  Do we care?
        Array->MaxOptionCount += 100;
        gen_option *NewOptions = PushArray(&Gen->Memory, Array->MaxOptionCount, gen_option);
        CopyArray(Array->OptionCount, Array->Options, NewOptions);
        Array->Options = NewOptions;
    }

    gen_option *Result = Array->Options + Array->OptionCount++;
    Result->Room = Room;

    return(Result);
}

internal world_generator *
BeginWorldGen(world *World, game_assets *Assets)
{
    world_generator *Gen = BootstrapPushStruct(world_generator, Memory);
    Gen->World = World;
    Gen->Assets = Assets;

    f32 TileSideInMeters = 1.4f;
    f32 TileDepthInMeters = World->ChunkDimInMeters.z; // TOOD(casey): Probably this will not be true, maybe?  We shall see.
    Gen->TileDim = V3(TileSideInMeters, TileSideInMeters, TileDepthInMeters);

    return(Gen);
}

internal gen_room *
GetOtherRoom(gen_connection *Connection, gen_room *FromRoom)
{
    gen_room *Result = Connection->A;
    if(Connection->A == FromRoom)
    {
        Assert(Connection->B != FromRoom); // NOTE(casey): Should only fire on connections that are from and to the same room!
        Result = Connection->B;
    }
    else
    {
        Assert(Connection->B == FromRoom);
    }

    return(Result);
}

internal gen_room_connection *
GetRoomConnectionTo(gen_room *FromRoom, gen_room *ToRoom)
{
    gen_room_connection *Result = 0;

    for(gen_room_connection *Test = FromRoom->FirstConnection;
        Test;
        Test = Test->Next)
    {
        if(GetOtherRoom(Test->Connection, FromRoom) == ToRoom)
        {
            Result = Test;
            break;
        }
    }

    return(Result);
}

internal void
PushRoom(gen_room_stack *Stack, gen_room *Room)
{
    Assert(Room);

    if(!Stack->FirstFree)
    {
        Stack->FirstFree = PushStruct(Stack->Memory, gen_room_stack_entry);
    }

    gen_room_stack_entry *Entry = Stack->FirstFree;
    Stack->FirstFree = Entry->Prev;

    Entry->Room = Room;
    Entry->Prev = Stack->Top;
    Stack->Top = Entry;
}

internal b32x
HasEntries(gen_room_stack *Stack)
{
    b32x Result = (Stack->Top != 0);
    return(Result);
}

internal gen_room *
PopRoom(gen_room_stack *Stack)
{
    gen_room *Result = 0;

    gen_room_stack_entry *Popped = Stack->Top;
    if(Popped)
    {
        Result = Popped->Room;
        Assert(Result);

        Stack->Top = Popped->Prev;

        Popped->Prev = Stack->FirstFree;
        Stack->FirstFree = Popped;

        // NOTE(casey): Just for safety's sake
        Popped->Room = 0;
    }

    return(Result);
}

internal void
PushConnectedRooms(gen_room_stack *Stack, gen_room *Room, u32 GenerationIndex)
{
    for(gen_room_connection *RoomCon = Room->FirstConnection;
        RoomCon;
        RoomCon = RoomCon->Next)
    {
        gen_connection *Connection = RoomCon->Connection;
        gen_room *OtherRoom = GetOtherRoom(Connection, Room);
        if(OtherRoom->GenerationIndex != GenerationIndex)
        {
            PushRoom(Stack, OtherRoom);
        }
    }
}

internal void
EndWorldGen(world_generator *Gen)
{
    ClearUnpackedEntityCache(Gen->World);
    Clear(&Gen->TempMemory);
    Clear(&Gen->Memory);
}

internal gen_entity_pattern *
AddPattern(world_generator *Gen, gen_room *Room, gen_create_pattern *Creator)
{
    gen_entity_pattern *Result = PushStruct(&Gen->Memory, gen_entity_pattern);
    Result->Next = Room->FirstPattern;
    Result->Creator = Creator;

    Room->FirstPattern = Result;

    return(Result);
}

internal gen_entity_pattern *
PlaceNPC(world_generator *Gen, asset_tag_id NameTag)
{
    gen_entity_pattern *Result = 0;

    for(gen_option_iterator Iter = IterateOptions(Gen, GenOption_Orphan);
        IsValid(&Iter);
        Advance(&Iter))
    {
        gen_room *Room = Iter.Room;

        // TODO(casey): Check this room to see if it meets our other criteria
        // (whatever those happen to be)
        if(1)
        {
            Result = AddPattern(Gen, Room, NPCPattern);
            Result->BaseAsset = GetTagHash(Asset_None, NameTag);
            Finish(&Iter);
        }
    }

    return(Result);
}

#if 0
internal gen_dungeon
CreateDungeon(world_generator *Gen, s32 FloorCount)
{
    gen_dungeon Result = {};

    gen_room_spec *DungeonSpec = GenSpec(Gen);
    SetSize(Gen, DungeonSpec, 17, 9, 1);

    FloorCount = 1;

    gen_room *RoomAbove = 0;
    for(s32 FloorIndex = 0;
        FloorIndex < FloorCount;
        ++FloorIndex)
    {
        temporary_memory Temp = BeginTemporaryMemory(&Gen->TempMemory);

        gen_room *FloorEntranceRoom = GenRoom(Gen, DungeonSpec, "Floor Entrance");
        if(RoomAbove)
        {
            Connect(Gen, RoomAbove, BoxIndex_North, FloorEntranceRoom);
        }
        else
        {
            Result.EntranceRoom = FloorEntranceRoom;
            gen_entity_pattern *Snake = AddPattern(Gen, FloorEntranceRoom, SnakePattern);
            Snake->BaseAsset = GetTagHash(Asset_None, Tag_Undead, Tag_Bones);
        }

        gen_room *PrevRoom = FloorEntranceRoom;
        s32 PathCount = RandomBetween(Gen->Entropy, 4 + FloorIndex/2, 6 + FloorIndex);

        gen_room **Chain = PushArray(&Gen->TempMemory, PathCount, gen_room *);
        for(s32 PathIndex = 0;
            PathIndex < PathCount;
            ++PathIndex)
        {
            gen_room *Room = GenRoom(Gen, DungeonSpec, "Dungeon Path");
            Chain[PathIndex] = Room;

            gen_connection *Connection = Connect(Gen, PrevRoom, Room, BoxMask_Planar);
            Connection->AToB->DoorBrainID = AddBrain(Gen->World);

            #if 1
            gen_entity_pattern *Switches = AddPattern(Gen, PrevRoom, TileSwitchPattern);
            Switches->BrainID = Connection->AToB->DoorBrainID;
            #endif

            gen_entity_pattern *Snake = AddPattern(Gen, Room, SnakePattern);
            Snake->BaseAsset = GetTagHash(Asset_None, Tag_Undead, Tag_Bones);

            gen_entity_pattern *Enemy = AddPattern(Gen, Room, SingleEnemyPattern);
            Enemy->BaseAsset = GetTagHash(Asset_None, Tag_Undead, Tag_Bones);

            //AddPattern(Gen, Room, StandardLightingPattern);

            PrevRoom = Room;
        }

        // TODO(casey): Need a utility here that removes path rooms when they are
        // chosen, to avoid over-connecting a room with special rooms.
        gen_room *Shop = GenRoom(Gen, DungeonSpec, "Shop");
        Connect(Gen, Chain[RandomChoice(Gen->Entropy, PathCount)], Shop, BoxMask_Planar);
        AddPattern(Gen, Shop, StandardLightingPattern);

        gen_room *ItemRoom = GenRoom(Gen, DungeonSpec, "Item Room");
        Connect(Gen, Chain[RandomChoice(Gen->Entropy, PathCount)], ItemRoom, BoxMask_Planar);
        AddPattern(Gen, ItemRoom, StandardLightingPattern);

        gen_room *FloorExitRoom = GenRoom(Gen, DungeonSpec, "Floor Exit");
        Connect(Gen, PrevRoom, FloorExitRoom, BoxMask_Planar);
        AddPattern(Gen, FloorExitRoom, StandardLightingPattern);

        RoomAbove = FloorExitRoom;

        EndTemporaryMemory(Temp);
    }

    Result.ExitRoom = RoomAbove;

    return(Result);
}
#endif

internal void PlaceDebugMarker(world_generator *Gen, v2 P, v3 DebugColor)
{
    v3 Radius = V3(7.0f, 5.0f, 1.0f);
    world_position MinP = MapIntoChunkSpace(Gen->World, {}, V3(P, 0.0f) - Radius);
    world_position MaxP = MapIntoChunkSpace(Gen->World, {}, V3(P, 0.0f) + Radius);
    AddWorldRoom(Gen->World, MinP, MaxP, DebugColor);
}

internal world_gen_location *GetLocation(world_generator *Gen, u32 Index)
{
    Assert(Index < Gen->LocationCount);
    world_gen_location *Result = Gen->Locations + Index;
    return Result;
}

internal world_gen_location *PushLocation(world_generator *Gen, world_gen_location_type Type, v2 P)
{
    Assert(Gen->LocationCount < Gen->MaxLocationCount);
    u32 LocIndex = Gen->LocationCount++;
    world_gen_location *Loc = GetLocation(Gen, LocIndex);
    *Loc = {};
    Loc->Type = Type;
    Loc->P = P;
    return Loc;
}

internal u32 FlagOf(world_gen_location_type Type)
{
    u32 Result = (u32)(1 << Type);
    return Result;
}

internal b32 Matches(world_gen_location *Loc, flag32(world_gen_location_type) ExcludeTypes)
{
    b32 Result = !(FlagOf(Loc->Type) & ExcludeTypes);
    return Result;
}

internal world_gen_distance GetClosestLocationTo(world_generator *Gen, v2 P, flag32(world_gen_location_type) ExcludeTypes = 0)
{
    world_gen_distance Result;
    Result.DistSq = F32Max;
    Result.LocIndex = 0;
    for(u32 TestIndex = 0; TestIndex < Gen->LocationCount; ++TestIndex)
    {
        world_gen_location *Test = GetLocation(Gen, TestIndex);
        if(Matches(Test, ExcludeTypes))
        {
            f32 DistSq = LengthSq(P - Test->P);
            if(Result.DistSq > DistSq)
            {
                Result.DistSq = DistSq;
                Result.LocIndex = TestIndex;
            }
        }
    }

    return Result;
}

internal f32 GetClosestDistanceSq(world_generator *Gen, v2 P, flag32(world_gen_location_type) ExcludeTypes = 0)
{
    f32 Result = GetClosestLocationTo(Gen, P, ExcludeTypes).DistSq;
    return Result;
}

internal b32 IsInForest(world_generator *Gen, v2 P)
{
    b32 Result = (P.y > Gen->OrphangeYLine);
    return Result;
}

internal world_gen_pick_v2  PickPointInAnnulus(world_generator *Gen, world_gen_annulus Ann)
{
    world_gen_pick_v2 Result = {};

    // TODO(casey): Use a better point-in-annulus generator, this wastes tons of time
    // picking points :(
    for(u32 Iteration = 0; Iteration < 10000; ++Iteration)
    {
        f32 DistFromCenter = RandomBetween(Gen->Entropy, Ann.MinDist, Ann.MaxDist);
        Result.P = (Ann.P + DistFromCenter*Arm2(RandomBetween(Gen->Entropy, 0.0f, Tau32)));

        if(IsInForest(Gen, Result.P))
        {
            Result.PassedFilters = true;
            for(u32 FilterIndex = 0; FilterIndex < ArrayCount(Ann.Filters); ++FilterIndex)
            {
                world_gen_filter Filter = Ann.Filters[FilterIndex];
                if((Filter.MinDist > 0) &&
                   (GetClosestDistanceSq(Gen, Result.P, Filter.Exclude) < Square(Filter.MinDist)))
                {
                    Result.PassedFilters = false;
                    break;
                }
            }

            if(Result.PassedFilters)
            {
                break;
            }
        }
    }

    return Result;
}

internal void LayoutOverwold(world_generator *Gen)
{
    u32 DungeonCount = RandomBetween(Gen->Entropy, 6, 9);
    u32 ItemRoomCount = RandomBetween(Gen->Entropy, 6, 9);
    u32 MonsterGeneratorCount = RandomBetween(Gen->Entropy, 12, DungeonCount + ItemRoomCount);
    u32 NavRoomCount = RandomBetween(Gen->Entropy, 4, DungeonCount);

    Gen->MaxLocationCount = (1 + DungeonCount + ItemRoomCount + MonsterGeneratorCount + NavRoomCount);
    Gen->Locations = PushArray(&Gen->TempMemory, Gen->MaxLocationCount, world_gen_location);

    v2 OrphanageP = V2(0, 0);
    Gen->OrphangeYLine = OrphanageP.y + 5.0f;
    world_gen_location *Orphanage = PushLocation(Gen, WorldLoc_Orphanage, V2(0, 0));
    Orphanage->ConnectedToOrphanage = true;

    for(u32 PlacedCount = 0; PlacedCount < DungeonCount; ++PlacedCount)
    {
        world_gen_annulus Annulus = {};
        Annulus.P = OrphanageP;
        Annulus.MinDist = 80.0f;
        Annulus.MaxDist = 160.0f;
        Annulus.Filters[0].MinDist = 60.0f;

        world_gen_pick_v2 Pick = PickPointInAnnulus(Gen, Annulus);
        Assert(Pick.PassedFilters);
        world_gen_location *PlacedLoc = PushLocation(Gen, WorldLoc_Dungeon, Pick.P);
    }

    for(u32 PlacedCount = 0; PlacedCount < ItemRoomCount; ++PlacedCount)
    {
        world_gen_annulus Annulus = {};
        Annulus.P = OrphanageP;
        Annulus.MinDist = 40.0f;
        Annulus.MaxDist = 250.0f;
        Annulus.Filters[0].MinDist = 20.0f;
        Annulus.Filters[1].Exclude = ~FlagOf(WorldLoc_ItemRoom);
        Annulus.Filters[1].MinDist = 60.0f;

        world_gen_pick_v2 Pick = PickPointInAnnulus(Gen, Annulus);
        Assert(Pick.PassedFilters);
        world_gen_location *PlacedLoc = PushLocation(Gen, WorldLoc_ItemRoom, Pick.P);
    }

    for(u32 PlacedCount = 0; PlacedCount < MonsterGeneratorCount; ++PlacedCount)
    {
        u32 MinCoverCount = U32Max;
        v2 PickLoc = {};
        u32 CoveredTypes = FlagOf(WorldLoc_Dungeon) | FlagOf(WorldLoc_ItemRoom);
        for(u32 LocIndex = 0; LocIndex < Gen->LocationCount; ++LocIndex)
        {
            world_gen_location *TestLoc = GetLocation(Gen, LocIndex);
            if((FlagOf(TestLoc->Type) & CoveredTypes) &&
               (MinCoverCount > TestLoc->OverlappingGeneratorCount))
            {
                MinCoverCount = TestLoc->OverlappingGeneratorCount;
                PickLoc = TestLoc->P;
            }
        }

        f32 GeneratorRadius = 50.0f;

        world_gen_annulus Annulus = {};
        Annulus.P = PickLoc;
        Annulus.MinDist = 20.0f;
        Annulus.MaxDist = GeneratorRadius;
        Annulus.Filters[0].MinDist = 20.0f;
#if 0
        Annulus.Filters[1].Exclude = ~FlagOf(WorldLoc_MonsterGenerator);
        Annulus.Filters[1].MinDist = 60.0f;
#endif

        world_gen_pick_v2 Pick = PickPointInAnnulus(Gen, Annulus);
        Assert(Pick.PassedFilters);
        world_gen_location *PlacedLoc = PushLocation(Gen, WorldLoc_MonsterGenerator, Pick.P);

        for(u32 LocIndex = 0; LocIndex < Gen->LocationCount; ++LocIndex)
        {
            world_gen_location *TestLoc = GetLocation(Gen, LocIndex);
            if(LengthSq(PlacedLoc->P - TestLoc->P) < Square(GeneratorRadius))
            {
                ++TestLoc->OverlappingGeneratorCount;
            }
        }
    }

    for(u32 PlacedCount = 0; PlacedCount < NavRoomCount; ++PlacedCount)
    {
        world_gen_annulus Annulus = {};
        Annulus.P = OrphanageP;
        Annulus.MinDist = 0.0f;
        Annulus.MaxDist = 200.0f;
        Annulus.Filters[0].MinDist = 30.0f;

        world_gen_pick_v2 Pick = PickPointInAnnulus(Gen, Annulus);
        if(Pick.PassedFilters)
        {
            world_gen_location *PlacedLoc = PushLocation(Gen, WorldLoc_NavRoom, Pick.P);
        }
        else
        {
            break;
        }
    }

    for(u32 LocAIndex = 0; LocAIndex < Gen->LocationCount; ++LocAIndex)
    {
        for(u32 LocBIndex = (LocAIndex + 1); LocBIndex < Gen->LocationCount; ++LocBIndex)
        {
            world_gen_location *A = Gen->Locations + LocAIndex;
            world_gen_location *B = Gen->Locations + LocBIndex;
            if(LengthSq(A->P - B->P) < Square(60.0f))
            {
                b32 Viable = true;
                for(u32 LocCIndex = 0; LocCIndex < Gen->LocationCount; ++LocCIndex)
                {
                    if((LocCIndex != LocAIndex) && (LocCIndex != LocBIndex))
                    {
                        world_gen_location *C = Gen->Locations + LocCIndex;
                        if(DistanceBetweenLineSegmentAndPointSq(A->P, B->P, C->P) < Square(20.0f))
                        {
                            Viable = false;
                            break;
                        }
                    }
                }

//                if(Viable)
                {
                    AddWorldConnection(Gen->World, LocAIndex, LocBIndex, Viable ? V3(1, 1, 1) : V3(1, 0, 0));
                }
            }
        }
    }

    for(;;)
    {
        for(;;)
        {
            b32 ChangedSomething = false;
            for(u32 ConIndex = 0; ConIndex < Gen->World->ConnectionCount; ++ConIndex)
            {
                world_room_connection *Con = Gen->World->Connections + ConIndex;
                world_gen_location *A = Gen->Locations + Con->RoomA;
                world_gen_location *B = Gen->Locations + Con->RoomB;
                if((A->ConnectedToOrphanage && !B->ConnectedToOrphanage) ||
                   (!A->ConnectedToOrphanage && B->ConnectedToOrphanage))
                {
                    A->ConnectedToOrphanage = B->ConnectedToOrphanage = true;
                    ChangedSomething = true;
                }
            }

            if(!ChangedSomething)
            {
                break;
            }
        }

        u32 MinConAIndex = 0;
        u32 MinConBIndex = 0;
        f32 MinConDistSq = F32Max;
        for(u32 LocAIndex = 0; LocAIndex < Gen->LocationCount; ++LocAIndex)
        {
            world_gen_location *LocA = Gen->Locations + LocAIndex;
            if(!LocA->ConnectedToOrphanage)
            {
                for(u32 LocBIndex = 0; LocBIndex < Gen->LocationCount; ++LocBIndex)
                {
                    world_gen_location *LocB = Gen->Locations + LocBIndex;
                    if(LocB->ConnectedToOrphanage)
                    {
                        f32 DistSq = LengthSq(LocA->P - LocB->P);
                        if(MinConDistSq > DistSq)
                        {
                            MinConAIndex = LocAIndex;
                            MinConBIndex = LocBIndex;
                            MinConDistSq = DistSq;
                        }
                    }
                }
            }
        }

        if(MinConAIndex != MinConBIndex)
        {
            AddWorldConnection(Gen->World, MinConAIndex, MinConBIndex, V3(1, 0, 1));
        }
        else
        {
            break;
        }
    }

#if 0
    /* TODO(casey): This code sets the LocA type to "none" if the room is removed, which
       means that the room will not be created. This is fine, but it means the debug display
       will erroneously connect rooms because all of the indexes in the world_room_connection
       records _are now wrong_. So we would need to track indexes correctly in order to make
       this work if we want to reenable it.
    */
    for(u32 LocAIndex = 0; LocAIndex < Gen->LocationCount; ++LocAIndex)
    {
        world_gen_location *LocA = Gen->Locations + LocAIndex;
        if(LocA->Type == WorldLoc_NavRoom)
        {
            u32 ConCount = 0;
            for(u32 ConIndex = 0; ConIndex < Gen->World->ConnectionCount; ++ConIndex)
            {
                world_room_connection *Con = Gen->World->Connections + ConIndex;
                if((Con->RoomA == LocAIndex) || (Con->RoomB == LocAIndex))
                {
                    ++ConCount;
                }
            }

            if(ConCount <= 1)
            {
                LocA->Type = WorldLoc_None;
                for(u32 ConIndex = 0; ConIndex < Gen->World->ConnectionCount; ++ConIndex)
                {
                    world_room_connection *Con = Gen->World->Connections + ConIndex;
                    if((Con->RoomA == LocAIndex) || (Con->RoomB == LocAIndex))
                    {
                        Con->RoomA = Con->RoomB = 0;
                    }
                }
            }
        }
    }
#endif

    rectangle2 WorldBounds = InvertedInfinityRectangle2();
    f32 WorldScale = 0.7f;
    for(u32 LocAIndex = 0; LocAIndex < Gen->LocationCount; ++LocAIndex)
    {
        world_gen_location *LocA = Gen->Locations + LocAIndex;
        LocA->P *= WorldScale;

        rectangle2 LocRect = RectCenterDim(LocA->P, V2(40.0f, 30.0f));
        WorldBounds = Union(WorldBounds, LocRect);
    }

    Gen->WorldBounds = WorldBounds;
}

internal void CreateDungeon(world_generator *Gen, world_gen_location *Loc)
{
    PlaceDebugMarker(Gen, Loc->P, V3(1, 1, 0));
}

internal void CreateMonsterGenerator(world_generator *Gen, world_gen_location *Loc)
{
    PlaceDebugMarker(Gen, Loc->P, V3(1, 0, 0));
}

internal void CreateItemRoom(world_generator *Gen, world_gen_location *Loc)
{
    PlaceDebugMarker(Gen, Loc->P, V3(0, 1, 1));
}

internal void CreateNavRoom(world_generator *Gen, world_gen_location *Loc)
{
    PlaceDebugMarker(Gen, Loc->P, V3(0, 1, 0));
}

internal void CreateOrphanage(world_generator *Gen, world_gen_location *Loc)
{
    PlaceDebugMarker(Gen, Loc->P, V3(0, 1, 0));

#if 0
    gen_orphanage Result = {};

    gen_apron_spec *ApronSpec = GenApronSpec(Gen);

    gen_room_spec *GardenSpec = GenSpec(Gen, ApronSpec);
    GardenSpec->Outdoors = true;

    gen_room_spec *BasicForestSpec = GenSpec(Gen, ApronSpec);
    BasicForestSpec->Outdoors = true;

    gen_room_spec *BedroomSpec = GenSpec(Gen, ApronSpec);
    BedroomSpec->StoneFloor = true;

    gen_room_spec *SaveSlotSpec = GenSpec(Gen, ApronSpec);
    gen_room_spec *MainRoomSpec = GenSpec(Gen, ApronSpec);
    gen_room_spec *TailorSpec = GenSpec(Gen, ApronSpec);
    gen_room_spec *KitchenSpec = GenSpec(Gen, ApronSpec);
    gen_room_spec *VerticalHallwaySpec = GenSpec(Gen, ApronSpec);
    gen_room_spec *HorizontalHallwaySpec = GenSpec(Gen, ApronSpec);

    gen_room *MainRoom = GenRoom(Gen, MainRoomSpec, "Orphanage Main Room");
    gen_room *HeroSaveSlotA = GenRoom(Gen, SaveSlotSpec, "Save Slot A");
    gen_room *HeroSaveSlotB = GenRoom(Gen, SaveSlotSpec, "Save Slot B");
    gen_room *HeroSaveSlotC = GenRoom(Gen, SaveSlotSpec, "Save Slot C");
    gen_room *FrontHall = GenRoom(Gen, VerticalHallwaySpec, "Orphanage Front Hallway");
    gen_room *BackHall = GenRoom(Gen, HorizontalHallwaySpec, "Orphanage Back Hallway");
    gen_room *BedroomA = GenRoom(Gen, BedroomSpec, "Orphanage Bedroom A");
    gen_room *BedroomB = GenRoom(Gen, BedroomSpec, "Orphanage Bedroom B");
    gen_room *BedroomC = GenRoom(Gen, BedroomSpec, "Orphanage Bedroom C");
    gen_room *BedroomD = GenRoom(Gen, BedroomSpec, "Orphanage Bedroom D");
    gen_room *TailorRoom = GenRoom(Gen, TailorSpec, "Orphanage Tailor's Room");
    gen_room *Kitchen = GenRoom(Gen, KitchenSpec, "Orphanage Kitchen");
    gen_room *Garden = GenRoom(Gen, GardenSpec, "Orphanage Garden");
    gen_room *ForestPath = GenRoom(Gen, BasicForestSpec, "Orphanage Forest Path");
    gen_room *ForestEntrance = GenRoom(Gen, BasicForestSpec, "Orphanage Forest Entrance");
#if 0
    gen_room *SideAlley = GenRoom(Gen, BasicForestSpec, "Orphange Side Alley");
#endif
    //    AddPattern(Gen, MainRoom, StandardLightingPattern);
    AddPattern(Gen, HeroSaveSlotA, StandardLightingPattern);
    AddPattern(Gen, HeroSaveSlotB, StandardLightingPattern);
    AddPattern(Gen, HeroSaveSlotC, StandardLightingPattern);
    AddPattern(Gen, FrontHall, StandardLightingPattern);
    AddPattern(Gen, BackHall, StandardLightingPattern);
    AddPattern(Gen, BedroomA, StandardLightingPattern);
    AddPattern(Gen, BedroomB, StandardLightingPattern);
    AddPattern(Gen, BedroomC, StandardLightingPattern);
    AddPattern(Gen, BedroomD, StandardLightingPattern);
    AddPattern(Gen, TailorRoom, StandardLightingPattern);
    AddPattern(Gen, Kitchen, StandardLightingPattern);
    // AddPattern(Gen, Garden, StandardLightingPattern);
    // AddPattern(Gen, ForestPath, StandardLightingPattern);
    // AddPattern(Gen, ForestEntrance, StandardLightingPattern);

    AddOption(Gen, MainRoom, GenOption_Cat);
    //AddOption(Gen, MainRoom, GenOption_Orphan);
    AddOption(Gen, BedroomA, GenOption_Cat);
    AddOption(Gen, BedroomA, GenOption_Orphan);
    AddOption(Gen, BedroomB, GenOption_Cat);
    AddOption(Gen, BedroomB, GenOption_Orphan);
    AddOption(Gen, BedroomC, GenOption_Cat);
    AddOption(Gen, BedroomC, GenOption_Orphan);
    AddOption(Gen, BedroomD, GenOption_Cat);
    AddOption(Gen, BedroomD, GenOption_Orphan);
    AddOption(Gen, TailorRoom, GenOption_Cat);
    AddOption(Gen, TailorRoom, GenOption_Orphan);
    AddOption(Gen, Kitchen, GenOption_Cat);
    AddOption(Gen, Kitchen, GenOption_Orphan);
    AddOption(Gen, Garden, GenOption_Orphan);

    SetSize(Gen, MainRoomSpec, 13, 13);
    SetSize(Gen, TailorSpec, 8, 6);
    SetSize(Gen, KitchenSpec, 8, 6);
    SetSize(Gen, VerticalHallwaySpec, 5, 13);
    SetSize(Gen, BedroomSpec, 8, 6);
    SetSize(Gen, HorizontalHallwaySpec, 13, 5);
    SetSize(Gen, SaveSlotSpec, 5, 6);
    SetSize(Gen, GardenSpec, 13, 13);
    SetSize(Gen, BasicForestSpec, 13, 13);
#if 0
    SetSize(Gen, SideAlley);
#endif

    Connect(Gen, MainRoom, BoxIndex_North, ForestPath);
    Connect(Gen, MainRoom, BoxIndex_West, TailorRoom);
    Connect(Gen, MainRoom, BoxIndex_West, Kitchen);
    Connect(Gen, MainRoom, BoxIndex_South, FrontHall);

    Connect(Gen, FrontHall, BoxIndex_East, BedroomD);
    Connect(Gen, FrontHall, BoxIndex_East, BedroomB);
    Connect(Gen, FrontHall, BoxIndex_West, BedroomC);
    Connect(Gen, FrontHall, BoxIndex_West, BedroomA);
    Connect(Gen, FrontHall, BoxIndex_South, BackHall);

    Connect(Gen, BackHall, BoxIndex_South, HeroSaveSlotA);
    Connect(Gen, BackHall, BoxIndex_South, HeroSaveSlotB);
    Connect(Gen, BackHall, BoxIndex_South, HeroSaveSlotC);
    Connect(Gen, BackHall, BoxIndex_East, Garden);

#if 0
    Connect(Gen, Garden, BoxIndex_North, SideAlley);
    Connect(Gen, SideAlley, BoxIndex_North, ForestPath);
#endif
    Connect(Gen, ForestPath, BoxIndex_North, ForestEntrance);

    Result.ForestEntrance = ForestEntrance;
    Result.HeroBedroom = HeroSaveSlotA;
#endif
}

internal gen_result
CreateWorld(world *World, game_assets *Assets)
{
    gen_result Result = {};

    world_generator *Gen = BeginWorldGen(World, Assets);

    Gen->Entropy = &World->GameEntropy;

    LayoutOverwold(Gen);

    for(u32 LocationIndex = 0; LocationIndex < Gen->LocationCount; ++LocationIndex)
    {
        world_gen_location *Loc = GetLocation(Gen, LocationIndex);
        switch(Loc->Type)
        {
            case WorldLoc_Orphanage:
            {
                CreateOrphanage(Gen, Loc);
            } break;

            case WorldLoc_Dungeon:
            {
                CreateDungeon(Gen, Loc);
            } break;

            case WorldLoc_MonsterGenerator:
            {
                CreateMonsterGenerator(Gen, Loc);
            } break;

            case WorldLoc_ItemRoom:
            {
                CreateItemRoom(Gen, Loc);
            } break;

            case WorldLoc_NavRoom:
            {
                CreateNavRoom(Gen, Loc);
            } break;
        }
    }

    for(u32 LocIndex = 0; LocIndex < Gen->LocationCount; ++LocIndex)
    {
        world_gen_location *Loc = Gen->Locations + LocIndex;
        world_room *Room = Gen->World->Rooms + LocIndex;
        if(!Loc->ConnectedToOrphanage)
        {
            Room->DebugColor = V3(1, 0, 1);
        }
    }

    s32 BlockDim = 16;
    v2s MinTileI = TileFromPoint(Gen, Gen->WorldBounds.Min);
    v2s MaxTileI = TileFromPoint(Gen, Gen->WorldBounds.Max);

    s32 XCount = ((MaxTileI.x - MinTileI.x) + BlockDim - 1) / BlockDim;
    s32 YCount = ((MaxTileI.y - MinTileI.y) + BlockDim - 1) / BlockDim;

    for(s32 BlockY = 0; BlockY < YCount; ++BlockY)
    {
        for(s32 BlockX = 0; BlockX < XCount; ++BlockX)
        {
            rectangle2i BlockVol = {};
            BlockVol.Min = V2S(MinTileI.x + BlockX*BlockDim, MinTileI.y + BlockY*BlockDim);
            BlockVol.Max = V2S(BlockVol.Min.x + BlockDim, BlockVol.Min.y + BlockDim);

            GenerateBlock(Gen, BlockVol);
        }
    }

#if 0
    gen_orphanage Orphanage = CreateOrphanage(Gen);
    gen_dungeon Dungeon = CreateDungeon(Gen, 7);
    Connect(Gen, Orphanage.ForestEntrance, BoxIndex_North, Dungeon.EntranceRoom);

    gen_room *StartRoom = Orphanage.HeroBedroom;

    PlaceNPC(Gen, Tag_Hannah);
    PlaceNPC(Gen, Tag_Fred);
    PlaceNPC(Gen, Tag_Molly);

    PlaceNPC(Gen, Tag_Baby);
    PlaceNPC(Gen, Tag_Brahm);
    PlaceNPC(Gen, Tag_Carla);
    PlaceNPC(Gen, Tag_Cassidy);
    PlaceNPC(Gen, Tag_Drew);
    PlaceNPC(Gen, Tag_Dylan);
    PlaceNPC(Gen, Tag_Giles);
    PlaceNPC(Gen, Tag_Kline);
    PlaceNPC(Gen, Tag_Laird);
    PlaceNPC(Gen, Tag_Lambert);
    PlaceNPC(Gen, Tag_Rhoda);
    PlaceNPC(Gen, Tag_Slade);
    PlaceNPC(Gen, Tag_Sunny);
    PlaceNPC(Gen, Tag_Viva);

    Layout(Gen, StartRoom);
    GenerateWorld(Gen);
#endif

    rectangle2i HeroRoom = Gen->StartingRoom;
    Result.InitialCameraP = ChunkPositionFromTilePosition(
        Gen,
        (HeroRoom.Min.x + HeroRoom.Max.x)/2,
        (HeroRoom.Min.y + HeroRoom.Max.y)/2,
                                                          0);

    world_position MinP = MapIntoChunkSpace(Gen->World, {}, V3(Gen->WorldBounds.Min, 0.0f));
    world_position MaxP = MapIntoChunkSpace(Gen->World, {}, V3(Gen->WorldBounds.Max, 0.0f));
    AddWorldRoom(Gen->World, MinP, MaxP, V3(1, 1, 1));

    EndWorldGen(Gen);

    return(Result);
}

internal void
CreateWorld(game_mode_world *WorldMode, game_assets *Assets)
{
    // TODO(casey): If we _do_ want to go with transient-memory-bounds,
    // we _could_ just use that as the arena in the generator
    gen_result Generated = CreateWorld(WorldMode->World, Assets);
    WorldMode->Camera.P = WorldMode->Camera.SimulationCenter = Generated.InitialCameraP;

    WorldMode->StandardRoomDimension = V3(17.0f*1.4f,
                                          9.0f*1.4f,
                                          WorldMode->TypicalFloorHeight);
    WorldMode->StandardApronRadius = 2.0f*WorldMode->StandardRoomDimension;
}
