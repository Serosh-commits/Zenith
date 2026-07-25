#pragma once

#include "llvm/ADT/DenseMapInfo.h"

namespace zenith::tok {

enum TokenKind : unsigned short {
#define TOK(X) X,
#include "zenith/Lex/TokenKinds.def"
    NUM_TOKENS
};

enum PPKeywordKind {
#define PPKEYWORD(X) pp_##X,
#include "zenith/Lex/TokenKinds.def"
    NUM_PP_KEYWORDS
};

enum KeywordFlags {
    KEYALL = 0x1,
    KEYCXX = 0x2,
    KEYC99 = 0x4,
    KEYCXX11 = 0x8,
    KEYCXX20 = 0x10,
    KEYC23 = 0x20
};

const char *getTokenName(TokenKind Kind);
const char *getPunctuatorSpelling(TokenKind Kind);
const char *getKeywordSpelling(TokenKind Kind);

inline bool isAnyIdentifier(TokenKind K) {
    return K == identifier || K == raw_identifier;
}

inline bool isLiteral(TokenKind K) {
    return K >= numeric_constant && K <= string_literal;
}

inline bool isAnnotation(TokenKind K) {
    return K >= annot_cxxscope && K <= annot_embed;
}

}

namespace llvm {
template <> struct DenseMapInfo<zenith::tok::PPKeywordKind> {
    static inline zenith::tok::PPKeywordKind getEmptyKey() {
        return zenith::tok::pp_not_keyword;
    }
    static inline zenith::tok::PPKeywordKind getTombstoneKey() {
        return zenith::tok::NUM_PP_KEYWORDS;
    }
    static unsigned getHashValue(const zenith::tok::PPKeywordKind &Val) {
        return static_cast<unsigned>(Val);
    }
    static bool isEqual(const zenith::tok::PPKeywordKind &LHS,
                        const zenith::tok::PPKeywordKind &RHS) {
        return LHS == RHS;
    }
};
}
