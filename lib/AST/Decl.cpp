#include "zenith/AST/Decl.h"
#include "zenith/AST/ASTContext.h"
#include "zenith/Basic/IdentifierTable.h"

namespace zenith {

Decl* DeclContext::lookup(::llvm::StringRef Name) const {
    for (Decl* D : Decls) {
        if (auto* ND = dynamic_cast<NamedDecl*>(D)) {
            if (ND->getName() == Name) {
                return D;
            }
        }
    }
    return nullptr;
}

::llvm::StringRef NamedDecl::getName() const {
    if (Name) return Name->getName();
    return "";
}

TranslationUnitDecl::TranslationUnitDecl(ASTContext &C)
    : Decl(DeclKind::TranslationUnit, SourceLocation(), nullptr),
      DeclContext(DeclKind::TranslationUnit) {}

TranslationUnitDecl *TranslationUnitDecl::Create(ASTContext &C) {
    return new TranslationUnitDecl(C);
}

}
