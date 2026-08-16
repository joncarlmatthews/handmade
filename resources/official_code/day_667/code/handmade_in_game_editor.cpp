/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

/*
f32 BestHit = F32Max;
u32 HitEntityIndex = 0;
u32 HitEntityPiece = 0;
v3 HitP = {};
v3 HitDim = {};
BestHit = tHit;
HitEntityIndex = EntityIndex;
HitEntityPiece = PieceIndex;
HitP = WorldP;
HitDim = WorldRadius;
*/

internal b32x
IsEmpty(in_game_edit *List)
{
    b32x Result = (List->Next == List);
    Assert((Result && (List->Prev == List)) ||
           (!Result && (List->Prev != List)));
    return(Result);
}

internal void
Link(in_game_edit *Prev, in_game_edit *Edit)
{
    Edit->Prev = Prev;
    Edit->Next = Prev->Next;

    Edit->Prev->Next = Edit;
    Edit->Next->Prev = Edit;
}

internal void
Sentinelize(in_game_edit *Edit)
{
    Edit->Prev = Edit;
    Edit->Next = Edit;
}

internal void
Unlink(in_game_edit *Edit)
{
    Edit->Prev->Next = Edit->Next;
    Edit->Next->Prev = Edit->Prev;

    Sentinelize(Edit);
}

internal in_game_edit *
PopFirst(in_game_edit *Prev)
{
    in_game_edit *Result = 0;
    if(Prev->Next != Prev)
    {
        Result = Prev->Next;
        Unlink(Prev->Next);
    }

    return(Result);
}

internal void
PushFirst(in_game_edit *Prev, in_game_edit *Pushed)
{
    Link(Prev, Pushed);
}

internal void
AddHit(editable_hit_test *HitTest, dev_id ID, u32 AssetIndex, f32 SortKey)
{
    Assert(HitTest);

    editable_asset_group *DestGroup = HitTest->DestGroup;

    b32x Added = false;
    for(u32 TestIndex = 0;
        TestIndex < DestGroup->AssetCount;
        ++TestIndex)
    {
        editable_asset *Dest = DestGroup->Assets + TestIndex;
        if(Dest->SortKey > SortKey)
        {
            if(DestGroup->AssetCount < ArrayCount(DestGroup->Assets))
            {
                ++DestGroup->AssetCount;
            }

            for(u32 CopyIndex = DestGroup->AssetCount - 1;
                CopyIndex > TestIndex;
                --CopyIndex)
            {
                DestGroup->Assets[CopyIndex] = DestGroup->Assets[CopyIndex - 1];
            }

            Dest->ID = ID;
            Dest->AssetIndex = AssetIndex;
            Dest->SortKey = SortKey;

            Added = true;
            break;
        }
    }

    if(!Added && (DestGroup->AssetCount < ArrayCount(DestGroup->Assets)))
    {
        editable_asset *Dest = DestGroup->Assets + DestGroup->AssetCount++;

        Dest->ID = ID;
        Dest->AssetIndex = AssetIndex;
        Dest->SortKey = SortKey;
    }
}

internal editable_hit_test
BeginHitTest(in_game_editor *Editor, game_input *Input, dev_mode DevMode)
{
    editable_hit_test Result = {};

    Editor->DevMode = DevMode;

    if(Editor->DevMode == DevMode_editing_assets)
    {
        Result.DestGroup = &Editor->HotGroup;
        Result.DestGroup->AssetCount = 0;

        Result.HighlightColor = V4(1, 1, 0, 1);
        Result.HighlightID = Editor->HighlightID;
        Result.ClipSpaceMouseP = Input->ClipSpaceMouseP.xy;
    }

    Result.Editor = Editor;

    return(Result);
}

internal b32x
ShouldHitTest(editable_hit_test *HitTest)
{
    b32x Result = (HitTest->DestGroup != 0);

    return(Result);
}

internal void
EndHitTest(in_game_editor *Editor, game_input *Input, editable_hit_test *HitTest)
{
}

internal hha_align_point *
AlignPointFromAssetAndIndex(game_assets *Assets, u32 AssetIndex, u32 PointIndex)
{
    asset *Asset = GetAsset(Assets, AssetIndex);
    Assert(Asset->HHA.Type == HHAAsset_Bitmap);
    hha_bitmap *Bitmap = &Asset->HHA.Bitmap;
    hha_align_point *Point = Bitmap->AlignPoints + PointIndex;
    return(Point);
}
internal in_game_edit *
GetOrCreateEditInProgress(in_game_editor *Editor, in_game_edit *Match)
{
    in_game_edit *Result = 0;
    for(in_game_edit *Test = Editor->InProgressSentinel.Next;
        !Result && (Test != &Editor->InProgressSentinel);
        Test = Test->Next)
    {
        if(Match->Type == Test->Type)
        {
            switch(Test->Type)
            {
                case Edit_align_point_edit:
                {
                    if((Match->AlignPointEdit.AssetIndex == Test->AlignPointEdit.AssetIndex) &&
                       (Match->AlignPointEdit.AlignPointIndex == Test->AlignPointEdit.AlignPointIndex))
                    {
                        Result = Test;
                    }
                } break;
            }
        }
    }

    if(!Result)
    {
        Result = PushStruct(&Editor->UndoMemory, in_game_edit);
        *Result = *Match;
        PushFirst(&Editor->InProgressSentinel, Result);
    }

    return(Result);
}

internal asset *
GetAssetForEdit(in_game_editor *Editor, in_game_edit *Edit)
{
    asset *Result = 0;

    switch(Edit->Type)
    {
        case Edit_align_point_edit:
        {
            Result = GetAsset(Editor->Assets, Edit->AlignPointEdit.AssetIndex);
        } break;
    }

    return(Result);
}

internal void
EditAlignPoint(in_game_editor *Editor,
               u32 AssetIndex, u32 PointIndex,
               hha_align_point_type Type, b32x ToParent,
               f32 Size, v2 PPercent)
{
    game_assets *Assets = Editor->Assets;
    hha_align_point *Point = AlignPointFromAssetAndIndex(Assets, AssetIndex, PointIndex);

    in_game_edit Match = {};
    Match.Type = Edit_align_point_edit;
    Match.AlignPointEdit.AssetIndex = AssetIndex;
    Match.AlignPointEdit.AlignPointIndex = PointIndex;
    Match.AlignPointEdit.Change[EditChange_From] = *Point;

    in_game_edit *Edit = GetOrCreateEditInProgress(Editor, &Match);
    SetAlignPoint(Point, Type, ToParent, Size, PPercent);
    Edit->AlignPointEdit.Change[EditChange_To] = *Point;
}

internal void
ApplyEditChange(in_game_editor *Editor, in_game_edit *Edit, in_game_edit_change_type Type)
{
    game_assets *Assets = Editor->Assets;

    switch(Edit->Type)
    {
        case Edit_align_point_edit:
        {
            align_point_edit *AlignEdit = &Edit->AlignPointEdit;
            hha_align_point *Point =
                AlignPointFromAssetAndIndex(Assets, AlignEdit->AssetIndex,
                                            AlignEdit->AlignPointIndex);
            *Point = AlignEdit->Change[Type];
        } break;

        InvalidDefaultCase;
    }
}

internal b32x
ShouldDrawAlignPoint(editable_hit_test *HitTest, u32 APIndex)
{
    b32x Result = false;
    if(APIndex < ArrayCount(HitTest->Editor->DrawAlignPoint))
    {
        Result = HitTest->Editor->DrawAlignPoint[APIndex];
    }

    return(Result);
}

internal b32x
ShouldDrawChildren(editable_hit_test *HitTest, dev_id ParentID)
{
    b32x Result = (HitTest->Editor->DrawChildren ||
                   !AreEqual(HitTest->Editor->HighlightID, ParentID));
    return(Result);
}

internal void
AnnotationLabel(dev_ui_layout *Layout, asset_file *AssetFile, memory_arena *TempArena,
                char *Name, u32 Count, u64 Offset)
{
    BeginRow(Layout);
    string Value = ReadAssetString(AssetFile, TempArena, Count, Offset);
    Labelf(Layout, "%s: %S", Name, Value);
    EndRow(Layout);
}

internal b32x
IsDirty(in_game_editor *Editor)
{
    b32x Result = (Editor->UndoSentinel.Next != Editor->CleanEdit);
    return(Result);
}

internal b32x
UndoAvailable(in_game_editor *Editor)
{
    b32x Result = !IsEmpty(&Editor->UndoSentinel);
    return(Result);
}

internal b32x
RedoAvailable(in_game_editor *Editor)
{
    b32x Result = !IsEmpty(&Editor->RedoSentinel);
    return(Result);
}

internal void
Undo(in_game_editor *Editor)
{
    game_assets *Assets = Editor->Assets;

    in_game_edit *Edit = PopFirst(&Editor->UndoSentinel);
    Assert(Edit);
    ApplyEditChange(Editor, Edit, EditChange_From);
    PushFirst(&Editor->RedoSentinel, Edit);
}

internal void
Redo(in_game_editor *Editor)
{
    game_assets *Assets = Editor->Assets;

    in_game_edit *Edit = PopFirst(&Editor->RedoSentinel);
    Assert(Edit);
    ApplyEditChange(Editor, Edit, EditChange_To);
    PushFirst(&Editor->UndoSentinel, Edit);
}

internal void
SetActiveAsset(in_game_editor *Editor, editable_asset *Asset)
{
    if(Asset)
    {
        Editor->HighlightID = Asset->ID;
        Editor->ActiveAssetIndex = Asset->AssetIndex;
    }
    else
    {
        ZeroStruct(Editor->HighlightID);
        Editor->ActiveAssetIndex = 0;
    }
}

internal b32x
EditExistsIn(in_game_edit *Sentinel, in_game_edit *Match)
{
    b32x Result = (Sentinel == Match);

    for(in_game_edit *Test = Sentinel->Next;
        !Result && (Test != Sentinel);
        Test = Test->Next)
    {
        Result = (Test == Match);
    }

    return(Result);
}

internal in_game_edit *
FindCleanEditList(in_game_editor *Editor)
{
    in_game_edit *Result = 0;

    if(EditExistsIn(&Editor->UndoSentinel, Editor->CleanEdit))
    {
        Result = &Editor->UndoSentinel;
    }
    else
    {
        Result = &Editor->RedoSentinel;
        Assert(EditExistsIn(&Editor->RedoSentinel, Editor->CleanEdit));
    }

    return(Result);
}

internal void
SaveAllChanges(in_game_editor *Editor)
{
#if 0
    in_game_edit *List = FindCleanEditList(Editor);
    for(in_game_edit *Edit = List->Next;
        Edit != Editor->CleanEdit;
        Edit = Edit->Next)
    {
        asset *Asset = GetAssetForEdit(Editor, Edit);
        if(Asset)
        {
            asset_file *File = GetFile(Editor->Assets, Asset->FileIndex);
            if(File)
            {
                File->Modified = true;
            }
        }
    }
#endif

    Editor->CleanEdit = Editor->UndoSentinel.Next;
    SynchronizeAssetFileChanges(Editor->Assets, true);
}

internal void
CameraEditor(dev_ui_layout *Layout, game_mode_world *WorldMode)
{
    BeginRow(Layout);
    if(Button(Layout, DevIDFromPointer(WorldMode), BundleZ("SAVE STARTUP LOCATION \\fb03")))
    {
    }

    if(Button(Layout, DevIDFromPointer(WorldMode), BundleZ("LOAD STARTUP LOCATION")))
    {
    }
    EndRow(Layout);

    EditablePx(Layout, DevIDFromPointer(WorldMode), BundleZ("FogMin"), 0.0f, &WorldMode->FogMin, 50.0f);
    EditablePx(Layout, DevIDFromPointer(WorldMode), BundleZ("FogSpan"), 0.0f, &WorldMode->FogSpan, 50.0f);
    EditablePx(Layout, DevIDFromPointer(WorldMode), BundleZ("AlphaMin"), 0.0f, &WorldMode->AlphaMin, 50.0f);
    EditablePx(Layout, DevIDFromPointer(WorldMode), BundleZ("AlphaSpan"), 0.0f, &WorldMode->AlphaSpan, 50.0f);
}

internal void
CollisionEditor(dev_ui_layout *Layout, game_mode_world *WorldMode)
{
#if HANDMADE_INTERNAL
    Labelf(Layout, "Collision Recorder");
    EditablePx(Layout, DevIDFromPointer(WorldMode), BundleZ("TimeStep"), 0.01f, &WorldMode->TimeStepScale, 10.0f);

    EditablePx(Layout, DevIDFromPointer(WorldMode), BundleZ("Capture Frames"), 0.0f,
                   &GlobalDiagramGroup.CaptureFrameCount, 
                   32.0, 0.01f);
    
    if(Button(Layout, DevIDFromPointer(WorldMode), BundleZ("Disable Diagrams")))
    {
        DIAGRAM_Reset();
        DIAGRAM_Capture(true);
    }
    
    b32 IsCaptured = DIAGRAM_IsCaptured();
    if(Button(Layout, DevIDFromPointer(WorldMode), IsCaptured ? BundleZ("Resume") : BundleZ("Capture")))
    {
        DIAGRAM_Capture(!IsCaptured);
    }

    if(Button(Layout, DevIDFromPointer(WorldMode), 0, BundleZ("Single Stepping"), true,
                     WorldMode->SingleStep ? V4(0, 1, 0, 1) : V4(1, 0, 0, 1)))
    {
        WorldMode->SingleStep = !WorldMode->SingleStep;
    }
    
    if(Button(Layout, DevIDFromPointer(WorldMode), BundleZ("Step")))
    {
        WorldMode->StepLatch = true;
    }

    if(GlobalDiagramGroup.DiagramFilterDepth)
    {
        Labelf(Layout, "Diagram Filter: %u/%u", 
                   GlobalDiagramGroup.DiagramFilterDepth,
                   (u32)GlobalDiagramGroup.DiagramFilterIndex[GlobalDiagramGroup.DiagramFilterDepth - 1]);
    }
    else
    {
        Labelf(Layout, "Diagram Filter: off");
    }
    
    BeginRow(Layout);
    if(Button(Layout, DevIDFromPointer(WorldMode), BundleZ("Up")))
    {
        if(GlobalDiagramGroup.DiagramFilterDepth > 0)
        {
            --GlobalDiagramGroup.DiagramFilterDepth;
        }
    }
    if(Button(Layout, DevIDFromPointer(WorldMode), BundleZ("Down")))
    {
        if(GlobalDiagramGroup.DiagramFilterDepth < HANDMADE_DIAGRAM_MAX_DEPTH)
        {
            ++GlobalDiagramGroup.DiagramFilterDepth;
        }
    }
    EndRow(Layout);

    if(GlobalDiagramGroup.DiagramFilterDepth)
    {
        EditablePx(Layout, DevIDFromPointer(WorldMode), BundleZ("Diagram"), 0.0f,
                   &GlobalDiagramGroup.DiagramFilterIndex[GlobalDiagramGroup.DiagramFilterDepth - 1], 
                   1000.0f, 0.1f);
    }
#endif
}

internal void
HHAEditor(in_game_editor *Editor, dev_ui_layout *Layout)
{
    game_assets *Assets = Editor->Assets;

    BeginRow(Layout);
    if(Button(Layout, DevIDFromPointer(Editor), BundleZ("RELOAD HHTs"), true))
    {
        SynchronizeAssetFileChanges(Editor->Assets, false);
    }
    if(Button(Layout, DevIDFromPointer(Editor), BundleZ("SAVE HHTs"), true))
    {
        SaveAllChanges(Editor);
    }
    EndRow(Layout);

    BeginRow(Layout);
    EndRow(Layout);

    BeginRow(Layout);
    if(Button(Layout, DevIDFromPointer(Editor), BundleZ("REVERT"), IsDirty(Editor)))
    {
        if(EditExistsIn(&Editor->UndoSentinel, Editor->CleanEdit))
        {
            while(Editor->CleanEdit != Editor->UndoSentinel.Next)
            {
                Undo(Editor);
            }
        }
        else
        {
            Assert(EditExistsIn(&Editor->RedoSentinel, Editor->CleanEdit));
            while(Editor->CleanEdit != Editor->UndoSentinel.Next)
            {
                Redo(Editor);
            }
        }
    }
    EndRow(Layout);

    stream *Stream = &Assets->ErrorStream;
    for(stream_chunk *Chunk = Stream->First;
        Chunk;
        Chunk = Chunk->Next)
    {
        BeginRow(Layout);
        Label(Layout, Chunk->Contents);
        EndRow(Layout);
    }
}

internal void
AssetEditor(in_game_editor *Editor, dev_ui_layout *Layout)
{
    memory_arena TempArena = {};

    game_assets *Assets = Editor->Assets;

    u32 AssetIndex = Editor->ActiveAssetIndex;
    asset *Asset = GetAsset(Assets, AssetIndex);
    if(Asset)
    {
        asset_file *AssetFile = GetFile(Assets, Asset->FileIndex);

        hha_asset *HHA = &Asset->HHA;
        hha_annotation *Anno = &Asset->Annotation;

        switch(HHA->Type)
        {
            case HHAAsset_Bitmap:
            {
                hha_bitmap *Bitmap = &HHA->Bitmap;
                char *SectionName = "Alignment Points";

                EditableBoolean(Layout, DevIDFromPointer(&Editor->DrawChildren), BundleZ("Show Children"), &Editor->DrawChildren);

                for(u32 PointIndex = 0;
                    PointIndex < ArrayCount(Bitmap->AlignPoints);
                    ++PointIndex)
                {
                    hha_align_point *Point = Bitmap->AlignPoints + PointIndex;

                    b32 ToParent = IsToParent(*Point);
                    u32 Type = GetType(*Point);
                    v2 PPercent = GetPPercent(*Point);
                    f32 Size = GetSize(*Point);

                    char PointNameBuf[64];
                    string PointName;
                    PointName.Count = FormatString(sizeof(PointNameBuf), PointNameBuf, "[%d]", PointIndex);
                    PointName.Data = (u8 *)PointNameBuf;

                    BeginRow(Layout);
                    b32 *DrawToggle = &Editor->DrawAlignPoint[PointIndex];
                    if(Button(Layout, DevIDFromPointer(DrawToggle), 0, PointName,
                              (Type != HHAAlign_None),
                              *DrawToggle ? GetDebugColor4(PointIndex) : V4(0.25f, 0.25f, 0.25f, 1.0f)))
                    {
                        *DrawToggle = !*DrawToggle;
                    }

                    if(Type == HHAAlign_None)
                    {
                        if(Button(Layout, DevIDFromPointer(Point), BundleZ("[ADD]")))
                        {
                            EditAlignPoint(Editor, AssetIndex, PointIndex,
                                           HHAAlign_Default, false, 1.0f, V2(0.5f, 0.5f));
                        }
                    }
                    else
                    {
                        if(Button(Layout, DevIDFromPointer(Point), BundleZ("[DEL]")))
                        {
                            EditAlignPoint(Editor, AssetIndex, PointIndex,
                                           HHAAlign_None, false, 1.0f, V2(0.5f, 0.5f));
                        }

                        dev_ui_edit_block ChangeBlock = BeginEditBlock(Layout);
                        EditableBoolean(Layout, DevIDFromPointer(Point), BundleZ("ToPar"), &ToParent);
                        EditableSize(Layout, DevIDFromPointer(Point), BundleZ("S"), &Size);
                        EditablePxy(Layout, DevIDFromPointer(Point), BundleZ("P"),
                                    0.0f, &PPercent.x, 1.0f,
                                    0.0f, &PPercent.y, 1.0f);
                        EditableType(Layout, DevIDFromPointer(Point), BundleZ(""), AlignPointNameFromType((hha_align_point_type)Type), &Type);
                        if(EndEditBlock(Layout, ChangeBlock))
                        {
                            if(Type == 0)
                            {
                                Type = (HHAAlign_Count - 1);
                            }
                            if(Type >= HHAAlign_Count)
                            {
                                Type = 1;
                            }
                            EditAlignPoint(Editor, AssetIndex, PointIndex,
                                           (hha_align_point_type)Type, ToParent, Size, PPercent);
                        }
                    }
                    EndRow(Layout);
                }
            } break;

            case HHAAsset_Sound:
            {
                hha_sound *Sound = &HHA->Sound;
            } break;

            case HHAAsset_Font:
            {
                hha_font *Font = &HHA->Font;
            } break;

            default:
            {
            } break;
        }

        for(u32 TagIndex = HHA->FirstTagIndex;
            TagIndex < HHA->OnePastLastTagIndex;
            ++TagIndex)
        {
            hha_tag *Tag = GetTag(Assets, TagIndex);
            BeginRow(Layout);
            Labelf(Layout, "%S = %f", TagNameFromID((asset_tag_id)Tag->ID), Tag->Value);
            EndRow(Layout);
        }

        AnnotationLabel(Layout, AssetFile, &TempArena, "Source",
                        Anno->SourceFileBaseNameCount,
                        Anno->SourceFileBaseNameOffset);
        AnnotationLabel(Layout, AssetFile, &TempArena, "Name",
                        Anno->AssetNameCount,
                        Anno->AssetNameOffset);
        AnnotationLabel(Layout, AssetFile, &TempArena, "Description",
                        Anno->AssetDescriptionCount,
                        Anno->AssetDescriptionOffset);
        AnnotationLabel(Layout, AssetFile, &TempArena, "Author",
                        Anno->AuthorCount,
                        Anno->AuthorOffset);
        AnnotationLabel(Layout, AssetFile, &TempArena, "Errors",
                        Anno->ErrorStreamCount,
                        Anno->ErrorStreamOffset);
    }

    Clear(&TempArena);
}

internal void
InitializeEditor(in_game_editor *Editor, game_assets *Assets)
{
    Editor->UndoMemory.AllocationFlags |= PlatformMemory_NotRestored;
    Editor->Assets = Assets;

    Sentinelize(&Editor->UndoSentinel);
    Sentinelize(&Editor->RedoSentinel);
    Sentinelize(&Editor->InProgressSentinel);

    Editor->CleanEdit = Editor->UndoSentinel.Next;

    for(u32 APIndex = 0;
        APIndex < ArrayCount(Editor->DrawAlignPoint);
        ++APIndex)
    {
        Editor->DrawAlignPoint[APIndex] = true;
    }

    Editor->DrawChildren = true;
}

internal void
UpdateAndRenderEditor(in_game_editor *Editor, dev_ui *UI, game_state *GameState)
{
    if((Editor->DevMode >= DevMode_first_editor) &&
       (Editor->DevMode <= DevMode_last_editor))
    {
        dev_ui_layout Layout_ = BeginLayoutBox(UI, RectMinMax(UI->UISpace.Min,
                                                              V2(UI->UISpace.Min.x + 700,
                                                                 UI->UISpace.Max.y)));
        dev_ui_layout *Layout = &Layout_;

        Layout->ToExecute = Editor->ToExecute;

        BeginRow(Layout);
        for(u32 AssetIndexIndex = 0;
            AssetIndexIndex < Editor->ActiveGroup.AssetCount;
            ++AssetIndexIndex)
        {
            editable_asset *EditAsset = Editor->ActiveGroup.Assets + AssetIndexIndex;
            bitmap_id BitmapID = {EditAsset->AssetIndex};
            if(BitmapButton(Layout, EditAsset->ID, EditAsset, 64.0f, 64.0f, BitmapID,
                            true, (EditAsset->AssetIndex == Editor->ActiveAssetIndex)))
            {
                SetActiveAsset(Editor, EditAsset);
            }
        }
        EndRow(Layout);

        BeginRow(Layout);
        if(Button(Layout, DevIDFromPointer(Editor), BundleZ("UNDO"), UndoAvailable(Editor)))
        {
            Undo(Editor);
        }

        if(Button(Layout, DevIDFromPointer(Editor), BundleZ("REDO"), RedoAvailable(Editor)))
        {
            Redo(Editor);
        }

        EndRow(Layout);

        switch(Editor->DevMode)
        {
            case DevMode_editing_assets:
            {
                AssetEditor(Editor, Layout);
            } break;

            case DevMode_camera:
            {
                if(GameState->GameMode == GameMode_World)
                {
                    CameraEditor(Layout, GameState->WorldMode);
                }
            } break;

            case DevMode_editing_hha:
            {
                HHAEditor(Editor, Layout);
            } break;

            case DevMode_collision:
            {
                if(GameState->GameMode == GameMode_World)
                {
                    CollisionEditor(Layout, GameState->WorldMode);
                }
            } break;
        }

        if(IsValid(&Editor->ToExecute) &&
           !IsValid(&Editor->NextToExecute))
        {
            // TODO(casey): Allow "chains" of undos so a single undo can
            // represent many undos
            while(!IsEmpty(&Editor->InProgressSentinel))
            {
                PushFirst(&Editor->UndoSentinel, PopFirst(&Editor->InProgressSentinel));
            }
        }
        Editor->ToExecute = Editor->NextToExecute;
        ZeroStruct(Editor->NextToExecute);
    }
}

internal void
EditorInteract(in_game_editor *Editor, dev_ui *UI, game_input *Input)
{
    if(!IsValid(&UI->NextHotInteraction))
    {
        UI->NextHotInteraction.Type = DevUI_Interaction_PickAsset;
    }

    if(!Input->AltDown)
    {
        u32 TransitionCount = Input->MouseButtons[PlatformMouseButton_Left].HalfTransitionCount;
        b32 MouseButton = Input->MouseButtons[PlatformMouseButton_Left].EndedDown;
        if(TransitionCount % 2)
        {
            MouseButton = !MouseButton;
        }

        for(u32 TransitionIndex = 0;
            TransitionIndex <= TransitionCount;
            ++TransitionIndex)
        {
            b32x MouseMove = false;
            b32x MouseDown = false;
            b32x MouseUp = false;
            if(TransitionIndex == 0)
            {
                MouseMove = true;
            }
            else
            {
                MouseDown = MouseButton;
                MouseUp = !MouseButton;
            }

            b32x EndInteraction = false;

            switch(UI->Interaction.Type)
            {
                case DevUI_Interaction_ImmediateButton:
                case DevUI_Interaction_Draggable:
                {
                    if(MouseUp)
                    {
                        Editor->NextToExecute = UI->Interaction;
                        EndInteraction = true;
                    }
                } break;

                case DevUI_Interaction_PickAsset:
                {
                    if(MouseUp)
                    {
                        Editor->ActiveGroup = Editor->HotGroup;
                        if(Editor->ActiveGroup.AssetCount)
                        {
                            SetActiveAsset(Editor, Editor->HotGroup.Assets + 0);
                        }
                        else
                        {
                            SetActiveAsset(Editor, 0);
                        }

                        EndInteraction = true;
                    }
                } break;

                case DevUI_Interaction_None:
                {
                    UI->HotInteraction = UI->NextHotInteraction;
                    if(MouseDown)
                    {
                        UI->Interaction = UI->HotInteraction;
                    }
                } break;

                default:
                {
                    if(MouseUp)
                    {
                        EndInteraction = true;
                    }
                } break;
            }

            if(EndInteraction)
            {
                ClearInteraction(&UI->Interaction);
            }

            MouseButton = !MouseButton;
        }
    }
}
