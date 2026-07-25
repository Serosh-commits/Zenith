#pragma once
#include "zenith/Basic/SourceLocation.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/ArrayRef.h"

namespace zenith {

class IdentifierInfo;

enum class DeclKind {
    TranslationUnit,
    Var,
    Function,
    ParmVar,
    Typedef,
    Record,
    Field,
    Enum,
    EnumConstant
};

class DeclContext;

class Decl {
    DeclKind Kind;
    SourceLocation Loc;
    DeclContext *ParentDC;
public:
    Decl(DeclKind K, SourceLocation L, DeclContext *DC)
        : Kind(K), Loc(L), ParentDC(DC) {}
    virtual ~Decl() = default;

    DeclKind getKind() const { return Kind; }
    SourceLocation getLocation() const { return Loc; }
    DeclContext *getDeclContext() const { return ParentDC; }
};

class DeclContext {
    DeclKind Kind;
    ::llvm::SmallVector<Decl*, 8> Decls;
public:
    DeclContext(DeclKind K) : Kind(K) {}
    virtual ~DeclContext() = default;

    void addDecl(Decl* D) { Decls.push_back(D); }
    ::llvm::ArrayRef<Decl*> decls() const { return Decls; }
    Decl* lookup(::llvm::StringRef Name) const;
};

}
