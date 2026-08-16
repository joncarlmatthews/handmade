/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

struct loaded_font;
struct debug_element;
struct debug_variable_link;

enum dev_ui_interaction_type
{
    DevUI_Interaction_None,
    
    DevUI_Interaction_NOP,
    
    DevUI_Interaction_AutoModifyVariable,
    
    DevUI_Interaction_ToggleValue,
    DevUI_Interaction_DragValue,
    DevUI_Interaction_TickValue,
    DevUI_Interaction_TearValue,
    
    DevUI_Interaction_Resize,
    DevUI_Interaction_Move,
    
    DevUI_Interaction_Select,
    
    DevUI_Interaction_ToggleExpansion,
    
    DevUI_Interaction_SetUInt32,
    DevUI_Interaction_SetPointer,
    
    DevUI_Interaction_ImmediateButton,
    DevUI_Interaction_Draggable,
    DevUI_Interaction_PickAsset,
};

enum dev_ui_text_op
{
    DevUI_DrawText,
    DevUI_SizeText,
};

struct dev_ui_interaction
{
    dev_id ID;
    dev_ui_interaction_type Type;
    
    void *Target;
    union
    {
        void *Generic;
        void *Pointer;
        u32 UInt32;
        debug_variable_link *Link;
        v2 *P;
        debug_element *Element;
    };
};

// TODO(casey): This needs some work - need to think about where this lives
// or what we want it to be - it's just for generic tooltipping, I think?
struct tooltip_buffer
{
    umm Size;
    char *Data;
};
struct dev_ui_line_buffer
{
    u32 LineCount;
    char LineText[16][256];
};

enum dev_ui_section_picker_move
{
    DevUI_PickerMove_Init,
    DevUI_PickerMove_Keep,
    DevUI_PickerMove_Prev,
    DevUI_PickerMove_Next,
};
struct dev_ui_section_picker
{
    dev_ui_section_picker_move Move;
    dev_id PrevSectionID;
    dev_id CurrentSectionID;
    string CurrentName;
    b32x CurrentSectionExists;
};

struct dev_ui
{
    font_id FontID;
    hha_font *FontInfo;
    v3 ToolTipTransform;
    v3 TextTransform;
    v3 ShadowTransform;
    v3 UITransform;
    v3 BackingTransform;
    f32 FontScale;
    
    rectangle2 UISpace;
    
    v2 MouseP;
    v2 dMouseP;
    v2 LastMouseP;
    b32 AltUI;
    dev_ui_interaction Interaction;
    
    dev_ui_interaction HotInteraction;
    dev_ui_interaction NextHotInteraction;
    
    //
    // NOTE(casey): Per-frame
    //
    
    dev_ui_line_buffer ToolTips;
    
    render_group RenderGroup; // TODO(casey): Move to dev_ui_layout?
    loaded_font *Font;
    rectangle2 DefaultClipRect;
};

struct dev_ui_edit_block
{
    b32 PrevEditOccurred;
};

struct dev_ui_layout
{
    dev_ui *UI;
    v2 MouseP;
    v2 BaseCorner;
    
    u32 Depth;
    
    v2 At;
    f32 LineAdvance;
    f32 NextYDelta;
    f32 SpacingX;
    f32 SpacingY;
    f32 Thickness;
    
    u32 NoLineFeed;
    b32 LineInitialized;
    
    b32 EditOccurred;
    
    dev_ui_interaction ToExecute;
};

struct dev_ui_layout_element
{
    // NOTE(casey): Storage;
    dev_ui_layout *Layout;
    v2 *Dim;
    v2 *Size;
    dev_ui_interaction Interaction;
    
    // NOTE(casey): Out
    rectangle2 Bounds;
};

internal b32x
AreEqual(dev_id A, dev_id B)
{
    b32x Result = ((A.Value[0].U64 == B.Value[0].U64) &&
                   (A.Value[1].U64 == B.Value[1].U64));
    
    return(Result);
}

internal b32x
IsValid(dev_id A)
{
    b32x Result = (A.Value[0].U64 || A.Value[1].U64);
    return(Result);
}

#define DevIDFromPointer(Ptr) DevIDFromPointer_((Ptr), (char *)(FILE_AND_LINE))
internal dev_id
DevIDFromPointer_(void *Ptr, char *String)
{
    dev_id Result = {Ptr, String};
    return(Result);
}

#define DevIDFromU32s(A, B) DevIDFromU32s_((A), (B), (char *)(FILE_AND_LINE))
internal dev_id
DevIDFromU32s_(u32 A, u32 B, char *String)
{
    dev_id Result;
    Result.Value[0].U32[0] = A;
    Result.Value[0].U32[1] = B;
    Result.Value[1].Void = String;
    
    return(Result);
}

internal void
CopyType(dev_id From, dev_id *To)
{
    To->Value[1].Void = From.Value[1].Void;
}