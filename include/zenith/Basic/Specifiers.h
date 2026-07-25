#pragma once

namespace zenith {

enum class StorageClass {
    None,
    Extern,
    Static,
    Register,
    Auto
};

enum class AccessSpecifier {
    Public,
    Protected,
    Private,
    None
};

enum class TypeSpecifierWidth {
    Unspecified,
    Short,
    Long,
    LongLong
};

enum class TypeSpecifierSign {
    Unspecified,
    Signed,
    Unsigned
};

}
