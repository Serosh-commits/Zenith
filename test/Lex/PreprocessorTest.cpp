#include <gtest/gtest.h>
#include "zenith/Lex/Preprocessor.h"
#include "zenith/Basic/SourceManager.h"
#include "zenith/Basic/FileManager.h"
#include "zenith/Basic/Diagnostic.h"
#include "llvm/Support/MemoryBuffer.h"

using namespace zenith;

TEST(PreprocessorTest, SimpleMacroDefineAndExpand) {
    FileManager FM;
    DiagnosticsEngine Diags;
    SourceManager SM(FM, Diags);
    LangOptions LangOpts;
    HeaderSearch Headers(FM);

    Preprocessor PP(LangOpts, SM, Diags, Headers, FM);

    const char *Source = "#define FOO 42\nint x = FOO;\n";
    auto Buf = ::llvm::MemoryBuffer::getMemBuffer(Source);
    FileID FID = SM.createFileID(std::move(Buf));
    PP.EnterMainSourceFile(FID);

    Token Tok;
    PP.Lex(Tok);
    EXPECT_EQ(Tok.getKind(), tok::kw_int);

    PP.Lex(Tok);
    EXPECT_EQ(Tok.getKind(), tok::identifier);
    EXPECT_EQ(Tok.getIdentifierInfo()->getName(), "x");

    PP.Lex(Tok);
    EXPECT_EQ(Tok.getKind(), tok::equal);

    PP.Lex(Tok);
    EXPECT_EQ(Tok.getKind(), tok::numeric_constant);

    PP.Lex(Tok);
    EXPECT_EQ(Tok.getKind(), tok::semi);

    PP.Lex(Tok);
    EXPECT_EQ(Tok.getKind(), tok::eof);
}

TEST(PreprocessorTest, IfdefElseBranchSkipping) {
    FileManager FM;
    DiagnosticsEngine Diags;
    SourceManager SM(FM, Diags);
    LangOptions LangOpts;
    HeaderSearch Headers(FM);

    Preprocessor PP(LangOpts, SM, Diags, Headers, FM);

    const char *Source = "#ifdef BAR\nint a;\n#else\nint b;\n#endif\n";
    auto Buf = ::llvm::MemoryBuffer::getMemBuffer(Source);
    FileID FID = SM.createFileID(std::move(Buf));
    PP.EnterMainSourceFile(FID);

    Token Tok;
    PP.Lex(Tok);
    EXPECT_EQ(Tok.getKind(), tok::kw_int);

    PP.Lex(Tok);
    EXPECT_EQ(Tok.getKind(), tok::identifier);
    EXPECT_EQ(Tok.getIdentifierInfo()->getName(), "b");

    PP.Lex(Tok);
    EXPECT_EQ(Tok.getKind(), tok::semi);

    PP.Lex(Tok);
    EXPECT_EQ(Tok.getKind(), tok::eof);
}

TEST(PreprocessorTest, UndefDirective) {
    FileManager FM;
    DiagnosticsEngine Diags;
    SourceManager SM(FM, Diags);
    LangOptions LangOpts;
    HeaderSearch Headers(FM);

    Preprocessor PP(LangOpts, SM, Diags, Headers, FM);

    const char *Source = "#define BAZ 100\n#undef BAZ\n#ifdef BAZ\nint yes;\n#else\nint no;\n#endif\n";
    auto Buf = ::llvm::MemoryBuffer::getMemBuffer(Source);
    FileID FID = SM.createFileID(std::move(Buf));
    PP.EnterMainSourceFile(FID);

    Token Tok;
    PP.Lex(Tok);
    EXPECT_EQ(Tok.getKind(), tok::kw_int);

    PP.Lex(Tok);
    EXPECT_EQ(Tok.getKind(), tok::identifier);
    EXPECT_EQ(Tok.getIdentifierInfo()->getName(), "no");
}

TEST(PreprocessorTest, ObjectLikeMultiTokenMacro) {
    FileManager FM;
    DiagnosticsEngine Diags;
    SourceManager SM(FM, Diags);
    LangOptions LangOpts;
    HeaderSearch Headers(FM);

    Preprocessor PP(LangOpts, SM, Diags, Headers, FM);

    const char *Source = "#define A int b\nA c;\n";
    auto Buf = ::llvm::MemoryBuffer::getMemBuffer(Source);
    FileID FID = SM.createFileID(std::move(Buf));
    PP.EnterMainSourceFile(FID);

    Token Tok;
    PP.Lex(Tok);
    EXPECT_EQ(Tok.getKind(), tok::kw_int);

    PP.Lex(Tok);
    EXPECT_EQ(Tok.getKind(), tok::identifier);
    EXPECT_EQ(Tok.getIdentifierInfo()->getName(), "b");

    PP.Lex(Tok);
    EXPECT_EQ(Tok.getKind(), tok::identifier);
    EXPECT_EQ(Tok.getIdentifierInfo()->getName(), "c");

    PP.Lex(Tok);
    EXPECT_EQ(Tok.getKind(), tok::semi);
}
