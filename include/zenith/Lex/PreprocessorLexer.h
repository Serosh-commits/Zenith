#pragma once

#include "zenith/Basic/SourceLocation.h"
#include "zenith/Lex/Token.h"
#include "llvm/ADT/SmallVector.h"

namespace zenith {

class Preprocessor;

class PreprocessorLexer {
protected:
    Preprocessor *PP = nullptr;
    const FileID FID;
    unsigned InitialNumSLocEntries = 0;

    bool ParsingPreprocessorDirective = false;
    bool ParsingFilename = false;
    bool LexingRawMode = false;

    ::llvm::SmallVector<PPConditionalInfo, 4> ConditionalStack;

public:
    PreprocessorLexer() : FID(FileID()) {}
    explicit PreprocessorLexer(Preprocessor *pp, FileID fid)
        : PP(pp), FID(fid) {}
    virtual ~PreprocessorLexer() = default;

    virtual void IndirectLex(Token &Result) = 0;
    virtual SourceLocation getSourceLocation() = 0;

    FileID getFileID() const { return FID; }
    bool isLexingRawMode() const { return LexingRawMode; }

    void pushConditionalLevel(SourceLocation IfLoc, bool WasSkipping,
                              bool FoundNonSkip, bool FoundElse) {
        PPConditionalInfo CI;
        CI.IfLoc = IfLoc;
        CI.WasSkipping = WasSkipping;
        CI.FoundNonSkip = FoundNonSkip;
        CI.FoundElse = FoundElse;
        ConditionalStack.push_back(CI);
    }

    bool popConditionalLevel(PPConditionalInfo &CI) {
        if (ConditionalStack.empty()) return true;
        CI = ConditionalStack.pop_back_val();
        return false;
    }

    bool popConditionalLevel() {
        if (ConditionalStack.empty()) return true;
        ConditionalStack.pop_back();
        return false;
    }

    unsigned getConditionalStackDepth() const { return ConditionalStack.size(); }

    const PPConditionalInfo *peekConditionalLevel() const {
        if (ConditionalStack.empty()) return nullptr;
        return &ConditionalStack.back();
    }
};

}
