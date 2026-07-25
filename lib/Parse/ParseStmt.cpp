#include "zenith/Parse/Parser.h"

namespace zenith {

StmtResult Parser::ParseStatement() {
    if (Tok.getKind() == tok::l_brace) return ParseCompoundStatement();
    if (Tok.getKind() == tok::kw_if) return ParseIfStatement();
    if (Tok.getKind() == tok::kw_while) return ParseWhileStatement();
    if (Tok.getKind() == tok::kw_for) return ParseForStatement();
    if (Tok.getKind() == tok::kw_return) return ParseReturnStatement();
    if (Tok.getKind() == tok::semi) {
        SourceLocation L = Tok.getLocation();
        ConsumeToken();
        return new NullStmt(L);
    }
    return ParseExpressionOrDeclarationStatement();
}

StmtResult Parser::ParseCompoundStatement() {
    SourceLocation L = ConsumeBrace();
    Actions.PushScope(Scope::BlockScope);
    std::vector<Stmt*> Stmts;
    while (Tok.getKind() != tok::r_brace && Tok.getKind() != tok::eof) {
        Stmts.push_back(ParseStatement().get());
    }
    SourceLocation R = ConsumeBrace();
    Actions.PopScope();
    return Actions.ActOnCompoundStmt(L, R, Stmts);
}

StmtResult Parser::ParseIfStatement() {
    SourceLocation L = Tok.getLocation();
    ConsumeToken();
    ConsumeParen();
    ExprResult Cond = ParseExpression();
    ConsumeParen();
    StmtResult Then = ParseStatement();
    StmtResult Else = StmtError();
    if (Tok.getKind() == tok::kw_else) {
        ConsumeToken();
        Else = ParseStatement();
    }
    return Actions.ActOnIfStmt(L, Cond.get(), Then.get(), Else.get());
}

StmtResult Parser::ParseWhileStatement() {
    SourceLocation L = Tok.getLocation();
    ConsumeToken();
    ConsumeParen();
    ExprResult Cond = ParseExpression();
    ConsumeParen();
    StmtResult Body = ParseStatement();
    return Actions.ActOnWhileStmt(L, Cond.get(), Body.get());
}

StmtResult Parser::ParseForStatement() {
    SourceLocation L = Tok.getLocation();
    ConsumeToken();
    ConsumeParen();
    StmtResult Init;
    if (Tok.getKind() != tok::semi) Init = ParseExpressionOrDeclarationStatement();
    else ConsumeToken();
    ExprResult Cond;
    if (Tok.getKind() != tok::semi) Cond = ParseExpression();
    ConsumeToken();
    ExprResult Inc;
    if (Tok.getKind() != tok::r_paren) Inc = ParseExpression();
    ConsumeParen();
    StmtResult Body = ParseStatement();
    return Actions.ActOnForStmt(L, Init.get(), Cond.get(), Inc.get(), Body.get());
}

StmtResult Parser::ParseReturnStatement() {
    SourceLocation L = Tok.getLocation();
    ConsumeToken();
    ExprResult E;
    if (Tok.getKind() != tok::semi) E = ParseExpression();
    ConsumeToken();
    return Actions.ActOnReturnStmt(L, E.get());
}

StmtResult Parser::ParseExpressionOrDeclarationStatement() {
    if (Tok.getKind() == tok::kw_int || Tok.getKind() == tok::kw_void) {
        Decl *D = ParseDeclaration();
        return Actions.ActOnDeclStmt(D);
    }
    ExprResult E = ParseExpression();
    ConsumeToken();
    return Actions.ActOnExprStmt(E.get());
}

}
