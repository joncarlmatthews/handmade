/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

#if !defined(HANDMADE_DIAGRAMS)
#define HANDMADE_DIAGRAMS HANDMADE_INTERNAL
#endif

#if HANDMADE_DIAGRAMS
enum diagram_entry_type
{
    Diagram_Null,

    Diagram_Begin,
    Diagram_End,

    Diagram_Color,
    Diagram_Thick,
    Diagram_Text,
    Diagram_Stick,

    Diagram_Sphere,
    Diagram_Line,
    Diagram_Box,
    Diagram_Point,
    Diagram_Overlay,

    Diagram_Count,
};

struct diagram_entry
{
    diagram_entry *Next;
    union
    {
        v3 P[2];
        v4 Color;
        f32 Thick;
        buffer Text;
    };

    diagram_entry_type Type;
};

struct diagram_attributes
{
    v4 Color;
    f32 Thick;
};

internal b32x DIAGRAM_IsOn(void);

internal b32x DIAGRAM_IsCaptured(void);
internal void DIAGRAM_Capture(b32x State);

internal void DIAGRAM_SetFilter(u64 Value);
internal void DIAGRAM_Filter(u64 Value);

internal void DIAGRAM_Begin(void);
internal void DIAGRAM_End(void);

internal void DIAGRAM_Color(v3 Color);
internal void DIAGRAM_Color(f32 R, f32 G, f32 B);
internal void DIAGRAM_Thickness(f32 Thick);
internal void DIAGRAM_Text(char *Format, ...);
internal void DIAGRAM_Stick(void);

internal void DIAGRAM_Sphere(v3 Center, f32 Radius);
internal void DIAGRAM_Line(v3 From, v3 To);
internal void DIAGRAM_Box(rectangle3 Box);
internal void DIAGRAM_Point(v3 P);
internal void DIAGRAM_Overlay(void);

struct world;
struct world_position;
internal void DIAGRAM_SetOrigin(world *World, world_position OriginP);

internal void DIAGRAM_Reset(void);

#else
#define DIAGRAM_IsOn(...) 0
#define DIAGRAM_IsCaptured(...) 0
#define DIAGRAM_Capture(...)
#define DIAGRAM_SetFilter(...)
#define DIAGRAM_Filter(...)
#define DIAGRAM_Begin(...)
#define DIAGRAM_End(...)
#define DIAGRAM_Color(...)
#define DIAGRAM_Thickness(...)
#define DIAGRAM_Stick(...)
#define DIAGRAM_Text(...)
#define DIAGRAM_Sphere(...)
#define DIAGRAM_Line(...)
#define DIAGRAM_Box(...)
#define DIAGRAM_Point(...)
#define DIAGRAM_Overlay(...)
#define DIAGRAM_SetOrigin(...)
#define DIAGRAM_Reset(...)

#define RenderDiagrams(...)

#endif
