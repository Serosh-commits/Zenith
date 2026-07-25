#include <gtest/gtest.h>
#include "zenith/Lex/Token.h"
#include "zenith/Basic/IdentifierTable.h"

using namespace zenith;

TEST(TokenTest, StartTokenAndKind) {
    Token Tok;
    Tok.startToken();
    EXPECT_EQ(Tok.getKind(), tok::unknown);
    EXPECT_EQ(Tok.getFlags(), 0u);

    Tok.setKind(tok::kw_int);
    EXPECT_EQ(Tok.getKind(), tok::kw_int);
    EXPECT_TRUE(Tok.is(tok::kw_int));
    EXPECT_TRUE(Tok.isNot(tok::kw_void));
    EXPECT_TRUE(Tok.isOneOf(tok::kw_float, tok::kw_int, tok::kw_char));
    EXPECT_TRUE(Tok.isNoneOf(tok::kw_double, tok::kw_void));
}

TEST(TokenTest, TokenFlagsAndBitmask) {
    Token Tok;
    Tok.startToken();

    EXPECT_FALSE(Tok.isAtStartOfLine());
    EXPECT_FALSE(Tok.hasLeadingSpace());

    Tok.setFlag(Token::StartOfLine);
    EXPECT_TRUE(Tok.isAtStartOfLine());

    Tok.setFlag(Token::LeadingSpace);
    EXPECT_TRUE(Tok.hasLeadingSpace());

    Tok.clearFlag(Token::StartOfLine);
    EXPECT_FALSE(Tok.isAtStartOfLine());

    Tok.setFlagValue(Token::DisableExpand, true);
    EXPECT_TRUE(Tok.isExpandDisabled());

    Tok.setFlagValue(Token::NeedsCleaning, true);
    EXPECT_TRUE(Tok.needsCleaning());

    Tok.setFlagValue(Token::HasUDSuffix, true);
    EXPECT_TRUE(Tok.hasUDSuffix());

    Tok.setFlagValue(Token::HasUCN, true);
    EXPECT_TRUE(Tok.hasUCN());
}

TEST(TokenTest, LocationsAndLength) {
    Token Tok;
    Tok.startToken();
    Tok.setKind(tok::identifier);

    SourceLocation Loc(100);
    Tok.setLocation(Loc);
    Tok.setLength(15);

    EXPECT_EQ(Tok.getLocation(), Loc);
    EXPECT_EQ(Tok.getLength(), 15u);
    EXPECT_EQ(Tok.getEndLoc(), Loc.getLocWithOffset(15));
}

TEST(TokenTest, AnnotationTokenSemantics) {
    Token Tok;
    Tok.startToken();
    Tok.setKind(tok::annot_typename);

    SourceLocation StartLoc(50);
    SourceLocation EndLoc(75);
    Tok.setLocation(StartLoc);
    Tok.setAnnotationEndLoc(EndLoc);

    EXPECT_TRUE(Tok.isAnnotation());
    EXPECT_EQ(Tok.getLocation(), StartLoc);
    EXPECT_EQ(Tok.getAnnotationEndLoc(), EndLoc);
    EXPECT_EQ(Tok.getEndLoc(), EndLoc);
}

TEST(TokenTest, TokenKindCategoryPredicates) {
    Token NumericTok;
    NumericTok.startToken();
    NumericTok.setKind(tok::numeric_constant);
    EXPECT_TRUE(NumericTok.isLiteral());

    Token StringTok;
    StringTok.startToken();
    StringTok.setKind(tok::string_literal);
    EXPECT_TRUE(StringTok.isLiteral());

    Token IdentTok;
    IdentTok.startToken();
    IdentTok.setKind(tok::identifier);
    EXPECT_TRUE(IdentTok.isAnyIdentifier());

    Token RawIdentTok;
    RawIdentTok.startToken();
    RawIdentTok.setKind(tok::raw_identifier);
    EXPECT_TRUE(RawIdentTok.isAnyIdentifier());

    Token AnnotTok;
    AnnotTok.startToken();
    AnnotTok.setKind(tok::annot_cxxscope);
    EXPECT_TRUE(AnnotTok.isAnnotation());
}

TEST(TokenTest, RawIdentifierAndLiteralAccessors) {
    Token Tok;
    Tok.startToken();
    Tok.setKind(tok::raw_identifier);

    const char *RawStr = "my_variable";
    Tok.setRawIdentifierData(RawStr);
    Tok.setLength(11);

    EXPECT_EQ(Tok.getRawIdentifier(), "my_variable");

    Token LitTok;
    LitTok.startToken();
    LitTok.setKind(tok::numeric_constant);
    LitTok.setLiteralData("12345");
    EXPECT_STREQ(LitTok.getLiteralData(), "12345");
}
