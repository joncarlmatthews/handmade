/* ========================================================================
   $File: $
   $Date: 2007/09/23 05:44:28PM $
   $Revision: 22 $
   $Creator: Casey Muratori $
   $Notice: $
   ======================================================================== */

typedef char int8;
typedef char unsigned uint8;
typedef int int32;
typedef int int32x;
typedef int unsigned uint32;
typedef int unsigned uint32x;
typedef int bool32;
typedef int bool32x;

union file_time
{
    FILETIME Win;
    struct
    {
        __int64 Raw;
    };
};

struct display_strings
{
    char TitleLine[256];
    char BiLine[256];
    char CornerTag[256];
};

enum menu_action_type
{
    MenuAction_None,
    MenuAction_StartSession,
    MenuAction_SetStreamTitle,
    MenuAction_RefreshSettings,
    MenuAction_CloseMenu,
    MenuAction_EndSession,
    MenuAction_ExitCTray,
    MenuAction_EditDisplayTopic,
};
struct menu_action
{
    menu_action_type Type;
    union
    {
        int TitleIndex;
        file_time Hour;
    };
};

struct menu_action_stack
{
    int ActionCount;
    menu_action Actions[512];
};
