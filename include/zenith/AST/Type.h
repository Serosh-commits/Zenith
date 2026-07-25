#pragma once
#include "zenith/AST/QualType.h"
#include "llvm/ADT/StringRef.h"
#include <vector>

namespace zenith {

enum class TypeKind {
    Builtin,
    Pointer,
    FunctionProto,
    Typedef
};

class Type {
    TypeKind Kind;
protected:
    Type(TypeKind K) : Kind(K) {}
public:
    TypeKind getKind() const { return Kind; }
    bool isBuiltinType() const { return Kind == TypeKind::Builtin; }
    bool isPointerType() const { return Kind == TypeKind::Pointer; }
    bool isFunctionType() const { return Kind == TypeKind::FunctionProto; }
};

class BuiltinType : public Type {
public:
    enum Kind {
        Void, Bool, Char, Short, Int, Long, LongLong,
        Float, Double, UChar, UShort, UInt, ULong, ULongLong
    };
private:
    Kind BuiltinKind;
public:
    BuiltinType(Kind K) : Type(TypeKind::Builtin), BuiltinKind(K) {}
    Kind getBuiltinKind() const { return BuiltinKind; }
    ::llvm::StringRef getName() const;
};

class PointerType : public Type {
    QualType PointeeType;
public:
    PointerType(QualType Pointee) : Type(TypeKind::Pointer), PointeeType(Pointee) {}
    QualType getPointeeType() const { return PointeeType; }
};

class FunctionProtoType : public Type {
    QualType ReturnType;
    std::vector<QualType> ParamTypes;
public:
    FunctionProtoType(QualType Ret, std::vector<QualType> Params)
        : Type(TypeKind::FunctionProto), ReturnType(Ret), ParamTypes(std::move(Params)) {}
    QualType getReturnType() const { return ReturnType; }
    size_t getNumParams() const { return ParamTypes.size(); }
    QualType getParamType(unsigned i) const { return ParamTypes[i]; }
};

}
