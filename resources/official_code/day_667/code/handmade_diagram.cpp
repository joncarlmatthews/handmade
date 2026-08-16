/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

#if HANDMADE_DIAGRAMS

#define HANDMADE_DIAGRAM_MAX_DEPTH 8

struct diagram_group
{
    memory_arena Arena;
    diagram_entry *First;
    diagram_entry *Last;

    b32 Captured;

    u64 Filter;
    b32 FilterPassed;

    u32 ResetCounter;
    f32 CaptureFrameCount;

    world_position LastOriginP;

    u32 DiagramFilterDepth;
    f32 DiagramFilterIndex[HANDMADE_DIAGRAM_MAX_DEPTH];

    char FormatBuffer[1024];
};

global diagram_group GlobalDiagramGroup;

internal b32x DIAGRAM_IsOn(void)
{
    b32x Result = !GlobalDiagramGroup.Captured && GlobalDiagramGroup.FilterPassed;
    return Result;
}

internal b32x DIAGRAM_IsCaptured(void)
{
    b32x Result = GlobalDiagramGroup.Captured;
    return Result;
}

internal void DIAGRAM_Capture(b32x State)
{
    GlobalDiagramGroup.Captured = State;
}

internal diagram_entry *AppendDiagramEntry(diagram_entry_type Type)
{
    diagram_group *Group = &GlobalDiagramGroup;

    diagram_entry *Entry = 0;
    if(DIAGRAM_IsOn())
    {
        Entry = PushStruct(&Group->Arena, diagram_entry);
        Entry->Type = Type;
        Group->Last = (Group->Last ? Group->Last->Next : Group->First) = Entry;
    }

    return Entry;
}

internal void DIAGRAM_SetFilter(u64 Value)
{
    GlobalDiagramGroup.Filter = Value;
}

internal void DIAGRAM_Filter(u64 Value)
{
    GlobalDiagramGroup.FilterPassed = (GlobalDiagramGroup.Filter == Value);
}

internal void DIAGRAM_Begin(void)
{
    AppendDiagramEntry(Diagram_Begin);
}

internal void DIAGRAM_End(void)
{
    AppendDiagramEntry(Diagram_End);
}

internal void DIAGRAM_Color(v3 Color)
{
    diagram_entry *Entry = AppendDiagramEntry(Diagram_Color);
    if(Entry)
    {
        Entry->Color = V4(Color, 1);
    }
}

internal void DIAGRAM_Color(f32 R, f32 G, f32 B)
{
    DIAGRAM_Color(V3(R, G, B));
}

internal void DIAGRAM_Thickness(f32 Thick)
{
    diagram_entry *Entry = AppendDiagramEntry(Diagram_Thick);
    if(Entry)
    {
        Entry->Thick = Thick;
    }
}

internal void DIAGRAM_Text(char *Format, ...)
{
    diagram_entry *Entry = AppendDiagramEntry(Diagram_Text);
    if(Entry)
    {
        diagram_group *Group = &GlobalDiagramGroup;

        va_list ArgList;

        // TODO(casey): If arenas supported rewind, you would push 1024, do the printf,
        // and then rewind back to where it actually used.  This copies for no reason
        // other than that the memory_arena code is janky.
        va_start(ArgList, Format);
        umm Result = FormatStringList(ArrayCount(Group->FormatBuffer), Group->FormatBuffer, Format, ArgList);
        va_end(ArgList);

        char *Data = (char *)PushCopy(&Group->Arena, Result, Group->FormatBuffer, NoClear());

        Entry->Text = BundleString(Result, Data);
    }
}

internal void DIAGRAM_Stick(void)
{
    AppendDiagramEntry(Diagram_Stick);
}

internal void DIAGRAM_Sphere(v3 Center, f32 Radius)
{
    diagram_entry *Entry = AppendDiagramEntry(Diagram_Sphere);
    if(Entry)
    {
        Entry->P[0] = Center;
        Entry->P[1] = V3(Radius, Radius, Radius);
    }
}

internal void DIAGRAM_Line(v3 From, v3 To)
{
    diagram_entry *Entry = AppendDiagramEntry(Diagram_Line);
    if(Entry)
    {
        Entry->P[0] = From;
        Entry->P[1] = To;
    }
}

internal void DIAGRAM_Box(rectangle3 Box)
{
    diagram_entry *Entry = AppendDiagramEntry(Diagram_Box);
    if(Entry)
    {
        Entry->P[0] = Box.Min;
        Entry->P[1] = Box.Max;
    }
}

internal void DIAGRAM_Point(v3 P)
{
    diagram_entry *Entry = AppendDiagramEntry(Diagram_Point);
    if(Entry)
    {
        Entry->P[0] = P;
    }
}

internal void DIAGRAM_Overlay(void)
{
    AppendDiagramEntry(Diagram_Overlay);
}

internal void DIAGRAM_SetOrigin(world *World, world_position OriginP)
{
    diagram_group *Group = &GlobalDiagramGroup;

    v3 Offset = Subtract(World, &Group->LastOriginP, &OriginP);

    if(Offset.x || Offset.y || Offset.z)
    {
        for(diagram_entry *Entry = Group->First;
            Entry;
            Entry = Entry->Next)
        {
            switch(Entry->Type)
            {
                case Diagram_Sphere:
                {
                    Entry->P[0] += Offset;
                } break;

                case Diagram_Line:
                case Diagram_Box:
                case Diagram_Point:
                {
                    Entry->P[0] += Offset;
                    Entry->P[1] += Offset;
                } break;

                default:
                {
                } break;
            }
        }

        Group->LastOriginP = OriginP;
    }
}


internal void RenderDiagrams(render_group *RenderGroup)
{
    diagram_group *Group = &GlobalDiagramGroup;

    diagram_attributes Sticky;
    Sticky.Color = {1, 1, 1, 1};
    Sticky.Thick = 0.01f;

    diagram_attributes Current = Sticky;
    buffer Text = {};

    b32 ShouldDraw = true;
    u32 DiagramDepth = 0;
    u32 DiagramIndex[HANDMADE_DIAGRAM_MAX_DEPTH] = {};
    b32 PassedFilter[HANDMADE_DIAGRAM_MAX_DEPTH] = {};

    for(diagram_entry *Entry = Group->First;
        Entry;
        Entry = Entry->Next)
    {
        switch(Entry->Type)
        {
            case Diagram_Begin:
            {
                if(DiagramDepth < ArrayCount(DiagramIndex))
                {
                    PassedFilter[DiagramDepth] = ShouldDraw;
                    if(DiagramDepth < Group->DiagramFilterDepth)
                    {
                        if(DiagramIndex[DiagramDepth] != (u32)Group->DiagramFilterIndex[DiagramDepth])
                        {
                            ShouldDraw = false;
                        }
                    }

                    ++DiagramIndex[DiagramDepth];
                }

                ++DiagramDepth;
            } break;

            case Diagram_End:
            {
                if(DiagramDepth < ArrayCount(DiagramIndex))
                {
                    DiagramIndex[DiagramDepth] = 0;
                }
                
                --DiagramDepth;
                if(DiagramDepth < ArrayCount(DiagramIndex))
                {
                    ShouldDraw = PassedFilter[DiagramDepth];
                }
            } break;

            case Diagram_Color:
            {
                Current.Color = Entry->Color;
            } break;

            case Diagram_Thick:
            {
                Current.Thick = Entry->Thick;
            } break;

            case Diagram_Text:
            {
                Text = Entry->Text;
            } break;

            case Diagram_Stick:
            {
                Sticky = Current;
            } break;

            case Diagram_Sphere:
            {
                if(ShouldDraw)
                {
                    v3 Center = Entry->P[0];
                    v3 Radius = Entry->P[1];

                    u32 StepCount = 8;
                    for(u32 StepIndex = 0;
                        StepIndex < StepCount;
                        ++StepIndex)
                    {
                        v2 Arm0 = Arm2(Tau32*(f32)StepIndex / (f32)StepCount);
                        v2 Arm1 = Arm2(Tau32*(f32)(StepIndex + 1) / (f32)StepCount);

                        PushLineSegment(RenderGroup,
                                        Center + V3(Radius.x*Arm0.x, Radius.y*Arm0.y, 0), Current.Color,
                                        Center + V3(Radius.x*Arm1.x, Radius.y*Arm1.y, 0), Current.Color,
                                        Current.Thick);
                        PushLineSegment(RenderGroup,
                                        Center + V3(Radius.x*Arm0.x, 0, Radius.z*Arm0.y), Current.Color,
                                        Center + V3(Radius.x*Arm1.x, 0, Radius.z*Arm1.y), Current.Color,
                                        Current.Thick);
                        PushLineSegment(RenderGroup,
                                        Center + V3(0, Radius.y*Arm0.x, Radius.z*Arm0.y), Current.Color,
                                        Center + V3(0, Radius.y*Arm1.x, Radius.z*Arm1.y), Current.Color,
                                        Current.Thick);

                    }
                }

                Current = Sticky;
            } break;

            case Diagram_Line:
            {
                if(ShouldDraw)
                {
                    PushLineSegment(RenderGroup, Entry->P[0], Current.Color, Entry->P[1], Current.Color, Current.Thick);
                }
                Current = Sticky;
            } break;

            case Diagram_Box:
            {
                if(ShouldDraw)
                {
                    PushVolumeOutline(RenderGroup, RectMinMax(Entry->P[0], Entry->P[1]), Current.Color, Current.Thick);
                }
                Current = Sticky;
            } break;

            case Diagram_Point:
            {
                if(ShouldDraw)
                {
                    v3 Radius = {Current.Thick, Current.Thick, Current.Thick};
                    PushCube(RenderGroup, RenderGroup->WhiteTexture, Entry->P[0], Radius, Current.Color);
                }
                Current = Sticky;
            } break;

            case Diagram_Overlay:
            {
                if(ShouldDraw)
                {
                }
            } break;
        }
    }
    
    Assert(DiagramDepth == 0);
}

internal void DIAGRAM_Reset()
{
    diagram_group *Group = &GlobalDiagramGroup;

    if(!Group->Captured)
    {
        if((f32)Group->ResetCounter++ >= Group->CaptureFrameCount)
        {
            Clear(&Group->Arena);
            Group->First = Group->Last = 0;

            Group->ResetCounter = 0;
        }
    }
}

#endif