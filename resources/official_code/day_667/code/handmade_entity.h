/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

struct entity;

struct entity_id
{
    u32 Value;
};
inline b32x AreEqual(entity_id A, entity_id B)
{
    b32x Result = (A.Value == B.Value);

    return(Result);
}
inline void Clear(entity_id *A)
{
    A->Value = 0;
}
internal b32x IsValid(entity_id A)
{
    b32x Result = (A.Value != 0);
    return(Result);
}

#define HIT_POINT_SUBDIVISIONS 4
struct hit_point
{
    // TODO(casey): Bake this down into one variable
    uint8 Flags;
    uint8 FilledAmount;
};

enum entity_flags
{
    EntityFlag_Deleted = (1 << 0),
    EntityFlag_Active = (1 << 1),
    EntityFlag_AppliesCollision = (1 << 2),
    EntityFlag_ReceivesCollision = (1 << 3),
    EntityFlag_OccupiesTile = (1 << 4),
    EntityFlag_Unpacked = (1 << 5),
    EntityFlag_SupportsOccupation = (1 << 6),
};

enum entity_animation
{
    Animation_Planted,
    Animation_Hopping,
    Animation_Floating,
};

struct bitmap_piece
{
    // NOTE(casey): Parents must ALWAYS come before children
    u8 ParentPiece;
    enum8(hha_align_point_type) ParentAlignType;
    enum8(hha_align_point_type) ChildAlignType;
    u8 Reserved;
};

struct cube_piece
{
    cube_uv_layout UVLayout;
    u8 Emission;
    u8 Reserved;
};

enum entity_visible_piece_flag
{
    PieceMove_AxesDeform = 0x1,
    PieceMove_BobOffset = 0x2,
    PieceType_Cube = 0x4,
    PieceType_Light = 0x8,
    PieceType_Occluder = 0x10,
};
struct entity_visible_piece
{
    asset_tag_hash Asset;

    v4 Color;
    v3 Offset;
    v3 Dim;
    f32 Variant;

    u32 Flags;

    union
    {
        bitmap_piece Bitmap;
        cube_piece Cube;
    };
};
internal b32x
IsBitmap(entity_visible_piece *Piece)
{
    b32x Result = !(Piece->Flags & (PieceType_Cube | PieceType_Light));
    return(Result);
}

enum camera_behavior
{
    Camera_Inspect = 0x1,
    Camera_Offset = 0x2,
    Camera_ViewPlayerX = 0x4,
    Camera_ViewPlayerY = 0x8,
    Camera_GeneralVelocityConstraint = 0x10,
    Camera_DirectionalVelocityConstraint = 0x20,
};

struct ground_cover
{
    bitmap_id Bitmap;

#if 0
    v3 P;
    v3 Color;
    f32 Scale;
#else
    u32 Color;
    v3 N;
    v2 UV;
    v3 P[4];
#endif
};

enum ground_cover_type
{
    CoverType_None,

    CoverType_Rocks,
    CoverType_Flowers,
    CoverType_ThickGrass,
    CoverType_SplotchGrass,

    CoverType_Count,
};

struct ground_cover_spec
{
    u8 Type;
    u8 Density;
    u8 ReservedA;
    u8 ReservedB;
};

enum move_square_type
{
    MoveSquare_A,
    MoveSquare_B,
    MoveSquare_C,

    MoveSquare_Count,
};

struct stats
{
    f32 Damage[MoveSquare_Count];
    f32 Speed[MoveSquare_Count];
    
    // NOTE(casey): How many moves can queue back-to-back without returning
    u32 MaxMoveGroupCount;
};

enum move_queue_flag
{
    MoveFlag_Interruptible = 0x1,
    MoveFlag_Ephemeral = 0x2,
};

struct move_pattern_entry
{
    move_square_type Type;
    v2s CanonicalDelta;
    flag32(move_queue_flag) Flags;
};

struct move_pattern
{
    u32 MoveCount;
    move_pattern_entry Moves[8];
};

struct move_queue_entry
{
    v2s Delta;
    f32 Damage;
    f32 Speed;
    flag32(move_queue_flag) Flags;
};

struct entity_collider_group
{
    entity_id IDs[4];
};

#define ENTITY_MAX_GROUND_COVER 128
#define ENTITY_MAX_PIECE_COUNT 4 // TODO(casey): This should go away once we're packing
struct entity
{
    //
    // NOTE(casey): Things we've thought about
    //

    entity_id ID;
    brain_slot BrainSlot;
    brain_id BrainID;

    u32 CameraBehavior;
    f32 CameraMinVelocity;
    f32 CameraMaxVelocity;
    f32 CameraMinTime;
    v3 CameraOffset;
    v3 CameraVelocityDirection;

    //
    // NOTE(casey): Everything below here is NOT worked out yet
    //

    u32 SwitchProperties;
    u32 SwitchState;

    u32 Flags;

    v2s TileIndex;
    v3 P;
    v3 dP;

    // r32 DistanceLimit;

    rectangle3 CollisionVolume;

    f32 FacingDirection;
    f32 tBob;
    f32 dtBob;

    // TODO(casey): Should hitpoints themselves be entities?
    u32 HitPointMax;
    hit_point HitPoint[16];

    // TODO(casey): Only for stairwells!
    v2 WalkableDim;
    f32 WalkableHeight;

    // TODO(casey): Shouldn't this be in the "non-stored" portion?
    // {
    entity_animation Animation;
    f32 tMovement;
    f32 dtMovement;
    // }
    v2s CameFrom;

    v3 AngleBase;
    f32 AngleCurrent;
    f32 AngleStart;
    f32 AngleTarget;
    f32 AngleCurrentDistance;
    f32 AngleBaseDistance;
    f32 AngleSwipeDistance;

    v2 XAxis;
    v2 YAxis;

    v2 FloorDisplace;

    b32 WasOccupiedLastCheck;
                
    u32 PieceCount;
    entity_visible_piece Pieces[ENTITY_MAX_PIECE_COUNT];

    ground_cover_spec GroundCoverSpecs[4];

    stats BaseStats;
    
    //
    // NOTE(casey): Everything below this line isn't actually stored in long-term storage
    //

    u64 DiscardEverythingAfter;

    r32 ddtBob;

    u32 GroundCoverCount;
    ground_cover GroundCover[ENTITY_MAX_GROUND_COVER];

    u32 MoveQueueIndex;
    u32 MoveQueueCount;
    u32 MoveGroupCount;
    move_queue_entry MoveQueue[16];
    
    stats Stats;
    stats StatsAccum;
    
    entity_collider_group LastFrameColliders;
};

#define InvalidP V3(100000.0f, 100000.0f, 100000.0f)

inline bool32
IsSet(entity *Entity, uint32 Flag)
{
    bool32 Result = Entity->Flags & Flag;

    return(Result);
}

inline b32 IsDeleted(entity *E) {b32 Result = IsSet(E, EntityFlag_Deleted); return(Result);}

inline void
AddFlags(entity *Entity, uint32 Flag)
{
    Entity->Flags |= Flag;
}

inline void
ClearFlags(entity *Entity, uint32 Flag)
{
    Entity->Flags &= ~Flag;
}

inline void
MakeEntitySpatial(entity *Entity, v3 P, v3 dP)
{
    Entity->P = P;
    Entity->dP = dP;
}

inline real32
GetStairGround(entity *Entity, v3 AtGroundPoint)
{
    rectangle2 RegionRect = RectCenterDim(Entity->P.xy, Entity->WalkableDim);
    v2 Bary = Clamp01(GetBarycentric(RegionRect, AtGroundPoint.xy));
    real32 Result = Entity->P.z + Bary.y*Entity->WalkableHeight;

    return(Result);
}

internal void AdvanceEntityStats(entity *Entity);
