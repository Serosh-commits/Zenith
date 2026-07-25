#include "zenith/AST/Type.h"

namespace zenith {

::llvm::StringRef BuiltinType::getName() const {
    switch (BuiltinKind) {
        case Void: return "void";
        case Bool: return "_Bool";
        case Char: return "char";
        case Short: return "short";
        case Int: return "int";
        case Long: return "long";
        case LongLong: return "long long";
        case Float: return "float";
        case Double: return "double";
        case UChar: return "unsigned char";
        case UShort: return "unsigned short";
        case UInt: return "unsigned int";
        case ULong: return "unsigned long";
        case ULongLong: return "unsigned long long";
    }
    return "";
}

}
