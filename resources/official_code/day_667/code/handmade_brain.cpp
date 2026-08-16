/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

internal b32
IsValid(brain_id ID)
{
    b32 Result = (ID.Value);
    return(Result);
}

internal void SetAnimation(entity *Entity, entity_animation Animation, f32 dt)
{
    Entity->tMovement = 0.0f;
    Entity->dtMovement = dt;
    Entity->Animation = Animation;
}

inline void
MarkBrainActives(brain *Brain)
{
    u32 BrainFlags = 0;
    for(u32 SlotIndex = 0;
        SlotIndex < MAX_BRAIN_SLOT_COUNT;
        ++SlotIndex)
    {
        entity *Entity = GetEntityInSlot(Brain, SlotIndex);
        if(Entity)
        {
            BrainFlags |= Entity->Flags;
        }
    }

    if(BrainFlags & EntityFlag_Active)
    {
        Brain->Active = true;
        for(u32 SlotIndex = 0;
            SlotIndex < MAX_BRAIN_SLOT_COUNT;
            ++SlotIndex)
        {
            entity *Entity = GetEntityInSlot(Brain, SlotIndex);
            if(Entity)
            {
                Entity->Flags |= EntityFlag_Active;
            }
        }
    }
    else
    {
        Brain->Active = false;
    }
}

function v2s SingleTileDeltaFrom(v2 V)
{
    v2s Result = {};
    
    if(AbsoluteValue(V.x) > AbsoluteValue(V.y))
    {
        Result.x = (V.x < 0) ? -1 : 1;
    }
    else
    {
        Result.y = (V.y < 0) ? -1 : 1;
    }
}

function v2s SingleTileDeltaFrom(v3 V)
{
    v2s Result = SingleTileDeltaFrom(V.xy);
    return Result;
}

function f32 GetFacingDirectionFromSingleTileDelta(v2s dTile)
{
    Assert((dTile.x == 0) || (dTile.y == 0));
    Assert((dTile.x >= -1) && (dTile.x <= 1));
    Assert((dTile.y >= -1) && (dTile.y <= 1));

    f32 Result = (0.25f - 0.25f*(f32)dTile.x);
    if(dTile.y == -1) Result += 0.50f;
    
    // TODO(casey): Remove the *= Tau32 if we fix the system to not use 0-Tau32
    Result *= Tau32;
    
    return Result;
}

internal void
ExecuteBrainHero(game_state *GameState, game_input *Input, sim_region *SimRegion, brain *Brain, r32 dt)
{
    controlled_hero ConHero_ = {};
    controlled_hero *ConHero = &ConHero_;
    brain_hero *Parts = &Brain->Hero;
    entity *Head = Parts->Head;

    r32 dZ = 0.0f;
    b32 Exited = false;
    b32 DebugSpawn = false;
    b32 Dodging = false;
    f32 ClutchLevel = 0.0f;

    b32 Attacked = false;
    v2s AttackX = {};
    v2s AttackY = {};
    f32 FacingDirection = 0;
    
    if(Input)
    {
        u32 ControllerIndex = Brain->ID.Value - ReservedBrainID_FirstHero;
        game_controller_input *Controller = GetController(Input, ControllerIndex);
        ConHero = GameState->ControlledHeroes + ControllerIndex;

        ClutchLevel = Controller->ClutchMax;

        if(Controller->IsAnalog)
        {
            // NOTE(casey): Use analog movement tuning
            ConHero->ddP = V2(Controller->StickAverageX, Controller->StickAverageY);
        }
        else
        {
            // NOTE(casey): Use digital movement tuning
            r32 Recenter = 0.5f;
            if(WasPressed(Controller->MoveUp))
            {
                ConHero->ddP.x = 0.0f;
                ConHero->ddP.y = 1.0f;
            }
            if(WasPressed(Controller->MoveDown))
            {
                ConHero->ddP.x = 0.0f;
                ConHero->ddP.y = -1.0f;
            }
            if(WasPressed(Controller->MoveLeft))
            {
                ConHero->ddP.x = -1.0f;
                ConHero->ddP.y = 0.0f;
            }
            if(WasPressed(Controller->MoveRight))
            {
                ConHero->ddP.x = 1.0f;
                ConHero->ddP.y = 0.0f;
            }

            if(!IsDown(Controller->MoveLeft) &&
               !IsDown(Controller->MoveRight))
            {
                ConHero->ddP.x = 0.0f;
                if(IsDown(Controller->MoveUp))
                {
                    ConHero->ddP.y = 1.0f;
                }
                if(IsDown(Controller->MoveDown))
                {
                    ConHero->ddP.y = -1.0f;
                }
            }

            if(!IsDown(Controller->MoveUp) &&
               !IsDown(Controller->MoveDown))
            {
                ConHero->ddP.y = 0.0f;
                if(IsDown(Controller->MoveLeft))
                {
                    ConHero->ddP.x = -1.0f;
                }
                if(IsDown(Controller->MoveRight))
                {
                    ConHero->ddP.x = 1.0f;
                }
            }

            if(WasPressed(Controller->Start))
            {
                DebugSpawn = true;
            }
        }
        
        // TODO(casey): Remove the *= Tau32 if we fix the system to not use 0-Tau32
        FacingDirection *= Tau32;
        
        if(Controller->ClutchMax > 0.5f)
        {
            Dodging = true;
        }

        if(Head && WasPressed(Controller->Start))
        {
#if 0
            entity *ClosestHero = 0;
            real32 ClosestHeroDSq = Square(10.0f); // NOTE(casey): Ten meter maximum search!
            for(entity_iterator Iter = IterateAllEntities(SimRegion);
                Iter.Entity;
                Advance(&Iter))
            {
                entity *TestEntity = Iter.Entity;
                if((TestEntity->BrainID.Value != Head->BrainID.Value) &&
                   TestEntity->BrainID.Value)
                {
                    real32 TestDSq = LengthSq(TestEntity->P - Head->P);
                    if(ClosestHeroDSq > TestDSq)
                    {
                        ClosestHero = TestEntity;
                        ClosestHeroDSq = TestDSq;
                    }
                }
            }

            if(ClosestHero)
            {
                brain_id OldBrainID = Head->BrainID;
                brain_slot OldBrainSlot = Head->BrainSlot;
                Head->BrainID = ClosestHero->BrainID;
                Head->BrainSlot = ClosestHero->BrainSlot;
                ClosestHero->BrainID = OldBrainID;
                ClosestHero->BrainSlot = OldBrainSlot;
            }
#endif
        }

        if(Controller->ActionRight.EndedDown &&
           Controller->ActionRight.HalfTransitionCount)
        {
            Attacked = true;
            AttackX = V2S( 1,  0);
            AttackY = V2S( 0,  1);
            FacingDirection = 0;
        }
        if(Controller->ActionUp.EndedDown &&
           Controller->ActionUp.HalfTransitionCount)
        {
            Attacked = true;
            AttackX = V2S( 0,  1);
            AttackY = V2S(-1,  0);
            FacingDirection = 0.25f;
        }
        if(Controller->ActionLeft.EndedDown &&
           Controller->ActionLeft.HalfTransitionCount)
        {
            Attacked = true;
            AttackX = V2S(-1,  0);
            AttackY = V2S( 0, -1);
            FacingDirection = 0.5f;
        }
        if(Controller->ActionDown.EndedDown &&
           Controller->ActionDown.HalfTransitionCount)
        {
            Attacked = true;
            AttackX = V2S( 0, -1);
            AttackY = V2S( 1,  0);
            FacingDirection = 0.75f;
        }
        
        if(WasPressed(Controller->Back))
        {
            Exited = true;
        }
    }

    entity *Glove = Parts->Glove;

    entity *Body = Parts->Body;
    if(Head && Body)
    {
        if(Attacked)
        {
            Head->FacingDirection = FacingDirection;
        }

        v3 ddPNormal = {};
        v3 ddP = V3(ConHero->ddP, 0);
        f32 ddPLength = LengthSq(ddP);
        if(ddPLength > 0.05f)
        {
            f32 NormalizeC = (1.0f / SquareRoot(ddPLength));
            ddPNormal = NormalizeC*ddP;
            if(ddPLength > 1.0f)
            {
                ddP *= NormalizeC;
            }
        }

        f32 InvClutch = (1.0f - ClutchLevel);
        Head->P = Body->P + 0.1f*ddP + V3(0, 0, 0.3f + 0.1f*InvClutch);

        b32 HopRequested =
            (!Dodging &&
             (Body->Animation == Animation_Planted) &&
             (ddPLength > 0.5f));
        if(HopRequested)
        {
            v3 dTarget = 1.25f*ddPNormal;
            v2s dTileIndex = SingleTileDeltaFrom(dTarget);
            
            v2s HopTargetP = Body->TileIndex + dTileIndex;
            Body->CameFrom = Body->TileIndex;
            if(TransactionalOccupy(SimRegion, Body->ID, &Body->TileIndex, HopTargetP))
            {
                SetAnimation(Body, Animation_Hopping, 4);
            }
        }

        Body->FacingDirection = Head->FacingDirection;
        if(Body->Animation == Animation_Planted)
        {
            if(Head)
            {
                r32 HeadDistance = 0.0f;
                HeadDistance = Length(Head->P - Body->P);

                r32 MaxHeadDistance = 0.5f;
                r32 tHeadDistance = Clamp01MapToRange(0.0f, HeadDistance, MaxHeadDistance);
                Body->ddtBob = -20.0f*tHeadDistance;
            }
        }

        v3 HeadDelta = {};
        if(Head)
        {
            HeadDelta = Head->P - Body->P;
        }
        Body->FloorDisplace = (0.25f*HeadDelta).xy;

        // TODO(casey): Probably want this to be more of an "engaged bob target"
        f32 ClutchBob = 0.25f*InvClutch;
        if(Dodging)
        {
            Body->tBob = ClutchBob;
        }
    }
    
    if(Body && Glove)
    {
        if(Glove->MoveQueueIndex == Glove->MoveQueueCount)
        {
            if(Glove->tMovement == 1.0f)
            {
                if(!AreEqual(Glove->TileIndex, Body->TileIndex))
                {
                    SetAnimation(Glove, Animation_Floating, 8);
                    Glove->CameFrom = Glove->TileIndex;
                    Glove->TileIndex = Body->TileIndex;
                }
            }
            
            Glove->FacingDirection = Body->FacingDirection;
        
            f32 GloveDistanceSq = LengthSq(Glove->P - Body->P);
            b32 GloveIsWithBody = (GloveDistanceSq < Square(0.5f));
            if(GloveIsWithBody)
            {
                Glove->MoveGroupCount = 0;
                Glove->MoveQueueIndex = 0;
                Glove->MoveQueueCount = 0;
            }
        }
                
        if(Attacked)
        {
#if 0
            move_pattern AttackPattern =
            {
                3,
                {
                    {MoveSquare_A, {1, 0}, 0},
                    {MoveSquare_B, {1, 0}, MoveFlag_Interruptible},
                    {MoveSquare_C, {0, 1}, MoveFlag_Interruptible}
                },
            };
#else
            move_pattern AttackPattern =
            {
                1,
                {
                    {MoveSquare_A, {1, 0}, 0},
                },
            };
#endif
            
            if(Glove->MoveGroupCount < Glove->Stats.MaxMoveGroupCount)
            {
                ++Glove->MoveGroupCount;
                if((Glove->MoveQueueCount + AttackPattern.MoveCount) <= ArrayCount(Glove->MoveQueue))
                {
                    for(u32 MoveIndex = 0; MoveIndex < AttackPattern.MoveCount; ++MoveIndex)
                    {
                        move_pattern_entry Source = AttackPattern.Moves[MoveIndex];
                        move_queue_entry *Dest = Glove->MoveQueue + Glove->MoveQueueCount++;
                        
                        Dest->Delta = Source.CanonicalDelta.x*AttackX + Source.CanonicalDelta.y*AttackY;
                        Dest->Damage = Glove->Stats.Damage[Source.Type];
                        Dest->Speed = Glove->Stats.Speed[Source.Type];
                        Dest->Flags = Source.Flags;
                    }
                }
                else
                {
                    Assert(!"Move queue overflow");
                }
            }
        }

        // TODO(casey): Set a return move when there's no moves in the queue?
        // Or where does the movement code live for that?
        
#if 0
        f32 GloveFloatHeight = 0.5f;
        if(Attacked)
        {
            sound_id BloopID = GetBestMatchSoundFrom(GameState->Assets, GetTagHash(Asset_Audio, Tag_Bloop));
            PlaySound(&GameState->AudioState, BloopID);
        }
#endif
    }
    
    if(Exited)
    {
        DeleteEntity(SimRegion, Head);
        DeleteEntity(SimRegion, Body);
        ConHero->BrainID.Value = 0;
    }
}

internal void
ExecuteBrainSwitches(game_state *GameState, sim_region *SimRegion, brain *Brain, r32 dt)
{
    brain_switches *Switches = &Brain->Switches;

    b32 SwitchesSet = true;
    for(u32 TileIndex = 0;
            TileIndex < ArrayCount(Switches->Tiles);
            ++TileIndex)
    {
        entity *Tile = Switches->Tiles[TileIndex];
        if(Tile)
        {
            b32 Occupied = !TileCanBeOccupied(SimRegion, Tile->TileIndex);
            if(Tile->WasOccupiedLastCheck != Occupied)
            {
                if(Occupied)
                {
                    Tile->SwitchState = 1;
                    Tile->Pieces[0].Color = V4(0, 1, 1, 1);
                    Tile->Pieces[0].Cube.Emission = 25;
                    Tile->Pieces[1].Color = V4(0, 1, 1, 1);
                    Tile->Pieces[1].Cube.Emission = 25;
                }

                // TODO(casey): This is wrong anyway, so why did I bother porting it?
                if(Occupied && !Tile->WasOccupiedLastCheck)
                {
                    Tile->P.z -= 0.25f;
                }

                if(!Occupied && Tile->WasOccupiedLastCheck)
                {
                    Tile->P.z += 0.25f;
                }

                // TODO(casey): Should this happen outside
                // in a final sweep, or... ?
                Tile->WasOccupiedLastCheck = Occupied;
            }

            if(Tile->SwitchState == 0)
            {
                SwitchesSet = false;
            }
        }
    }

    if(SwitchesSet)
    {
        for(u32 UnlockIndex = 0;
            UnlockIndex < ArrayCount(Switches->Unlocks);
            ++UnlockIndex)
        {
            entity *Unlock = Switches->Unlocks[UnlockIndex];
            if(Unlock && (Unlock->Flags & EntityFlag_OccupiesTile))
            {
                Unlock->Flags &= ~EntityFlag_OccupiesTile;
                Unlock->P.z -= 2.0f;
            }
        }
    }
}

internal void
ExecuteBrainSnake(sim_region *SimRegion, random_series *Entropy, brain *Brain, r32 dt)
{
    brain_snake *Parts = &Brain->Snake;

    entity *Head = Parts->Segments[0];
    if(Head)
    {
        v3 Delta = {RandomBilateral(Entropy), RandomBilateral(Entropy), 0.0f};
        v2s dTileIndex = SingleTileDeltaFrom(Delta);
        
        if(Head->Animation == Animation_Planted)
        {
            Head->CameFrom = Head->TileIndex;
            if(TransactionalOccupy(SimRegion, Head->ID, &Head->TileIndex, Head->TileIndex + dTileIndex))
            {
                Head->FacingDirection = GetFacingDirectionFromSingleTileDelta(dTileIndex);
                
                SetAnimation(Head, Animation_Hopping, 4);
                
                v2s LastOccupying = Head->CameFrom;
                for(u32 SegmentIndex = 1;
                    SegmentIndex < ArrayCount(Parts->Segments);
                    ++SegmentIndex)
                {
                    entity *Segment = Parts->Segments[SegmentIndex];
                    if(Segment)
                    {
                        v2s SegDelta = LastOccupying - Segment->TileIndex;
                        Segment->CameFrom = Segment->TileIndex;
                        TransactionalOccupy(SimRegion, Segment->ID, &Segment->TileIndex, LastOccupying);
                        LastOccupying = Segment->CameFrom;
                        
                        Segment->FacingDirection = GetFacingDirectionFromSingleTileDelta(SegDelta);
                        SetAnimation(Segment, Animation_Hopping, 4);
                    }
                }
            }
        }
    }
}

inline void
ExecuteBrain(game_state *GameState, random_series *Entropy, game_input *Input,
             sim_region *SimRegion, brain *Brain, r32 dt)
{
    switch(Brain->Type)
    {
        case Type_brain_hero:
        {
            ExecuteBrainHero(GameState, Input, SimRegion, Brain, dt);
        } break;

        case Type_brain_snake:
        {
            ExecuteBrainSnake(SimRegion, Entropy, Brain, dt);
        } break;

        case Type_brain_familiar:
        {
            brain_familiar *Parts = &Brain->Familiar;
            entity *Head = Parts->Head;
            if(Head)
            {
                Head->Animation = Animation_Floating;

                closest_entity Closest = GetClosestEntityWithBrain(SimRegion, Head->P, Type_brain_hero);
                if(Closest.Entity)
                {
                    v2s TileDelta = (Closest.Entity->TileIndex - Head->TileIndex);
                    v2s SingleDelta = SingleTileDeltaFrom(V2(TileDelta));
                    v2s TargetSquare = Head->TileIndex + SingleDelta;
                    TransactionalOccupy(SimRegion, Head->ID, &Head->TileIndex, TargetSquare);
                }
            }
        } break;

        case Type_brain_switches:
        {
            ExecuteBrainSwitches(GameState, SimRegion, Brain, dt);
        } break;

        InvalidDefaultCase;
    }
}