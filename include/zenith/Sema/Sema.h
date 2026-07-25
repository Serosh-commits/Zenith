#pragma once
#include "zenith/AST/ASTContext.h"
#include "zenith/AST/Stmt.h"
#include "zenith/AST/Expr.h"
#include "zenith/Sema/Scope.h"
#include "zenith/Sema/Ownership.h"
#include "zenith/Sema/DeclSpec.h"
#include "zenith/Basic/SourceManager.h"
#include "zenith/Basic/Diagnostic.h"
#include "zenith/Lex/Preprocessor.h"

namespace zenith {

class Sema {
    ASTContext &Context;
    DiagnosticsEngine &Diags;
    SourceManager &SourceMgr;
    Preprocessor &PP;
    Scope *CurScope = nullptr;

public:
    Sema(ASTContext &C, DiagnosticsEngine &D, SourceManager &SM, Preprocessor &P)
        : Context(C), Diags(D), SourceMgr(SM), PP(P) {}

    void PushScope(unsigned Flags) {
        CurScope = new Scope(CurScope, Flags);
    }
    void PopScope() {
        Scope *Old = CurScope;
        CurScope = Old->getParent();
        delete Old;
    }

    Scope *getCurScope() const { return CurScope; }

    Decl *ActOnVariableDeclarator(Scope *S, DeclSpec &DS, IdentifierInfo *Name, SourceLocation Loc, Expr *Init);
    Decl *ActOnFunctionDeclarator(Scope *S, IdentifierInfo *Name, QualType ReturnType, ::llvm::ArrayRef<ParmVarDecl*> Params, SourceLocation Loc);
    void ActOnStartOfFunctionDef(Scope *S, Decl *D);
    Decl *ActOnFinishFunctionBody(Decl *D, Stmt *Body);
    ParmVarDecl *ActOnParamDeclarator(DeclSpec &DS, IdentifierInfo *Name, SourceLocation Loc);

    ExprResult ActOnIntegerConstant(SourceLocation Loc, uint64_t Val);
    ExprResult ActOnIdExpression(Scope *S, IdentifierInfo *Name, SourceLocation Loc);
    ExprResult ActOnBinOp(SourceLocation OpLoc, tok::TokenKind Kind, Expr *LHS, Expr *RHS);
    ExprResult ActOnUnaryOp(SourceLocation OpLoc, tok::TokenKind Kind, Expr *Input);
    ExprResult ActOnCallExpr(Expr *Fn, SourceLocation LParenLoc, ::llvm::ArrayRef<Expr*> Args, SourceLocation RParenLoc);
    ExprResult ActOnParenExpr(SourceLocation L, SourceLocation R, Expr *E);

    StmtResult ActOnCompoundStmt(SourceLocation L, SourceLocation R, ::llvm::ArrayRef<Stmt*> Elts);
    StmtResult ActOnReturnStmt(SourceLocation Loc, Expr *RetValExpr);
    StmtResult ActOnIfStmt(SourceLocation IfLoc, Expr *Cond, Stmt *Then, Stmt *Else);
    StmtResult ActOnWhileStmt(SourceLocation WhileLoc, Expr *Cond, Stmt *Body);
    StmtResult ActOnForStmt(SourceLocation ForLoc, Stmt *Init, Expr *Cond, Expr *Inc, Stmt *Body);
    StmtResult ActOnDeclStmt(Decl *D);
    StmtResult ActOnExprStmt(Expr *E);

    QualType BuildTypeFromDeclSpec(const DeclSpec &DS);
    QualType BuildPointerType(QualType Pointee);
};

}
