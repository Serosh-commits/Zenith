#pragma once

#include "zenith/Lex/Lexer.h"
#include "zenith/Basic/LangOptions.h"
#include "zenith/Basic/SourceManager.h"
#include "zenith/Basic/Diagnostic.h"
#include "zenith/Basic/IdentifierTable.h"
#include "zenith/Basic/FileManager.h"
#include "zenith/Lex/HeaderSearch.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/SmallVector.h"
#include <vector>
#include <memory>
#include <deque>

namespace zenith {

struct MacroInfo {
    SourceLocation DefLoc;
    SourceLocation UndefLoc;
    std::vector<IdentifierInfo*> Params;
    std::vector<Token> ReplacementTokens;
    bool IsObjectLike = true;
    bool IsFunctionLike = false;
    bool IsDisabled = false;
    bool IsUsed = false;

    MacroInfo() = default;
    MacroInfo(SourceLocation DefLoc) : DefLoc(DefLoc) {}

    bool isObjectLike() const { return IsObjectLike; }
    bool isFunctionLike() const { return IsFunctionLike; }
    bool isDisabled() const { return IsDisabled; }
    void setDisabled(bool D) { IsDisabled = D; }

    unsigned getNumParams() const { return Params.size(); }
    const std::vector<IdentifierInfo*> &params() const { return Params; }
    const std::vector<Token> &tokens() const { return ReplacementTokens; }
};

class Preprocessor {
    LangOptions &LangOpts;
    SourceManager &SourceMgr;
    DiagnosticsEngine &Diags;
    IdentifierTable Identifiers;
    HeaderSearch &Headers;
    FileManager &FileMgr;

    std::vector<std::unique_ptr<Lexer>> IncludeStack;
    Lexer *CurLexer = nullptr;

    ::llvm::StringMap<MacroInfo*> Macros;

    struct ActiveMacroScope {
        MacroInfo *MI;
        size_t QueueSizeBefore;
    };

    std::deque<Token> TokenQueue;
    std::vector<ActiveMacroScope> ActiveMacroExpansions;

    bool SkippingUntilDirective = false;

    bool HandleDirective(Token &Result);
    void SkipExcludedConditionalBlock();
    void HandleDefineDirective(Token &Result);
    void HandleUndefDirective(Token &Result);
    void HandleIncludeDirective(Token &Result);
    void HandleIfdefDirective(Token &Result, bool isIfndef);
    void HandleIfDirective(Token &Result);
    void HandleElseDirective(Token &Result);
    void HandleElifDirective(Token &Result);
    void HandleEndifDirective(Token &Result);

    bool ExpandMacro(Token &Result, IdentifierInfo *II, MacroInfo *MI);
    bool drainTokenQueue(Token &Result);
    bool parseInvocationArgs(std::vector<std::vector<Token>> &Args);
    void preExpandArgs(std::vector<std::vector<Token>> &Args);
    void restoreDisabledMacros();
    void enqueueReplacementTokens(MacroInfo *MI, Token &Identifier, const std::vector<std::vector<Token>> &Args);

public:
    Preprocessor(LangOptions &LangOpts, SourceManager &SourceMgr,
                 DiagnosticsEngine &Diags, HeaderSearch &Headers,
                 FileManager &FileMgr);
    ~Preprocessor();

    LangOptions &getLangOpts() const { return LangOpts; }
    SourceManager &getSourceManager() const { return SourceMgr; }
    DiagnosticsEngine &getDiagnostics() const { return Diags; }
    IdentifierTable &getIdentifierTable() { return Identifiers; }
    HeaderSearch &getHeaderSearch() const { return Headers; }
    FileManager &getFileManager() const { return FileMgr; }

    void Lex(Token &Result);
    void EnterMainSourceFile(FileID FID);
    void EnterSourceFile(FileID FID, SourceLocation IncludeLoc);

    MacroInfo *getMacroInfo(IdentifierInfo *II) const;
    void setMacroInfo(IdentifierInfo *II, MacroInfo *MI);

    bool isLexing() const { return CurLexer != nullptr; }
};

}
