#include "zenith/Parse/Parser.h"

namespace zenith {

bool Parser::ParseTranslationUnit() {
    ConsumeToken();
    while (Tok.getKind() != tok::eof) {
        bool Res = ParseTopLevelDecl();
        if (Res && Tok.getKind() == tok::unknown) break;
    }
    return false;
}

bool Parser::ParseTopLevelDecl() {
    ParseDeclaration();
    return false;
}

}
