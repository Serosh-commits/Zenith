#include "zenith/Sema/Sema.h"

namespace zenith {

QualType Sema::BuildTypeFromDeclSpec(const DeclSpec &DS) {
    QualType T = Context.IntTy;
    if (DS.getTypeSpecType() == tok::kw_void) T = Context.VoidTy;
    else if (DS.getTypeSpecType() == tok::kw_char) T = Context.CharTy;
    else if (DS.getTypeSpecType() == tok::kw_int) T = Context.IntTy;
    else if (DS.getTypeSpecType() == tok::kw_float) T = Context.FloatTy;
    else if (DS.getTypeSpecType() == tok::kw_double) T = Context.DoubleTy;
    
    if (DS.isConst()) T = T.withConst();
    if (DS.isVolatile()) T = T.withVolatile();
    return T;
}

QualType Sema::BuildPointerType(QualType Pointee) {
    return Context.getPointerType(Pointee);
}

}
