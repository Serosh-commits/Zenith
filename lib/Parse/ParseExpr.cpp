#include "zenith/Parse/Parser.h"
#include <cstdlib>
#include <string>

namespace zenith {

ExprResult Parser::ParseExpression() {
    return ParseAssignmentExpression();
}

ExprResult Parser::ParseAssignmentExpression() {
    ExprResult LHS = ParseBinaryExpression(1);
    if (Tok.getKind() == tok::equal) {
        SourceLocation OpLoc = Tok.getLocation();
        ConsumeToken();
        ExprResult RHS = ParseAssignmentExpression();
        LHS = Actions.ActOnBinOp(OpLoc, tok::equal, LHS.get(), RHS.get());
    }
    return LHS;
}

ExprResult Parser::ParseBinaryExpression(unsigned MinPrec) {
    ExprResult LHS = ParseCastExpression();
    while (true) {
        unsigned Prec = 0;
        if (Tok.getKind() == tok::plus || Tok.getKind() == tok::minus) Prec = 11;
        else if (Tok.getKind() == tok::star || Tok.getKind() == tok::slash) Prec = 12;
        else if (Tok.getKind() == tok::greater || Tok.getKind() == tok::less ||
                 Tok.getKind() == tok::greaterequal || Tok.getKind() == tok::lessequal) Prec = 9;
        else if (Tok.getKind() == tok::equalequal || Tok.getKind() == tok::exclaimequal) Prec = 8;
        
        if (Prec < MinPrec) break;
        tok::TokenKind Op = Tok.getKind();
        SourceLocation OpLoc = Tok.getLocation();
        ConsumeToken();
        ExprResult RHS = ParseBinaryExpression(Prec + 1);
        LHS = Actions.ActOnBinOp(OpLoc, Op, LHS.get(), RHS.get());
    }
    return LHS;
}

ExprResult Parser::ParseCastExpression() {
    if (Tok.getKind() == tok::numeric_constant) {
        SourceLocation Loc = Tok.getLocation();
        uint64_t Val = 0;
        if (const char *Lit = Tok.getLiteralData()) {
            Val = std::strtoull(Lit, nullptr, 10);
        }
        ConsumeToken();
        return Actions.ActOnIntegerConstant(Loc, Val);
    }
    if (Tok.getKind() == tok::identifier || Tok.getKind() == tok::raw_identifier) {
        IdentifierInfo *II = Tok.getIdentifierInfo();
        SourceLocation Loc = Tok.getLocation();
        ConsumeToken();
        return ParsePostfixExpression(Actions.ActOnIdExpression(Actions.getCurScope(), II, Loc));
    }
    if (Tok.getKind() == tok::l_paren) {
        return ParseParenExpression();
    }
    return ExprError();
}

ExprResult Parser::ParsePostfixExpression(ExprResult LHS) {
    if (Tok.getKind() == tok::l_paren) {
        SourceLocation L = ConsumeParen();
        std::vector<Expr*> Args;
        while (Tok.getKind() != tok::r_paren && Tok.getKind() != tok::eof) {
            Args.push_back(ParseExpression().get());
            if (Tok.getKind() == tok::comma) ConsumeToken();
        }
        SourceLocation R = ConsumeParen();
        return Actions.ActOnCallExpr(LHS.get(), L, Args, R);
    }
    return LHS;
}

ExprResult Parser::ParseParenExpression() {
    SourceLocation L = ConsumeParen();
    ExprResult E = ParseExpression();
    SourceLocation R = ConsumeParen();
    return Actions.ActOnParenExpr(L, R, E.get());
}

ExprResult Parser::ParseUnaryExpression() {
    return ParseCastExpression();
}

}
