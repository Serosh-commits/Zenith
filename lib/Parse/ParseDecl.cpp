#include "zenith/Parse/Parser.h"

namespace zenith {

void Parser::ParseDeclarationSpecifiers(DeclSpec &DS) {
    while (true) {
        if (Tok.getKind() == tok::kw_int || Tok.getKind() == tok::kw_void) {
            DS.SetTypeSpecType(Tok.getKind());
            ConsumeToken();
        } else {
            break;
        }
    }
}

Decl *Parser::ParseDeclaration() {
    DeclSpec DS;
    ParseDeclarationSpecifiers(DS);
    IdentifierInfo *Name = nullptr;
    SourceLocation NameLoc = Tok.getLocation();
    if (Tok.getKind() == tok::identifier) {
        Name = Tok.getIdentifierInfo();
        ConsumeToken();
    }
    if (Tok.getKind() == tok::l_paren) {
        return ParseFunctionDefinition(DS, Name, NameLoc);
    }
    ExprResult Init;
    if (Tok.getKind() == tok::equal) {
        ConsumeToken();
        Init = ParseExpression();
    }
    ExpectAndConsume(tok::semi);
    return Actions.ActOnVariableDeclarator(Actions.getCurScope(), DS, Name, NameLoc, Init.get());
}

Decl *Parser::ParseFunctionDefinition(DeclSpec &DS, IdentifierInfo *Name, SourceLocation NameLoc) {
    ConsumeParen();
    Actions.PushScope(Scope::FnScope | Scope::DeclScope);
    ::llvm::SmallVector<ParmVarDecl*, 4> Params;
    while (Tok.getKind() != tok::r_paren && Tok.getKind() != tok::eof) {
        Params.push_back(ParseParameterDeclaration());
        if (Tok.getKind() == tok::comma) ConsumeToken();
    }
    ConsumeParen();
    Decl *FD = Actions.ActOnFunctionDeclarator(Actions.getCurScope(), Name, Actions.BuildTypeFromDeclSpec(DS), Params, NameLoc);
    Actions.ActOnStartOfFunctionDef(Actions.getCurScope(), FD);
    StmtResult Body = ParseCompoundStatement();
    Actions.PopScope();
    return Actions.ActOnFinishFunctionBody(FD, Body.get());
}

ParmVarDecl *Parser::ParseParameterDeclaration() {
    DeclSpec DS;
    ParseDeclarationSpecifiers(DS);
    IdentifierInfo *Name = nullptr;
    SourceLocation Loc = Tok.getLocation();
    if (Tok.getKind() == tok::identifier) {
        Name = Tok.getIdentifierInfo();
        ConsumeToken();
    }
    return Actions.ActOnParamDeclarator(DS, Name, Loc);
}

}
