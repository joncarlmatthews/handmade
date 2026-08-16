/* ========================================================================
   $File: C:\work\handmade\tags\handmade_tokenizer.h $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright by Molly Rocket, Inc., All Rights Reserved. $
   ======================================================================== */

struct stream;

enum token_type
{
    Token_Unknown,
    
    Token_OpenParen,
    Token_CloseParen,
    Token_Colon,
    Token_Semicolon,
    Token_Asterisk,
    Token_OpenBracket,
    Token_CloseBracket,
    Token_OpenBrace,
    Token_CloseBrace,
    Token_Equals,
    Token_Comma,
    Token_Or,
    Token_Pound,
    
    Token_String,
    Token_Identifier,
    Token_Number,
    
    Token_Spacing,
    Token_EndOfLine,
    Token_Comment,
    
    Token_EndOfStream,
};

struct token
{
    string FileName;
    s32 ColumnNumber;
    s32 LineNumber;
    
    token_type Type;
    string Text;
    f32 F32;
    s32 S32;
};

struct tokenizer
{
    string FileName;
    s32 ColumnNumber;
    s32 LineNumber;
    stream *ErrorStream;
    
    string Input;
    char At[2];
    
    b32x Error;
};

internal b32x Parsing(tokenizer *Tokenizer);
internal void Error(tokenizer *Tokenizer, token OnToken, char *Format, ...);
internal void Error(tokenizer *Tokenizer, char *Format, ...);

internal b32x IsValid(token Token);
internal b32x TokenEquals(token Token, char *Match);
internal token GetTokenRaw(tokenizer *Tokenizer);
internal token PeekTokenRaw(tokenizer *Tokenizer);
internal token GetToken(tokenizer *Tokenizer);
internal token PeekToken(tokenizer *Tokenizer);
internal token RequireToken(tokenizer *Tokenizer, token_type DesiredType);
internal token RequireIdentifier(tokenizer *Tokenizer, char *Match);
internal token RequireIntegerRange(tokenizer *Tokenizer, s32 MinValue, s32 MaxValue);
internal b32x OptionalToken(tokenizer *Tokenizer, token_type DesiredType);
internal tokenizer Tokenize(string Input, string FileName);
