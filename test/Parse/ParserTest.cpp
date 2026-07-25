#include <gtest/gtest.h>
#include "zenith/Parse/Parser.h"
#include "zenith/Sema/Sema.h"
#include "zenith/AST/ASTContext.h"
#include "zenith/Basic/SourceManager.h"
#include "zenith/Basic/FileManager.h"
#include "zenith/Basic/Diagnostic.h"
#include "zenith/Basic/TargetInfo.h"
#include "llvm/Support/MemoryBuffer.h"

using namespace zenith;

TEST(ParserTest, FunctionDefinitionParsing) {
    FileManager FM;
    DiagnosticsEngine Diags;
    SourceManager SM(FM, Diags);
    LangOptions LangOpts;
    HeaderSearch Headers(FM);
    Preprocessor PP(LangOpts, SM, Diags, Headers, FM);

    const char *Source = "int add(int a, int b) { return a + b; }\n";
    auto Buf = ::llvm::MemoryBuffer::getMemBuffer(Source);
    FileID FID = SM.createFileID(std::move(Buf));
    PP.EnterMainSourceFile(FID);

    auto TargetPtr = TargetInfo::CreateTargetInfo();
    ASTContext Context(*TargetPtr);
    Sema Actions(Context, Diags, SM, PP);
    Parser P(PP, Actions, Diags);

    P.ParseTranslationUnit();

    TranslationUnitDecl *TU = Context.getTranslationUnitDecl();
    ASSERT_NE(TU, nullptr);
    EXPECT_EQ(TU->decls().size(), 1u);

    Decl *D = TU->decls()[0];
    EXPECT_EQ(D->getKind(), DeclKind::Function);

    FunctionDecl *FD = dynamic_cast<FunctionDecl*>(D);
    ASSERT_NE(FD, nullptr);
    EXPECT_EQ(FD->getName(), "add");
    EXPECT_EQ(FD->getNumParams(), 2u);
    EXPECT_NE(FD->getBody(), nullptr);
}

TEST(ParserTest, VariableDeclarationsAndStatements) {
    FileManager FM;
    DiagnosticsEngine Diags;
    SourceManager SM(FM, Diags);
    LangOptions LangOpts;
    HeaderSearch Headers(FM);
    Preprocessor PP(LangOpts, SM, Diags, Headers, FM);

    const char *Source = "int main() { int x = 10; if (x > 5) { return x; } return 0; }\n";
    auto Buf = ::llvm::MemoryBuffer::getMemBuffer(Source);
    FileID FID = SM.createFileID(std::move(Buf));
    PP.EnterMainSourceFile(FID);

    auto TargetPtr = TargetInfo::CreateTargetInfo();
    ASTContext Context(*TargetPtr);
    Sema Actions(Context, Diags, SM, PP);
    Parser P(PP, Actions, Diags);

    P.ParseTranslationUnit();

    TranslationUnitDecl *TU = Context.getTranslationUnitDecl();
    ASSERT_NE(TU, nullptr);
    EXPECT_EQ(TU->decls().size(), 1u);

    FunctionDecl *MainFD = dynamic_cast<FunctionDecl*>(TU->decls()[0]);
    ASSERT_NE(MainFD, nullptr);
    EXPECT_EQ(MainFD->getName(), "main");
}
