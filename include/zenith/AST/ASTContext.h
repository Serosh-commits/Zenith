#pragma once
#include "zenith/AST/QualType.h"
#include "zenith/AST/Type.h"
#include "zenith/AST/Decl.h"
#include "llvm/Support/Allocator.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/ArrayRef.h"

namespace zenith {

class TargetInfo;

class ASTContext {
    ::llvm::BumpPtrAllocator Allocator;
    TranslationUnitDecl *TUDecl;
    ::llvm::DenseMap<const Type*, QualType> PointerTypes;

public:
    QualType VoidTy, BoolTy, CharTy, ShortTy, IntTy, LongTy, LongLongTy;
    QualType UCharTy, UShortTy, UIntTy, ULongTy, ULongLongTy;
    QualType FloatTy, DoubleTy;

    TargetInfo &Target;

    ASTContext(TargetInfo &T);
    
    void *Allocate(uint64_t Size, unsigned Align = 8) {
        return Allocator.Allocate(Size, Align);
    }

    TranslationUnitDecl *getTranslationUnitDecl() const { return TUDecl; }
    
    QualType getPointerType(QualType Pointee);
    QualType getFunctionType(QualType Return, ::llvm::ArrayRef<QualType> Params);
};

}
