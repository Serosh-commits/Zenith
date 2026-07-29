#include "zenith/Basic/SourceLocation.h"
#include "zenith/Basic/SourceManager.h"
#include "llvm/ADT/DenseMapInfo.h"
#include "llvm/ADT/FoldingSet.h"
#include "llvm/Support/raw_ostream.h"
#include <cassert>
#include <string>
#include <utility>

namespace zenith {

static_assert(std::is_trivially_destructible_v<SourceLocation>,
              "SourceLocation must be trivially destructible because it is used in unions");

static_assert(std::is_trivially_destructible_v<SourceRange>,
              "SourceRange must be trivially destructible because it is used in unions");

unsigned SourceLocation::getHashValue() const {
    return llvm::DenseMapInfo<UIntTy>::getHashValue(ID);
}

void SourceLocation::print(::llvm::raw_ostream &OS, const SourceManager &SM) const {
    if (isInvalid()) {
        OS << "<invalid loc>";
        return;
    }

    if (isFileID()) {
        PresumedLoc PLoc = SM.getPresumedLoc(*this);
        if (PLoc.isValid()) {
            OS << PLoc.getFilename() << ':' << PLoc.getLine() << ':' << PLoc.getColumn();
            return;
        }
    }

    OS << "<loc:" << getRawEncoding() << ">";
}

std::string SourceLocation::printToString(const SourceManager &SM) const {
    std::string S;
    ::llvm::raw_string_ostream OS(S);
    print(OS, SM);
    return S;
}

void SourceLocation::dump(const SourceManager &SM) const {
    print(::llvm::errs(), SM);
    ::llvm::errs() << "\n";
}

FileID FullSourceLoc::getFileID() const {
    return SrcMgr->getFileID(*this);
}

FullSourceLoc FullSourceLoc::getExpansionLoc() const {
    return FullSourceLoc(SrcMgr->getExpansionLoc(*this), *SrcMgr);
}

FullSourceLoc FullSourceLoc::getSpellingLoc() const {
    return FullSourceLoc(SrcMgr->getSpellingLoc(*this), *SrcMgr);
}

FullSourceLoc FullSourceLoc::getFileLoc() const {
    return FullSourceLoc(SrcMgr->getFileLoc(*this), *SrcMgr);
}

PresumedLoc FullSourceLoc::getPresumedLoc() const {
    return SrcMgr->getPresumedLoc(*this);
}

unsigned FullSourceLoc::getExpansionLineNumber() const {
    SourceLocation ExpLoc = SrcMgr->getExpansionLoc(*this);
    FileID FID = SrcMgr->getFileID(ExpLoc);
    unsigned Offset = ExpLoc.getOffset() - SrcMgr->getLocForStartOfFile(FID).getOffset();
    return SrcMgr->getLineNumber(FID, Offset);
}

unsigned FullSourceLoc::getExpansionColumnNumber() const {
    SourceLocation ExpLoc = SrcMgr->getExpansionLoc(*this);
    FileID FID = SrcMgr->getFileID(ExpLoc);
    unsigned Offset = ExpLoc.getOffset() - SrcMgr->getLocForStartOfFile(FID).getOffset();
    return SrcMgr->getColumnNumber(FID, Offset);
}

unsigned FullSourceLoc::getSpellingLineNumber() const {
    SourceLocation SpLoc = SrcMgr->getSpellingLoc(*this);
    FileID FID = SrcMgr->getFileID(SpLoc);
    unsigned Offset = SpLoc.getOffset() - SrcMgr->getLocForStartOfFile(FID).getOffset();
    return SrcMgr->getLineNumber(FID, Offset);
}

unsigned FullSourceLoc::getSpellingColumnNumber() const {
    SourceLocation SpLoc = SrcMgr->getSpellingLoc(*this);
    FileID FID = SrcMgr->getFileID(SpLoc);
    unsigned Offset = SpLoc.getOffset() - SrcMgr->getLocForStartOfFile(FID).getOffset();
    return SrcMgr->getColumnNumber(FID, Offset);
}

const char *FullSourceLoc::getCharacterData() const {
    return SrcMgr->getCharacterData(*this);
}

unsigned FullSourceLoc::getFileOffset() const {
    FileID FID = SrcMgr->getFileID(*this);
    return getOffset() - SrcMgr->getLocForStartOfFile(FID).getOffset();
}

bool FullSourceLoc::isBeforeInTranslationUnitThan(SourceLocation Loc) const {
    return SrcMgr->isBeforeInTranslationUnit(*this, Loc);
}

bool FullSourceLoc::isBeforeInTranslationUnitThan(FullSourceLoc Loc) const {
    return isBeforeInTranslationUnitThan((SourceLocation)Loc);
}

}
