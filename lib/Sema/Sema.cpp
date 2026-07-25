#include "zenith/Sema/Sema.h"
#include "zenith/Sema/Lookup.h"

namespace zenith {

bool LookupName(LookupResult &R, Scope *S) {
    for (Scope *Cur = S; Cur; Cur = Cur->getParent()) {
        for (Decl *D : Cur->decls()) {
            if (auto *ND = dynamic_cast<NamedDecl*>(D)) {
                if (ND->getIdentifier() == R.getLookupName()) {
                    R.addDecl(ND);
                }
            }
        }
        if (!R.empty()) return true;
    }
    return false;
}

StmtResult Sema::ActOnCompoundStmt(SourceLocation L, SourceLocation R, ::llvm::ArrayRef<Stmt*> Elts) {
    return new CompoundStmt(std::vector<Stmt*>(Elts.begin(), Elts.end()), L, R);
}

StmtResult Sema::ActOnReturnStmt(SourceLocation Loc, Expr *RetValExpr) {
    return new ReturnStmt(Loc, RetValExpr);
}

StmtResult Sema::ActOnIfStmt(SourceLocation IfLoc, Expr *Cond, Stmt *Then, Stmt *Else) {
    return new IfStmt(Cond, Then, Else);
}

StmtResult Sema::ActOnWhileStmt(SourceLocation WhileLoc, Expr *Cond, Stmt *Body) {
    return new WhileStmt(Cond, Body);
}

StmtResult Sema::ActOnForStmt(SourceLocation ForLoc, Stmt *Init, Expr *Cond, Expr *Inc, Stmt *Body) {
    return new ForStmt(Init, Cond, Inc, Body);
}

StmtResult Sema::ActOnDeclStmt(Decl *D) {
    return new DeclStmt(D);
}

StmtResult Sema::ActOnExprStmt(Expr *E) {
    return E;
}

}
