#pragma once

namespace zenith {

class Expr;
class Stmt;
class Decl;

template<typename PtrTy>
class ActionResult {
    PtrTy Val;
    bool Invalid;
public:
    ActionResult(bool Invalid = true) : Val(nullptr), Invalid(Invalid) {}
    ActionResult(PtrTy V) : Val(V), Invalid(false) {}

    bool isInvalid() const { return Invalid; }
    bool isUsable() const { return !Invalid && Val; }
    PtrTy get() const { return Val; }
};

using ExprResult = ActionResult<Expr*>;
using StmtResult = ActionResult<Stmt*>;
using DeclResult = ActionResult<Decl*>;

inline ExprResult ExprError() { return ExprResult(true); }
inline StmtResult StmtError() { return StmtResult(true); }
inline DeclResult DeclError() { return DeclResult(true); }

}
