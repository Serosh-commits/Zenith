#pragma once
#include "zenith/AST/DeclBase.h"
#include "zenith/AST/QualType.h"
#include "zenith/Basic/IdentifierTable.h"

namespace zenith {

class Stmt;
class Expr;
class ASTContext;

class NamedDecl : public Decl {
    IdentifierInfo *Name;
public:
    NamedDecl(DeclKind K, SourceLocation L, DeclContext *DC, IdentifierInfo *II)
        : Decl(K, L, DC), Name(II) {}

    ::llvm::StringRef getName() const;
    IdentifierInfo *getIdentifier() const { return Name; }
};

class ValueDecl : public NamedDecl {
    QualType DeclType;
public:
    ValueDecl(DeclKind K, SourceLocation L, DeclContext *DC, IdentifierInfo *II, QualType T)
        : NamedDecl(K, L, DC, II), DeclType(T) {}

    QualType getType() const { return DeclType; }
};

enum class StorageClass {
    None,
    Extern,
    Static,
    PrivateExtern,
    Auto,
    Register
};

class VarDecl : public ValueDecl {
    Expr *Init = nullptr;
    StorageClass SC;
public:
    VarDecl(DeclKind K, SourceLocation L, DeclContext *DC, IdentifierInfo *II, QualType T, StorageClass S = StorageClass::None)
        : ValueDecl(K, L, DC, II, T), SC(S) {}

    Expr *getInit() const { return Init; }
    void setInit(Expr *I) { Init = I; }
    StorageClass getStorageClass() const { return SC; }
};

class ParmVarDecl : public VarDecl {
public:
    ParmVarDecl(SourceLocation L, DeclContext *DC, IdentifierInfo *II, QualType T, StorageClass S = StorageClass::None)
        : VarDecl(DeclKind::ParmVar, L, DC, II, T, S) {}
};

class FunctionDecl : public ValueDecl, public DeclContext {
    ::llvm::SmallVector<ParmVarDecl*, 4> Params;
    Stmt *Body = nullptr;
public:
    FunctionDecl(SourceLocation L, DeclContext *DC, IdentifierInfo *II, QualType T)
        : ValueDecl(DeclKind::Function, L, DC, II, T), DeclContext(DeclKind::Function) {}

    Stmt *getBody() const { return Body; }
    void setBody(Stmt *B) { Body = B; }
    size_t getNumParams() const { return Params.size(); }
    ParmVarDecl *getParamDecl(unsigned i) const { return Params[i]; }
    void setParams(::llvm::ArrayRef<ParmVarDecl*> P) { Params.assign(P.begin(), P.end()); }
};

class TranslationUnitDecl : public Decl, public DeclContext {
    TranslationUnitDecl(ASTContext &C);
public:
    static TranslationUnitDecl *Create(ASTContext &C);
};

class TypedefDecl : public NamedDecl {
    QualType UnderlyingType;
public:
    TypedefDecl(SourceLocation L, DeclContext *DC, IdentifierInfo *II, QualType T)
        : NamedDecl(DeclKind::Typedef, L, DC, II), UnderlyingType(T) {}
    QualType getUnderlyingType() const { return UnderlyingType; }
};

}
