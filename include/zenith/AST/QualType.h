#pragma once
#include <cstdint>

namespace zenith {

class Type;

enum Qualifiers : unsigned {
    Const = 0x1,
    Volatile = 0x2,
    Restrict = 0x4
};

class QualType {
    uintptr_t Value = 0;
public:
    QualType() = default;
    QualType(const Type* T, unsigned Quals = 0)
        : Value(reinterpret_cast<uintptr_t>(T) | (Quals & 0x3)) {}

    const Type* getTypePtr() const {
        return reinterpret_cast<const Type*>(Value & ~0x3ULL);
    }

    bool isConstQualified() const {
        return (Value & Qualifiers::Const) != 0;
    }

    bool isVolatileQualified() const {
        return (Value & Qualifiers::Volatile) != 0;
    }

    unsigned getQualifiers() const {
        return Value & 0x3;
    }

    QualType withConst() const {
        return QualType(getTypePtr(), getQualifiers() | Qualifiers::Const);
    }

    QualType withVolatile() const {
        return QualType(getTypePtr(), getQualifiers() | Qualifiers::Volatile);
    }

    bool isNull() const {
        return Value == 0;
    }

    bool operator==(const QualType& Other) const {
        return Value == Other.Value;
    }

    bool operator!=(const QualType& Other) const {
        return Value != Other.Value;
    }
};

}
