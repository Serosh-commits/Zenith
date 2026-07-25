#pragma once
#include "zenith/AST/Decl.h"
#include "zenith/Basic/SourceLocation.h"
#include <vector>

namespace zenith {

enum class StmtKind {
    Compound,
    DeclStmt,
    If,
    While,
    For,
    Return,
    Null,
    IntegerLiteralKind,
    FloatingLiteralKind,
    StringLiteralKind,
    DeclRefExprKind,
    BinaryOperatorKind,
    UnaryOperatorKind,
    CallExprKind,
    ParenExprKind,
    ImplicitCastExprKind,
    MemberExprKind
};

class Stmt {
    StmtKind Kind;
public:
    Stmt(StmtKind K) : Kind(K) {}
    virtual ~Stmt() = default;
    StmtKind getKind() const { return Kind; }
};

class CompoundStmt : public Stmt {
    std::vector<Stmt*> Body;
    SourceLocation LBraceLoc, RBraceLoc;
public:
    CompoundStmt(std::vector<Stmt*> B, SourceLocation L, SourceLocation R)
        : Stmt(StmtKind::Compound), Body(std::move(B)), LBraceLoc(L), RBraceLoc(R) {}

    ::llvm::ArrayRef<Stmt*> body() const { return Body; }
    auto body_begin() const { return Body.begin(); }
    auto body_end() const { return Body.end(); }
};

class DeclStmt : public Stmt {
    Decl *TheDecl;
public:
    DeclStmt(Decl *D) : Stmt(StmtKind::DeclStmt), TheDecl(D) {}
    Decl *getDecl() const { return TheDecl; }
};

class Expr;

class ReturnStmt : public Stmt {
    Expr *RetValue;
    SourceLocation RetLoc;
public:
    ReturnStmt(SourceLocation L, Expr *E) : Stmt(StmtKind::Return), RetValue(E), RetLoc(L) {}
    Expr *getRetValue() const { return RetValue; }
};

class IfStmt : public Stmt {
    Expr *Cond;
    Stmt *Then;
    Stmt *Else;
public:
    IfStmt(Expr *C, Stmt *T, Stmt *E) : Stmt(StmtKind::If), Cond(C), Then(T), Else(E) {}
    Expr *getCond() const { return Cond; }
    Stmt *getThen() const { return Then; }
    Stmt *getElse() const { return Else; }
};

class WhileStmt : public Stmt {
    Expr *Cond;
    Stmt *Body;
public:
    WhileStmt(Expr *C, Stmt *B) : Stmt(StmtKind::While), Cond(C), Body(B) {}
    Expr *getCond() const { return Cond; }
    Stmt *getBody() const { return Body; }
};

class ForStmt : public Stmt {
    Stmt *Init;
    Expr *Cond;
    Expr *Inc;
    Stmt *Body;
public:
    ForStmt(Stmt *I, Expr *C, Expr *IncE, Stmt *B) : Stmt(StmtKind::For), Init(I), Cond(C), Inc(IncE), Body(B) {}
    Stmt *getInit() const { return Init; }
    Expr *getCond() const { return Cond; }
    Expr *getInc() const { return Inc; }
    Stmt *getBody() const { return Body; }
};

class NullStmt : public Stmt {
    SourceLocation SemiLoc;
public:
    NullStmt(SourceLocation L) : Stmt(StmtKind::Null), SemiLoc(L) {}
};

}
