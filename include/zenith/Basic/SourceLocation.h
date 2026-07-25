#pragma once
#include "llvm/ADT/DenseMapInfo.h"
#include "llvm/Support/raw_ostream.h"
#include <compare>
#include <cstdint>
#include <functional>
#include <utility>

namespace zenith {

class SourceManager;

class FileID {
    int ID = 0;

    friend class SourceManager;
    friend class ASTWriter;
    friend class ASTReader;

    static FileID get(int V) {
        FileID F;
        F.ID = V;
        return F;
    }

    int getOpaqueValue() const { return ID; }

public:
    FileID() = default;
    explicit FileID(int id) : ID(id) {}

    bool isValid() const { return ID != 0; }
    bool isInvalid() const { return ID == 0; }

    int getID() const { return ID; }

    static FileID getSentinel() { return get(-1); }
    unsigned getHashValue() const { return static_cast<unsigned>(ID); }

    bool operator==(const FileID &RHS) const { return ID == RHS.ID; }
    auto operator<=>(const FileID &RHS) const = default;
};

using FileIDAndOffset = std::pair<FileID, unsigned>;

class SourceLocation {
public:
    using UIntTy = uint32_t;
    using IntTy = int32_t;

private:
    UIntTy ID = 0;

    enum : UIntTy { MacroIDBit = 1ULL << (8 * sizeof(UIntTy) - 1) };

    friend class SourceManager;
    friend class ASTWriter;
    friend class ASTReader;
    friend class SourceLocationEncoding;

public:
    UIntTy getOffset() const { return ID & ~MacroIDBit; }

private:
    static SourceLocation getFileLoc(UIntTy Offset) {
        SourceLocation L;
        L.ID = Offset;
        return L;
    }

    static SourceLocation getMacroLoc(UIntTy Offset) {
        SourceLocation L;
        L.ID = MacroIDBit | Offset;
        return L;
    }

public:
    SourceLocation() = default;
    constexpr SourceLocation(unsigned id) : ID(id) {}

    bool isValid() const { return ID != 0; }
    bool isInvalid() const { return ID == 0; }
    bool isFileID() const { return (ID & MacroIDBit) == 0; }
    bool isMacroID() const { return (ID & MacroIDBit) != 0; }

    SourceLocation getLocWithOffset(IntTy Offset) const {
        SourceLocation L;
        L.ID = ID + Offset;
        return L;
    }

    UIntTy getRawEncoding() const { return ID; }

    static SourceLocation getFromRawEncoding(UIntTy Encoding) {
        SourceLocation X;
        X.ID = Encoding;
        return X;
    }

    void *getPtrEncoding() const {
        return (void *)(uintptr_t)getRawEncoding();
    }

    static SourceLocation getFromPtrEncoding(const void *Encoding) {
        return getFromRawEncoding((UIntTy)(uintptr_t)Encoding);
    }

    static bool isPairOfFileLocations(SourceLocation Start, SourceLocation End) {
        return Start.isValid() && Start.isFileID() && End.isValid() && End.isFileID();
    }

    unsigned getHashValue() const;

    void print(::llvm::raw_ostream &OS, const SourceManager &SM) const;
    std::string printToString(const SourceManager &SM) const;
    void dump(const SourceManager &SM) const;

    auto operator<=>(const SourceLocation &) const = default;
    bool operator==(const SourceLocation &) const = default;
};

static_assert(sizeof(SourceLocation) == 4);
static_assert(std::is_trivially_copyable_v<SourceLocation>);

class SourceRange {
    SourceLocation B;
    SourceLocation E;

public:
    SourceRange() = default;
    SourceRange(SourceLocation loc) : B(loc), E(loc) {}
    SourceRange(SourceLocation begin, SourceLocation end) : B(begin), E(end) {}

    SourceLocation getBegin() const { return B; }
    SourceLocation getEnd() const { return E; }

    void setBegin(SourceLocation b) { B = b; }
    void setEnd(SourceLocation e) { E = e; }

    bool isValid() const { return B.isValid() && E.isValid(); }
    bool isInvalid() const { return !isValid(); }

    bool fullyContains(const SourceRange &other) const {
        return B <= other.B && E >= other.E;
    }

    bool operator==(const SourceRange &X) const {
        return B == X.B && E == X.E;
    }

    bool operator!=(const SourceRange &X) const {
        return B != X.B || E != X.E;
    }
};

class CharSourceRange {
    SourceRange Range;
    bool IsTokenRange = false;

public:
    CharSourceRange() = default;
    CharSourceRange(SourceRange R, bool ITR) : Range(R), IsTokenRange(ITR) {}

    static CharSourceRange getTokenRange(SourceRange R) {
        return CharSourceRange(R, true);
    }

    static CharSourceRange getTokenRange(SourceLocation B, SourceLocation E) {
        return getTokenRange(SourceRange(B, E));
    }

    static CharSourceRange getCharRange(SourceRange R) {
        return CharSourceRange(R, false);
    }

    static CharSourceRange getCharRange(SourceLocation B, SourceLocation E) {
        return getCharRange(SourceRange(B, E));
    }

    bool isTokenRange() const { return IsTokenRange; }
    bool isCharRange() const { return !IsTokenRange; }

    SourceLocation getBegin() const { return Range.getBegin(); }
    SourceLocation getEnd() const { return Range.getEnd(); }
    SourceRange getAsRange() const { return Range; }

    void setBegin(SourceLocation b) { Range.setBegin(b); }
    void setEnd(SourceLocation e) { Range.setEnd(e); }
    void setTokenRange(bool TR) { IsTokenRange = TR; }

    bool isValid() const { return Range.isValid(); }
    bool isInvalid() const { return !isValid(); }
};

class PresumedLoc {
    const char *Filename = nullptr;
    FileID ID;
    unsigned Line = 0;
    unsigned Col = 0;
    SourceLocation IncludeLoc;

public:
    PresumedLoc() = default;
    PresumedLoc(const char *FN, FileID FID, unsigned Ln, unsigned Co, SourceLocation IL)
        : Filename(FN), ID(FID), Line(Ln), Col(Co), IncludeLoc(IL) {}

    bool isValid() const { return Filename != nullptr; }
    bool isInvalid() const { return Filename == nullptr; }

    const char *getFilename() const { return Filename; }
    FileID getFileID() const { return ID; }
    unsigned getLine() const { return Line; }
    unsigned getColumn() const { return Col; }
    SourceLocation getIncludeLoc() const { return IncludeLoc; }
};

class FullSourceLoc : public SourceLocation {
    const SourceManager *SrcMgr = nullptr;

public:
    FullSourceLoc() = default;

    explicit FullSourceLoc(SourceLocation Loc, const SourceManager &SM)
        : SourceLocation(Loc), SrcMgr(&SM) {}

    bool hasManager() const { return SrcMgr != nullptr; }

    const SourceManager &getManager() const { return *SrcMgr; }

    FileID getFileID() const;
    FullSourceLoc getExpansionLoc() const;
    FullSourceLoc getSpellingLoc() const;
    PresumedLoc getPresumedLoc() const;

    unsigned getExpansionLineNumber() const;
    unsigned getExpansionColumnNumber() const;
    unsigned getSpellingLineNumber() const;
    unsigned getSpellingColumnNumber() const;

    const char *getCharacterData() const;
    unsigned getFileOffset() const;

    bool isBeforeInTranslationUnitThan(SourceLocation Loc) const;
    bool isBeforeInTranslationUnitThan(FullSourceLoc Loc) const;

    struct BeforeThanCompare {
        bool operator()(const FullSourceLoc &lhs, const FullSourceLoc &rhs) const {
            return lhs.isBeforeInTranslationUnitThan(rhs);
        }
    };

    friend bool operator==(const FullSourceLoc &LHS, const FullSourceLoc &RHS) {
        return LHS.getRawEncoding() == RHS.getRawEncoding() &&
               LHS.SrcMgr == RHS.SrcMgr;
    }
};

}

namespace llvm {

template <> struct DenseMapInfo<zenith::FileID> {
    static inline zenith::FileID getEmptyKey() { return zenith::FileID(); }
    static inline zenith::FileID getTombstoneKey() { return zenith::FileID::getSentinel(); }
    static unsigned getHashValue(zenith::FileID S) { return S.getHashValue(); }
    static bool isEqual(zenith::FileID LHS, zenith::FileID RHS) { return LHS == RHS; }
};

template <> struct DenseMapInfo<zenith::SourceLocation> {
    static inline zenith::SourceLocation getEmptyKey() {
        constexpr zenith::SourceLocation::UIntTy Zero = 0;
        return zenith::SourceLocation::getFromRawEncoding(~Zero);
    }

    static inline zenith::SourceLocation getTombstoneKey() {
        constexpr zenith::SourceLocation::UIntTy Zero = 0;
        return zenith::SourceLocation::getFromRawEncoding(~Zero - 1);
    }

    static unsigned getHashValue(zenith::SourceLocation Loc) {
        return Loc.getHashValue();
    }

    static bool isEqual(zenith::SourceLocation LHS, zenith::SourceLocation RHS) {
        return LHS == RHS;
    }
};

template <> struct DenseMapInfo<zenith::SourceRange> {
    static inline zenith::SourceRange getEmptyKey() {
        return zenith::SourceRange(DenseMapInfo<zenith::SourceLocation>::getEmptyKey());
    }

    static inline zenith::SourceRange getTombstoneKey() {
        return zenith::SourceRange(DenseMapInfo<zenith::SourceLocation>::getTombstoneKey());
    }

    static unsigned getHashValue(zenith::SourceRange R) {
        return llvm::detail::combineHashValue(
            R.getBegin().getHashValue(),
            R.getEnd().getHashValue());
    }

    static bool isEqual(zenith::SourceRange LHS, zenith::SourceRange RHS) {
        return LHS == RHS;
    }
};

}

namespace std {
template <> struct hash<zenith::SourceLocation> {
    size_t operator()(const zenith::SourceLocation &Loc) const {
        return hash<unsigned>()(Loc.getRawEncoding());
    }
};
}
