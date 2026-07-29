#include "zenith/Lex/Token.h"
#include "zenith/Basic/IdentifierTable.h"

namespace zenith::tok {

const char *getTokenName(TokenKind Kind) {
    switch (Kind) {
#define TOK(X) case X: return #X;
#define KEYWORD(X,Y) case kw_ ## X: return #X;
#include "zenith/Lex/TokenKinds.def"
    default: return "unknown";
    }
}

const char *getPunctuatorSpelling(TokenKind Kind) {
    switch (Kind) {
#define PUNCTUATOR(X,Y) case X: return Y;
#include "zenith/Lex/TokenKinds.def"
    default: return nullptr;
    }
}

const char *getKeywordSpelling(TokenKind Kind) {
    switch (Kind) {
#define KEYWORD(X,Y) case kw_ ## X: return #X;
#include "zenith/Lex/TokenKinds.def"
    default: return nullptr;
    }
}

}

namespace zenith {

::llvm::StringRef Token::getName() const {
    if (is(tok::raw_identifier))
        return getRawIdentifier();
    if (IdentifierInfo *II = getIdentifierInfo()) {
        return II->getName();
    }
    return ::llvm::StringRef(tok::getTokenName(getKind()));
}

}
