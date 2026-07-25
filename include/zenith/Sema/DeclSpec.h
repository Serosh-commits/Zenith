#pragma once
#include "zenith/AST/Decl.h"
#include "zenith/Lex/Token.h"

namespace zenith {

enum class TypeSpecifierWidth { Unspecified, Short, Long, LongLong };
enum class TypeSpecifierSign { Unspecified, Signed, Unsigned };

class DeclSpec {
    TypeSpecifierWidth Width = TypeSpecifierWidth::Unspecified;
    TypeSpecifierSign Sign = TypeSpecifierSign::Unspecified;
    tok::TokenKind TypeSpecType = tok::unknown;
    bool IsConst = false;
    bool IsVolatile = false;
    StorageClass SC = StorageClass::None;

public:
    void SetTypeSpecWidth(TypeSpecifierWidth W) { Width = W; }
    void SetTypeSpecSign(TypeSpecifierSign S) { Sign = S; }
    void SetTypeSpecType(tok::TokenKind T) { TypeSpecType = T; }
    void SetStorageClass(StorageClass S) { SC = S; }
    void SetConst(bool C) { IsConst = C; }
    void SetVolatile(bool V) { IsVolatile = V; }

    TypeSpecifierWidth getTypeSpecWidth() const { return Width; }
    TypeSpecifierSign getTypeSpecSign() const { return Sign; }
    tok::TokenKind getTypeSpecType() const { return TypeSpecType; }
    StorageClass getStorageClass() const { return SC; }
    bool isConst() const { return IsConst; }
    bool isVolatile() const { return IsVolatile; }
};

}
