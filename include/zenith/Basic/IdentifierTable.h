#pragma once
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "zenith/Lex/TokenKinds.h"

namespace zenith {

class LangOptions;

class IdentifierInfo {
    ::llvm::StringRef Name;
    tok::TokenKind TokenID = tok::identifier;
    bool IsKeyword = false;
    bool IsBuiltin = false;
    void* FETokenInfo = nullptr;

public:
    IdentifierInfo() = default;

    ::llvm::StringRef getName() const { return Name; }
    void setName(::llvm::StringRef name) { Name = name; }

    tok::TokenKind getTokenID() const { return TokenID; }
    void setTokenID(tok::TokenKind kind) { TokenID = kind; }

    bool isKeyword() const { return IsKeyword; }
    void setIsKeyword(bool kw) { IsKeyword = kw; }

    bool isBuiltin() const { return IsBuiltin; }
    void setIsBuiltin(bool b) { IsBuiltin = b; }

    void* getFETokenInfo() const { return FETokenInfo; }
    void setFETokenInfo(void* info) { FETokenInfo = info; }
};

class IdentifierTable {
    ::llvm::StringMap<IdentifierInfo> HashTable;

public:
    IdentifierTable() = default;

    IdentifierInfo& get(::llvm::StringRef Name);
    void AddKeywords(const LangOptions& LangOpts);
};

}
