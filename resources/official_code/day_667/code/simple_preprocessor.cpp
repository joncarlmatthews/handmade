/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "handmade_types.h"
#include "handmade_intrinsics.h"
#include "handmade_shared.h"
#include "handmade_stream.h"
#include "handmade_stream.cpp"
#include "handmade_tokenizer.h"
#include "handmade_tokenizer.cpp"

struct meta_struct
{
    char *Name;
    meta_struct *Next;
};
static meta_struct *FirstMetaStruct;

static string
ReadEntireFileIntoMemory(char *FileName)
{
    string Result = {};
    
    FILE *File = fopen(FileName, "r");
    if(File)
    {
        fseek(File, 0, SEEK_END);
        Result.Count = ftell(File);
        fseek(File, 0, SEEK_SET);
        
        Result.Data = (u8 *)malloc(Result.Count);
        fread(Result.Data, Result.Count, 1, File);
        
        fclose(File);
    }
    
    return(Result);
}

static bool
ParseIntrospectionParams(tokenizer *Tokenizer)
{
    bool Valid = true;
    
    for(;;)
    {
        token Token = GetToken(Tokenizer);
        if(TokenEquals(Token, "IGNORED"))
        {
            Valid = false;
            break;
        }
        
        if((Token.Type == Token_CloseParen) ||
           (Token.Type == Token_EndOfStream))
        {
            break;
        }
    }
    
    return(Valid);
}

static void
ParseMember(tokenizer *Tokenizer, token StructTypeToken, token MemberTypeToken)
{
#if 1
    bool Parsing = true;
    bool IsPointer = false;
    while(Parsing)
    {
        token Token = GetToken(Tokenizer);
        switch(Token.Type)
        {
            case Token_Asterisk:
            {
                IsPointer = true;
            } break;
            
            case Token_Identifier:
            {
                printf("   {%s, MetaType_%.*s, \"%.*s\", PointerToU32(&((%.*s *)0)->%.*s)},\n",
                       IsPointer ? "MetaMemberFlag_IsPointer" : "0",
                       (int)MemberTypeToken.Text.Count, MemberTypeToken.Text.Data,
                       (int)Token.Text.Count, Token.Text.Data,
                       (int)StructTypeToken.Text.Count, StructTypeToken.Text.Data,
                       (int)Token.Text.Count, Token.Text.Data);
            } break;
            
            case Token_Semicolon:
            case Token_EndOfStream:
            {
                
                Parsing = false;
            } break;
        }
    }
#else
    token Token = GetToken(Tokenizer);
    switch(Token.Type)
    {
        case Token_Asterisk:
        {
            ParseMember(Tokenizer, Token);
        } break;
        
        case Token_Identifier:
        {
            printf("DEBUG_VALUE(%.*s);\n", (int)Token.Text.Count, Token.Text.Data);
        } break;
    }
#endif
}

static void
ParseStruct(tokenizer *Tokenizer)
{
    token NameToken = GetToken(Tokenizer);
    if(OptionalToken(Tokenizer, Token_OpenBrace))
    {
        printf("member_definition MembersOf_%.*s[] = \n", (int)NameToken.Text.Count, NameToken.Text.Data);
        printf("{\n");
        for(;;)
        {
            token MemberToken = GetToken(Tokenizer);
            if(MemberToken.Type == Token_CloseBrace)
            {
                break;
            }
            else
            {
                ParseMember(Tokenizer, NameToken, MemberToken);
            }
        }
        printf("};\n");
        
        meta_struct *Meta = (meta_struct *)malloc(sizeof(meta_struct));
        Meta->Name = (char *)malloc(NameToken.Text.Count + 1);
        memcpy(Meta->Name, NameToken.Text.Data, NameToken.Text.Count);
        Meta->Name[NameToken.Text.Count] = 0;
        Meta->Next = FirstMetaStruct;
        FirstMetaStruct = Meta;
    }
}

static void
ParseIntrospectable(tokenizer *Tokenizer)
{
    RequireToken(Tokenizer, Token_OpenParen);
    if(Parsing(Tokenizer))
    {
        if(ParseIntrospectionParams(Tokenizer))
        {
            token TypeToken = GetToken(Tokenizer);
            if(TokenEquals(TypeToken, "struct"))
            {
                ParseStruct(Tokenizer);
            }
            else
            {
                fprintf(stderr, "ERROR: Introspection is only supported for structs right now :(\n");
            }
        }
    }
}

int
main(int ArgCount, char **Args)
{
    char *FileNames[] =
    {
        "handmade_sim_region.h",
        "handmade_platform.h",
        "handmade_math.h",
        "handmade_world.h",
    };
    for(int FileIndex = 0;
        FileIndex < (sizeof(FileNames)/sizeof(FileNames[0]));
        ++FileIndex)
    {
        string FileContents = ReadEntireFileIntoMemory(FileNames[FileIndex]);
        tokenizer Tokenizer = Tokenize(FileContents);
        
        bool Parsing = true;
        while(Parsing)
        {
            token Token = GetToken(&Tokenizer);
            switch(Token.Type)
            {
                case Token_EndOfStream:
                {
                    Parsing = false;
                } break;
                
                case Token_Unknown:
                {
                } break;
                
                case Token_Identifier:
                {
                    if(TokenEquals(Token, "introspect"))
                    {
                        ParseIntrospectable(&Tokenizer);
                    }
                } break;
                
                default:
                {
                    //                printf("%d: %.*s\n", Token.Type, (int)Token.Text.Count, Token.Text.Data);
                } break;
            }
        }
    }
    
    printf("#define META_HANDLE_TYPE_DUMP(MemberPtr, NextIndentLevel) \\\n");
    for(meta_struct *Meta = FirstMetaStruct;
        Meta;
        Meta = Meta->Next)
    {
        printf("    case MetaType_%s: {DEBUGTextLine(Member->Name); DEBUGDumpStruct(ArrayCount(MembersOf_%s), MembersOf_%s, MemberPtr, (NextIndentLevel));} break; %s\n",
               Meta->Name, Meta->Name, Meta->Name,
               Meta->Next ? "\\" : "");
    }
}
