#include <gtest/gtest.h>
#include "zenith/AST/ASTContext.h"
#include "zenith/AST/Decl.h"
#include "zenith/AST/Expr.h"
#include "zenith/AST/Stmt.h"
#include "zenith/Basic/TargetInfo.h"

using namespace zenith;

TEST(ASTTest, QualTypeAndBuiltinTypes) {
    auto TargetPtr = TargetInfo::CreateTargetInfo();
    ASTContext Context(*TargetPtr);

    EXPECT_FALSE(Context.IntTy.isNull());
    EXPECT_FALSE(Context.IntTy.isConstQualified());

    QualType ConstInt = Context.IntTy.withConst();
    EXPECT_TRUE(ConstInt.isConstQualified());
    EXPECT_EQ(ConstInt.getTypePtr(), Context.IntTy.getTypePtr());

    QualType PtrToInt = Context.getPointerType(Context.IntTy);
    EXPECT_TRUE(PtrToInt.getTypePtr()->isPointerType());
}

TEST(ASTTest, FunctionDeclAndTranslationUnit) {
    auto TargetPtr = TargetInfo::CreateTargetInfo();
    ASTContext Context(*TargetPtr);

    TranslationUnitDecl *TU = Context.getTranslationUnitDecl();
    ASSERT_NE(TU, nullptr);
    EXPECT_EQ(TU->getKind(), DeclKind::TranslationUnit);

    SourceLocation Loc(100);
    QualType FnType = Context.getFunctionType(Context.IntTy, {});
    FunctionDecl *FD = new FunctionDecl(Loc, TU, nullptr, FnType);
    TU->addDecl(FD);

    EXPECT_EQ(TU->decls().size(), 1u);
    EXPECT_EQ(TU->decls()[0], FD);
    EXPECT_EQ(FD->getKind(), DeclKind::Function);
}

TEST(ASTTest, ExprNodesAndStatements) {
    auto TargetPtr = TargetInfo::CreateTargetInfo();
    ASTContext Context(*TargetPtr);

    SourceLocation Loc(50);
    IntegerLiteral *IL = IntegerLiteral::Create(Context, 42, Context.IntTy, Loc);
    EXPECT_EQ(IL->getValue(), 42u);
    EXPECT_EQ(IL->getType(), Context.IntTy);

    BinaryOperator *BO = new BinaryOperator(IL, IL, BinaryOpcode::Add, Context.IntTy, Loc);
    EXPECT_EQ(BO->getLHS(), IL);
    EXPECT_EQ(BO->getRHS(), IL);
    EXPECT_EQ(BO->getOpcode(), BinaryOpcode::Add);

    ReturnStmt *RS = new ReturnStmt(Loc, BO);
    EXPECT_EQ(RS->getRetValue(), BO);
}
