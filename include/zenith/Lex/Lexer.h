#pragma once

#include "zenith/Lex/PreprocessorLexer.h"
#include "zenith/Basic/SourceLocation.h"
#include "zenith/Basic/LangOptions.h"
#include "zenith/Basic/SourceManager.h"
#include "zenith/Basic/IdentifierTable.h"
#include "llvm/Support/MemoryBuffer.h"

namespace zenith {

class Lexer : public PreprocessorLexer {
    const char *BufferStart = nullptr;
    const char *BufferEnd = nullptr;
    const char *BufferPtr = nullptr;
    SourceLocation FileLoc;
    const LangOptions &LangOpts;
    bool IsAtStartOfLine = true;
    bool HasLeadingSpace = false;
    IdentifierTable *Identifiers = nullptr;

    bool LexTokenInternal(Token &Result);
    void FormTokenWithChars(Token &Result, const char *TokEnd, tok::TokenKind Kind);
    char getAndAdvanceChar(const char *&Ptr);
    void LexIdentifier(Token &Result, const char *CurPtr);
    void LexNumericConstant(Token &Result, const char *CurPtr);
    void LexStringLiteral(Token &Result, const char *CurPtr);
    void LexCharConstant(Token &Result, const char *CurPtr);
    bool SkipWhitespace(Token &Result, const char *CurPtr);
    bool SkipLineComment(Token &Result, const char *CurPtr);
    bool SkipBlockComment(Token &Result, const char *CurPtr);

    static bool isObviouslySimpleCharacter(char C) {
        return (C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z') ||
               (C >= '0' && C <= '9') || C == '_' || C == ' ' || C == '\t';
    }

public:
    Lexer(FileID FID, const ::llvm::MemoryBuffer *InputFile, SourceManager &SM, const LangOptions &LO);
    Lexer(SourceLocation FileLoc, const LangOptions &LO, const char *BufStart, const char *BufPtr, const char *BufEnd);

    bool Lex(Token &Result);
    void IndirectLex(Token &Result) override { Lex(Result); }

    void setIdentifierTable(IdentifierTable *IT) { Identifiers = IT; }

    SourceLocation getSourceLocation(const char *Loc) const;
    SourceLocation getSourceLocation() override { return getSourceLocation(BufferPtr); }

    const char *getBufferPtr() const { return BufferPtr; }
    const char *getBufferStart() const { return BufferStart; }
    const char *getBufferEnd() const { return BufferEnd; }

    static unsigned MeasureTokenLength(SourceLocation Loc, const SourceManager &SM,
                                       const LangOptions &LangOpts);

    static ::llvm::StringRef getSpelling(const Token &Tok, const SourceManager &SM,
                                         const LangOptions &LangOpts);
};

}
