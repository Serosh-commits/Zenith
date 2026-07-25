#pragma once
#include "zenith/Lex/Preprocessor.h"
#include "zenith/Sema/Sema.h"
#include "zenith/Basic/Diagnostic.h"

namespace zenith {

class Parser {
    Preprocessor &PP;
    Sema &Actions;
    DiagnosticsEngine &Diags;
    Token Tok;

public:
    Parser(Preprocessor &PP, Sema &Actions, DiagnosticsEngine &Diags)
        : PP(PP), Actions(Actions), Diags(Diags) {
        ConsumeToken();
    }

    void ConsumeToken() {
        PP.Lex(Tok);
    }

    bool ExpectAndConsume(tok::TokenKind Expected) {
        if (Tok.getKind() == Expected) {
            ConsumeToken();
            return true;
        }
        return false;
    }

    SourceLocation ConsumeParen() { SourceLocation L = Tok.getLocation(); ConsumeToken(); return L; }
    SourceLocation ConsumeBrace() { SourceLocation L = Tok.getLocation(); ConsumeToken(); return L; }
    SourceLocation ConsumeBracket() { SourceLocation L = Tok.getLocation(); ConsumeToken(); return L; }

    bool ParseTranslationUnit();
    bool ParseTopLevelDecl();
    Decl *ParseDeclaration();
    void ParseDeclarationSpecifiers(DeclSpec &DS);
    Decl *ParseFunctionDefinition(DeclSpec &DS, IdentifierInfo *Name, SourceLocation NameLoc);
    ParmVarDecl *ParseParameterDeclaration();

    ExprResult ParseExpression();
    ExprResult ParseAssignmentExpression();
    ExprResult ParseBinaryExpression(unsigned MinPrec);
    ExprResult ParseCastExpression();
    ExprResult ParsePostfixExpression(ExprResult LHS);
    ExprResult ParseParenExpression();
    ExprResult ParseUnaryExpression();

    StmtResult ParseStatement();
    StmtResult ParseCompoundStatement();
    StmtResult ParseIfStatement();
    StmtResult ParseWhileStatement();
    StmtResult ParseForStatement();
    StmtResult ParseReturnStatement();
    StmtResult ParseExpressionOrDeclarationStatement();
};

}
