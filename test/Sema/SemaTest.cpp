#include <gtest/gtest.h>
#include "zenith/Sema/Sema.h"
#include "zenith/AST/ASTContext.h"
#include "zenith/Basic/SourceManager.h"
#include "zenith/Basic/FileManager.h"
#include "zenith/Basic/Diagnostic.h"
#include "zenith/Basic/TargetInfo.h"

using namespace zenith;

TEST(SemaTest, ScopePushPopAndDeclRegistration) {
    FileManager FM;
    DiagnosticsEngine Diags;
    SourceManager SM(FM, Diags);
    LangOptions LangOpts;
    HeaderSearch Headers(FM);
    Preprocessor PP(LangOpts, SM, Diags, Headers, FM);

    auto TargetPtr = TargetInfo::CreateTargetInfo();
    ASTContext Context(*TargetPtr);
    Sema Actions(Context, Diags, SM, PP);

    EXPECT_EQ(Actions.getCurScope(), nullptr);

    Actions.PushScope(Scope::FnScope | Scope::DeclScope);
    Scope *S1 = Actions.getCurScope();
    ASSERT_NE(S1, nullptr);
    EXPECT_TRUE(S1->isFunctionScope());

    Actions.PushScope(Scope::BlockScope);
    Scope *S2 = Actions.getCurScope();
    EXPECT_EQ(S2->getParent(), S1);

    Actions.PopScope();
    EXPECT_EQ(Actions.getCurScope(), S1);

    Actions.PopScope();
    EXPECT_EQ(Actions.getCurScope(), nullptr);
}

TEST(SemaTest, ActOnDeclarationsAndExpressions) {
    FileManager FM;
    DiagnosticsEngine Diags;
    SourceManager SM(FM, Diags);
    LangOptions LangOpts;
    HeaderSearch Headers(FM);
    Preprocessor PP(LangOpts, SM, Diags, Headers, FM);

    auto TargetPtr = TargetInfo::CreateTargetInfo();
    ASTContext Context(*TargetPtr);
    Sema Actions(Context, Diags, SM, PP);

    SourceLocation Loc(10);
    DeclSpec DS;
    DS.SetTypeSpecType(tok::kw_int);

    IdentifierInfo &VarName = PP.getIdentifierTable().get("my_var");
    Decl *D = Actions.ActOnVariableDeclarator(nullptr, DS, &VarName, Loc, nullptr);

    ASSERT_NE(D, nullptr);
    EXPECT_EQ(D->getKind(), DeclKind::Var);

    ExprResult IntConst = Actions.ActOnIntegerConstant(Loc, 42);
    EXPECT_FALSE(IntConst.isInvalid());
    EXPECT_NE(IntConst.get(), nullptr);

    ExprResult BinOp = Actions.ActOnBinOp(Loc, tok::plus, IntConst.get(), IntConst.get());
    EXPECT_FALSE(BinOp.isInvalid());
    EXPECT_NE(BinOp.get(), nullptr);
}
