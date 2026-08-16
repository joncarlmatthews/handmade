/* ========================================================================
   $File: C:\work\handmade\tags\handmade_tokenizer.cpp $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright by Molly Rocket, Inc., All Rights Reserved. $
   ======================================================================== */

internal string
GetTokenTypeName(token_type Type)
{
    switch(Type)
    {
        case Token_OpenParen: {return(BundleZ("open parentheses"));}
        case Token_CloseParen: {return(BundleZ("close parentheses"));}
        case Token_Colon: {return(BundleZ("colon"));}
        case Token_Semicolon: {return(BundleZ("semicolon"));}
        case Token_Asterisk: {return(BundleZ("asterisk"));}
        case Token_OpenBracket: {return(BundleZ("open bracket"));}
        case Token_CloseBracket: {return(BundleZ("close bracket"));}
        case Token_OpenBrace: {return(BundleZ("open brace"));}
        case Token_CloseBrace: {return(BundleZ("close brace"));}
        case Token_Equals: {return(BundleZ("equals"));}
        case Token_Comma: {return(BundleZ("comma"));}
        case Token_Or: {return(BundleZ("or"));}
        case Token_Pound: {return(BundleZ("pound"));}
        case Token_String: {return(BundleZ("string"));}
        case Token_Identifier: {return(BundleZ("identifier"));}
        case Token_Number: {return(BundleZ("number"));}
        case Token_Spacing: {return(BundleZ("whitespace"));}
        case Token_EndOfLine: {return(BundleZ("end of line"));}
        case Token_Comment: {return(BundleZ("comment"));}
        case Token_EndOfStream: {return(BundleZ("end of stream"));}
    }
    
    return(BundleZ("unknown"));
};

internal b32x
Parsing(tokenizer *Tokenizer)
{
    b32x Result = (!Tokenizer->Error);
    return(Result);
}

internal void
ErrorArgList(tokenizer *Tokenizer, token OnToken, char *Format, va_list ArgList)
{
    // TODO(casey): In the future, if we want to, we can change stream_chunk to
    // take strings and then we could pass the OnToken filename/line here.
    Outf(Tokenizer->ErrorStream, "\\#f00%S(%u,%u)\\#fff: \"%S\" - ", OnToken.FileName, OnToken.LineNumber, OnToken.ColumnNumber, OnToken.Text);
    OutfArgList(DEBUG_MEMORY_NAME("ErrorArgList") Tokenizer->ErrorStream, Format, ArgList);
    Outf(Tokenizer->ErrorStream, "\n");
    
    Tokenizer->Error = true;
}

internal void
Error(tokenizer *Tokenizer, token OnToken, char *Format, ...)
{
    va_list ArgList;
    va_start(ArgList, Format);
    
    ErrorArgList(Tokenizer, OnToken, Format, ArgList);
    
    va_end(ArgList);
}

internal void
Error(tokenizer *Tokenizer, char *Format, ...)
{
    va_list ArgList;
    va_start(ArgList, Format);
    
    token OnToken = PeekTokenRaw(Tokenizer);
    Error(Tokenizer, OnToken, Format, ArgList);
    
    va_end(ArgList);
}

internal void
Refill(tokenizer *Tokenizer)
{
    if(Tokenizer->Input.Count == 0)
    {
        Tokenizer->At[0] = 0;
        Tokenizer->At[1] = 0;
    }
    else if(Tokenizer->Input.Count == 1)
    {
        Tokenizer->At[0] = Tokenizer->Input.Data[0];
        Tokenizer->At[1] = 0;
    }
    else
    {
        Tokenizer->At[0] = Tokenizer->Input.Data[0];
        Tokenizer->At[1] = Tokenizer->Input.Data[1];
    }
}

internal void
AdvanceChars(tokenizer *Tokenizer, u32 Count)
{
    Tokenizer->ColumnNumber += Count;
    Advance(&Tokenizer->Input, Count);
    Refill(Tokenizer);
}

internal b32x
TokenEquals(token Token, char *Match)
{
    b32x Result = StringsAreEqual(Token.Text, Match);
    return(Result);
}

internal b32x
IsValid(token Token)
{
    b32x Result = (Token.Type != Token_Unknown);
    return(Result);
}

internal token
GetTokenRaw(tokenizer *Tokenizer)
{
    token Token = {};
    Token.FileName = Tokenizer->FileName;
    Token.ColumnNumber = Tokenizer->ColumnNumber;
    Token.LineNumber = Tokenizer->LineNumber;
    Token.Text = Tokenizer->Input;
    
    char C = Tokenizer->At[0];
    AdvanceChars(Tokenizer, 1);
    switch(C)
    {
        case '\0': {Token.Type = Token_EndOfStream;} break;
        
        case '(': {Token.Type = Token_OpenParen;} break;
        case ')': {Token.Type = Token_CloseParen;} break;
        case ':': {Token.Type = Token_Colon;} break;
        case ';': {Token.Type = Token_Semicolon;} break;
        case '*': {Token.Type = Token_Asterisk;} break;
        case '[': {Token.Type = Token_OpenBracket;} break;
        case ']': {Token.Type = Token_CloseBracket;} break;
        case '{': {Token.Type = Token_OpenBrace;} break;
        case '}': {Token.Type = Token_CloseBrace;} break;
        case '=': {Token.Type = Token_Equals;} break;
        case ',': {Token.Type = Token_Comma;} break;
        case '|': {Token.Type = Token_Or;} break;
        case '#': {Token.Type = Token_Pound;} break;
        
        case '"':
        {
            Token.Type = Token_String;
            
            while(Tokenizer->At[0] &&
                  Tokenizer->At[0] != '"')
            {
                if((Tokenizer->At[0] == '\\') &&
                   Tokenizer->At[1])
                {
                    AdvanceChars(Tokenizer, 1);
                }
                AdvanceChars(Tokenizer, 1);
            }
            
            if(Tokenizer->At[0] == '"')
            {
                AdvanceChars(Tokenizer, 1);
            }
        } break;
        
        default:
        {
            if(IsSpacing(C))
            {
                Token.Type = Token_Spacing;
                while(IsSpacing(Tokenizer->At[0]))
                {
                    AdvanceChars(Tokenizer, 1);
                }
            }
            else if(IsEndOfLine(C))
            {
                Token.Type = Token_EndOfLine;
                if(((C == '\r') &&
                    (Tokenizer->At[0] == '\n')) ||
                   ((C == '\n') &&
                    (Tokenizer->At[0] == '\r')))
                {
                    AdvanceChars(Tokenizer, 1);
                }
                
                Tokenizer->ColumnNumber = 1;
                ++Tokenizer->LineNumber;
            }
            else if((C == '/') &&
                    (Tokenizer->At[0] == '/'))
            {
                Token.Type = Token_Comment;
                
                AdvanceChars(Tokenizer, 2);
                while(Tokenizer->At[0] && !IsEndOfLine(Tokenizer->At[0]))
                {
                    AdvanceChars(Tokenizer, 1);
                }
            }
            else if((C == '/') &&
                    (Tokenizer->At[0] == '*'))
            {
                Token.Type = Token_Comment;
                
                AdvanceChars(Tokenizer, 2);
                while(Tokenizer->At[0] &&
                      !((Tokenizer->At[0] == '*') &&
                        (Tokenizer->At[1] == '/')))
                {
                    if(((Tokenizer->At[0] == '\r') &&
                        (Tokenizer->At[1] == '\n')) ||
                       ((Tokenizer->At[0] == '\n') &&
                        (Tokenizer->At[1] == '\r')))
                    {
                        AdvanceChars(Tokenizer, 1);
                    }
                    
                    if(IsEndOfLine(Tokenizer->At[0]))
                    {
                        ++Tokenizer->LineNumber;
                    }
                    
                    AdvanceChars(Tokenizer, 1);
                }
                
                if(Tokenizer->At[0] == '*')
                {
                    AdvanceChars(Tokenizer, 2);
                }
            }
            else if(IsAlpha(C))
            {
                Token.Type = Token_Identifier;
                
                while(IsAlpha(Tokenizer->At[0]) ||
                      IsNumber(Tokenizer->At[0]) ||
                      (Tokenizer->At[0] == '_'))
                {
                    AdvanceChars(Tokenizer, 1);
                }
            }
            else if(IsNumber(C))
            {
                f32 Number = (f32)(C - '0');
                
                while(IsNumber(Tokenizer->At[0]))
                {
                    f32 Digit = (f32)(Tokenizer->At[0] - '0');
                    Number = 10.0f*Number + Digit;
                    AdvanceChars(Tokenizer, 1);
                }
                
                if(Tokenizer->At[0] == '.')
                {
                    AdvanceChars(Tokenizer, 1);
                    f32 Coefficient = 0.1f;
                    while(IsNumber(Tokenizer->At[0]))
                    {
                        f32 Digit = (f32)(Tokenizer->At[0] - '0');
                        Number += Coefficient*Digit;
                        Coefficient *= 0.1f;
                        AdvanceChars(Tokenizer, 1);
                    }
                }
                
                Token.Type = Token_Number;
                Token.F32 = Number;
                Token.S32 = RoundReal32ToInt32(Number);
            }
            else
            {
                Token.Type = Token_Unknown;
            }
        } break;
    }
    
    Token.Text.Count = (Tokenizer->Input.Data - Token.Text.Data);
    
    return(Token);
}

internal token
PeekTokenRaw(tokenizer *Tokenizer)
{
    tokenizer Temp = *Tokenizer;
    token Result = GetTokenRaw(Tokenizer);
    *Tokenizer = Temp;
    return(Result);
}

internal token
GetToken(tokenizer *Tokenizer)
{
    token Token;
    for(;;)
    {
        Token = GetTokenRaw(Tokenizer);
        if((Token.Type == Token_Spacing) ||
           (Token.Type == Token_EndOfLine) ||
           (Token.Type == Token_Comment))
        {
            // NOTE(casey): Ignore these when we're getting "real" tokens
        }
        else
        {
            if(Token.Type == Token_String)
            {
                if(Token.Text.Count &&
                   (Token.Text.Data[0] == '"'))
                {
                    ++Token.Text.Data;
                    --Token.Text.Count;
                }
                
                if(Token.Text.Count &&
                   (Token.Text.Data[Token.Text.Count - 1] == '"'))
                {
                    --Token.Text.Count;
                }
            }
            
            break;
        }
    }
    
    return(Token);
}

internal token
PeekToken(tokenizer *Tokenizer)
{
    tokenizer Temp = *Tokenizer;
    token Result = GetToken(&Temp);
    return(Result);
}

internal token
RequireToken(tokenizer *Tokenizer, token_type DesiredType)
{
    token Token = GetToken(Tokenizer);
    if(Token.Type != DesiredType)
    {
        Error(Tokenizer, Token, "Unexpected token type (expected %S)",
              GetTokenTypeName(DesiredType));
    }
    
    return(Token);
}

internal token
RequireIdentifier(tokenizer *Tokenizer, char *Match)
{
    token ID = RequireToken(Tokenizer, Token_Identifier);
    if(!TokenEquals(ID, Match))
    {
        Error(Tokenizer, ID, "Expected \"%s\"", Match);
    }
    
    return(ID);
}

internal token
RequireIntegerRange(tokenizer *Tokenizer, s32 MinValue, s32 MaxValue)
{
    token Token = RequireToken(Tokenizer, Token_Number);
    if(Token.Type == Token_Number)
    {
        if((Token.S32 >= MinValue) &&
           (Token.S32 <= MaxValue))
        {
            // NOTE(casey): Valid
        }
        else
        {
            Error(Tokenizer, Token, "Expected a number between %d and %d", MinValue, MaxValue);
        }
    }
    
    return(Token);
}

internal b32x
OptionalToken(tokenizer *Tokenizer, token_type DesiredType)
{
    token Token = PeekToken(Tokenizer);
    b32x Result = (Token.Type == DesiredType);
    if(Result)
    {
        GetToken(Tokenizer);
    }
    
    return(Result);
}

internal tokenizer
Tokenize(string Input, string FileName)
{
    tokenizer Result = {};
    
    Result.FileName = FileName;
    Result.ColumnNumber = 1;
    Result.LineNumber = 1;
    Result.Input = Input;
    Refill(&Result);
    
    return(Result);
}
