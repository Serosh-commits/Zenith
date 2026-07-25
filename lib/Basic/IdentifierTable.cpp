#include "zenith/Basic/IdentifierTable.h"
#include "zenith/Basic/LangOptions.h"

namespace zenith {

IdentifierInfo& IdentifierTable::get(::llvm::StringRef Name) {
    auto& Entry = HashTable[Name];
    if (Entry.getName().empty()) {
        Entry.setName(HashTable.find(Name)->getKey());
    }
    return Entry;
}

void IdentifierTable::AddKeywords(const LangOptions& LangOpts) {
    auto AddKw = [&](::llvm::StringRef Name, tok::TokenKind Kind) {
        IdentifierInfo& Info = get(Name);
        Info.setTokenID(Kind);
        Info.setIsKeyword(true);
    };

    AddKw("auto", tok::kw_auto);
    AddKw("break", tok::kw_break);
    AddKw("case", tok::kw_case);
    AddKw("char", tok::kw_char);
    AddKw("const", tok::kw_const);
    AddKw("continue", tok::kw_continue);
    AddKw("default", tok::kw_default);
    AddKw("do", tok::kw_do);
    AddKw("double", tok::kw_double);
    AddKw("else", tok::kw_else);
    AddKw("enum", tok::kw_enum);
    AddKw("extern", tok::kw_extern);
    AddKw("float", tok::kw_float);
    AddKw("for", tok::kw_for);
    AddKw("goto", tok::kw_goto);
    AddKw("if", tok::kw_if);
    AddKw("int", tok::kw_int);
    AddKw("long", tok::kw_long);
    AddKw("register", tok::kw_register);
    AddKw("return", tok::kw_return);
    AddKw("short", tok::kw_short);
    AddKw("signed", tok::kw_signed);
    AddKw("sizeof", tok::kw_sizeof);
    AddKw("static", tok::kw_static);
    AddKw("struct", tok::kw_struct);
    AddKw("switch", tok::kw_switch);
    AddKw("typedef", tok::kw_typedef);
    AddKw("union", tok::kw_union);
    AddKw("unsigned", tok::kw_unsigned);
    AddKw("void", tok::kw_void);
    AddKw("volatile", tok::kw_volatile);
    AddKw("while", tok::kw_while);

    if (LangOpts.CPlusPlus) {
        AddKw("class", tok::kw_class);
        AddKw("namespace", tok::kw_namespace);
        AddKw("new", tok::kw_new);
        AddKw("delete", tok::kw_delete);
        AddKw("bool", tok::kw_bool);
        AddKw("true", tok::kw_true);
        AddKw("false", tok::kw_false);
        AddKw("virtual", tok::kw_virtual);
        AddKw("public", tok::kw_public);
        AddKw("private", tok::kw_private);
        AddKw("protected", tok::kw_protected);
        AddKw("template", tok::kw_template);
        AddKw("typename", tok::kw_typename);
        AddKw("using", tok::kw_using);
        AddKw("throw", tok::kw_throw);
        AddKw("try", tok::kw_try);
        AddKw("catch", tok::kw_catch);
        AddKw("operator", tok::kw_operator);
        AddKw("const_cast", tok::kw_const_cast);
        AddKw("static_cast", tok::kw_static_cast);
        AddKw("dynamic_cast", tok::kw_dynamic_cast);
        AddKw("reinterpret_cast", tok::kw_reinterpret_cast);
        AddKw("nullptr", tok::kw_nullptr);
    }
}

}
