#pragma once
#include "zenith/AST/Stmt.h"
#include "zenith/AST/QualType.h"
#include "zenith/AST/Decl.h"
#include <string>

namespace zenith {

class ASTContext;

class Expr : public Stmt {
    QualType ExprType;
public:
    Expr(StmtKind K, QualType T) : Stmt(K), ExprType(T) {}
    QualType getType() const { return ExprType; }
    void setType(QualType T) { ExprType = T; }
};

class IntegerLiteral : public Expr {
    uint64_t Value;
    SourceLocation Loc;
public:
    IntegerLiteral(uint64_t Val, QualType T, SourceLocation L)
        : Expr(StmtKind::IntegerLiteralKind, T), Value(Val), Loc(L) {}
    
    static IntegerLiteral *Create(ASTContext &C, uint64_t Val, QualType T, SourceLocation L);
    uint64_t getValue() const { return Value; }
    SourceLocation getLocation() const { return Loc; }
};

class FloatingLiteral : public Expr {
    double Value;
    SourceLocation Loc;
public:
    FloatingLiteral(double Val, QualType T, SourceLocation L)
        : Expr(StmtKind::FloatingLiteralKind, T), Value(Val), Loc(L) {}
    double getValue() const { return Value; }
    SourceLocation getLocation() const { return Loc; }
};

class StringLiteral : public Expr {
    std::string Value;
    SourceLocation Loc;
public:
    StringLiteral(std::string Val, QualType T, SourceLocation L)
        : Expr(StmtKind::StringLiteralKind, T), Value(std::move(Val)), Loc(L) {}
    ::llvm::StringRef getValue() const { return Value; }
    SourceLocation getLocation() const { return Loc; }
};

class DeclRefExpr : public Expr {
    ValueDecl *TheDecl;
    SourceLocation Loc;
public:
    DeclRefExpr(ValueDecl *D, QualType T, SourceLocation L)
        : Expr(StmtKind::DeclRefExprKind, T), TheDecl(D), Loc(L) {}
    ValueDecl *getDecl() const { return TheDecl; }
    SourceLocation getLocation() const { return Loc; }
};

enum class BinaryOpcode {
    Add, Sub, Mul, Div, Rem, Shl, Shr, LT, GT, LE, GE, EQ, NE,
    And, Xor, Or, LAnd, LOr, Assign, AddAssign, SubAssign, MulAssign, DivAssign, Comma
};

class BinaryOperator : public Expr {
    Expr *LHS;
    Expr *RHS;
    BinaryOpcode Opc;
    SourceLocation OpLoc;
public:
    BinaryOperator(Expr *L, Expr *R, BinaryOpcode O, QualType T, SourceLocation LLoc)
        : Expr(StmtKind::BinaryOperatorKind, T), LHS(L), RHS(R), Opc(O), OpLoc(LLoc) {}
    Expr *getLHS() const { return LHS; }
    Expr *getRHS() const { return RHS; }
    BinaryOpcode getOpcode() const { return Opc; }
    SourceLocation getOperatorLoc() const { return OpLoc; }
};

enum class UnaryOpcode {
    PostInc, PostDec, PreInc, PreDec, AddrOf, Deref, Plus, Minus, Not, LNot
};

class UnaryOperator : public Expr {
    Expr *SubExpr;
    UnaryOpcode Opc;
    SourceLocation OpLoc;
public:
    UnaryOperator(Expr *S, UnaryOpcode O, QualType T, SourceLocation L)
        : Expr(StmtKind::UnaryOperatorKind, T), SubExpr(S), Opc(O), OpLoc(L) {}
};

class CallExpr : public Expr {
    Expr *Callee;
    std::vector<Expr*> Args;
    SourceLocation LParenLoc, RParenLoc;
public:
    CallExpr(Expr *C, std::vector<Expr*> A, QualType T, SourceLocation LP, SourceLocation RP)
        : Expr(StmtKind::CallExprKind, T), Callee(C), Args(std::move(A)), LParenLoc(LP), RParenLoc(RP) {}
    
    Expr *getCallee() const { return Callee; }
    size_t getNumArgs() const { return Args.size(); }
    Expr *getArg(unsigned i) const { return Args[i]; }
};

class ParenExpr : public Expr {
    Expr *SubExpr;
    SourceLocation LParen, RParen;
public:
    ParenExpr(Expr *S, SourceLocation L, SourceLocation R)
        : Expr(StmtKind::ParenExprKind, S->getType()), SubExpr(S), LParen(L), RParen(R) {}
};

enum class CastKind {
    LValueToRValue, IntegralCast, FloatingCast, NoOp
};

class ImplicitCastExpr : public Expr {
    Expr *SubExpr;
    CastKind Kind;
public:
    ImplicitCastExpr(Expr *S, CastKind K, QualType T)
        : Expr(StmtKind::ImplicitCastExprKind, T), SubExpr(S), Kind(K) {}
};

}
