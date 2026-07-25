#include "zenith/Sema/Sema.h"
#include "zenith/AST/Decl.h"
#include "zenith/Sema/Lookup.h"

namespace zenith {

Decl *Sema::ActOnVariableDeclarator(Scope *S, DeclSpec &DS, IdentifierInfo *Name, SourceLocation Loc, Expr *Init) {
    QualType T = BuildTypeFromDeclSpec(DS);
    TranslationUnitDecl *TU = Context.getTranslationUnitDecl();
    DeclContext *DC = (S && S->getParent()) ? nullptr : TU;
    VarDecl *VD = new VarDecl(DeclKind::Var, Loc, DC, Name, T, DS.getStorageClass());
    if (Init) VD->setInit(Init);
    if (S) S->AddDecl(VD);
    if (!S || !S->getParent()) TU->addDecl(VD);
    return VD;
}

Decl *Sema::ActOnFunctionDeclarator(Scope *S, IdentifierInfo *Name, QualType ReturnType, ::llvm::ArrayRef<ParmVarDecl*> Params, SourceLocation Loc) {
    QualType FT = Context.getFunctionType(ReturnType, {});
    TranslationUnitDecl *TU = Context.getTranslationUnitDecl();
    FunctionDecl *FD = new FunctionDecl(Loc, TU, Name, FT);
    FD->setParams(Params);
    if (S) S->AddDecl(FD);
    if (TU) TU->addDecl(FD);
    return FD;
}

void Sema::ActOnStartOfFunctionDef(Scope *S, Decl *D) {
    PushScope(Scope::FnScope | Scope::DeclScope);
}

Decl *Sema::ActOnFinishFunctionBody(Decl *D, Stmt *Body) {
    if (auto *FD = dynamic_cast<FunctionDecl*>(D)) {
        FD->setBody(Body);
    }
    PopScope();
    return D;
}

ParmVarDecl *Sema::ActOnParamDeclarator(DeclSpec &DS, IdentifierInfo *Name, SourceLocation Loc) {
    QualType T = BuildTypeFromDeclSpec(DS);
    return new ParmVarDecl(Loc, nullptr, Name, T, DS.getStorageClass());
}

}
