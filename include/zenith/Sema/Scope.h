#pragma once
#include "zenith/AST/Decl.h"
#include "llvm/ADT/SmallVector.h"

namespace zenith {

class Scope {
public:
    enum ScopeFlags {
        FnScope = 0x01,
        BlockScope = 0x02,
        ControlScope = 0x04,
        DeclScope = 0x08,
        BreakScope = 0x10,
        ContinueScope = 0x20
    };

private:
    Scope *Parent;
    unsigned Flags;
    unsigned short Depth;
    ::llvm::SmallVector<Decl*, 8> DeclsInScope;
public:
    Scope(Scope *P, unsigned F) : Parent(P), Flags(F), Depth(P ? P->getDepth() + 1 : 0) {}

    Scope *getParent() const { return Parent; }
    unsigned getFlags() const { return Flags; }
    unsigned short getDepth() const { return Depth; }

    void AddDecl(Decl *D) { DeclsInScope.push_back(D); }
    ::llvm::ArrayRef<Decl*> decls() const { return DeclsInScope; }

    bool isFunctionScope() const { return (Flags & FnScope) != 0; }
    bool isBlockScope() const { return (Flags & BlockScope) != 0; }
};

}
