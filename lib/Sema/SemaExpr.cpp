#include "zenith/Sema/Sema.h"
#include "zenith/AST/Expr.h"
#include "zenith/Sema/Lookup.h"

namespace zenith {

ExprResult Sema::ActOnIntegerConstant(SourceLocation Loc, uint64_t Val) {
    return IntegerLiteral::Create(Context, Val, Context.IntTy, Loc);
}

ExprResult Sema::ActOnIdExpression(Scope *S, IdentifierInfo *Name, SourceLocation Loc) {
    LookupResult R(Name, Loc);
    if (S && LookupName(R, S)) {
        if (auto *VD = dynamic_cast<ValueDecl*>(R.getFoundDecl())) {
            return new DeclRefExpr(VD, VD->getType(), Loc);
        }
    }
    return new DeclRefExpr(nullptr, Context.IntTy, Loc);
}

ExprResult Sema::ActOnBinOp(SourceLocation OpLoc, tok::TokenKind Kind, Expr *LHS, Expr *RHS) {
    if (!LHS || !RHS) return ExprError();
    BinaryOpcode Opc = BinaryOpcode::Add;
    if (Kind == tok::plus) Opc = BinaryOpcode::Add;
    else if (Kind == tok::minus) Opc = BinaryOpcode::Sub;
    else if (Kind == tok::star) Opc = BinaryOpcode::Mul;
    else if (Kind == tok::slash) Opc = BinaryOpcode::Div;
    else if (Kind == tok::equal) Opc = BinaryOpcode::Assign;
    else if (Kind == tok::greater) Opc = BinaryOpcode::GT;
    else if (Kind == tok::less) Opc = BinaryOpcode::LT;
    else if (Kind == tok::equalequal) Opc = BinaryOpcode::EQ;
    return new BinaryOperator(LHS, RHS, Opc, LHS->getType(), OpLoc);
}

ExprResult Sema::ActOnUnaryOp(SourceLocation OpLoc, tok::TokenKind Kind, Expr *Input) {
    if (!Input) return ExprError();
    UnaryOpcode Opc = UnaryOpcode::Plus;
    return new UnaryOperator(Input, Opc, Input->getType(), OpLoc);
}

ExprResult Sema::ActOnCallExpr(Expr *Fn, SourceLocation LParenLoc, ::llvm::ArrayRef<Expr*> Args, SourceLocation RParenLoc) {
    if (!Fn) return ExprError();
    return new CallExpr(Fn, std::vector<Expr*>(Args.begin(), Args.end()), Context.IntTy, LParenLoc, RParenLoc);
}

ExprResult Sema::ActOnParenExpr(SourceLocation L, SourceLocation R, Expr *E) {
    if (!E) return ExprError();
    return new ParenExpr(E, L, R);
}

}
