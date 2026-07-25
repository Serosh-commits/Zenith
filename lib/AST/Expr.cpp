#include "zenith/AST/Expr.h"
#include "zenith/AST/ASTContext.h"

namespace zenith {

IntegerLiteral *IntegerLiteral::Create(ASTContext &C, uint64_t Val, QualType T, SourceLocation L) {
    return new IntegerLiteral(Val, T, L);
}

}
