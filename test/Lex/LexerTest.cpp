#include <gtest/gtest.h>
#include "zenith/Lex/Lexer.h"
#include "zenith/Basic/SourceManager.h"
#include "zenith/Basic/FileManager.h"
#include "zenith/Basic/Diagnostic.h"
#include "llvm/Support/MemoryBuffer.h"

using namespace zenith;

TEST(LexerTest, BasicTokensAndKeywords) {
    FileManager FM;
    DiagnosticsEngine Diags;
    SourceManager SM(FM, Diags);
    LangOptions LangOpts;
    IdentifierTable Identifiers;
    Identifiers.AddKeywords(LangOpts);

    const char *Source = "int main() { return 0; }";
    auto Buf = ::llvm::MemoryBuffer::getMemBuffer(Source);
    FileID FID = SM.createFileID(std::move(Buf));

    Lexer Lex(FID, nullptr, SM, LangOpts);
    Lex.setIdentifierTable(&Identifiers);

    Token Tok;

    Lex.Lex(Tok);
    EXPECT_EQ(Tok.getKind(), tok::kw_int);
    EXPECT_TRUE(Tok.isAtStartOfLine());

    Lex.Lex(Tok);
    EXPECT_EQ(Tok.getKind(), tok::identifier);
    EXPECT_EQ(Tok.getIdentifierInfo()->getName(), "main");
    EXPECT_TRUE(Tok.hasLeadingSpace());

    Lex.Lex(Tok);
    EXPECT_EQ(Tok.getKind(), tok::l_paren);

    Lex.Lex(Tok);
    EXPECT_EQ(Tok.getKind(), tok::r_paren);

    Lex.Lex(Tok);
    EXPECT_EQ(Tok.getKind(), tok::l_brace);

    Lex.Lex(Tok);
    EXPECT_EQ(Tok.getKind(), tok::kw_return);

    Lex.Lex(Tok);
    EXPECT_EQ(Tok.getKind(), tok::numeric_constant);
    EXPECT_EQ(Lexer::getSpelling(Tok, SM, LangOpts), "0");

    Lex.Lex(Tok);
    EXPECT_EQ(Tok.getKind(), tok::semi);

    Lex.Lex(Tok);
    EXPECT_EQ(Tok.getKind(), tok::r_brace);

    Lex.Lex(Tok);
    EXPECT_EQ(Tok.getKind(), tok::eof);
}

TEST(LexerTest, CompoundPunctuatorsAndOperators) {
    FileManager FM;
    DiagnosticsEngine Diags;
    SourceManager SM(FM, Diags);
    LangOptions LangOpts;

    const char *Source = "++ -- == != <= >= << >> && || -> :: += -= *= /= %= &= |= ^= <<= >>= ... ##";
    auto Buf = ::llvm::MemoryBuffer::getMemBuffer(Source);
    FileID FID = SM.createFileID(std::move(Buf));

    Lexer Lex(FID, nullptr, SM, LangOpts);
    Token Tok;

    tok::TokenKind Expected[] = {
        tok::plusplus, tok::minusminus, tok::equalequal, tok::exclaimequal,
        tok::lessequal, tok::greaterequal, tok::lessless, tok::greatergreater,
        tok::ampamp, tok::pipepipe, tok::arrow, tok::coloncolon,
        tok::plusequal, tok::minusequal, tok::starequal, tok::slashequal,
        tok::percentequal, tok::ampequal, tok::pipeequal, tok::caretequal,
        tok::lesslessequal, tok::greatergreaterequal, tok::ellipsis, tok::hashhash,
        tok::eof
    };

    for (tok::TokenKind Exp : Expected) {
        Lex.Lex(Tok);
        EXPECT_EQ(Tok.getKind(), Exp);
    }
}

TEST(LexerTest, CommentsAndWhitespaceSkipping) {
    FileManager FM;
    DiagnosticsEngine Diags;
    SourceManager SM(FM, Diags);
    LangOptions LangOpts;

    const char *Source = "// line comment\n/* block\ncomment */ 100";
    auto Buf = ::llvm::MemoryBuffer::getMemBuffer(Source);
    FileID FID = SM.createFileID(std::move(Buf));

    Lexer Lex(FID, nullptr, SM, LangOpts);
    Token Tok;

    Lex.Lex(Tok);
    EXPECT_EQ(Tok.getKind(), tok::numeric_constant);
    EXPECT_EQ(Lexer::getSpelling(Tok, SM, LangOpts), "100");
    EXPECT_TRUE(Tok.hasLeadingSpace());
}
