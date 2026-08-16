/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

internal b32x
IsValid(dev_ui_interaction *Interaction)
{
    b32x Result = (Interaction->Type != DevUI_Interaction_None);
    return(Result);
}

internal void
ClearInteraction(dev_ui_interaction *Interaction)
{
    Interaction->Type = DevUI_Interaction_None;
    Interaction->Generic = 0;
}

internal rectangle2
TextOp(dev_ui *UI, dev_ui_text_op Op, v2 P, string String, v4 Color = V4(1, 1, 1, 1), r32 AtZ = 0.0f)
{
    render_group *RenderGroup = &UI->RenderGroup;
    loaded_font *Font = UI->Font;
    hha_font *Info = UI->FontInfo;
    f32 FontScale = UI->FontScale;
    v3 ShadowTransform = UI->ShadowTransform;
    v3 TextTransform = UI->TextTransform;
    
    rectangle2 Result = InvertedInfinityRectangle2();
    if(Font)
    {
        u32 PrevCodePoint = 0;
        r32 CharScale = FontScale;
        r32 AtY = P.y;
        r32 AtX = P.x;
        for(umm AtIndex = 0;
            AtIndex < String.Count;
            )
        {
            char At0 = String.Data[AtIndex];
            char At1 = ((AtIndex + 1) < String.Count) ? String.Data[AtIndex + 1] : 0;
            char At2 = ((AtIndex + 2) < String.Count) ? String.Data[AtIndex + 2] : 0;
            char At3 = ((AtIndex + 3) < String.Count) ? String.Data[AtIndex + 3] : 0;
            char At4 = ((AtIndex + 4) < String.Count) ? String.Data[AtIndex + 4] : 0;
            
            if((At0 == '\\') &&
               (At1 == '#') &&
               (At2 != 0) &&
               (At3 != 0) &&
               (At4 != 0))
            {
                r32 CScale = 1.0f / 9.0f;
                Color = V4(Clamp01(CScale*(r32)(At2 - '0')),
                           Clamp01(CScale*(r32)(At3 - '0')),
                           Clamp01(CScale*(r32)(At4 - '0')),
                           1.0f);
                AtIndex += 5;
            }
            else if((At0 == '\\') &&
                    (At1 == '^') &&
                    (At2 != 0))
            {
                r32 CScale = 1.0f / 9.0f;
                CharScale = FontScale*Clamp01(CScale*(r32)(At2 - '0'));
                AtIndex += 3;
            }
            else
            {
                u32 CodePoint = At0;
                if((At0 == '\\') &&
                   (IsHex(At1)) &&
                   (IsHex(At2)) &&
                   (IsHex(At3)) &&
                   (IsHex(At4)))
                {
                    CodePoint = ((GetHex(At1) << 12) |
                                 (GetHex(At2) << 8) |
                                 (GetHex(At3) << 4) |
                                 (GetHex(At4) << 0));
                    AtIndex += 4;
                }
                
                r32 AdvanceX = CharScale*GetHorizontalAdvanceForPair(Info, Font, PrevCodePoint, CodePoint);
                AtX += AdvanceX;
                
                if(CodePoint != ' ')
                {
                    bitmap_id BitmapID = GetBitmapForGlyph(RenderGroup->Assets, Info, Font, CodePoint);
                    hha_bitmap *BitmapInfo = GetBitmapInfo(RenderGroup->Assets, BitmapID);
                    
                    v2 AlignP = GetFirstAlign(BitmapInfo);
                    
                    //  + CharScale*(f32)(Info->AscenderHeight + Info->DescenderHeight - BitmapInfo->Dim[1])
                    r32 BitmapScale = CharScale*(r32)BitmapInfo->Dim[1];
                    v3 BitmapOffset = V3(AtX, AtY, AtZ);
                    if(Op == DevUI_DrawText)
                    {
                        // TODO(casey): Should ShadowTransform include the 2.0f / -2.0f offset?
                        PushBitmap(RenderGroup, BitmapID, BitmapScale,
                                   ShadowTransform + BitmapOffset + V3(2.0f, -2.0f, 0.0f), AlignP, V4(0, 0, 0, 1.0f));
                        PushBitmap(RenderGroup, BitmapID, BitmapScale,
                                   TextTransform + BitmapOffset, AlignP, Color);
                    }
                    else
                    {
                        Assert(Op == DevUI_SizeText);
                        
                        renderer_texture TextureHandle = GetBitmap(RenderGroup->Assets, BitmapID);
                        if(IsValid(TextureHandle))
                        {
                            used_bitmap_dim Dim = GetBitmapDim(RenderGroup, TextureHandle, BitmapScale, BitmapOffset, AlignP);
                            rectangle2 GlyphDim = RectMinDim(Dim.P.xy, Dim.Size);
                            Result = Union(Result, GlyphDim);
                        }
                    }
                }
                
                PrevCodePoint = CodePoint;
                
                ++AtIndex;
            }
        }
    }
    
    return(Result);
}

inline b32
InteractionsAreEqual(dev_ui_interaction A, dev_ui_interaction B)
{
    b32 Result = (AreEqual(A.ID, B.ID) &&
                  (A.Type == B.Type) &&
                  (A.Target == B.Target) &&
                  (A.Generic == B.Generic));
    
    return(Result);
}

inline b32
InteractionIsHot(dev_ui *UI, dev_ui_interaction B)
{
    b32 Result = InteractionsAreEqual(UI->HotInteraction, B);
    
    if(B.Type == DevUI_Interaction_None)
    {
        Result = false;
    }
    
    return(Result);
}

inline void
TextOutAt(dev_ui *UI, v2 P, char *String, v4 Color = V4(1, 1, 1, 1), r32 AtZ = 0.0f)
{
    TextOp(UI, DevUI_DrawText, P, WrapZ(String), Color, AtZ);
}

inline void
TextOutAt(dev_ui *UI, v2 P, string String, v4 Color = V4(1, 1, 1, 1), r32 AtZ = 0.0f)
{
    TextOp(UI, DevUI_DrawText, P, String, Color, AtZ);
}

inline rectangle2
GetTextSize(dev_ui *UI, char *String)
{
    rectangle2 Result = TextOp(UI, DevUI_SizeText, V2(0, 0), WrapZ(String));
    
    return(Result);
}

inline rectangle2
GetTextSize(dev_ui *UI, string String)
{
    rectangle2 Result = TextOp(UI, DevUI_SizeText, V2(0, 0), String);
    
    return(Result);
}

inline rectangle2
GetTextSize(dev_ui *UI, v2 At, char *String)
{
    rectangle2 Result = TextOp(UI, DevUI_SizeText, At, WrapZ(String));
    
    return(Result);
}

inline f32
GetLineAdvance(dev_ui *UI)
{
    f32 Result = UI->FontScale*GetLineAdvanceFor(UI->FontInfo);
    return(Result);
}

inline f32
GetBaseline(dev_ui *UI)
{
    f32 Result = UI->FontScale*GetStartingBaselineY(UI->FontInfo);
    return(Result);
}

inline dev_ui_interaction
SetPointerInteraction(dev_id DebugID, void **Target, void *Value)
{
    dev_ui_interaction Result = {};
    Result.ID = DebugID;
    Result.Type = DevUI_Interaction_SetPointer;
    Result.Target = Target;
    Result.Pointer = Value;
    
    return(Result);
}

inline dev_ui_interaction
SetUInt32Interaction(dev_id DebugID, u32 *Target, u32 Value)
{
    dev_ui_interaction Result = {};
    Result.ID = DebugID;
    Result.Type = DevUI_Interaction_SetUInt32;
    Result.Target = Target;
    Result.UInt32 = Value;
    
    return(Result);
}

inline dev_ui_layout
BeginLayout(dev_ui *UI, v2 UpperLeftCorner)
{
    dev_ui_layout Layout = {};
    Layout.UI = UI;
    Layout.MouseP = UI->MouseP;
    Layout.BaseCorner = Layout.At = UpperLeftCorner;
    Layout.LineAdvance = UI->FontScale*GetLineAdvanceFor(UI->FontInfo);
    Layout.SpacingY = 4.0f;
    Layout.SpacingX = 4.0f;
    Layout.Thickness = 6.0f;
    
    return(Layout);
}

inline void
EndLayout(dev_ui_layout *Layout)
{
}

inline dev_ui_layout_element
BeginElementRectangle(dev_ui_layout *Layout, v2 *Dim)
{
    dev_ui_layout_element Element = {};
    
    Element.Layout = Layout;
    Element.Dim = Dim;
    
    return(Element);
}

inline void
MakeElementSizable(dev_ui_layout_element *Element)
{
    Element->Size = Element->Dim;
}

inline void
DefaultInteraction(dev_ui_layout_element *Element, dev_ui_interaction Interaction)
{
    Element->Interaction = Interaction;
}

inline void
AdvanceElement(dev_ui_layout *Layout, rectangle2 ElRect)
{
    Layout->NextYDelta = Minimum(Layout->NextYDelta, GetMinCorner(ElRect).y - Layout->At.y);
    
    if(Layout->NoLineFeed)
    {
        Layout->At.x = GetMaxCorner(ElRect).x + Layout->SpacingX;
    }
    else
    {
        Layout->At.y += Layout->NextYDelta - Layout->SpacingY;
        Layout->LineInitialized = false;
    }
}

inline void
EndElement(dev_ui_layout_element *Element)
{
    dev_ui_layout *Layout = Element->Layout;
    dev_ui *UI = Layout->UI;
    render_group *RenderGroup = &UI->RenderGroup;
    
    if(!Layout->LineInitialized)
    {
        Layout->At.x = Layout->BaseCorner.x + Layout->Depth*2.0f*Layout->LineAdvance;
        Layout->LineInitialized = true;
        Layout->NextYDelta = 0.0f;
    }
    
    r32 SizeHandlePixels = 4.0f;
    
    v2 Frame = {0, 0};
    if(Element->Size)
    {
        Frame.x = SizeHandlePixels;
        Frame.y = SizeHandlePixels;
    }
    
    v2 TotalDim = *Element->Dim + 2.0f*Frame;
    
    v2 TotalMinCorner = V2(Layout->At.x,
                           Layout->At.y - TotalDim.y);
    v2 TotalMaxCorner = TotalMinCorner + TotalDim;
    
    v2 InteriorMinCorner = TotalMinCorner + Frame;
    v2 InteriorMaxCorner = InteriorMinCorner + *Element->Dim;
    
    rectangle2 TotalBounds = RectMinMax(TotalMinCorner, TotalMaxCorner);
    Element->Bounds = RectMinMax(InteriorMinCorner, InteriorMaxCorner);
    
    if(Element->Interaction.Type && IsInRectangle(Element->Bounds, Layout->MouseP))
    {
        UI->NextHotInteraction = Element->Interaction;
    }
    
    if(Element->Size)
    {
        PushRect(RenderGroup, RectMinMax(V2(TotalMinCorner.x, InteriorMinCorner.y),
                                         V2(InteriorMinCorner.x, InteriorMaxCorner.y)), V3(0, 0, 0),
                 V4(0, 0, 0, 1));
        PushRect(RenderGroup, RectMinMax(V2(InteriorMaxCorner.x, InteriorMinCorner.y),
                                         V2(TotalMaxCorner.x, InteriorMaxCorner.y)), V3(0, 0, 0),
                 V4(0, 0, 0, 1));
        PushRect(RenderGroup, RectMinMax(V2(InteriorMinCorner.x, TotalMinCorner.y),
                                         V2(InteriorMaxCorner.x, InteriorMinCorner.y)), V3(0, 0, 0),
                 V4(0, 0, 0, 1));
        PushRect(RenderGroup, RectMinMax(V2(InteriorMinCorner.x, InteriorMaxCorner.y),
                                         V2(InteriorMaxCorner.x, TotalMaxCorner.y)), V3(0, 0, 0),
                 V4(0, 0, 0, 1));
        
        dev_ui_interaction SizeInteraction = {};
        SizeInteraction.Type = DevUI_Interaction_Resize;
        SizeInteraction.P = Element->Size;
        
        rectangle2 SizeBox = AddRadiusTo(
            RectMinMax(V2(InteriorMaxCorner.x, TotalMinCorner.y),
                       V2(TotalMaxCorner.x, InteriorMinCorner.y)), V2(4.0f, 4.0f));
        PushRect(RenderGroup, SizeBox, V3(0, 0, 0),
                 (InteractionIsHot(UI, SizeInteraction) ? V4(1, 1, 0, 1) : V4(1, 1, 1, 1)));
        if(IsInRectangle(SizeBox, Layout->MouseP))
        {
            UI->NextHotInteraction = SizeInteraction;
        }
    }
    
    AdvanceElement(Layout, TotalBounds);
}

inline dev_ui_layout
BeginLayoutBox(dev_ui *UI, rectangle2 Box, v4 BackdropColor = V4(0, 0, 0, 0.75f))
{
    v2 UpperLeftCorner = V2(Box.Min.x + 6.0f, Box.Max.y - 6.0f);
    dev_ui_layout Layout = BeginLayout(UI, UpperLeftCorner);
    
    if(IsInRectangle(Box, Layout.MouseP))
    {
        UI->NextHotInteraction.Type = DevUI_Interaction_NOP;
    }
    
    PushRect(&UI->RenderGroup, Box, UI->BackingTransform, BackdropColor);
    
    return(Layout);
}

internal v2
BasicTextElement(dev_ui_layout *Layout, string Text, dev_ui_interaction ItemInteraction,
                 v4 ItemColor = V4(0.8f, 0.8f, 0.8f, 1), v4 HotColor = V4(1, 1, 1, 1),
                 r32 Border = 0.0f, v4 BackdropColor = V4(0, 0, 0, 0))
{
    dev_ui *UI = Layout->UI;
    
    rectangle2 TextBounds = GetTextSize(UI, Text);
    v2 Dim = {GetDim(TextBounds).x + 2.0f*Border, Layout->LineAdvance + 2.0f*Border};
    
    dev_ui_layout_element Element = BeginElementRectangle(Layout, &Dim);
    DefaultInteraction(&Element, ItemInteraction);
    EndElement(&Element);
    
    b32 IsHot = InteractionIsHot(UI, ItemInteraction);
    
    if(BackdropColor.w > 0.0f)
    {
        PushRect(&UI->RenderGroup, Element.Bounds, UI->BackingTransform, BackdropColor);
    }
    TextOutAt(UI, V2(GetMinCorner(Element.Bounds).x + Border,
                     GetMaxCorner(Element.Bounds).y - Border - GetBaseline(UI)),
              Text, IsHot ? HotColor : ItemColor);
    
    return(Dim);
}

internal void
BeginRow(dev_ui_layout *Layout)
{
    ++Layout->NoLineFeed;
}

internal void
Label(dev_ui_layout *Layout, string Name)
{
    dev_ui_interaction NullInteraction = {};
    BasicTextElement(Layout, Name, NullInteraction, V4(1, 1, 1, 1), V4(1, 1, 1, 1),
                     Layout->Thickness);
}

internal void
ActionButton(dev_ui_layout *Layout, string Name, dev_ui_interaction Interaction)
{
    BasicTextElement(Layout, Name, Interaction,
                     V4(0.5f, 0.5f, 0.5f, 1.0f), V4(1, 1, 1, 1),
                     Layout->Thickness, V4(0, 0.5f, 1.0f, 1.0f));
}

internal void
BooleanButton(dev_ui_layout *Layout, string Name, b32 Highlight, dev_ui_interaction Interaction)
{
    BasicTextElement(Layout, Name, Interaction,
                     Highlight ? V4(1, 1, 1, 1) : V4(0.5f, 0.5f, 0.5f, 1.0f), V4(1, 1, 1, 1),
                     Layout->Thickness, V4(0.0f, 0.5f, 1.0f, 1.0f));
}

internal void
EndRow(dev_ui_layout *Layout)
{
    Assert(Layout->NoLineFeed > 0);
    --Layout->NoLineFeed;
    
    AdvanceElement(Layout, RectMinMax(Layout->At, Layout->At));
}

internal tooltip_buffer
AddLine(dev_ui_line_buffer *Buffer)
{
    tooltip_buffer Result;
    Result.Size = sizeof(Buffer->LineText[0]);
    if(Buffer->LineCount < ArrayCount(Buffer->LineText))
    {
        Result.Data = Buffer->LineText[Buffer->LineCount++];
    }
    else
    {
        Result.Data = Buffer->LineText[Buffer->LineCount - 1];
    }
    
    return(Result);
}

internal void
DrawLineBuffer(dev_ui *UI, dev_ui_line_buffer *Buffer, dev_ui_layout *Layout)
{
    render_group *RenderGroup = &UI->RenderGroup;
    transient_clip_rect TransientClipRect(RenderGroup, UI->DefaultClipRect);
    
    for(u32 ToolTipIndex = 0;
        ToolTipIndex < Buffer->LineCount;
        ++ToolTipIndex)
    {
        char *Text = Buffer->LineText[ToolTipIndex];
        
        rectangle2 TextBounds = GetTextSize(UI, Text);
        v2 Dim = {GetDim(TextBounds).x, Layout->LineAdvance};
        
        dev_ui_layout_element Element = BeginElementRectangle(Layout, &Dim);
        EndElement(&Element);
        
        PushRect(RenderGroup, AddRadiusTo(Element.Bounds, V2(4.0f, 4.0f)),
                 UI->ToolTipTransform, V4(0, 0, 0, 0.75f));
        
        // TODO(casey): Eventually, it's probably the right thing to go ahead and make
        // Z flow through the debug system sensibly, but for now we just don't care.
        TextOutAt(UI, V2(GetMinCorner(Element.Bounds).x,
                         GetMaxCorner(Element.Bounds).y - GetBaseline(UI)),
                  Text, V4(1, 1, 1, 1), 4000.0f);
        
    }
}

internal b32x
BeginSection(dev_ui_layout *Layout, dev_ui_section_picker *Picker, dev_id SectionID, string Name)
{
    switch(Picker->Move)
    {
        case DevUI_PickerMove_Init:
        {
            Picker->CurrentSectionID = SectionID;
            Picker->Move = DevUI_PickerMove_Keep;
        } break;
        
        case DevUI_PickerMove_Prev:
        {
            if(AreEqual(Picker->CurrentSectionID, SectionID))
            {
                Picker->CurrentSectionID = Picker->PrevSectionID;
                Picker->Move = DevUI_PickerMove_Keep;
                Picker->CurrentSectionExists = true;
            }
        } break;
        
        case DevUI_PickerMove_Next:
        {
            if(AreEqual(Picker->CurrentSectionID, Picker->PrevSectionID))
            {
                Picker->CurrentSectionID = SectionID;
                Picker->Move = DevUI_PickerMove_Keep;
            }
        } break;
    }
    
    b32x Result = AreEqual(Picker->CurrentSectionID, SectionID);
    Picker->PrevSectionID = SectionID;
    
    if(Result)
    {
        Picker->CurrentName = Name;
        Picker->CurrentSectionExists = true;
    }
    
    return(Result);
}

internal void
EndSection(dev_ui_layout *Layout)
{
}

internal void
Labelf(dev_ui_layout *Layout, char *Format, ...)
{
    va_list ArgList;
    va_start(ArgList, Format);
    
    char Temp[64];
    string TempString;
    TempString.Count = FormatStringList(sizeof(Temp), Temp, Format, ArgList);
    TempString.Data = (u8 *)Temp;
    Label(Layout, TempString);
    va_end(ArgList);
}

internal b32x
Button(dev_ui_layout *Layout, dev_id ID, void *Classifier, string LabelText, b32x Enabled,
       v4 BackdropColor = V4(0, 0.35f, 0.7f, 1.0f))
{
    dev_ui_interaction Interaction = {};
    Interaction.ID = ID;
    Interaction.Type = DevUI_Interaction_ImmediateButton;
    Interaction.Target = Classifier;
    
    b32x Result = false;
    if(IsValid(ID))
    {
        Result = InteractionsAreEqual(Interaction, Layout->ToExecute);
    }
    
    if(Enabled)
    {
        BasicTextElement(Layout, LabelText, Interaction,
                         V4(0.75f, 0.75f, 0.75f, 1.0f), V4(1, 1, 1, 1),
                         Layout->Thickness, BackdropColor);
    }
    else
    {
        Interaction.Type = DevUI_Interaction_NOP;
        BasicTextElement(Layout, LabelText, Interaction,
                         V4(0.5f, 0.5f, 0.5f, 1.0f), V4(0.5f, 0.5f, 0.5f, 1),
                         Layout->Thickness, V4(0.25f, 0.25f, 0.25f, 1.0f));
    }
    
    return(Result);
}

internal b32x 
BitmapButton(dev_ui_layout *Layout, dev_id ID, void *Classifier, 
             f32 Width, f32 Height, bitmap_id BitmapID,
             b32x Enabled, b32x Highlighted)
{
    dev_ui_interaction Interaction = {};
    Interaction.ID = ID;
    Interaction.Type = DevUI_Interaction_ImmediateButton;
    Interaction.Target = Classifier;
    
    b32x Result = false;
    if(IsValid(ID))
    {
        Result = InteractionsAreEqual(Interaction, Layout->ToExecute);
    }
    
    dev_ui *UI = Layout->UI;
    render_group *RenderGroup = &UI->RenderGroup;
    f32 Border = Layout->Thickness;
    v2 Dim = {Width + 2.0f*Border, Height + 2.0f*Border};
    
    dev_ui_layout_element Element = BeginElementRectangle(Layout, &Dim);
    DefaultInteraction(&Element, Interaction);
    EndElement(&Element);
    
    b32 IsHot = InteractionIsHot(UI, Interaction);
    
    renderer_texture TextureHandle = GetBitmap(RenderGroup->Assets, BitmapID);
    if(IsValid(TextureHandle))
    {
        v2 DimXY = GetDim(Element.Bounds);
        v3 XAxis = V3(DimXY.x, 0, 0);
        v3 YAxis = V3(0, DimXY.y, 0);
        v3 MinP = V3(GetCenter(Element.Bounds), UI->UITransform.z) - 0.5f*XAxis - 0.5f*YAxis;
        PushSprite(RenderGroup, TextureHandle, MinP, XAxis, YAxis);
    }
    else
    {
        LoadBitmap(RenderGroup->Assets, BitmapID);
    }
    
    if(Highlighted)
    {
        PushRectOutline(&UI->RenderGroup, Element.Bounds, UI->TextTransform, V4(1, 1, 1, 1), 3.0f);
    }
    
    return(Result);
}

internal b32x
Button(dev_ui_layout *Layout, dev_id ID, string LabelText, b32x Enabled = true)
{
    b32x Result = Button(Layout, ID, 0, LabelText, Enabled);
    
    return(Result);
}

internal b32x
Buttonf(dev_ui_layout *Layout, dev_id ID, b32x Enabled, char *Format, ...)
{
    va_list ArgList;
    va_start(ArgList, Format);
    
    char Temp[64];
    string TempString;
    TempString.Count = FormatStringList(sizeof(Temp), Temp, Format, ArgList);
    TempString.Data = (u8 *)Temp;
    Label(Layout, TempString);
    va_end(ArgList);
    
    b32x Result = Button(Layout, ID, TempString, Enabled);
    return(Result);
}

internal dev_ui_edit_block
BeginEditBlock(dev_ui_layout *Layout)
{
    dev_ui_edit_block Result = {};
    
    Result.PrevEditOccurred = Layout->EditOccurred;
    
    return(Result);
}

internal b32x
EndEditBlock(dev_ui_layout *Layout, dev_ui_edit_block Block)
{
    b32x Result = Layout->EditOccurred;
    Layout->EditOccurred = Block.PrevEditOccurred;
    
    return(Result);
}

internal void
EditableBoolean(dev_ui_layout *Layout, dev_id ID, string LabelText, b32 *Value)
{
    char Temp[64];
    string TempString;
    TempString.Count = FormatString(sizeof(Temp), Temp, "%s%S", *Value ? "+" : "-", LabelText);
    TempString.Data = (u8 *)Temp;
    
    if(Button(Layout, ID, 0, TempString, true,
              *Value ? V4(0.1f, 0.5f, 0.1f, 1.0f) : V4(0.5f, 0.1f, 0.1f, 1.0f)))
    {
        *Value = !*Value;
        Layout->EditOccurred = true;
    }
}

internal void
EditableType(dev_ui_layout *Layout, dev_id ID, string LabelText, string ValueName, u32 *Value)
{
    string LeftLabel = ConstZ("<");
    string RightLabel = ConstZ(">");
    
    if(IsValid(LabelText))
    {
        Label(Layout, LabelText);
    }
    
    if(Button(Layout, ID, LeftLabel.Data, LeftLabel, true))
    {
        --(*Value);
        Layout->EditOccurred = true;
    }
    
    if(Button(Layout, ID, RightLabel.Data, RightLabel, true))
    {
        ++(*Value);
        Layout->EditOccurred = true;
    }
    
    Labelf(Layout, "%S", ValueName);
}

internal void
SectionPicker(dev_ui_layout *Layout, dev_ui_section_picker *Picker)
{
    dev_id ID = DevIDFromPointer(Picker);
    
    string LeftLabel = ConstZ("<");
    string RightLabel = ConstZ(">");
    
    if(Button(Layout, ID, LeftLabel.Data, LeftLabel, true))
    {
        Picker->Move = DevUI_PickerMove_Prev;
    }
    
    if(Button(Layout, ID, RightLabel.Data, RightLabel, true))
    {
        Picker->Move = DevUI_PickerMove_Next;
    }
    
    Labelf(Layout, "%S", Picker->CurrentName);
    
    if(!Picker->CurrentSectionExists)
    {
        Picker->Move = DevUI_PickerMove_Init;
    }
    Picker->CurrentSectionExists = false;
}

internal void
EditableSize(dev_ui_layout *Layout, dev_id ID, string LabelText, f32 *Value)
{
    dev_ui *UI = Layout->UI;
    
    char Temp[64];
    string TempString;
    TempString.Count = FormatString(sizeof(Temp), Temp, "%S(%.02f)", LabelText, *Value);
    TempString.Data = (u8 *)Temp;
    
    dev_ui_interaction Interaction = {};
    Interaction.ID = ID;
    Interaction.Type = DevUI_Interaction_Draggable;
    
    BasicTextElement(Layout, TempString, Interaction,
                     V4(0.8f, 0.8f, 0.8f, 1),
                     V4(1, 1, 1, 1), Layout->Thickness,
                     V4(0.7f, 0.5f, 0.3f, 0.5f));
    
    if(InteractionsAreEqual(Interaction, UI->Interaction))
    {
        // TODO(casey): Handle this as a real ODE?
        *Value += 0.001f*(*Value)*UI->dMouseP.y;
        Layout->EditOccurred = true;
    }
}

internal void
EditablePxy(dev_ui_layout *Layout, dev_id ID, string LabelText,
            f32 MinX, f32 *X, f32 MaxX,
            f32 MinY, f32 *Y, f32 MaxY)
{
    dev_ui *UI = Layout->UI;
    
    char Temp[64];
    string TempString;
    TempString.Count = FormatString(sizeof(Temp), Temp, "%S(%.02f,%.02f)", LabelText, *X, *Y);
    TempString.Data = (u8 *)Temp;
    
    dev_ui_interaction Interaction = {};
    Interaction.ID = ID;
    Interaction.Type = DevUI_Interaction_Draggable;
    
    BasicTextElement(Layout, TempString, Interaction,
                     V4(0.8f, 0.8f, 0.8f, 1),
                     V4(1, 1, 1, 1), Layout->Thickness,
                     V4(0.7f, 0.5f, 0.3f, 0.5f));
    
    if(InteractionsAreEqual(Interaction, UI->Interaction))
    {
        f32 dX = 0.001f*(MaxX - MinX);
        f32 dY = 0.001f*(MaxY - MinY);
        
        *X = Clamp(MinX, *X + dX*UI->dMouseP.x, MaxX);
        *Y = Clamp(MinY, *Y + dY*UI->dMouseP.y, MaxY);
        Layout->EditOccurred = true;
    }
}

internal void
EditablePx(dev_ui_layout *Layout, dev_id ID, string LabelText,
           f32 MinX, f32 *X, f32 MaxX, f32 dX = 0)
{
    dev_ui *UI = Layout->UI;
    
    char Temp[64];
    string TempString;
    TempString.Count = FormatString(sizeof(Temp), Temp, "%S(%.02f)", LabelText, *X);
    TempString.Data = (u8 *)Temp;
    
    dev_ui_interaction Interaction = {};
    Interaction.ID = ID;
    Interaction.Type = DevUI_Interaction_Draggable;
    
    BasicTextElement(Layout, TempString, Interaction,
                     V4(0.8f, 0.8f, 0.8f, 1),
                     V4(1, 1, 1, 1), Layout->Thickness,
                     V4(0.7f, 0.5f, 0.3f, 0.5f));
    
    if(InteractionsAreEqual(Interaction, UI->Interaction))
    {
        if(!dX)
        {
            dX = 0.001f*(MaxX - MinX);
        }
        
        *X = Clamp(MinX, *X + dX*UI->dMouseP.x, MaxX);
        Layout->EditOccurred = true;
    }
}

internal void
InitializeUI(dev_ui *UI, game_assets *Assets)
{
    UI->FontID = GetBestMatchFontFrom(Assets, GetTagHash(Asset_Font, Tag_DebugFont));
    
    UI->FontInfo = GetFontInfo(Assets, UI->FontID);
    UI->FontScale = 1.0f;
    
    UI->ToolTipTransform = V3(0, 0, -1000.0f);
    UI->TextTransform = V3(0, 0, -2000.0f);
    UI->ShadowTransform = V3(0, 0, -4000.0f);
    UI->UITransform = V3(0, 0, -3000.0f);
    UI->BackingTransform = V3(0, 0, -5000.0f);
    
    UI->LastMouseP = V2(0, 0);
    UI->AltUI = false;
    ZeroStruct(UI->Interaction);
    ZeroStruct(UI->HotInteraction);
    ZeroStruct(UI->NextHotInteraction);
}

internal void
BeginUIFrame(dev_ui *UI, game_assets *Assets, game_render_commands *Commands,
             game_input *Input)
{
    v2 RegionDim = V2From(Commands->Settings.RenderDim);
    
    UI->RenderGroup = BeginRenderGroup(Assets, Commands, Render_ClearDepth);
    render_group *RenderGroup = &UI->RenderGroup;
    UI->Font = PushFont(RenderGroup, UI->FontID);
    SetCameraTransform(RenderGroup, true, 1.0f,
                       V3(2.0f/RegionDim.Width, 0, 0),
                       V3(0, 2.0f/RegionDim.Width, 0),
                       V3(0, 0, 1),
                       V3(0, 0, 0),
                       -10000.0f, 10000.0f);
    
    UI->UISpace = RectCenterDim(V2(0, 0), RegionDim);
    
    UI->DefaultClipRect = RenderGroup->LastSetup.ClipRect;
    UI->ToolTips.LineCount = 0;
    
    v2 MouseClipP = Input->ClipSpaceMouseP.xy;
    UI->LastMouseP = UI->MouseP;
    UI->AltUI = Input->AltDown;
    UI->MouseP = Unproject(RenderGroup, &RenderGroup->GameXForm, MouseClipP, 0.0f).xy;
    UI->dMouseP = UI->MouseP - UI->LastMouseP;
}

internal void
EndUIFrame(dev_ui *UI)
{
    dev_ui_layout MouseLayout = BeginLayout(UI, UI->MouseP);
    DrawLineBuffer(UI, &UI->ToolTips, &MouseLayout);
    EndLayout(&MouseLayout);
    
    EndRenderGroup(&UI->RenderGroup);
    ZeroStruct(UI->NextHotInteraction);
}