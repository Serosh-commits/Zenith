#pragma once
#include "zenith/AST/Decl.h"
#include "zenith/Basic/SourceLocation.h"
#include "zenith/Sema/Scope.h"
#include "llvm/ADT/SmallVector.h"

namespace zenith {

enum class LookupResultKind {
    NotFound,
    Found,
    Ambiguous
};

class LookupResult {
    ::llvm::SmallVector<NamedDecl*, 4> Decls;
    IdentifierInfo *Name;
    SourceLocation NameLoc;
public:
    LookupResult(IdentifierInfo *II, SourceLocation L) : Name(II), NameLoc(L) {}

    LookupResultKind getResultKind() const {
        if (Decls.empty()) return LookupResultKind::NotFound;
        if (Decls.size() > 1) return LookupResultKind::Ambiguous;
        return LookupResultKind::Found;
    }

    NamedDecl *getFoundDecl() const {
        return getResultKind() == LookupResultKind::Found ? Decls[0] : nullptr;
    }

    bool empty() const { return Decls.empty(); }
    void addDecl(NamedDecl *D) { Decls.push_back(D); }
    IdentifierInfo *getLookupName() const { return Name; }
};

bool LookupName(LookupResult &R, Scope *S);

}
