/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

// TODO(casey): Shadows should be handled specially, because they need their own
// pass technically as well...
#define ShadowAlpha 0.5f

inline rectangle3 MakeSimpleGroundedCollision(f32 DimX, f32 DimY, f32 DimZ, f32 OffsetZ = 0.0f);
internal entity_visible_piece *AddPiece(entity *Entity, asset_tag_hash TagHash, v3 Dim, v3 Offset, v4 Color, u32 Flags = 0);
internal entity_visible_piece *AddPiece(entity *Entity, asset_tag_hash TagHash, f32 Height, v3 Offset, v4 Color, u32 Flags = 0);
internal entity_visible_piece *AddPieceLight(entity *Entity, f32 Radius, v3 Offset, f32 Emission, v3 Color);

internal void GenerateApron(world_generator *Gen, gen_apron *Apron);
internal void GenerateRoom(world_generator *Gen, gen_room_spec Spec, rectangle2i RoomVol);
internal void GenerateBlock(world_generator *Gen, rectangle2i BlockVolume);

internal f32 GetCameraOffsetZForCloseup(void);
