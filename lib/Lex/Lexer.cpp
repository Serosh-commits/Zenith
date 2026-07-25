#include "zenith/Lex/Lexer.h"
#include "zenith/Basic/SourceManager.h"
#include "llvm/Support/MemoryBuffer.h"

namespace zenith {

Lexer::Lexer(FileID FID, const ::llvm::MemoryBuffer *InputFile, SourceManager &SM, const LangOptions &LO)
    : PreprocessorLexer(nullptr, FID), LangOpts(LO) {
    if (!InputFile) InputFile = SM.getBuffer(FID);
    BufferStart = InputFile ? InputFile->getBufferStart() : nullptr;
    BufferEnd = InputFile ? InputFile->getBufferEnd() : nullptr;
    BufferPtr = BufferStart;
    FileLoc = SM.getLocForStartOfFile(FID);
}

Lexer::Lexer(SourceLocation Loc, const LangOptions &LO, const char *BufStart, const char *BufPtr, const char *BufEnd)
    : PreprocessorLexer(nullptr, FileID()), BufferStart(BufStart), BufferEnd(BufEnd), BufferPtr(BufPtr), FileLoc(Loc), LangOpts(LO) {
    LexingRawMode = true;
}

SourceLocation Lexer::getSourceLocation(const char *Loc) const {
    return FileLoc.getLocWithOffset(Loc - BufferStart);
}

char Lexer::getAndAdvanceChar(const char *&Ptr) {
    if (Ptr >= BufferEnd) return '\0';
    return *Ptr++;
}

void Lexer::FormTokenWithChars(Token &Result, const char *TokEnd, tok::TokenKind Kind) {
    unsigned Length = TokEnd - BufferPtr;
    Result.setLength(Length);
    Result.setLocation(getSourceLocation(BufferPtr));
    Result.setKind(Kind);
    if (IsAtStartOfLine) {
        Result.setFlag(Token::StartOfLine);
        IsAtStartOfLine = false;
    }
    if (HasLeadingSpace) {
        Result.setFlag(Token::LeadingSpace);
        HasLeadingSpace = false;
    }
    BufferPtr = TokEnd;
}

bool Lexer::SkipWhitespace(Token &Result, const char *CurPtr) {
    while (CurPtr < BufferEnd) {
        char C = *CurPtr;
        if (C == ' ' || C == '\t' || C == '\f' || C == '\v') {
            HasLeadingSpace = true;
            ++CurPtr;
        } else if (C == '\n' || C == '\r') {
            IsAtStartOfLine = true;
            HasLeadingSpace = false;
            if (C == '\r' && CurPtr + 1 < BufferEnd && CurPtr[1] == '\n')
                ++CurPtr;
            ++CurPtr;
        } else {
            break;
        }
    }
    BufferPtr = CurPtr;
    return CurPtr >= BufferEnd;
}

bool Lexer::SkipLineComment(Token &Result, const char *CurPtr) {
    while (CurPtr < BufferEnd && *CurPtr != '\n' && *CurPtr != '\r') {
        ++CurPtr;
    }
    BufferPtr = CurPtr;
    return false;
}

bool Lexer::SkipBlockComment(Token &Result, const char *CurPtr) {
    while (CurPtr + 1 < BufferEnd) {
        if (CurPtr[0] == '*' && CurPtr[1] == '/') {
            BufferPtr = CurPtr + 2;
            return false;
        }
        if (CurPtr[0] == '\n' || CurPtr[0] == '\r') {
            IsAtStartOfLine = true;
        }
        ++CurPtr;
    }
    BufferPtr = BufferEnd;
    return false;
}

void Lexer::LexIdentifier(Token &Result, const char *CurPtr) {
    const char *TokStart = CurPtr;
    while (CurPtr < BufferEnd) {
        char C = *CurPtr;
        if ((C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z') || (C >= '0' && C <= '9') || C == '_') {
            ++CurPtr;
        } else {
            break;
        }
    }

    ::llvm::StringRef Name(TokStart, CurPtr - TokStart);
    FormTokenWithChars(Result, CurPtr, tok::identifier);

    if (Identifiers && !LexingRawMode) {
        IdentifierInfo &II = Identifiers->get(Name);
        Result.setIdentifierInfo(&II);
        Result.setKind(II.getTokenID());
    } else {
        Result.setKind(tok::raw_identifier);
        Result.setRawIdentifierData(TokStart);
    }
}

void Lexer::LexNumericConstant(Token &Result, const char *CurPtr) {
    const char *TokStart = CurPtr;
    while (CurPtr < BufferEnd) {
        char C = *CurPtr;
        if ((C >= '0' && C <= '9') || (C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z') || C == '.' || C == '_') {
            if ((C == 'e' || C == 'E' || C == 'p' || C == 'P') && CurPtr + 1 < BufferEnd) {
                if (CurPtr[1] == '+' || CurPtr[1] == '-')
                    ++CurPtr;
            }
            ++CurPtr;
        } else {
            break;
        }
    }

    FormTokenWithChars(Result, CurPtr, tok::numeric_constant);
    Result.setLiteralData(TokStart);
}

void Lexer::LexStringLiteral(Token &Result, const char *CurPtr) {
    const char *TokStart = CurPtr;
    ++CurPtr;
    while (CurPtr < BufferEnd) {
        char C = *CurPtr++;
        if (C == '\\') {
            if (CurPtr < BufferEnd) ++CurPtr;
        } else if (C == '"') {
            break;
        }
    }

    FormTokenWithChars(Result, CurPtr, tok::string_literal);
    Result.setLiteralData(TokStart);
}

void Lexer::LexCharConstant(Token &Result, const char *CurPtr) {
    const char *TokStart = CurPtr;
    ++CurPtr;
    while (CurPtr < BufferEnd) {
        char C = *CurPtr++;
        if (C == '\\') {
            if (CurPtr < BufferEnd) ++CurPtr;
        } else if (C == '\'') {
            break;
        }
    }

    FormTokenWithChars(Result, CurPtr, tok::char_constant);
    Result.setLiteralData(TokStart);
}

bool Lexer::Lex(Token &Result) {
    Result.startToken();
    return LexTokenInternal(Result);
}

bool Lexer::LexTokenInternal(Token &Result) {
    const char *CurPtr = BufferPtr;

    if (SkipWhitespace(Result, CurPtr)) {
        FormTokenWithChars(Result, BufferEnd, tok::eof);
        return false;
    }

    CurPtr = BufferPtr;
    if (CurPtr >= BufferEnd) {
        FormTokenWithChars(Result, BufferEnd, tok::eof);
        return false;
    }

    char Char = *CurPtr;

    if ((Char >= 'a' && Char <= 'z') || (Char >= 'A' && Char <= 'Z') || Char == '_') {
        LexIdentifier(Result, CurPtr);
        return true;
    }

    if (Char >= '0' && Char <= '9') {
        LexNumericConstant(Result, CurPtr);
        return true;
    }

    switch (Char) {
    case '"':
        LexStringLiteral(Result, CurPtr);
        return true;
    case '\'':
        LexCharConstant(Result, CurPtr);
        return true;

    case '(': FormTokenWithChars(Result, CurPtr + 1, tok::l_paren); return true;
    case ')': FormTokenWithChars(Result, CurPtr + 1, tok::r_paren); return true;
    case '{': FormTokenWithChars(Result, CurPtr + 1, tok::l_brace); return true;
    case '}': FormTokenWithChars(Result, CurPtr + 1, tok::r_brace); return true;
    case '[': FormTokenWithChars(Result, CurPtr + 1, tok::l_square); return true;
    case ']': FormTokenWithChars(Result, CurPtr + 1, tok::r_square); return true;
    case ';': FormTokenWithChars(Result, CurPtr + 1, tok::semi); return true;
    case ',': FormTokenWithChars(Result, CurPtr + 1, tok::comma); return true;
    case '~': FormTokenWithChars(Result, CurPtr + 1, tok::tilde); return true;
    case '?': FormTokenWithChars(Result, CurPtr + 1, tok::question); return true;
    case '@': FormTokenWithChars(Result, CurPtr + 1, tok::at); return true;

    case '.':
        if (CurPtr + 2 < BufferEnd && CurPtr[1] == '.' && CurPtr[2] == '.') {
            FormTokenWithChars(Result, CurPtr + 3, tok::ellipsis);
        } else if (CurPtr + 1 < BufferEnd && CurPtr[1] >= '0' && CurPtr[1] <= '9') {
            LexNumericConstant(Result, CurPtr);
        } else {
            FormTokenWithChars(Result, CurPtr + 1, tok::period);
        }
        return true;

    case '+':
        if (CurPtr + 1 < BufferEnd && CurPtr[1] == '+')
            FormTokenWithChars(Result, CurPtr + 2, tok::plusplus);
        else if (CurPtr + 1 < BufferEnd && CurPtr[1] == '=')
            FormTokenWithChars(Result, CurPtr + 2, tok::plusequal);
        else
            FormTokenWithChars(Result, CurPtr + 1, tok::plus);
        return true;

    case '-':
        if (CurPtr + 1 < BufferEnd && CurPtr[1] == '-')
            FormTokenWithChars(Result, CurPtr + 2, tok::minusminus);
        else if (CurPtr + 1 < BufferEnd && CurPtr[1] == '=')
            FormTokenWithChars(Result, CurPtr + 2, tok::minusequal);
        else if (CurPtr + 1 < BufferEnd && CurPtr[1] == '>')
            FormTokenWithChars(Result, CurPtr + 2, tok::arrow);
        else
            FormTokenWithChars(Result, CurPtr + 1, tok::minus);
        return true;

    case '*':
        if (CurPtr + 1 < BufferEnd && CurPtr[1] == '=')
            FormTokenWithChars(Result, CurPtr + 2, tok::starequal);
        else
            FormTokenWithChars(Result, CurPtr + 1, tok::star);
        return true;

    case '/':
        if (CurPtr + 1 < BufferEnd && CurPtr[1] == '/') {
            SkipLineComment(Result, CurPtr + 2);
            return LexTokenInternal(Result);
        } else if (CurPtr + 1 < BufferEnd && CurPtr[1] == '*') {
            SkipBlockComment(Result, CurPtr + 2);
            return LexTokenInternal(Result);
        } else if (CurPtr + 1 < BufferEnd && CurPtr[1] == '=') {
            FormTokenWithChars(Result, CurPtr + 2, tok::slashequal);
        } else {
            FormTokenWithChars(Result, CurPtr + 1, tok::slash);
        }
        return true;

    case '%':
        if (CurPtr + 1 < BufferEnd && CurPtr[1] == '=')
            FormTokenWithChars(Result, CurPtr + 2, tok::percentequal);
        else
            FormTokenWithChars(Result, CurPtr + 1, tok::percent);
        return true;

    case '&':
        if (CurPtr + 1 < BufferEnd && CurPtr[1] == '&')
            FormTokenWithChars(Result, CurPtr + 2, tok::ampamp);
        else if (CurPtr + 1 < BufferEnd && CurPtr[1] == '=')
            FormTokenWithChars(Result, CurPtr + 2, tok::ampequal);
        else
            FormTokenWithChars(Result, CurPtr + 1, tok::amp);
        return true;

    case '|':
        if (CurPtr + 1 < BufferEnd && CurPtr[1] == '|')
            FormTokenWithChars(Result, CurPtr + 2, tok::pipepipe);
        else if (CurPtr + 1 < BufferEnd && CurPtr[1] == '=')
            FormTokenWithChars(Result, CurPtr + 2, tok::pipeequal);
        else
            FormTokenWithChars(Result, CurPtr + 1, tok::pipe);
        return true;

    case '^':
        if (CurPtr + 1 < BufferEnd && CurPtr[1] == '=')
            FormTokenWithChars(Result, CurPtr + 2, tok::caretequal);
        else
            FormTokenWithChars(Result, CurPtr + 1, tok::caret);
        return true;

    case '!':
        if (CurPtr + 1 < BufferEnd && CurPtr[1] == '=')
            FormTokenWithChars(Result, CurPtr + 2, tok::exclaimequal);
        else
            FormTokenWithChars(Result, CurPtr + 1, tok::exclaim);
        return true;

    case '=':
        if (CurPtr + 1 < BufferEnd && CurPtr[1] == '=')
            FormTokenWithChars(Result, CurPtr + 2, tok::equalequal);
        else
            FormTokenWithChars(Result, CurPtr + 1, tok::equal);
        return true;

    case '<':
        if (CurPtr + 1 < BufferEnd && CurPtr[1] == '=')
            FormTokenWithChars(Result, CurPtr + 2, tok::lessequal);
        else if (CurPtr + 1 < BufferEnd && CurPtr[1] == '<') {
            if (CurPtr + 2 < BufferEnd && CurPtr[2] == '=')
                FormTokenWithChars(Result, CurPtr + 3, tok::lesslessequal);
            else
                FormTokenWithChars(Result, CurPtr + 2, tok::lessless);
        } else
            FormTokenWithChars(Result, CurPtr + 1, tok::less);
        return true;

    case '>':
        if (CurPtr + 1 < BufferEnd && CurPtr[1] == '=')
            FormTokenWithChars(Result, CurPtr + 2, tok::greaterequal);
        else if (CurPtr + 1 < BufferEnd && CurPtr[1] == '>') {
            if (CurPtr + 2 < BufferEnd && CurPtr[2] == '=')
                FormTokenWithChars(Result, CurPtr + 3, tok::greatergreaterequal);
            else
                FormTokenWithChars(Result, CurPtr + 2, tok::greatergreater);
        } else
            FormTokenWithChars(Result, CurPtr + 1, tok::greater);
        return true;

    case ':':
        if (CurPtr + 1 < BufferEnd && CurPtr[1] == ':')
            FormTokenWithChars(Result, CurPtr + 2, tok::coloncolon);
        else
            FormTokenWithChars(Result, CurPtr + 1, tok::colon);
        return true;

    case '#':
        if (CurPtr + 1 < BufferEnd && CurPtr[1] == '#')
            FormTokenWithChars(Result, CurPtr + 2, tok::hashhash);
        else
            FormTokenWithChars(Result, CurPtr + 1, tok::hash);
        return true;

    default:
        FormTokenWithChars(Result, CurPtr + 1, tok::unknown);
        return true;
    }
}

unsigned Lexer::MeasureTokenLength(SourceLocation Loc, const SourceManager &SM,
                                   const LangOptions &LangOpts) {
    const char *Buffer = SM.getCharacterData(Loc);
    if (!Buffer) return 0;
    const char *Ptr = Buffer;
    while (*Ptr && isObviouslySimpleCharacter(*Ptr) && *Ptr != ' ' && *Ptr != '\t' && *Ptr != '\n') {
        ++Ptr;
    }
    return Ptr - Buffer;
}

::llvm::StringRef Lexer::getSpelling(const Token &Tok, const SourceManager &SM,
                                     const LangOptions &LangOpts) {
    if (Tok.is(tok::identifier))
        return Tok.getIdentifierInfo()->getName();
    if (Tok.is(tok::raw_identifier))
        return Tok.getRawIdentifier();
    if (const char *Punct = tok::getPunctuatorSpelling(Tok.getKind()))
        return Punct;
    if (const char *Kw = tok::getKeywordSpelling(Tok.getKind()))
        return Kw;
    return ::llvm::StringRef(SM.getCharacterData(Tok.getLocation()), Tok.getLength());
}

}
