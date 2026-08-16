/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

// TODO(casey): Stop using stdio!
//#include <stdio.h>
//#include <stdlib.h>

#include "handmade_debug.h"

struct debug_parsed_name
{
    u32 HashValue;
    u32 FileNameCount;
    u32 LineNumber;
    
    u32 NameLength;
    char *Name;
};
inline debug_parsed_name
DebugParseName(char *GUID, char *ProperName)
{
    debug_parsed_name Result = {};
    
    u32 PipeCount = 0;
    u32 NameStartsAt = 0;
    char *Scan = GUID;
    for(;
        *Scan;
        ++Scan)
    {
        if(*Scan == '|')
        {
            if(PipeCount == 0)
            {
                Result.FileNameCount = (u32)(Scan - GUID);
                Result.LineNumber = S32FromZ(Scan + 1);
            }
            else if(PipeCount == 1)
            {
                // TODO(casey): Why was this line left blank?
            }
            else
            {
                NameStartsAt = (u32)(Scan - GUID + 1);
            }
            
            ++PipeCount;
        }
        
        UpdateStringHash(&Result.HashValue, *Scan);
    }
    
#if 0
    // NOTE(casey): This is how it would work if GCC/Clang supported __FUNCTION__
    Result.NameLength = (u32)(Scan - GUID) - Result.NameStartsAt;
    Result.Name = GUID + Result.NameStartsAt;
#else
    Result.NameLength = StringLength(ProperName);
    Result.Name = ProperName;
#endif
    
    return(Result);
}

inline debug_element *
GetElementFromGUID(debug_state *DebugState, u32 Index, char *GUID)
{
    debug_element *Result = 0;
    
    for(debug_element *Chain = DebugState->ElementHash[Index];
        Chain;
        Chain = Chain->NextInHash)
    {
        if(StringsAreEqual(Chain->GUID, GUID))
        {
            Result = Chain;
            break;
        }
    }
    
    return(Result);
}

inline debug_element *
GetElementFromGUID(debug_state *DebugState, char *GUID)
{
    debug_element *Result = 0;
    
    if(GUID)
    {
        debug_parsed_name ParsedName = DebugParseName(GUID, 0);
        u32 Index = (ParsedName.HashValue % ArrayCount(DebugState->ElementHash));
        
        Result = GetElementFromGUID(DebugState, Index, GUID);
    }
    
    return(Result);
}

inline dev_id
DebugIDFromLink(debug_tree *Tree, debug_variable_link *Link)
{
    dev_id Result = {};
    
    Result.Value[0].Void = Tree;
    Result.Value[1].Void = Link;
    
    return(Result);
}

inline dev_id
DebugIDFromGUID(debug_tree *Tree, char *GUID)
{
    dev_id Result = {};
    
    Result.Value[0].Void = Tree;
    Result.Value[1].Void = GUID;
    
    return(Result);
}

inline debug_state *
DEBUGGetState(game_memory *Memory)
{
    debug_state *DebugState = 0;
    if(Memory)
    {
        DebugState = Memory->DebugState;
    }
    
    return(DebugState);
}

inline debug_state *
DEBUGGetState(void)
{
    debug_state *Result = DEBUGGetState(DebugGlobalMemory);
    
    return(Result);
}

internal debug_tree *
AddTree(debug_state *DebugState, debug_variable_link *Group, v2 AtP)
{
    debug_tree *Tree = PushStruct(&DebugState->DebugArena, debug_tree);
    
    Tree->UIP = AtP;
    Tree->Group = Group;
    
    DLIST_INSERT(&DebugState->TreeSentinel, Tree);
    
    return(Tree);
}

inline void
BeginDebugStatistic(debug_statistic *Stat)
{
    Stat->Min = F32Max;
    Stat->Max = -F32Max;
    Stat->Sum = 0.0f;
    Stat->Count = 0;
}

inline void
AccumDebugStatistic(debug_statistic *Stat, r64 Value)
{
    ++Stat->Count;
    
    if(Stat->Min > Value)
    {
        Stat->Min = Value;
    }
    
    if(Stat->Max < Value)
    {
        Stat->Max = Value;
    }
    
    Stat->Sum += Value;
}

inline void
EndDebugStatistic(debug_statistic *Stat)
{
    if(Stat->Count)
    {
        Stat->Avg = Stat->Sum / (r64)Stat->Count;
    }
    else
    {
        Stat->Min = 0.0f;
        Stat->Max = 0.0f;
        Stat->Avg = 0.0f;
    }
}

internal memory_index
DEBUGEventToText(char *Buffer, char *End, debug_element *Element, debug_event *Event, u32 Flags)
{
    char *At = Buffer;
    
    if(Flags & DEBUGVarToText_AddDebugUI)
    {
        At += FormatString(End - At, At, "#define DEBUGUI_");
    }
    
    if(Flags & DEBUGVarToText_AddName)
    {
        char *UseName = (Flags & DEBUGVarToText_ShowEntireGUID) ? Event->GUID : Event->Name;
        At += FormatString(End - At, At, "%s%s ", UseName, (Flags & DEBUGVarToText_Colon) ? ":" : "");
    }
    
    if(Flags & DEBUGVarToText_AddValue)
    {
        switch(Event->Type)
        {
            case DebugType_r32:
            {
                At += FormatString(End - At, At, "%f", Event->Value_r32);
                if(Flags & DEBUGVarToText_FloatSuffix)
                {
                    *At++ = 'f';
                }
            } break;
            
            case DebugType_string_ptr:
            {
                At += FormatString(End - At, At, "%s", Event->Value_string_ptr);
            } break;
            
            case DebugType_b32:
            {
                if(Flags & DEBUGVarToText_PrettyBools)
                {
                    At += FormatString(End - At, At, "%s",
                                       Event->Value_b32 ? "true" : "false");
                }
                else
                {
                    At += FormatString(End - At, At, "%d", Event->Value_b32);
                }
            } break;
            
            case DebugType_s32:
            {
                At += FormatString(End - At, At, "%d", Event->Value_s32);
            } break;
            
            case DebugType_u32:
            {
                At += FormatString(End - At, At, "%u", Event->Value_u32);
            } break;
            
            case DebugType_umm:
            {
                At += FormatString(End - At, At, "%m", Event->Value_umm);
            } break;
            
            case DebugType_v2:
            {
                At += FormatString(End - At, At, "V2(%f, %f)", Event->Value_v2.x, Event->Value_v2.y);
            } break;
            
            case DebugType_v3:
            {
                At += FormatString(End - At, At, "V3(%f, %f, %f)",
                                   Event->Value_v3.x, Event->Value_v3.y, Event->Value_v3.z);
            } break;
            
            case DebugType_v4:
            {
                At += FormatString(End - At, At, "V4(%f, %f, %f, %f)",
                                   Event->Value_v4.x, Event->Value_v4.y,
                                   Event->Value_v4.z, Event->Value_v4.w);
            } break;
            
            case DebugType_rectangle2:
            {
                At += FormatString(End - At, At, "Rect2(%f, %f -> %f, %f)",
                                   Event->Value_rectangle2.Min.x,
                                   Event->Value_rectangle2.Min.y,
                                   Event->Value_rectangle2.Max.x,
                                   Event->Value_rectangle2.Max.y);
            } break;
            
            case DebugType_rectangle3:
            {
                At += FormatString(End - At, At, "Rect2(%f, %f, %f -> %f, %f, %f)",
                                   Event->Value_rectangle3.Min.x,
                                   Event->Value_rectangle3.Min.y,
                                   Event->Value_rectangle3.Min.z,
                                   Event->Value_rectangle3.Max.x,
                                   Event->Value_rectangle3.Max.y,
                                   Event->Value_rectangle3.Max.z);
            } break;
            
            case DebugType_bitmap_id:
            {
            } break;
            
            default:
            {
                At += FormatString(End - At, At, "UNHANDLED: %s", Event->GUID);
            } break;
        }
    }
    
    if(Flags & DEBUGVarToText_LineFeedEnd)
    {
        *At++ = '\n';
    }
    
    if(Flags & DEBUGVarToText_NullTerminator)
    {
        *At++ = 0;
    }
    
    return(At - Buffer);
}

internal char *
GetName(debug_state *DebugState, umm BufferSize, char *Buffer, debug_element *Element)
{
    char *Result = Element->Name;
    switch(Element->Type)
    {
        case DebugType_LastFrameInfo:
        {
            debug_platform_memory_stats MemStats = Platform.DEBUGGetMemoryStats();
            debug_frame *MostRecentFrame = DebugState->Frames + DebugState->ViewingFrameOrdinal;
            FormatString(BufferSize, Buffer,
                         "%.02fms %de %dp %dd - Mem: %lu blocks, %lu used",
                         MostRecentFrame->WallSecondsElapsed * 1000.0f,
                         MostRecentFrame->StoredEventCount,
                         MostRecentFrame->ProfileBlockCount,
                         MostRecentFrame->DataBlockCount,
                         MemStats.BlockCount,
                         MemStats.TotalUsed);
            
            Result = Buffer;
        } break;
        
        case DebugType_FunctionSummary:
        {
            debug_element *HUDFunction = 0;
            char *FunctionName = "[unknown]";
            
            debug_stored_event *OldestStoredEvent =
                Element->Frames[DebugState->ViewingFrameOrdinal].OldestEvent;
            if(OldestStoredEvent)
            {
                debug_event *Event = &OldestStoredEvent->Event;
                FunctionName = Event->Value_string_ptr;
                HUDFunction = GetElementFromGUID(DebugState, FunctionName);
            }
            
            if(HUDFunction)
            {
                u64 MinDuration = U64Max;
                u64 MaxDuration = 0;
                u64 CurDuration = 0;
                u64 TotalDuration = 0;
                u32 TotalCount = 0;
                for(u32 FrameIndex = 0;
                    FrameIndex < DEBUG_FRAME_COUNT;
                    ++FrameIndex)
                {
                    debug_element_frame *Frame = HUDFunction->Frames + FrameIndex;
                    u64 Duration = 0;
                    if(Frame->OldestEvent)
                    {
                        for(debug_stored_event *SubEvent = Frame->OldestEvent;
                            SubEvent;
                            SubEvent = SubEvent->Next)
                        {
                            debug_profile_node *Node = &SubEvent->ProfileNode;
                            Duration += Node->Duration;
                        }
                        
                        if(MinDuration > Duration)
                        {
                            MinDuration = Duration;
                        }
                        
                        if(MaxDuration < Duration)
                        {
                            MaxDuration = Duration;
                        }
                        
                        if(FrameIndex == DebugState->ViewingFrameOrdinal)
                        {
                            CurDuration = Duration;
                        }
                        
                        TotalDuration += Duration;
                        ++TotalCount;
                    }
                }
                
                u32 CurKilocycles = (u32)(CurDuration / 1000);
                u32 MinKilocycles = (u32)(MinDuration / 1000);
                u32 MaxKilocycles = (u32)(MaxDuration / 1000);
                u32 AvgKilocycles = (u32)SafeRatio0((f32)TotalDuration, (1000.0f * (f32)TotalCount));
                
                FormatString(BufferSize, Buffer,
                             "%s: %dkcy cur | %dkcy min | %dkcy avg | %dkcy max",
                             FunctionName, CurKilocycles, MinKilocycles, AvgKilocycles, MaxKilocycles);
            }
            else
            {
                FormatString(BufferSize, Buffer,
                             "(%s: function not found)",
                             FunctionName);
            }
            
            Result = Buffer;
        } break;
    }
    
    return(Result);
}

internal debug_view *
GetOrCreateDebugViewFor(debug_state *DebugState, dev_id ID)
{
    // TODO(casey): Better hash function
    u32 HashIndex = ((ID.Value[0].U32[0] >> 2) + (ID.Value[1].U32[0] >> 2)) % ArrayCount(DebugState->ViewHash);
    debug_view **HashSlot = DebugState->ViewHash + HashIndex;
    
    debug_view *Result = 0;
    for(debug_view *Search = *HashSlot;
        Search;
        Search = Search->NextInHash)
    {
        if(AreEqual(Search->ID, ID))
        {
            Result = Search;
            break;
        }
    }
    
    if(!Result)
    {
        Result = PushStruct(&DebugState->DebugArena, debug_view);
        Result->ID = ID;
        Result->Type = DebugViewType_Unknown;
        Result->NextInHash = *HashSlot;
        *HashSlot = Result;
    }
    
    return(Result);
}

internal void
SetExpand(debug_state *DebugState, dev_id DevID, b32x Expanded)
{
    debug_view *View = GetOrCreateDebugViewFor(DebugState, DevID);
    View->Collapsible.ExpandedAlways = Expanded;
}

inline dev_ui_interaction
ElementInteraction(debug_state *DebugState, dev_id DebugID, dev_ui_interaction_type Type, debug_element *Element)
{
    dev_ui_interaction ItemInteraction = {};
    ItemInteraction.ID = DebugID;
    ItemInteraction.Type = Type;
    ItemInteraction.Element = Element;
    
    return(ItemInteraction);
}

inline dev_ui_interaction
DebugIDInteraction(dev_ui_interaction_type Type, dev_id ID)
{
    dev_ui_interaction ItemInteraction = {};
    ItemInteraction.ID = ID;
    ItemInteraction.Type = Type;
    
    return(ItemInteraction);
}

inline dev_ui_interaction
DebugLinkInteraction(dev_ui_interaction_type Type, debug_variable_link *Link)
{
    dev_ui_interaction ItemInteraction = {};
    ItemInteraction.Link = Link;
    ItemInteraction.Type = Type;
    
    return(ItemInteraction);
}

internal b32
IsSelected(debug_state *DebugState, dev_id ID)
{
    b32 Result = false;
    
    for(u32 Index = 0;
        Index < DebugState->SelectedIDCount;
        ++Index)
    {
        if(AreEqual(ID, DebugState->SelectedID[Index]))
        {
            Result = true;
            break;
        }
    }
    
    return(Result);
}

internal void
ClearSelection(debug_state *DebugState)
{
    DebugState->SelectedIDCount = 0;
}

internal void
AddToSelection(debug_state *DebugState, dev_id ID)
{
    if((DebugState->SelectedIDCount < ArrayCount(DebugState->SelectedID)) &&
       !IsSelected(DebugState, ID))
    {
        DebugState->SelectedID[DebugState->SelectedIDCount++] = ID;
    }
}

#if !defined(DEBUG_HIT)
internal void
DEBUG_HIT(dev_id ID, r32 ZValue)
{
    debug_state *DebugState = DEBUGGetState();
    if(DebugState)
    {
        DebugState->DevUI.NextHotInteraction =
            DebugIDInteraction(DevUI_Interaction_Select, ID);
    }
}
#endif

#if !defined(DEBUG_HIGHLIGHTED)
internal b32
DEBUG_HIGHLIGHTED(dev_id ID, v4 *Color)
{
    b32 Result = false;
    
    debug_state *DebugState = DEBUGGetState();
    if(DebugState)
    {
        if(IsSelected(DebugState, ID))
        {
            *Color = V4(0, 1, 1, 1);
            Result = true;
        }
        
        if(AreEqual(DebugState->DevUI.HotInteraction.ID, ID))
        {
            *Color = V4(1, 1, 0, 1);
            Result = true;
        }
    }
    
    return(Result);
}
#endif

#if !defined(DEBUG_REQUESTED)
internal b32
DEBUG_REQUESTED(dev_id ID)
{
    b32 Result = false;
    
    debug_state *DebugState = DEBUGGetState();
    if(DebugState)
    {
        Result = IsSelected(DebugState, ID)
            || AreEqual(DebugState->DevUI.HotInteraction.ID, ID);
    }
    
    return(Result);
}
#endif

internal u64
GetTotalClocks(debug_element_frame *Frame)
{
    u64 Result = 0;
    for(debug_stored_event *Event = Frame->OldestEvent;
        Event;
        Event = Event->Next)
    {
        Result += Event->ProfileNode.Duration;
    }
    return(Result);
}

internal void
DrawProfileBars(debug_state *DebugState, dev_id GraphID, rectangle2 ProfileRect, v2 MouseP,
                debug_profile_node *RootNode, r32 LaneStride, r32 LaneHeight, u32 DepthRemaining)
{
    dev_ui *UI = &DebugState->DevUI;
    render_group *RenderGroup = &UI->RenderGroup;
    
    r32 FrameSpan = (r32)(RootNode->Duration);
    r32 PixelSpan = GetDim(ProfileRect).x;
    
    r32 BaseZ = 100.0f - 10.0f*(r32)DepthRemaining;
    
    r32 Scale = 0.0f;
    if(FrameSpan > 0)
    {
        Scale = PixelSpan / FrameSpan;
    }
    
    for(debug_stored_event *StoredEvent = RootNode->FirstChild;
        StoredEvent;
        StoredEvent = StoredEvent->ProfileNode.NextSameParent)
    {
        debug_profile_node *Node = &StoredEvent->ProfileNode;
        debug_element *Element = Node->Element;
        Assert(Element);
        
        v3 Color = DebugColorTable[U32FromPointer(Element->GUID)%ArrayCount(DebugColorTable)];
        r32 ThisMinX = ProfileRect.Min.x + Scale*(r32)(Node->ParentRelativeClock);
        r32 ThisMaxX = ThisMinX + Scale*(r32)(Node->Duration);
        
        u32 LaneIndex = Node->ThreadOrdinal;
        r32 LaneY = ProfileRect.Max.y - LaneStride*LaneIndex;
        rectangle2 RegionRect = RectMinMax(V2(ThisMinX, LaneY - LaneHeight),
                                           V2(ThisMaxX, LaneY));
        
        PushRect(RenderGroup, RegionRect, UI->UITransform + V3(0, 0, BaseZ), V4(Color, 1));
        PushRectOutline(RenderGroup, RegionRect,
                        UI->UITransform + V3(0, 0, BaseZ+1.0f), V4(0,0,0, 1), 2.0f);
        
        // TODO(casey): Pull this out so all profilers share it.
        if(IsInRectangle(RegionRect, MouseP))
        {
            char Temp[256];
            
            tooltip_buffer TextBuffer = AddLine(&UI->ToolTips);
            FormatString(TextBuffer.Size, TextBuffer.Data, "%s|%s: %10ucy",
                         Element->GUID, GetName(DebugState, sizeof(Temp), Temp, Element), Node->Duration);
            
            // TODO(casey): It would be better to generate a graph+element debug ID here!
            debug_view *View = GetOrCreateDebugViewFor(DebugState, GraphID);
            UI->NextHotInteraction =
                SetPointerInteraction(GraphID, (void **)&View->ProfileGraph.GUID, Element->GUID);
        }
        
        if(DepthRemaining > 0)
        {
            DrawProfileBars(DebugState, GraphID, RegionRect, MouseP, Node, 0, LaneHeight/2,
                            DepthRemaining - 1);
        }
    }
}

internal b32x
DrawMemoryRange(debug_state *DebugState, rectangle2 ProfileRect, u64 StartingAddress, u64 OnePastLastAddress, v4 BlockColor, v2 MouseP)
{
    b32x Result = false;
    
    if(StartingAddress != OnePastLastAddress)
    {
        dev_ui *UI = &DebugState->DevUI;
        render_group *RenderGroup = &UI->RenderGroup;
        
        u64 BytesPerRow = Megabytes(8);
        u64 MinRowIndex = StartingAddress / BytesPerRow;
        u64 MaxRowIndex = (OnePastLastAddress - 1) / BytesPerRow;
        f32 RowHeight = 15.0f;
        
        for(u64 RowIndex = MinRowIndex;
            RowIndex <= MaxRowIndex;
            ++RowIndex)
        {
            rectangle2 Rect = {};
            
            u64 RowStartingAddress = RowIndex*BytesPerRow;
            
            Rect.Max.y = ProfileRect.Max.y - (f32)RowIndex * RowHeight;
            Rect.Min.y = Rect.Max.y - RowHeight;
            
            f32 tMin = ((RowIndex == MinRowIndex) ? 
                        (f32)(StartingAddress - RowStartingAddress) / (f32)BytesPerRow : 
                        0.0f);
            f32 tMax = ((RowIndex == MaxRowIndex) ?
                        (f32)(OnePastLastAddress - RowStartingAddress) / (f32)BytesPerRow :
                        1.0f);
            
            Rect.Min.x = Lerp(ProfileRect.Min.x, tMin, ProfileRect.Max.x);
            Rect.Max.x = Lerp(ProfileRect.Min.x, tMax, ProfileRect.Max.x);
            
            PushRect(RenderGroup, Rect, UI->UITransform, BlockColor);
            PushRectOutline(RenderGroup, Rect,
                            UI->UITransform + V3(0, 0, 1.0f), V4(0,0,0,1), 2.0f);
            if(IsInRectangle(Rect, MouseP))
            {
                Result = true;
            }
        }
    }
    
    return(Result);
    
    /*
    
                        f32 tMin = InvSizeAllocated*(f32)(Alloc->OffsetFromBlock);
                        f32 tMax =
                            InvSizeAllocated*(f32)
                            (Alloc->OffsetFromBlock + Alloc->SizeAllocated);
                            
                        v2 MinP = V2(Lerp(BlockRect.Min.x, tMin, BlockRect.Max.x),
                                     BlockRect.Min.y);
                        v2 MaxP = V2(Lerp(BlockRect.Min.x, tMax, BlockRect.Max.x),
                                     BlockRect.Max.y);;
                        rectangle2 RegionRect = RectMinMax(MinP, MaxP);
                        
                        PushRect(RenderGroup, RegionRect, UI->UITransform, V4(Color, 1));
                        PushRectOutline(RenderGroup, RegionRect,
                                        UI->UITransform + V3(0, 0, 1.0f), V4(0,0,0,1), 2.0f);
                                        
                                                                */
}

internal debug_arena_call_site *
GetCallSite(debug_state *DebugState, u32 SiteIndex)
{
    Assert(SiteIndex < DebugState->ArenaCallSiteCount);
    debug_arena_call_site *Result = DebugState->ArenaCallSites + SiteIndex;
    return(Result);
}

internal debug_arena_call_site *
GetCallSiteFor(debug_state *DebugState, debug_arena_allocation *Alloc)
{
    debug_arena_call_site *Result = GetCallSite(DebugState, Alloc->CallSiteIndex);
    return(Result);
}

internal void
DrawArenaInterval(debug_state *DebugState, dev_id GraphID, rectangle2 ProfileRect, v2 MouseP, debug_view_arena_graph *Graph)
{
    dev_ui *UI = &DebugState->DevUI;
    render_group *RenderGroup = &UI->RenderGroup;
    
    u32 AllocIndex = 0;
    u64 BlockStartAddress = 0;
    for(debug_arena *Arena = DebugState->FirstArena;
        Arena;
        Arena = Arena->Next)
    {
        if(!Arena->Suppress || Graph->ShowSuppressed)
        {
            for(debug_arena_block *Block = Arena->FirstBlock;
                Block;
                Block = Block->Next)
            {
                u64 BlockEndAddress = BlockStartAddress + (u64)Block->SizeAllocated;
                v4 BlockColor = (Arena->Suppress ? V4(0.15f, 0.15f, 0.15f, 1) : V4(0.5f, 0.5f, 0.5f, 1));
                
                DrawMemoryRange(DebugState, ProfileRect, BlockStartAddress, BlockEndAddress, BlockColor, MouseP);
                
                if(!Arena->Suppress)
                {
                    f32 InvSizeAllocated = SafeRatio1(1.0f, (f32)Block->SizeAllocated);
                    for(debug_arena_allocation *Alloc = Block->FirstAllocation;
                        Alloc;
                        Alloc = Alloc->Next, ++AllocIndex)
                    {
                        // TODO(casey): Maybe instead of coloring things differently based
                        // on allocation index, we could color them based on the line of
                        // code that caused the allocation?
                        v3 AllocColor = DebugColorTable[Alloc->CallSiteIndex%ArrayCount(DebugColorTable)];
                        
                        u64 AllocStartAddress = BlockStartAddress + Alloc->OffsetFromBlock;
                        u64 AllocEndAddress = AllocStartAddress + Alloc->SizeAllocated;
                        
                        if(DrawMemoryRange(DebugState, ProfileRect, AllocStartAddress, AllocEndAddress, V4(AllocColor, 1.0f), MouseP))
                        {
                            debug_arena_call_site *Site = GetCallSiteFor(DebugState, Alloc);
                            tooltip_buffer TextBuffer = AddLine(&UI->ToolTips);
                            FormatString(TextBuffer.Size, TextBuffer.Data, "%s|%s: %ub",
                                         Arena->Name, Site->GUID, Alloc->SizeAllocated);
                        }
                    }
                }
                
                BlockStartAddress = BlockEndAddress;
            }
        }
    }
}

internal debug_arena_call_site *
GetCallSiteFromIndex(debug_state *DebugState, u32 Index)
{
    debug_arena_call_site *Result = 0;
    if(Index < DebugState->ArenaCallSiteCount)
    {
        Result = DebugState->ArenaCallSites + Index;
    }
    
    return(Result);
}

internal void
DrawTopMemList(debug_state *DebugState, dev_id GraphID, rectangle2 ProfileRect, v2 MouseP, debug_view_arena_graph *Graph)
{
    dev_ui *UI = &DebugState->DevUI;
    temporary_memory Temp = BeginTemporaryMemory(&DebugState->DebugArena);
    
    u32 ArenaCount = 0;
    for(debug_arena *Arena = DebugState->FirstArena;
        Arena;
        Arena = Arena->Next)
    {
        ++ArenaCount;
    }
    
    debug_statistic *CallSiteStats = PushArray(Temp.Arena, DebugState->ArenaCallSiteCount, debug_statistic);
    
    debug_memory_entry *Entries = PushArray(Temp.Arena, ArenaCount, debug_memory_entry, NoClear());
    sort_entry *SortA = PushArray(Temp.Arena, ArenaCount, sort_entry, NoClear());
    sort_entry *SortB = PushArray(Temp.Arena, ArenaCount, sort_entry, NoClear());
    
    f64 TotalAllocated = 0.0;
    f64 TotalUsed = 0.0;
    u32 Index = 0;
    for(debug_arena *Arena = DebugState->FirstArena;
        Arena;
        Arena = Arena->Next)
    {
        if(!Arena->Suppress || Graph->ShowSuppressed)
        {
            debug_memory_entry *Entry = Entries + Index;
            sort_entry *Sort = SortA + Index;
            
            Entry->Arena = Arena;
            
            BeginDebugStatistic(&Entry->Allocated);
            BeginDebugStatistic(&Entry->Used);
            for(debug_arena_block *Block = Arena->FirstBlock;
                Block;
                Block = Block->Next)
            {
                for(debug_arena_allocation *Alloc = Block->FirstAllocation;
                    Alloc;
                    Alloc = Alloc->Next)
                {
                    AccumDebugStatistic(&CallSiteStats[Alloc->CallSiteIndex], (f64)Alloc->SizeAllocated);
                    AccumDebugStatistic(&Entry->Used, (f64)Alloc->SizeAllocated);
                }
                AccumDebugStatistic(&Entry->Allocated, (f64)Block->SizeAllocated);
            }
            EndDebugStatistic(&Entry->Allocated);
            EndDebugStatistic(&Entry->Used);
            TotalAllocated += Entry->Allocated.Sum;
            TotalUsed += Entry->Used.Sum;
            
            Sort->SortKey = -(f32)Entry->Allocated.Sum;
            Sort->Index = Index++;
        }
    }
    ArenaCount = Index;
    
    RadixSort(ArenaCount, SortA, SortB);
    
    r64 PA = 0.0f;
    if(TotalAllocated > 0)
    {
        PA = 100.0f / TotalAllocated;
    }
    
    r64 RunningSum = 0.0f;
    
    char TextBuffer[256];
    v2 At = V2(ProfileRect.Min.x, ProfileRect.Max.y - GetBaseline(UI));
    FormatString(sizeof(TextBuffer), TextBuffer,
                 " cur   peak          blck     allc");
    
    TextOutAt(UI, At, TextBuffer);
    At.y -= GetLineAdvance(UI);
    
    for(Index = 0;
        (Index < ArenaCount);
        ++Index)
    {
        debug_memory_entry *Entry = Entries + SortA[Index].Index;
        debug_statistic *Allocated = &Entry->Allocated;
        debug_statistic *Used = &Entry->Used;
        debug_arena *Arena = Entry->Arena;
        
        RunningSum += Allocated->Sum;
        
        FormatString(sizeof(TextBuffer), TextBuffer,
                     "%6m %6m %05.02f%% %4d %8d %s",
                     (umm)(Allocated->Sum),
                     (umm)(Arena->PeakTotalAlloc),
                     (PA*Allocated->Sum),
                     Allocated->Count,
                     Used->Count,
                     Arena->Name);
        
        TextOutAt(UI, At, TextBuffer);
        rectangle2 TextRect = GetTextSize(UI, At, TextBuffer);
        
#if 0
        if(IsInRectangle(TextRect, MouseP))
        {
            tooltip_buffer ToolTipBuffer = AddLine(&UI->ToolTips);
            FormatString(ToolTipBuffer.Size, ToolTipBuffer.Data,
                         "  %05.02fcy each; cumulative to this point: %02.02f%%",
                         SafeRatio0((f64)Stats->Sum, (f64)Stats->Count),
                         (PC*RunningSum));
        }
#endif
        
        if(At.y < ProfileRect.Min.y)
        {
            break;
        }
        else
        {
            At.y -= GetLineAdvance(UI);
        }
        
        if(Graph->ShowCallSites)
        {
            for(u32 SlotIndex = 0;
                SlotIndex < DebugState->ArenaCallSiteCount;
                ++SlotIndex)
            {
                debug_statistic *Stats = CallSiteStats + SlotIndex;
                debug_arena_call_site *Site = GetCallSiteFromIndex(DebugState, SlotIndex);
                if(Site->Arena == Arena)
                {
                    FormatString(sizeof(TextBuffer), TextBuffer,
                                 "%6m %6m %05.02f%%      %8d    %s",
                                 (umm)Stats->Sum,
                                 (umm)Site->PeakTotalAlloc,
                                 (PA*Stats->Sum),
                                 Stats->Count,
                                 Site->GUID);
                    
                    TextOutAt(UI, At, TextBuffer);
                    
                    if(At.y < ProfileRect.Min.y)
                    {
                        break;
                    }
                    else
                    {
                        At.y -= GetLineAdvance(UI);
                    }
                }
            }
        }
    }
    
    EndTemporaryMemory(Temp);
    
#undef CALL_SITE_HASH_SIZE
}


internal void
DrawProfileIn(debug_state *DebugState, dev_id GraphID, rectangle2 ProfileRect, v2 MouseP,
              debug_element *RootElement)
{
    u32 LaneCount = DebugState->FrameBarLaneCount;
    r32 LaneHeight = 0.0f;
    if(LaneCount > 0)
    {
        LaneHeight = GetDim(ProfileRect).y / (r32)LaneCount;
    }
    
    debug_element_frame *RootFrame = RootElement->Frames + DebugState->ViewingFrameOrdinal;
    r32 NextX = ProfileRect.Min.x;
    u64 TotalClock = GetTotalClocks(RootFrame);
    u64 RelativeClock = 0;
    for(debug_stored_event *Event = RootFrame->OldestEvent;
        Event;
        Event = Event->Next)
    {
        debug_profile_node *Node = &Event->ProfileNode;
        rectangle2 EventRect = ProfileRect;
        
        RelativeClock += Node->Duration;
        r32 t = (r32)((r64)RelativeClock / (r64)TotalClock);
        EventRect.Min.x = NextX;
        EventRect.Max.x = (1.0f - t)*ProfileRect.Min.x + t*ProfileRect.Max.x;
        NextX = EventRect.Max.x;
        
        DrawProfileBars(DebugState, GraphID, EventRect, MouseP, Node, LaneHeight, LaneHeight, 1);
    }
}

internal void
DrawFrameBars(debug_state *DebugState, dev_id GraphID, rectangle2 ProfileRect, v2 MouseP,
              debug_element *RootElement)
{
    dev_ui *UI = &DebugState->DevUI;
    render_group *RenderGroup = &UI->RenderGroup;
    
    u32 FrameCount = ArrayCount(RootElement->Frames);
    if(FrameCount > 0)
    {
        r32 BarWidth = (GetDim(ProfileRect).x / (r32)FrameCount);
        r32 AtX = ProfileRect.Min.x;
        for(u32 FrameIndex = 0;
            FrameIndex < FrameCount;
            ++FrameIndex)
        {
            debug_stored_event *RootEvent = RootElement->Frames[FrameIndex].MostRecentEvent;
            if(RootEvent)
            {
                debug_profile_node *RootNode = &RootEvent->ProfileNode;
                r32 FrameSpan = (r32)(RootNode->Duration);
                r32 PixelSpan = GetDim(ProfileRect).y;
                r32 Scale = 0.0f;
                if(FrameSpan > 0)
                {
                    Scale = PixelSpan / FrameSpan;
                }
                
                b32 Highlight = (FrameIndex == DebugState->ViewingFrameOrdinal);
                r32 HighDim = Highlight ? 1.0f : 0.5f;
                
                for(debug_stored_event *StoredEvent = RootNode->FirstChild;
                    StoredEvent;
                    StoredEvent = StoredEvent->ProfileNode.NextSameParent)
                {
                    debug_profile_node *Node = &StoredEvent->ProfileNode;
                    debug_element *Element = Node->Element;
                    Assert(Element);
                    
                    v3 Color = DebugColorTable[U32FromPointer(Element->GUID)%ArrayCount(DebugColorTable)];
                    r32 ThisMinY = ProfileRect.Min.y + Scale*(r32)(Node->ParentRelativeClock);
                    r32 ThisMaxY = ThisMinY + Scale*(r32)(Node->Duration);
                    
                    rectangle2 RegionRect = RectMinMax(V2(AtX, ThisMinY), V2(AtX + BarWidth, ThisMaxY));
                    
                    PushRect(RenderGroup, RegionRect, UI->UITransform, V4(HighDim*Color, 1));
                    PushRectOutline(RenderGroup, RegionRect, UI->UITransform + V3(0, 0, 1.0f), V4(0, 0, 0, 1), 2.0f);
                    
                    if(IsInRectangle(RegionRect, MouseP))
                    {
                        char Temp[256];
                        
                        tooltip_buffer TextBuffer = AddLine(&UI->ToolTips);
                        FormatString(TextBuffer.Size, TextBuffer.Data, "%s|%s: %10ucy",
                                     Element->GUID, GetName(DebugState, sizeof(Temp), Temp, Element), Node->Duration);
                        
                        debug_view *View = GetOrCreateDebugViewFor(DebugState, GraphID);
                        UI->NextHotInteraction =
                            SetPointerInteraction(GraphID, (void **)&View->ProfileGraph.GUID, Element->GUID);
                    }
                }
                
                AtX += BarWidth;
            }
        }
    }
}

struct debug_clock_entry
{
    debug_element *Element;
    debug_statistic Stats;
};
internal void
DrawTopClocksList(debug_state *DebugState, dev_id GraphID, rectangle2 ProfileRect, v2 MouseP,
                  debug_element *RootElement)
{
    dev_ui *UI = &DebugState->DevUI;
    temporary_memory Temp = BeginTemporaryMemory(&DebugState->DebugArena);
    
    u32 LinkCount = 0;
    for(debug_variable_link *Link = GetSentinel(DebugState->ProfileGroup)->Next;
        Link != GetSentinel(DebugState->ProfileGroup);
        Link = Link->Next)
    {
        ++LinkCount;
    }
    
    debug_clock_entry *Entries = PushArray(Temp.Arena, LinkCount, debug_clock_entry, NoClear());
    sort_entry *SortA = PushArray(Temp.Arena, LinkCount, sort_entry, NoClear());
    sort_entry *SortB = PushArray(Temp.Arena, LinkCount, sort_entry, NoClear());
    
    r64 TotalTime = 0.0f;
    u32 Index = 0;
    for(debug_variable_link *Link = GetSentinel(DebugState->ProfileGroup)->Next;
        Link != GetSentinel(DebugState->ProfileGroup);
        Link = Link->Next, ++Index)
    {
        Assert(Link->FirstChild == GetSentinel(Link));
        
        debug_clock_entry *Entry = Entries + Index;
        sort_entry *Sort = SortA + Index;
        
        Entry->Element = Link->Element;
        debug_element *Element = Entry->Element;
        
        BeginDebugStatistic(&Entry->Stats);
        for(debug_stored_event *Event = Element->Frames[DebugState->ViewingFrameOrdinal].OldestEvent;
            Event;
            Event = Event->Next)
        {
            u64 ClocksWithChildren = Event->ProfileNode.Duration;
            u64 ClocksWithoutChildren = ClocksWithChildren - Event->ProfileNode.DurationOfChildren;
            AccumDebugStatistic(&Entry->Stats, (r64)ClocksWithoutChildren);
        }
        EndDebugStatistic(&Entry->Stats);
        TotalTime += Entry->Stats.Sum;
        
        Sort->SortKey = -(r32)Entry->Stats.Sum;
        Sort->Index = Index;
    }
    
    RadixSort(LinkCount, SortA, SortB);
    
    r64 PC = 0.0f;
    if(TotalTime > 0)
    {
        PC = 100.0f / TotalTime;
    }
    
    r64 RunningSum = 0.0f;
    
    v2 At = V2(ProfileRect.Min.x, ProfileRect.Max.y - GetBaseline(UI));
    for(Index = 0;
        (Index < LinkCount);
        ++Index)
    {
        debug_clock_entry *Entry = Entries + SortA[Index].Index;
        debug_statistic *Stats = &Entry->Stats;
        debug_element *Element = Entry->Element;
        
        RunningSum += Stats->Sum;
        
        char TempName[256];
        char TextBuffer[256];
        FormatString(sizeof(TextBuffer), TextBuffer,
                     "%10ucy %05.02f%% %4d %s",
                     (u32)Stats->Sum,
                     (PC*Stats->Sum),
                     Stats->Count,
                     GetName(DebugState, sizeof(TempName), TempName, Element));
        TextOutAt(UI, At, TextBuffer);
        rectangle2 TextRect = GetTextSize(UI, At, TextBuffer);
        
        if(IsInRectangle(TextRect, MouseP))
        {
            tooltip_buffer ToolTipBuffer = AddLine(&UI->ToolTips);
            FormatString(ToolTipBuffer.Size, ToolTipBuffer.Data,
                         "  %05.02fcy each; cumulative to this point: %02.02f%%",
                         SafeRatio0((f64)Stats->Sum, (f64)Stats->Count),
                         (PC*RunningSum));
        }
        
        if(At.y < ProfileRect.Min.y)
        {
            break;
        }
        else
        {
            At.y -= GetLineAdvance(UI);
        }
    }
    
    EndTemporaryMemory(Temp);
}

internal void
DrawFrameSlider(debug_state *DebugState, dev_id SliderID, rectangle2 TotalRect, v2 MouseP,
                debug_element *RootElement)
{
    dev_ui *UI = &DebugState->DevUI;
    render_group *RenderGroup = &UI->RenderGroup;
    
    u32 FrameCount = ArrayCount(RootElement->Frames);
    if(FrameCount > 0)
    {
        PushRect(RenderGroup, TotalRect, UI->BackingTransform, V4(0, 0, 0, 0.25f));
        
        r32 BarWidth = (GetDim(TotalRect).x / (r32)FrameCount);
        r32 AtX = TotalRect.Min.x;
        r32 ThisMinY = TotalRect.Min.y;
        r32 ThisMaxY = TotalRect.Max.y;
        for(u32 FrameIndex = 0;
            FrameIndex < FrameCount;
            ++FrameIndex)
        {
            rectangle2 RegionRect = RectMinMax(V2(AtX, ThisMinY), V2(AtX + BarWidth, ThisMaxY));
            
            v4 HiColor = V4(1, 1, 1, 1);
            b32 Highlight = false;
            if(FrameIndex == DebugState->ViewingFrameOrdinal)
            {
                HiColor = V4(1, 1, 0, 1);
                Highlight = true;
            }
            
            if(FrameIndex == DebugState->MostRecentFrameOrdinal)
            {
                HiColor = V4(0, 1, 0, 1);
                Highlight = true;
            }
            
            if(FrameIndex == DebugState->CollationFrameOrdinal)
            {
                HiColor = V4(1, 0, 0, 1);
                Highlight = true;
            }
            
            if(FrameIndex == DebugState->OldestFrameOrdinal)
            {
                HiColor = V4(0, 0.5f, 0, 1);
                Highlight = true;
            }
            
            if(Highlight)
            {
                PushRect(RenderGroup, RegionRect, UI->UITransform, HiColor);
            }
            PushRectOutline(RenderGroup, RegionRect, UI->UITransform + V3(0, 0, 1.0f), V4(0.5f,0.5f,0.5f, 1), 2.0f);
            
            if(IsInRectangle(RegionRect, MouseP))
            {
                tooltip_buffer TextBuffer = AddLine(&UI->ToolTips);
                FormatString(TextBuffer.Size, TextBuffer.Data, "%u", FrameIndex);
                
                UI->NextHotInteraction =
                    SetUInt32Interaction(SliderID, &DebugState->ViewingFrameOrdinal, FrameIndex);
            }
            
            AtX += BarWidth;
        }
    }
}

internal void
DEBUGDrawElement(debug_state *DebugState, dev_ui_layout *Layout, debug_tree *Tree, debug_element *Element, dev_id DebugID,
                 u32 FrameOrdinal)
{
    dev_ui *UI = Layout->UI;
    render_group *RenderGroup = &UI->RenderGroup;
    
    dev_ui_interaction ItemInteraction =
        ElementInteraction(DebugState, DebugID, DevUI_Interaction_AutoModifyVariable, Element);
    
    b32 IsHot = InteractionIsHot(UI, ItemInteraction);
    v4 ItemColor = IsHot ? V4(1, 1, 0, 1) : V4(1, 1, 1, 1);
    
    debug_stored_event *OldestStoredEvent =
        Element->Frames[DebugState->ViewingFrameOrdinal].OldestEvent;
    
    debug_event NullEvent = {};
    NullEvent.GUID = Element->GUID;
    NullEvent.Type = (u8)Element->Type;
    
    debug_view *View = GetOrCreateDebugViewFor(DebugState, DebugID);
    switch(Element->Type)
    {
        case DebugType_bitmap_id:
        {
            debug_event *Event = OldestStoredEvent ? &OldestStoredEvent->Event : 0;
            renderer_texture TextureHandle = {};
            r32 BitmapScale = View->InlineBlock.Dim.y;
            if(Event)
            {
                TextureHandle = GetBitmap(RenderGroup->Assets, Event->Value_bitmap_id);
                if(IsValid(TextureHandle))
                {
                    used_bitmap_dim Dim = GetBitmapDim(RenderGroup, TextureHandle, BitmapScale, V3(0.0f, 0.0f, 0.0f), V2(0, 0));
                    View->InlineBlock.Dim.x = Dim.Size.x;
                }
            }
            
            dev_ui_layout_element LayEl = BeginElementRectangle(Layout, &View->InlineBlock.Dim);
            MakeElementSizable(&LayEl);
            DefaultInteraction(&LayEl, ItemInteraction);
            EndElement(&LayEl);
            PushRect(RenderGroup, LayEl.Bounds, UI->BackingTransform, V4(0, 0, 0, 1.0f));
            
            if(IsValid(TextureHandle))
            {
                PushBitmap(RenderGroup, Event->Value_bitmap_id, BitmapScale,
                           V3(GetMinCorner(LayEl.Bounds), 1.0f) + UI->BackingTransform, V2(0, 0), V4(1, 1, 1, 1));
            }
        } break;
        
        case DebugType_ThreadIntervalGraph:
        case DebugType_FrameBarGraph:
        case DebugType_TopClocksList:
        {
            debug_view_profile_graph *Graph = &View->ProfileGraph;
            
            BeginRow(Layout);
            ActionButton(Layout, BundleZ("Root"), SetPointerInteraction(DebugID, (void **)&Graph->GUID, 0));
            BooleanButton(Layout, BundleZ("Threads"), (Element->Type == DebugType_ThreadIntervalGraph),
                          SetUInt32Interaction(DebugID, (u32 *)&Element->Type, DebugType_ThreadIntervalGraph));
            BooleanButton(Layout, BundleZ("Frames"), (Element->Type == DebugType_FrameBarGraph),
                          SetUInt32Interaction(DebugID, (u32 *)&Element->Type, DebugType_FrameBarGraph));
            BooleanButton(Layout, BundleZ("Clocks"), (Element->Type == DebugType_TopClocksList),
                          SetUInt32Interaction(DebugID, (u32 *)&Element->Type, DebugType_TopClocksList));
            EndRow(Layout);
            
            dev_ui_layout_element LayEl = BeginElementRectangle(Layout, &Graph->Block.Dim);
            if((Graph->Block.Dim.x == 0) && (Graph->Block.Dim.y == 0))
            {
                Graph->Block.Dim.x = 1400;
                Graph->Block.Dim.y = 280;
            }
            
            MakeElementSizable(&LayEl);
            //                DefaultInteraction(&LayEl, ItemInteraction);
            EndElement(&LayEl);
            
            PushRect(RenderGroup, LayEl.Bounds, UI->BackingTransform, V4(0, 0, 0, 0.75f));
            
            transient_clip_rect ClipRect(
                RenderGroup,
                GetClipRect(RenderGroup, LayEl.Bounds, UI->BackingTransform.z));
            
            debug_stored_event *RootNode = 0;
            
            u32 ViewingFrameOrdinal = DebugState->ViewingFrameOrdinal;
            debug_element *ViewingElement = GetElementFromGUID(DebugState, View->ProfileGraph.GUID);
            if(!ViewingElement)
            {
                ViewingElement = DebugState->RootProfileElement;
            }
            
            switch(Element->Type)
            {
                case DebugType_ThreadIntervalGraph:
                {
                    DrawProfileIn(DebugState, DebugID, LayEl.Bounds, Layout->MouseP, ViewingElement);
                } break;
                
                case DebugType_FrameBarGraph:
                {
                    DrawFrameBars(DebugState, DebugID, LayEl.Bounds, Layout->MouseP, ViewingElement);
                } break;
                
                case DebugType_TopClocksList:
                {
                    DrawTopClocksList(DebugState, DebugID, LayEl.Bounds, Layout->MouseP, ViewingElement);
                } break;
            }
        } break;
        
        case DebugType_FrameSlider:
        {
            v2 *Dim = &View->InlineBlock.Dim;
            if((Dim->x == 0) && (Dim->y == 0))
            {
                Dim->x = 1400;
                Dim->y = 32;
            }
            
            dev_ui_layout_element LayEl = BeginElementRectangle(Layout, Dim);
            MakeElementSizable(&LayEl);
            EndElement(&LayEl);
            
            BeginRow(Layout);
            BooleanButton(Layout, BundleZ("Pause"), DebugState->Paused,
                          SetUInt32Interaction(DebugID, (u32 *)&DebugState->Paused, !DebugState->Paused));
            ActionButton(Layout, BundleZ("Oldest"),
                         SetUInt32Interaction(DebugID, &DebugState->ViewingFrameOrdinal,
                                              DebugState->OldestFrameOrdinal));
            ActionButton(Layout, BundleZ("Most Recent"),
                         SetUInt32Interaction(DebugID, &DebugState->ViewingFrameOrdinal,
                                              DebugState->MostRecentFrameOrdinal));
            EndRow(Layout);
            
            DrawFrameSlider(DebugState, DebugID, LayEl.Bounds, Layout->MouseP, Element);
        } break;
        
        case DebugType_LastFrameInfo:
        case DebugType_FunctionSummary:
        {
            char Text[256];
            GetName(DebugState, sizeof(Text), Text, Element);
            BasicTextElement(Layout, WrapZ(Text), ItemInteraction);
        } break;
        
        case DebugType_MemoryByArena:
        case DebugType_MemoryByFrame:
        case DebugType_MemoryBySize:
        {
            debug_view_arena_graph *Graph = &View->ArenaGraph;
            
            BeginRow(Layout);
            //ActionButton(Layout, BundleZ("Root"), SetPointerInteraction(DebugID, (void **)&Graph->GUID, 0));
            BooleanButton(Layout, BundleZ("Arenas"), (Element->Type == DebugType_MemoryByArena),
                          SetUInt32Interaction(DebugID, (u32 *)&Element->Type, DebugType_MemoryByArena));
            BooleanButton(Layout, BundleZ("Frames"), (Element->Type == DebugType_MemoryByFrame),
                          SetUInt32Interaction(DebugID, (u32 *)&Element->Type, DebugType_MemoryByFrame));
            BooleanButton(Layout, BundleZ("Sizes"), (Element->Type == DebugType_MemoryBySize),
                          SetUInt32Interaction(DebugID, (u32 *)&Element->Type, DebugType_MemoryBySize));
            BooleanButton(Layout, BundleZ("Debug"), Graph->ShowSuppressed,
                          SetUInt32Interaction(DebugID, (u32 *)&Graph->ShowSuppressed, !Graph->ShowSuppressed));
            BooleanButton(Layout, BundleZ("Call Sites"), Graph->ShowCallSites,
                          SetUInt32Interaction(DebugID, (u32 *)&Graph->ShowCallSites, !Graph->ShowCallSites));
            EndRow(Layout);
            
            dev_ui_layout_element LayEl = BeginElementRectangle(Layout, &Graph->Block.Dim);
            if((Graph->Block.Dim.x == 0) && (Graph->Block.Dim.y == 0))
            {
                Graph->Block.Dim.x = 1400;
                Graph->Block.Dim.y = 280;
            }
            
            MakeElementSizable(&LayEl);
            //                DefaultInteraction(&LayEl, ItemInteraction);
            EndElement(&LayEl);
            
            PushRect(RenderGroup, LayEl.Bounds, UI->BackingTransform, V4(0, 0, 0, 0.75f));
            
            transient_clip_rect TransientClipRect(
                RenderGroup,
                GetClipRect(RenderGroup, LayEl.Bounds, UI->BackingTransform.z));
            
            switch(Element->Type)
            {
                case DebugType_MemoryByArena:
                {
                    DrawArenaInterval(DebugState, DebugID, LayEl.Bounds, Layout->MouseP, Graph);
                } break;
                
                case DebugType_MemoryByFrame:
                {
                } break;
                
                case DebugType_MemoryBySize:
                {
                    DrawTopMemList(DebugState, DebugID, LayEl.Bounds, Layout->MouseP, Graph);
                } break;
            }
#if 0
            debug_view_arena_graph *Graph = &View->ArenaGraph;
            
            BeginRow(Layout);
            char TempName[256];
            // TODO(casey): Get rid of the WrapZ here.
            Label(Layout, WrapZ(GetName(DebugState, sizeof(TempName), TempName, Element)));
            BooleanButton(Layout, BundleZ("Occupancy"), (Element->Type == DebugType_ArenaOccupancy),
                          SetUInt32Interaction(DebugID, (u32 *)&Element->Type, DebugType_ArenaOccupancy));
            EndRow(Layout);
            
            
#endif
        } break;
        
        default:
        {
            debug_event *Event = OldestStoredEvent ? &OldestStoredEvent->Event : &NullEvent;
            char Text[256];
            DEBUGEventToText(Text, Text + sizeof(Text), Element, Event,
                             DEBUGVarToText_AddName|
                             DEBUGVarToText_AddValue|
                             DEBUGVarToText_NullTerminator|
                             DEBUGVarToText_Colon|
                             DEBUGVarToText_PrettyBools);
            
            BasicTextElement(Layout, WrapZ(Text), ItemInteraction);
        } break;
    }
}

internal void
DrawTreeLink(debug_state *DebugState, dev_ui_layout *Layout, debug_tree *Tree, debug_variable_link *Link)
{
    dev_ui *UI = &DebugState->DevUI;
    u32 FrameOrdinal = DebugState->ViewingFrameOrdinal;
    
    if(CanHaveChildren(Link))
    {
        dev_id ID = DebugIDFromLink(Tree, Link);
        debug_view *View = GetOrCreateDebugViewFor(DebugState, ID);
        dev_ui_interaction ItemInteraction = DebugIDInteraction(DevUI_Interaction_ToggleExpansion, ID);
        if(UI->AltUI)
        {
            ItemInteraction = DebugLinkInteraction(DevUI_Interaction_TearValue, Link);
        }
        
        char TempName[256];
        char *Text = GetName(DebugState, sizeof(TempName), TempName, Link->Element);
        
        rectangle2 TextBounds = GetTextSize(UI, Text);
        v2 Dim = {GetDim(TextBounds).x, Layout->LineAdvance};
        
        dev_ui_layout_element Element = BeginElementRectangle(Layout, &Dim);
        DefaultInteraction(&Element, ItemInteraction);
        EndElement(&Element);
        
        b32 IsHot = InteractionIsHot(UI, ItemInteraction);
        v4 ItemColor = IsHot ? V4(1, 1, 0, 1) : V4(1, 1, 1, 1);
        
        TextOutAt(UI, V2(GetMinCorner(Element.Bounds).x,
                         GetMaxCorner(Element.Bounds).y - GetBaseline(UI)),
                  Text, ItemColor);
        
        if(View->Collapsible.ExpandedAlways)
        {
            ++Layout->Depth;
            
            for(debug_variable_link *SubLink = Link->FirstChild;
                SubLink != GetSentinel(Link);
                SubLink = SubLink->Next)
            {
                DrawTreeLink(DebugState, Layout, Tree, SubLink);
            }
            
            --Layout->Depth;
        }
    }
    else
    {
        dev_id DebugID = DebugIDFromLink(Tree, Link);
        DEBUGDrawElement(DebugState, Layout, Tree, Link->Element, DebugID, FrameOrdinal);
    }
}

internal void
DrawTree(debug_state *DebugState, debug_tree *Tree, v2 P, debug_variable_link *Group)
{
    dev_ui_layout Layout = BeginLayout(&DebugState->DevUI, P);
    if(Group)
    {
        DrawTreeLink(DebugState, &Layout, Tree, Group);
    }
    EndLayout(&Layout);
}

internal void
DrawTrees(debug_state *DebugState, v2 MouseP)
{
    dev_ui *UI = &DebugState->DevUI;
    render_group *RenderGroup = &UI->RenderGroup;
    u32 FrameOrdinal = DebugState->ViewingFrameOrdinal;
    
    for(debug_tree *Tree = DebugState->TreeSentinel.Next;
        Tree != &DebugState->TreeSentinel;
        Tree = Tree->Next)
    {
        dev_ui_interaction MoveInteraction = {};
        MoveInteraction.Type = DevUI_Interaction_Move;
        MoveInteraction.P = &Tree->UIP;
        
        rectangle2 MoveBox = RectCenterHalfDim(Tree->UIP - V2(4.0f, 4.0f), V2(4.0f, 4.0f));
        PushRect(RenderGroup, MoveBox, V3(0, 0, 0),
                 InteractionIsHot(UI, MoveInteraction) ? V4(1, 1, 0, 1) : V4(1, 1, 1, 1));
        
        if(IsInRectangle(MoveBox, MouseP))
        {
            UI->NextHotInteraction = MoveInteraction;
        }
        
        DrawTree(DebugState, Tree, Tree->UIP, Tree->Group);
    }
    
#if 0
    u32 NewHotMenuIndex = ArrayCount(DebugVariableList);
    r32 BestDistanceSq = Real32Maximum;
    
    r32 MenuRadius = 400.0f;
    r32 AngleStep = Tau32 / (r32)ArrayCount(DebugVariableList);
    for(u32 MenuItemIndex = 0;
        MenuItemIndex < ArrayCount(DebugVariableList);
        ++MenuItemIndex)
    {
        debug_variable *Var = DebugVariableList + MenuItemIndex;
        char *Text = Var->Name;
        
        v4 ItemColor = Var->Value ? V4(1, 1, 1, 1) : V4(0.5f, 0.5f, 0.5f, 1);
        if(MenuItemIndex == DebugState->HotMenuIndex)
        {
            ItemColor = V4(1, 1, 0, 1);
        }
        
        r32 Angle = (r32)MenuItemIndex*AngleStep;
        v2 TextP = DebugState->MenuP + MenuRadius*Arm2(Angle);
        
        r32 ThisDistanceSq = LengthSq(TextP - MouseP);
        if(BestDistanceSq > ThisDistanceSq)
        {
            NewHotMenuIndex = MenuItemIndex;
            BestDistanceSq = ThisDistanceSq;
        }
        
        rectangle2 TextBounds = DEBUGGetTextSize(DebugState, Text);
        DEBUGTextOutAt(TextP - 0.5f*GetDim(TextBounds), Text, ItemColor);
    }
    
    if(LengthSq(MouseP - DebugState->MenuP) > Square(MenuRadius))
    {
        DebugState->HotMenuIndex = NewHotMenuIndex;
    }
    else
    {
        DebugState->HotMenuIndex = ArrayCount(DebugVariableList);
    }
#endif
}

internal void
DEBUGBeginInteract(debug_state *DebugState, game_input *Input, v2 MouseP)
{
    dev_ui *UI = &DebugState->DevUI;
    
    u32 FrameOrdinal = DebugState->MostRecentFrameOrdinal;
    if(UI->HotInteraction.Type)
    {
        if(UI->HotInteraction.Type == DevUI_Interaction_AutoModifyVariable)
        {
            switch(UI->HotInteraction.Element->Frames[FrameOrdinal].MostRecentEvent->Event.Type)
            {
                case DebugType_b32:
                {
                    UI->HotInteraction.Type = DevUI_Interaction_ToggleValue;
                } break;
                
                case DebugType_r32:
                {
                    UI->HotInteraction.Type = DevUI_Interaction_DragValue;
                } break;
                
                case DebugType_u32:
                case DebugType_s32:
                {
                    UI->HotInteraction.Type = DevUI_Interaction_TickValue;
                } break;
                
                case DebugType_OpenDataBlock:
                {
                    UI->HotInteraction.Type = DevUI_Interaction_ToggleValue;
                } break;
            }
        }
        
        switch(UI->HotInteraction.Type)
        {
            case DevUI_Interaction_TearValue:
            {
                debug_variable_link *RootGroup = CloneVariableLink(DebugState, UI->HotInteraction.Link);
                debug_tree *Tree = AddTree(DebugState, RootGroup, MouseP);
                UI->HotInteraction.Type = DevUI_Interaction_Move;
                UI->HotInteraction.P = &Tree->UIP;
            } break;
            
            case DevUI_Interaction_Select:
            {
                if(!Input->ShiftDown)
                {
                    ClearSelection(DebugState);
                }
                AddToSelection(DebugState, UI->HotInteraction.ID);
            } break;
        }
        
        UI->Interaction = UI->HotInteraction;
    }
    else
    {
        UI->Interaction.Type = DevUI_Interaction_NOP;
    }
}

internal debug_element *
GetElementFromEvent(debug_state *DebugState, debug_event *Event,
                    debug_variable_link *Parent, u32 Op);
void
DEBUGMarkEditedEvent(debug_state *DebugState, debug_event *Event)
{
    if(Event)
    {
        GlobalDebugTable->EditEvent = *Event;
        GlobalDebugTable->EditEvent.GUID =
            GetElementFromEvent(DebugState, Event, 0, DebugElement_AddToGroup|DebugElement_CreateHierarchy)->OriginalGUID;
    }
}

internal void
DEBUGEndInteract(debug_state *DebugState, game_input *Input, v2 MouseP)
{
    dev_ui *UI = &DebugState->DevUI;
    
    u32 FrameOrdinal = DebugState->MostRecentFrameOrdinal;
    switch(UI->Interaction.Type)
    {
        case DevUI_Interaction_ToggleExpansion:
        {
            debug_view *View = GetOrCreateDebugViewFor(DebugState, UI->Interaction.ID);
            View->Collapsible.ExpandedAlways = !View->Collapsible.ExpandedAlways;
        } break;
        
        case DevUI_Interaction_SetUInt32:
        {
            *(u32 *)UI->Interaction.Target = UI->Interaction.UInt32;
        } break;
        
        case DevUI_Interaction_SetPointer:
        {
            *(void **)UI->Interaction.Target = UI->Interaction.Pointer;
        } break;
        
        case DevUI_Interaction_TickValue:
        {
            debug_event *Event = UI->Interaction.Element ?
                &UI->Interaction.Element->Frames[FrameOrdinal].MostRecentEvent->Event : 0;
            switch(Event->Type)
            {
                case DebugType_u32:
                case DebugType_s32:
                {
                    if(Input->ShiftDown)
                    {
                        --Event->Value_u32;
                    }
                    else
                    {
                        ++Event->Value_u32;
                    }
                } break;
            }
            DEBUGMarkEditedEvent(DebugState, Event);
        } break;
        
        case DevUI_Interaction_ToggleValue:
        {
            debug_event *Event = &UI->Interaction.Element->Frames[FrameOrdinal].
                MostRecentEvent->Event;
            Assert(Event);
            switch(Event->Type)
            {
                case DebugType_b32:
                {
                    Event->Value_b32 = !Event->Value_b32;
                } break;
            }
            DEBUGMarkEditedEvent(DebugState, Event);
        } break;
    }
    
    ClearInteraction(&UI->Interaction);
}

internal void
DEBUGInteract(debug_state *DebugState, game_input *Input, v2 MouseP, v2 dMouseP)
{
    dev_ui *UI = &DebugState->DevUI;
    
    /*
        if(Input->MouseButtons[PlatformMouseButton_Right].EndedDown)
        {
            if(Input->MouseButtons[PlatformMouseButton_Right].HalfTransitionCount > 0)
            {
                DebugState->MenuP = MouseP;
            }
            DrawDebugMainMenu(DebugState, RenderGroup, MouseP);
        }
        else if(Input->MouseButtons[PlatformMouseButton_Right].HalfTransitionCount > 0)
    */
    if(UI->Interaction.Type)
    {
        u32 FrameOrdinal = DebugState->MostRecentFrameOrdinal;
        v2 *P = UI->Interaction.P;
        
        // NOTE(casey): Mouse move interaction
        switch(UI->Interaction.Type)
        {
            case DevUI_Interaction_DragValue:
            {
                debug_event *Event = UI->Interaction.Element ?
                    &UI->Interaction.Element->Frames[FrameOrdinal].MostRecentEvent->Event : 0;
                switch(Event->Type)
                {
                    case DebugType_r32:
                    {
                        Event->Value_r32 += 0.1f*dMouseP.x;
                    } break;
                }
                DEBUGMarkEditedEvent(DebugState, Event);
            } break;
            
            case DevUI_Interaction_Resize:
            {
                *P += V2(dMouseP.x, -dMouseP.y);
                P->x = Maximum(P->x, 10.0f);
                P->y = Maximum(P->y, 10.0f);
            } break;
            
            case DevUI_Interaction_Move:
            {
                *P += V2(dMouseP.x, dMouseP.y);
            } break;
        }
        
        // NOTE(casey): Click interaction
        for(u32 TransitionIndex = Input->MouseButtons[PlatformMouseButton_Left].HalfTransitionCount;
            TransitionIndex > 1;
            --TransitionIndex)
        {
            DEBUGEndInteract(DebugState, Input, MouseP);
            DEBUGBeginInteract(DebugState, Input, MouseP);
        }
        
        if(!Input->MouseButtons[PlatformMouseButton_Left].EndedDown)
        {
            DEBUGEndInteract(DebugState, Input, MouseP);
        }
    }
    else
    {
        UI->HotInteraction = UI->NextHotInteraction;
        
        for(u32 TransitionIndex = Input->MouseButtons[PlatformMouseButton_Left].HalfTransitionCount;
            TransitionIndex > 1;
            --TransitionIndex)
        {
            DEBUGBeginInteract(DebugState, Input, MouseP);
            DEBUGEndInteract(DebugState, Input, MouseP);
        }
        
        if(Input->MouseButtons[PlatformMouseButton_Left].EndedDown)
        {
            DEBUGBeginInteract(DebugState, Input, MouseP);
        }
    }
}

inline u32
GetLaneFromThreadIndex(debug_state *DebugState, u32 ThreadIndex)
{
    u32 Result = 0;
    
    // TODO(casey): Implement thread ID lookup.
    
    return(Result);
}

internal debug_thread *
GetDebugThread(debug_state *DebugState, u32 ThreadID)
{
    debug_thread *Result = 0;
    for(debug_thread *Thread = DebugState->FirstThread;
        Thread;
        Thread = Thread->Next)
    {
        if(Thread->ID == ThreadID)
        {
            Result = Thread;
            break;
        }
    }
    
    if(!Result)
    {
        FREELIST_ALLOCATE(Result, DebugState->FirstFreeThread, PushStruct(&DebugState->DebugArena, debug_thread));
        
        Result->ID = ThreadID;
        Result->LaneIndex = DebugState->FrameBarLaneCount++;
        Result->FirstOpenCodeBlock = 0;
        Result->FirstOpenDataBlock = 0;
        Result->Next = DebugState->FirstThread;
        DebugState->FirstThread = Result;
    }
    
    return(Result);
}

internal debug_variable_link *
CreateVariableLink(debug_state *DebugState, debug_element *Element)
{
    debug_variable_link *Link = PushStruct(&DebugState->DebugArena, debug_variable_link);
    DLIST_INIT(GetSentinel(Link));
    Link->Next = Link->Prev = 0;
    Link->Element = Element;
    
    return(Link);
}

internal debug_variable_link *
AddElementToGroup(debug_state *DebugState, debug_variable_link *Parent, debug_element *Element)
{
    debug_variable_link *Link = CreateVariableLink(DebugState, Element);
    
    if(Parent)
    {
        DLIST_INSERT_AS_LAST(GetSentinel(Parent), Link);
    }
    
    return(Link);
}

internal debug_variable_link *
AddLinkToGroup(debug_state *DebugState, debug_variable_link *Parent, debug_variable_link *Link)
{
    DLIST_INSERT_AS_LAST(GetSentinel(Parent), Link);
    return(Link);
}

internal debug_variable_link *
CloneVariableLink(debug_state *DebugState, debug_variable_link *DestGroup, debug_variable_link *Source)
{
    debug_variable_link *Dest = AddElementToGroup(DebugState, DestGroup, Source->Element);
    for(debug_variable_link *Child = Source->FirstChild;
        Child != GetSentinel(Source);
        Child = Child->Next)
    {
        CloneVariableLink(DebugState, Dest, Child);
    }
    
    return(Dest);
}

internal debug_variable_link *
CloneVariableLink(debug_state *DebugState, debug_variable_link *Source)
{
    debug_variable_link *Result = CloneVariableLink(DebugState, 0, Source);
    return(Result);
}

internal debug_element *
CreateNameElement(debug_state *DebugState, u32 NameLength, char *Name,
                  debug_type Type = DebugType_Name)
{
    debug_element *Result = PushStruct(&DebugState->DebugArena, debug_element);
    
    Result->Name = NameLength ? PushAndNullTerminate(&DebugState->DebugArena, NameLength, Name) : 0;
    Result->Type = Type;
    
    return(Result);
}

internal debug_variable_link *
GetOrCreateGroupWithName(debug_state *DebugState, debug_variable_link *Parent, u32 NameLength, char *Name)
{
    debug_variable_link *Result = 0;
    for(debug_variable_link *Link = Parent->FirstChild;
        Link != GetSentinel(Parent);
        Link = Link->Next)
    {
        if(Link->Element)
        {
            if(StringsAreEqual(NameLength, Name, Link->Element->Name))
            {
                Result = Link;
            }
        }
    }
    
    if(!Result)
    {
        Result = CreateVariableLink(DebugState, CreateNameElement(DebugState, NameLength, Name));
        AddLinkToGroup(DebugState, Parent, Result);
    }
    
    return(Result);
}

internal debug_variable_link *
GetGroupForHierarchicalName(debug_state *DebugState, debug_variable_link *Parent, char *Name, b32 CreateTerminal)
{
    debug_variable_link *Result = Parent;
    
    char *FirstSeparator = 0;
    char *Scan = Name;
    for(;
        *Scan;
        ++Scan)
    {
        if(*Scan == '/')
        {
            FirstSeparator = Scan;
            break;
        }
    }
    
    if(FirstSeparator || CreateTerminal)
    {
        u32 NameLength = 0;
        if(FirstSeparator)
        {
            NameLength = (u32)(FirstSeparator - Name);
        }
        else
        {
            NameLength = (u32)(Scan - Name);
        }
        
        Result = GetOrCreateGroupWithName(DebugState, Parent, NameLength, Name);
        if(FirstSeparator)
        {
            Result = GetGroupForHierarchicalName(DebugState, Result, FirstSeparator + 1, CreateTerminal);
        }
    }
    
    return(Result);
}

inline open_debug_block *
AllocateOpenDebugBlock(debug_state *DebugState, debug_element *Element,
                       u32 FrameIndex, debug_event *Event,
                       open_debug_block **FirstOpenBlock)
{
    open_debug_block *Result = 0;
    FREELIST_ALLOCATE(Result, DebugState->FirstFreeBlock, PushStruct(&DebugState->DebugArena, open_debug_block));
    
    Result->StartingFrameIndex = FrameIndex;
    Result->BeginClock = Event->Clock;
    Result->Element = Element;
    Result->NextFree = 0;
    
    Result->Parent = *FirstOpenBlock;
    *FirstOpenBlock = Result;
    
    return(Result);
}

inline void
DeallocateOpenDebugBlock(debug_state *DebugState, open_debug_block **FirstOpenBlock)
{
    open_debug_block *FreeBlock = *FirstOpenBlock;
    *FirstOpenBlock = FreeBlock->Parent;
    
    FreeBlock->NextFree = DebugState->FirstFreeBlock;
    DebugState->FirstFreeBlock = FreeBlock;
}

inline b32
EventsMatch(debug_event A, debug_event B)
{
    // TODO(casey): Have counters for blocks?
    b32 Result = (A.ThreadID == B.ThreadID);
    
    return(Result);
}

internal void
FreeFrame(debug_state *DebugState, u32 FrameOrdinal)
{
    Assert(FrameOrdinal < DEBUG_FRAME_COUNT);
    
    u32 FreedEventCount = 0;
    
    for(u32 ElementHashIndex = 0;
        ElementHashIndex < ArrayCount(DebugState->ElementHash);
        ++ElementHashIndex)
    {
        for(debug_element *Element = DebugState->ElementHash[ElementHashIndex];
            Element;
            Element = Element->NextInHash)
        {
            debug_element_frame *ElementFrame = Element->Frames + FrameOrdinal;
            while(ElementFrame->OldestEvent)
            {
                debug_stored_event *FreeEvent = ElementFrame->OldestEvent;
                ElementFrame->OldestEvent = FreeEvent->Next;
                FREELIST_DEALLOCATE(FreeEvent, DebugState->FirstFreeStoredEvent);
                ++FreedEventCount;
            }
            ZeroStruct(*ElementFrame);
        }
    }
    
    debug_frame *Frame = DebugState->Frames + FrameOrdinal;
    Assert(Frame->StoredEventCount == FreedEventCount);
    
    ZeroStruct(*Frame);
}

internal void
InitFrame(debug_state *DebugState, u64 BeginClock, debug_frame *Result)
{
    Result->FrameIndex = DebugState->TotalFrameCount++;
    Result->FrameBarScale = 1.0f;
    Result->BeginClock = BeginClock;
}

inline void
IncrementFrameOrdinal(u32 *Ordinal)
{
    *Ordinal = (*Ordinal+1) % DEBUG_FRAME_COUNT;
}

internal void
FreeOldestFrame(debug_state *DebugState)
{
    FreeFrame(DebugState, DebugState->OldestFrameOrdinal);
    
    if(DebugState->OldestFrameOrdinal == DebugState->MostRecentFrameOrdinal)
    {
        IncrementFrameOrdinal(&DebugState->MostRecentFrameOrdinal);
    }
    IncrementFrameOrdinal(&DebugState->OldestFrameOrdinal);
}

inline debug_frame *
GetCollationFrame(debug_state *DebugState)
{
    debug_frame *Result = DebugState->Frames + DebugState->CollationFrameOrdinal;
    
    return(Result);
}

internal debug_stored_event *
StoreEvent(debug_state *DebugState, debug_element *Element, debug_event *Event)
{
    debug_stored_event *Result = 0;
    while(!Result)
    {
        Result = DebugState->FirstFreeStoredEvent;
        if(Result)
        {
            DebugState->FirstFreeStoredEvent = Result->NextFree;
        }
        else
        {
            Result = PushStruct(&DebugState->DebugArena, debug_stored_event);
        }
    }
    
    debug_frame *CollationFrame = GetCollationFrame(DebugState);
    
    Result->Next = 0;
    //    Result->FrameIndex = CollationFrame->FrameIndex;
    Result->Event = *Event;
    
    ++CollationFrame->StoredEventCount;
    
    debug_element_frame *Frame = Element->Frames + DebugState->CollationFrameOrdinal;
    if(Frame->MostRecentEvent)
    {
        Frame->MostRecentEvent = Frame->MostRecentEvent->Next = Result;
    }
    else
    {
        Frame->OldestEvent = Frame->MostRecentEvent = Result;
    }
    
    return(Result);
}

internal debug_element *
GetElementFromEvent(debug_state *DebugState, debug_event *Event, debug_variable_link *Parent,
                    u32 Op)
{
    Assert(Event->GUID);
    
    if(!Parent)
    {
        Parent = DebugState->RootGroup;
    }
    
    debug_parsed_name ParsedName = DebugParseName(Event->GUID, Event->Name);
    u32 Index = (ParsedName.HashValue % ArrayCount(DebugState->ElementHash));
    
    debug_element *Result = GetElementFromGUID(DebugState, Index, Event->GUID);
    if(!Result)
    {
        Result = PushStruct(&DebugState->DebugArena, debug_element);
        
        Result->OriginalGUID = Event->GUID;
        Result->GUID = PushStringZ(&DebugState->DebugArena, Event->GUID);
        Result->FileNameCount = ParsedName.FileNameCount;
        Result->LineNumber = ParsedName.LineNumber;
        Result->Name = PushStringZ(&DebugState->DebugArena, ParsedName.Name);
        Result->Type = (debug_type)Event->Type;
        
        Result->NextInHash = DebugState->ElementHash[Index];
        DebugState->ElementHash[Index] = Result;
        
        debug_variable_link *ParentGroup = Parent;
        if(Op & DebugElement_CreateHierarchy)
        {
            char TempName[256];
            ParentGroup = GetGroupForHierarchicalName(
                DebugState, Parent, GetName(DebugState, sizeof(TempName), TempName, Result), false);
        }
        
        if(Op & DebugElement_AddToGroup)
        {
            AddElementToGroup(DebugState, ParentGroup, Result);
        }
    }
    
    return(Result);
}

internal debug_arena *
DEBUGGetArenaByLookupBlock(debug_state *DebugState, platform_memory_block *ArenaLookupBlock, b32x AllowCreation = false)
{
    debug_arena *Result = 0;
    umm LookupAddress = UMMFromPointer(ArenaLookupBlock);
    
    for(debug_arena *Test = DebugState->FirstArena;
        Test;
        Test = Test->Next)
    {
        Assert(Test->FirstBlock);
        if(Test->FirstBlock->MemoryAddress == LookupAddress)
        {
            Result = Test;
            break;
        }
    }
    
    if(!Result)
    {
        Assert(AllowCreation);
        Result = DebugState->FirstFreeArena;
        if(Result)
        {
            DebugState->FirstFreeArena = Result->Next;
        }
        else
        {
            Result = PushStruct(&DebugState->DebugArena, debug_arena);
        }
        
        Result->Name = "(unnamed)";
        Result->FirstBlock = 0;
        
        Result->Next = DebugState->FirstArena;
        DebugState->FirstArena = Result;
    }
    
    return(Result);
}

internal void
DEBUGArenaSetName(debug_state *DebugState, debug_event *Event)
{
    debug_memory_op *Op = &Event->Value_debug_memory_op;
    if(Op->Block)
    {
        debug_arena *Arena = DEBUGGetArenaByLookupBlock(DebugState, Op->Block);
        if(Arena)
        {
            Arena->Name = Event->Name;
            Arena->Suppress = (Op->AllocatedSize != 0);
        }
    }
}

internal void
DEBUGMoveToFreeList(debug_state *DebugState, debug_arena_allocation *First, debug_arena_allocation *Last)
{
    if(First)
    {
        for(debug_arena_allocation *Alloc = First;
            Alloc;
            Alloc = Alloc->Next)
        {
            debug_arena_call_site *Site = GetCallSite(DebugState, Alloc->CallSiteIndex);
            Site->PeakTotalAlloc -= Alloc->SizeAllocated;
            
            if(Alloc == Last)
            {
                break;
            }
        }
        
        Assert(Last);
        Last->Next = DebugState->FirstFreeAllocation;
        DebugState->FirstFreeAllocation = First;
    }
    else
    {
        Assert(!Last);
    }
}

internal void
DEBUGRemoveArena(debug_state *DebugState, debug_arena *Arena)
{
    if(DebugState->FirstArena == Arena)
    {
        DebugState->FirstArena = Arena->Next;
    }
    else
    {
        for(debug_arena *Scan = DebugState->FirstArena;
            Scan;
            Scan = Scan->Next)
        {
            if(Scan->Next == Arena)
            {
                Scan->Next = Arena->Next;
                break;
            }
        }
    }
    
    Arena->Next = DebugState->FirstFreeArena;
    DebugState->FirstFreeArena = Arena;
}

internal void
DEBUGArenaBlockFree(debug_state *DebugState, debug_event *Event)
{
    debug_memory_op *Op = &Event->Value_debug_memory_op;
    debug_arena *Arena = DEBUGGetArenaByLookupBlock(DebugState, Op->Block);
    
    debug_arena_block *FreeBlock = Arena->FirstBlock;
    Assert(FreeBlock->MemoryAddress == UMMFromPointer(Op->Block));
    
    // NOTE(casey): Move all the block's allocations to the free list
    DEBUGMoveToFreeList(DebugState, FreeBlock->FirstAllocation, FreeBlock->LastAllocation);
    
    // NOTE(casey): Remove the free block from the arena
    Arena->FirstBlock = FreeBlock->Next;
    
    // NOTE(casey): Add to the old block to the free list
    FreeBlock->Next = DebugState->FirstFreeArenaBlock;
    DebugState->FirstFreeArenaBlock = FreeBlock;
    
    Arena->CurTotalAlloc -= FreeBlock->SizeAllocated;
    if(Arena->FirstBlock == 0)
    {
        DEBUGRemoveArena(DebugState, Arena);
    }
}

internal void
DEBUGArenaBlockTruncate(debug_state *DebugState, debug_event *Event)
{
    debug_memory_op *Op = &Event->Value_debug_memory_op;
    debug_arena *Arena = DEBUGGetArenaByLookupBlock(DebugState, Op->Block);
    if(!Arena->Suppress)
    {
        debug_arena_block *Block = Arena->FirstBlock;
        Assert(Block->MemoryAddress == UMMFromPointer(Op->Block));
        
        debug_arena_allocation *LastFree = 0;
        debug_arena_allocation *FirstValid = Block->FirstAllocation;
        while(FirstValid)
        {
            if(FirstValid->OffsetFromBlock < Op->AllocatedSize)
            {
                break;
            }
            
            LastFree = FirstValid;
            FirstValid = FirstValid->Next;
        }
        
        if(Block->FirstAllocation != FirstValid)
        {
            DEBUGMoveToFreeList(DebugState, Block->FirstAllocation, LastFree);
            Block->FirstAllocation = FirstValid;
            if(Block->LastAllocation == LastFree)
            {
                Block->LastAllocation = 0;
            }
        }
    }
}

internal void
DEBUGArenaBlockAllocate(debug_state *DebugState, debug_event *Event)
{
    debug_memory_block_op *Op = &Event->Value_debug_memory_block_op;
    debug_arena *Arena = DEBUGGetArenaByLookupBlock(DebugState, Op->ArenaLookupBlock, true);
    
    debug_arena_block *Block = DebugState->FirstFreeArenaBlock;
    if(Block)
    {
        DebugState->FirstFreeArenaBlock = Block->Next;
    }
    else
    {
        Block = PushStruct(&DebugState->DebugArena, debug_arena_block);
    }
    
    Block->FirstAllocation = 0;
    Block->LastAllocation = 0;
    
    Block->MemoryAddress = UMMFromPointer(Op->Block);
    Block->SizeAllocated = Op->AllocatedSize;
    
    Block->Next = Arena->FirstBlock;
    Arena->FirstBlock = Block;
    
    Arena->CurTotalAlloc += Block->SizeAllocated;
    if(Arena->PeakTotalAlloc < Arena->CurTotalAlloc)
    {
        Arena->PeakTotalAlloc = Arena->CurTotalAlloc;
    }
}

internal u32
GetCallSiteFrom(debug_state *DebugState, debug_arena *Arena, char *GUID)
{
    debug_arena_call_site *Result = 0;
    
    // TODO(casey): We probably DO need a hash table, sadly...
    for(u32 SiteIndex = 0;
        SiteIndex < DebugState->ArenaCallSiteCount;
        ++SiteIndex)
    {
        debug_arena_call_site *Test = DebugState->ArenaCallSites + SiteIndex;
        if((Test->Arena == Arena) &&
           StringsAreEqual(Test->GUID, GUID))
        {
            Result = Test;
            break;
        }
    }
    
    if(!Result)
    {
        Assert(DebugState->ArenaCallSiteCount < ArrayCount(DebugState->ArenaCallSites));
        u32 Index = DebugState->ArenaCallSiteCount++;
        Result = DebugState->ArenaCallSites + Index;
        
        Result->Arena = Arena;
        Result->GUID = GUID;
        Result->Index = Index;
    }
    
    return(Result->Index);
}

internal void
DEBUGArenaAllocate(debug_state *DebugState, debug_event *Event)
{
    debug_memory_op *Op = &Event->Value_debug_memory_op;
    debug_arena *Arena = DEBUGGetArenaByLookupBlock(DebugState, Op->Block);
    if(!Arena->Suppress)
    {
        debug_arena_block *Block = Arena->FirstBlock;
        Assert(Block->MemoryAddress == UMMFromPointer(Op->Block));
        
        debug_arena_allocation *Alloc = DebugState->FirstFreeAllocation;
        if(Alloc)
        {
            DebugState->FirstFreeAllocation = Alloc->Next;
        }
        else
        {
            Alloc = PushStruct(&DebugState->DebugArena, debug_arena_allocation);
        }
        
        Alloc->CallSiteIndex = GetCallSiteFrom(DebugState, Arena, Event->GUID);
        Alloc->OffsetFromBlock = Op->OffsetInBlock;
        Alloc->SizeAllocated = Op->AllocatedSize;
        
        Alloc->Next = Block->FirstAllocation;
        Block->FirstAllocation = Alloc;
        if(!Block->LastAllocation)
        {
            Block->LastAllocation = Alloc;
        }
        
        debug_arena_call_site *Site = GetCallSite(DebugState, Alloc->CallSiteIndex);
        Site->PeakTotalAlloc += Alloc->SizeAllocated;
    }
}

internal void
CollateDebugRecords(debug_state *DebugState, u32 EventCount, debug_event *EventArray)
{
    for(u32 EventIndex = 0;
        EventIndex < EventCount;
        ++EventIndex)
    {
        debug_event *Event = EventArray + EventIndex;
        if(Event->Type == DebugType_FrameMarker)
        {
            debug_frame *CollationFrame = GetCollationFrame(DebugState);
            
            CollationFrame->EndClock = Event->Clock;
            if(CollationFrame->RootProfileNode)
            {
                CollationFrame->RootProfileNode->ProfileNode.Duration =
                    (CollationFrame->EndClock - CollationFrame->BeginClock);
            }
            
            CollationFrame->WallSecondsElapsed = Event->Value_r32;
            
            r32 ClockRange = (r32)(CollationFrame->EndClock - CollationFrame->BeginClock);
            ++DebugState->TotalFrameCount;
            
            if(DebugState->Paused)
            {
                FreeFrame(DebugState, DebugState->CollationFrameOrdinal);
            }
            else
            {
                DebugState->MostRecentFrameOrdinal = DebugState->CollationFrameOrdinal;
                IncrementFrameOrdinal(&DebugState->CollationFrameOrdinal);
                if(DebugState->CollationFrameOrdinal == DebugState->OldestFrameOrdinal)
                {
                    FreeOldestFrame(DebugState);
                }
                CollationFrame = GetCollationFrame(DebugState);
            }
            InitFrame(DebugState, Event->Clock, CollationFrame);
        }
        else
        {
            debug_frame *CollationFrame = GetCollationFrame(DebugState);
            
            Assert(CollationFrame);
            
            u32 FrameIndex = DebugState->TotalFrameCount - 1;
            debug_thread *Thread = GetDebugThread(DebugState, Event->ThreadID);
            u64 RelativeClock = Event->Clock - CollationFrame->BeginClock;
            
            debug_variable_link *DefaultParentGroup = DebugState->RootGroup;
            if(Thread->FirstOpenDataBlock)
            {
                DefaultParentGroup = Thread->FirstOpenDataBlock->Group;
            }
            
            switch(Event->Type)
            {
                case DebugType_BeginBlock:
                {
                    ++CollationFrame->ProfileBlockCount;
                    debug_element *Element =
                        GetElementFromEvent(DebugState, Event, DebugState->ProfileGroup,
                                            DebugElement_AddToGroup);
                    
                    debug_stored_event *ParentEvent = CollationFrame->RootProfileNode;
                    u64 ClockBasis = CollationFrame->BeginClock;
                    if(Thread->FirstOpenCodeBlock)
                    {
                        ParentEvent = Thread->FirstOpenCodeBlock->Node;
                        ClockBasis = Thread->FirstOpenCodeBlock->BeginClock;
                    }
                    else if(!ParentEvent)
                    {
                        debug_event NullEvent = {};
                        ParentEvent = StoreEvent(DebugState, DebugState->RootProfileElement, &NullEvent);
                        debug_profile_node *Node = &ParentEvent->ProfileNode;
                        Node->Element = 0;
                        Node->FirstChild = 0;
                        Node->NextSameParent = 0;
                        Node->ParentRelativeClock = 0;
                        Node->Duration = 0;
                        Node->DurationOfChildren = 0;
                        Node->ThreadOrdinal = 0;
                        Node->CoreIndex = 0;
                        
                        ClockBasis = CollationFrame->BeginClock;
                        CollationFrame->RootProfileNode = ParentEvent;
                    }
                    
                    debug_stored_event *StoredEvent = StoreEvent(DebugState, Element, Event);
                    debug_profile_node *Node = &StoredEvent->ProfileNode;
                    Node->Element = Element;
                    Node->FirstChild = 0;
                    Node->ParentRelativeClock = Event->Clock - ClockBasis;
                    Node->Duration = 0;
                    Node->DurationOfChildren = 0;
                    Node->ThreadOrdinal = (u16)Thread->LaneIndex;
                    Node->CoreIndex = Event->CoreIndex;
                    
                    Node->NextSameParent = ParentEvent->ProfileNode.FirstChild;
                    ParentEvent->ProfileNode.FirstChild = StoredEvent;
                    
                    open_debug_block *DebugBlock = AllocateOpenDebugBlock(
                        DebugState, Element, FrameIndex, Event,
                        &Thread->FirstOpenCodeBlock);
                    DebugBlock->Node = StoredEvent;
                } break;
                
                case DebugType_EndBlock:
                {
                    if(Thread->FirstOpenCodeBlock)
                    {
                        open_debug_block *MatchingBlock = Thread->FirstOpenCodeBlock;
                        Assert(Thread->ID == Event->ThreadID);
                        
                        debug_profile_node *Node = &MatchingBlock->Node->ProfileNode;
                        Node->Duration = Event->Clock - MatchingBlock->BeginClock;
                        
                        DeallocateOpenDebugBlock(DebugState, &Thread->FirstOpenCodeBlock);
                        
                        if(Thread->FirstOpenCodeBlock)
                        {
                            debug_profile_node *ParentNode =
                                &Thread->FirstOpenCodeBlock->Node->ProfileNode;
                            ParentNode->DurationOfChildren += Node->Duration;
                        }
                    }
                } break;
                
                case DebugType_OpenDataBlock:
                {
                    ++CollationFrame->DataBlockCount;
                    open_debug_block *DebugBlock = AllocateOpenDebugBlock(
                        DebugState, 0, FrameIndex, Event, &Thread->FirstOpenDataBlock);
                    
                    debug_parsed_name ParsedName = DebugParseName(Event->GUID, Event->Name);
                    DebugBlock->Group =
                        GetGroupForHierarchicalName(DebugState, DefaultParentGroup, ParsedName.Name, true);
                } break;
                
                case DebugType_CloseDataBlock:
                {
                    if(Thread->FirstOpenDataBlock)
                    {
                        open_debug_block *MatchingBlock = Thread->FirstOpenDataBlock;
                        Assert(Thread->ID == Event->ThreadID);
                        DeallocateOpenDebugBlock(DebugState, &Thread->FirstOpenDataBlock);
                    }
                } break;
                
                case DebugType_SetHUD:
                {
                    u32 DevMode = Event->Value_u32;
                    if(DevMode < ArrayCount(DebugState->DevModeLinks))
                    {
                        DebugState->DevModeLinks[DevMode] = DefaultParentGroup;
                        SetExpand(DebugState, DebugIDFromLink(&DebugState->HUDTree, DefaultParentGroup), true);
                    }
                } break;
                
                case DebugType_ArenaSetName:
                {
                    DEBUGArenaSetName(DebugState, Event);
                } break;
                
                case DebugType_ArenaBlockFree:
                {
                    DEBUGArenaBlockFree(DebugState, Event);
                } break;
                
                case DebugType_ArenaBlockAllocate:
                {
                    DEBUGArenaBlockAllocate(DebugState, Event);
                } break;
                
                case DebugType_ArenaBlockTruncate:
                {
                    DEBUGArenaBlockTruncate(DebugState, Event);
                } break;
                
                case DebugType_ArenaAllocate:
                {
                    DEBUGArenaAllocate(DebugState, Event);
                } break;
                
                default:
                {
                    debug_element *Element = GetElementFromEvent(DebugState, Event, DefaultParentGroup,  DebugElement_AddToGroup|DebugElement_CreateHierarchy);
                    Element->OriginalGUID = Event->GUID;
                    StoreEvent(DebugState, Element, Event);
                } break;
            }
        }
    }
}

internal debug_state *
DEBUGInit(game_assets *Assets)
{
    debug_state *DebugState = BootstrapPushStruct(debug_state, DebugArena);
    DEBUG_ARENA_SUPPRESS(&DebugState->DebugArena, "DEBUG");
    
    DebugState->CollationFrameOrdinal = 1;
    DebugState->TreeSentinel.Next = &DebugState->TreeSentinel;
    DebugState->TreeSentinel.Prev = &DebugState->TreeSentinel;
    
    DebugState->RootGroup = CreateVariableLink(DebugState,
                                               CreateNameElement(DebugState, 4, "Root"));
    DebugState->FunctionGroup = CreateVariableLink(DebugState,
                                                   CreateNameElement(DebugState, 9, "Functions"));
    
    DebugState->ProfileGroup = CreateVariableLink(DebugState,
                                                  CreateNameElement(DebugState, 7, "Profile"));
    DebugState->MemoryGroup = CreateVariableLink(DebugState,
                                                 CreateNameElement(DebugState, 6, "Memory"));
    
    debug_event RootProfileEvent = {};
    RootProfileEvent.GUID = DEBUG_NAME("RootProfile");
    RootProfileEvent.Name = "RootProfile";
    DebugState->RootProfileElement = GetElementFromEvent(DebugState, &RootProfileEvent, 0, 0);
    
    // TODO(casey): Stop doing any of the placement here!  It should be all done at draw time.
    v2 RenderDim = {1920, 1080};
    AddTree(DebugState, DebugState->RootGroup, V2(-0.5f*RenderDim.Width, 0.5f*RenderDim.Height));
    AddTree(DebugState, DebugState->FunctionGroup, V2(0.0f*RenderDim.Width, 0.5f*RenderDim.Height));
    
    InitializeUI(&DebugState->DevUI, Assets);
    
    DebugState->DevModeLinks[DevMode_profiling] = DebugState->ProfileGroup;
    DebugState->DevModeLinks[DevMode_rendering] = 0;
    DebugState->DevModeLinks[DevMode_lighting] = 0;
    DebugState->DevModeLinks[DevMode_memory] = DebugState->MemoryGroup;
    DebugState->DevModeLinks[DevMode_dump] = DebugState->RootGroup;
    
    return(DebugState);
}

internal void
DEBUGStart(debug_state *DebugState, game_render_commands *Commands,
           game_assets *Assets, game_input *Input)
{
    TIMED_FUNCTION();
    
    if(!DebugState->Paused)
    {
        DebugState->ViewingFrameOrdinal = DebugState->MostRecentFrameOrdinal;
    }
    
    BeginUIFrame(&DebugState->DevUI, Assets, Commands, Input);
}

internal void
DEBUGEnd(debug_state *DebugState, game_input *Input, game_memory *Memory)
{
    TIMED_FUNCTION();
    
    dev_ui *UI = &DebugState->DevUI;
    render_group *RenderGroup = &UI->RenderGroup;
    
    // DrawTrees(DebugState, UI->MouseP);
    
    if(Memory)
    {
        game_state *GameState = Memory->GameState;
        if(GameState->DevMode < ArrayCount(DebugState->DevModeLinks))
        {
            DrawTree(DebugState, &DebugState->HUDTree, V2(UI->UISpace.Min.x, UI->UISpace.Max.y),
                     DebugState->DevModeLinks[GameState->DevMode]);
        }
    }
    
    DEBUGInteract(DebugState, Input, UI->MouseP, UI->dMouseP);
    EndUIFrame(&DebugState->DevUI);
}

extern "C" DEBUG_GAME_FRAME_END(DEBUGGameFrameEnd)
{
    ZeroStruct(GlobalDebugTable->EditEvent);
    
    GlobalDebugTable->CurrentEventArrayIndex = !GlobalDebugTable->CurrentEventArrayIndex;
    u64 ArrayIndex_EventIndex = AtomicExchangeU64(&GlobalDebugTable->EventArrayIndex_EventIndex,
                                                  (u64)GlobalDebugTable->CurrentEventArrayIndex << 32);
    
    u32 EventArrayIndex = ArrayIndex_EventIndex >> 32;
    Assert(EventArrayIndex <= 1);
    u32 EventCount = ArrayIndex_EventIndex & 0xFFFFFFFF;
    
    if(!Memory->DebugState)
    {
        game_assets *Assets = DEBUGGetGameAssets(Memory);
        Memory->DebugState = DEBUGInit(Assets);
    }
    
    debug_state *DebugState = Memory->DebugState;
    if(DebugState)
    {
        game_assets *Assets = DEBUGGetGameAssets(Memory);
        
        if(RenderCommands)
        {
            DEBUGStart(DebugState, RenderCommands, Assets, Input);
        }
        
        CollateDebugRecords(DebugState, EventCount, GlobalDebugTable->Events[EventArrayIndex]);
        
        if(RenderCommands)
        {
            DEBUGEnd(DebugState, Input, Memory);
        }
    }
}
