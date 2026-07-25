#include "zenith/AST/ASTContext.h"

namespace zenith {

ASTContext::ASTContext(TargetInfo &T) : Target(T) {
    TUDecl = TranslationUnitDecl::Create(*this);

    VoidTy = QualType(new BuiltinType(BuiltinType::Void));
    BoolTy = QualType(new BuiltinType(BuiltinType::Bool));
    CharTy = QualType(new BuiltinType(BuiltinType::Char));
    ShortTy = QualType(new BuiltinType(BuiltinType::Short));
    IntTy = QualType(new BuiltinType(BuiltinType::Int));
    LongTy = QualType(new BuiltinType(BuiltinType::Long));
    LongLongTy = QualType(new BuiltinType(BuiltinType::LongLong));
    
    UCharTy = QualType(new BuiltinType(BuiltinType::UChar));
    UShortTy = QualType(new BuiltinType(BuiltinType::UShort));
    UIntTy = QualType(new BuiltinType(BuiltinType::UInt));
    ULongTy = QualType(new BuiltinType(BuiltinType::ULong));
    ULongLongTy = QualType(new BuiltinType(BuiltinType::ULongLong));
    
    FloatTy = QualType(new BuiltinType(BuiltinType::Float));
    DoubleTy = QualType(new BuiltinType(BuiltinType::Double));
}

QualType ASTContext::getPointerType(QualType Pointee) {
    const Type *Key = Pointee.getTypePtr();
    auto It = PointerTypes.find(Key);
    if (It != PointerTypes.end()) {
        return It->second;
    }
    QualType PT(new PointerType(Pointee));
    PointerTypes[Key] = PT;
    return PT;
}

QualType ASTContext::getFunctionType(QualType Return, ::llvm::ArrayRef<QualType> Params) {
    std::vector<QualType> ParamVec(Params.begin(), Params.end());
    return QualType(new FunctionProtoType(Return, std::move(ParamVec)));
}

}
