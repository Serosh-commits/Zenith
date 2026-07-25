#pragma once
#include <cstdint>
#include <memory>
#include "llvm/ADT/StringRef.h"

namespace zenith {

class TargetInfo {
    unsigned CharWidth = 8;
    unsigned ShortWidth = 16;
    unsigned IntWidth = 32;
    unsigned LongWidth = 64;
    unsigned LongLongWidth = 64;
    unsigned PointerWidth = 64;
    unsigned FloatWidth = 32;
    unsigned DoubleWidth = 64;
    bool BigEndian = false;

public:
    TargetInfo() = default;

    unsigned getCharWidth() const { return CharWidth; }
    unsigned getShortWidth() const { return ShortWidth; }
    unsigned getIntWidth() const { return IntWidth; }
    unsigned getLongWidth() const { return LongWidth; }
    unsigned getLongLongWidth() const { return LongLongWidth; }
    unsigned getPointerWidth() const { return PointerWidth; }
    unsigned getFloatWidth() const { return FloatWidth; }
    unsigned getDoubleWidth() const { return DoubleWidth; }
    bool isBigEndian() const { return BigEndian; }

    unsigned getSizeOfType(::llvm::StringRef typeName) const;

    static std::unique_ptr<TargetInfo> CreateTargetInfo();
};

}
