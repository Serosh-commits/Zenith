#include "zenith/Basic/TargetInfo.h"

namespace zenith {

unsigned TargetInfo::getSizeOfType(::llvm::StringRef typeName) const {
    if (typeName == "char") return CharWidth / 8;
    if (typeName == "short") return ShortWidth / 8;
    if (typeName == "int") return IntWidth / 8;
    if (typeName == "long") return LongWidth / 8;
    if (typeName == "long long") return LongLongWidth / 8;
    if (typeName == "float") return FloatWidth / 8;
    if (typeName == "double") return DoubleWidth / 8;
    if (typeName == "pointer") return PointerWidth / 8;
    return 0;
}

std::unique_ptr<TargetInfo> TargetInfo::CreateTargetInfo() {
    return std::make_unique<TargetInfo>();
}

}
