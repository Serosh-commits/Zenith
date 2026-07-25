#pragma once
#include "zenith/Basic/SourceLocation.h"
#include "zenith/Basic/FileEntry.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/MemoryBuffer.h"
#include <memory>
#include <vector>

namespace zenith {

class FileManager;
class DiagnosticsEngine;

struct ContentCache {
    const ::llvm::MemoryBuffer *Buffer = nullptr;
    unsigned *SourceLineCache = nullptr;
    unsigned NumLines = 0;
    const FileEntry *OrigEntry = nullptr;
    bool BufferOverridden = false;
    bool IsFileVolatile = false;
    bool IsTransient = false;

    ContentCache() = default;
    ContentCache(const FileEntry *Entry) : OrigEntry(Entry) {}
    ~ContentCache() { delete[] SourceLineCache; }

    ContentCache(const ContentCache &) = delete;
    ContentCache &operator=(const ContentCache &) = delete;

    const ::llvm::MemoryBuffer *getBufferOrNone() const { return Buffer; }

    void setBuffer(std::unique_ptr<::llvm::MemoryBuffer> Buf) {
        Buffer = Buf.release();
    }

    ::llvm::StringRef getBufferData() const {
        if (!Buffer) return "";
        return Buffer->getBuffer();
    }

    unsigned getSize() const {
        if (Buffer) return Buffer->getBufferSize();
        if (OrigEntry) return OrigEntry->Size;
        return 0;
    }
};

class LineOffsetMapping {
    unsigned *Data = nullptr;
    unsigned Size = 0;

public:
    LineOffsetMapping() = default;

    static void computeLineNumbers(const ContentCache &Cache);

    static void compute(const char *BufStart, const char *BufEnd,
                        unsigned *&LineOffsets, unsigned &NumLines) {
        ::llvm::SmallVector<unsigned, 256> Offsets;
        Offsets.push_back(0);
        for (const char *P = BufStart; P < BufEnd; ++P) {
            if (*P == '\n') {
                Offsets.push_back(P - BufStart + 1);
            } else if (*P == '\r') {
                if (P + 1 < BufEnd && P[1] == '\n')
                    ++P;
                Offsets.push_back(P - BufStart + 1);
            }
        }
        NumLines = Offsets.size();
        LineOffsets = new unsigned[NumLines];
        std::copy(Offsets.begin(), Offsets.end(), LineOffsets);
    }
};

namespace SrcMgr {

enum CharacteristicKind {
    C_User,
    C_System,
    C_ExternCSystem,
    C_User_ModuleMap,
    C_System_ModuleMap
};

struct FileInfo {
    ContentCache *Content = nullptr;
    SourceLocation IncludeLoc;
    CharacteristicKind FileCharacteristic = C_User;
    bool HasLineDirectives = false;

    const ContentCache &getContentCache() const { return *Content; }
};

struct ExpansionInfo {
    SourceLocation SpellingLoc;
    SourceLocation ExpansionLocStart;
    SourceLocation ExpansionLocEnd;
    bool ExpansionIsTokenRange = true;

    SourceLocation getSpellingLoc() const { return SpellingLoc; }
    SourceLocation getExpansionLocStart() const { return ExpansionLocStart; }
    SourceLocation getExpansionLocEnd() const { return ExpansionLocEnd; }

    static ExpansionInfo create(SourceLocation SpellingLoc,
                                SourceLocation Start, SourceLocation End,
                                bool IsTokenRange = true) {
        ExpansionInfo EI;
        EI.SpellingLoc = SpellingLoc;
        EI.ExpansionLocStart = Start;
        EI.ExpansionLocEnd = End;
        EI.ExpansionIsTokenRange = IsTokenRange;
        return EI;
    }

    bool isMacroArgExpansion() const {
        return ExpansionLocStart == ExpansionLocEnd;
    }

    bool isFunctionMacroExpansion() const {
        return ExpansionLocStart != ExpansionLocEnd;
    }
};

struct SLocEntry {
    unsigned Offset = 0;
    bool IsExpansion = false;

    union {
        FileInfo File;
        ExpansionInfo Expansion;
    };

    SLocEntry() : File() {}

    bool isFile() const { return !IsExpansion; }
    bool isExpansion() const { return IsExpansion; }

    const FileInfo &getFile() const { return File; }
    const ExpansionInfo &getExpansion() const { return Expansion; }
};

}

class SourceManager {
    std::vector<SrcMgr::SLocEntry> LocalSLocEntryTable;
    unsigned NextLocalOffset = 0;

    ::llvm::BumpPtrAllocator ContentCacheAlloc;
    std::vector<std::unique_ptr<ContentCache>> ContentCaches;

    FileManager &FileMgr;
    DiagnosticsEngine &Diag;

    mutable FileID LastFileIDLookup;

    FileID MainFileID;

public:
    SourceManager(FileManager &FM, DiagnosticsEngine &Diags);

    FileManager &getFileManager() const { return FileMgr; }
    DiagnosticsEngine &getDiagnostics() const { return Diag; }

    FileID getMainFileID() const { return MainFileID; }
    void setMainFileID(FileID FID) { MainFileID = FID; }

    FileID createFileID(const FileEntry *SourceFile,
                        SourceLocation IncludeLoc,
                        SrcMgr::CharacteristicKind FileCharacter = SrcMgr::C_User);

    FileID createFileID(std::unique_ptr<::llvm::MemoryBuffer> Buffer,
                        SrcMgr::CharacteristicKind FileCharacter = SrcMgr::C_User,
                        SourceLocation IncludeLoc = SourceLocation());

    SourceLocation createExpansionLoc(SourceLocation SpellingLoc,
                                      SourceLocation ExpansionLocStart,
                                      SourceLocation ExpansionLocEnd,
                                      unsigned Length,
                                      bool ExpansionIsTokenRange = true);

    FileID getFileID(SourceLocation SpellingLoc) const;

    SourceLocation getLocForStartOfFile(FileID FID) const;
    SourceLocation getLocForEndOfFile(FileID FID) const;

    std::pair<FileID, unsigned> getDecomposedLoc(SourceLocation Loc) const;
    std::pair<FileID, unsigned> getDecomposedExpansionLoc(SourceLocation Loc) const;
    std::pair<FileID, unsigned> getDecomposedSpellingLoc(SourceLocation Loc) const;

    SourceLocation getSpellingLoc(SourceLocation Loc) const;
    SourceLocation getExpansionLoc(SourceLocation Loc) const;
    SourceLocation getFileLoc(SourceLocation Loc) const;

    PresumedLoc getPresumedLoc(SourceLocation Loc) const;

    const char *getCharacterData(SourceLocation SL) const;

    ::llvm::StringRef getBufferData(FileID FID) const;
    const ::llvm::MemoryBuffer *getBuffer(FileID FID) const;

    unsigned getLineNumber(FileID FID, unsigned FilePos) const;
    unsigned getColumnNumber(FileID FID, unsigned FilePos) const;

    unsigned getSpellingLineNumber(SourceLocation Loc) const;
    unsigned getSpellingColumnNumber(SourceLocation Loc) const;
    unsigned getExpansionLineNumber(SourceLocation Loc) const;
    unsigned getExpansionColumnNumber(SourceLocation Loc) const;

    bool isBeforeInTranslationUnit(SourceLocation LHS, SourceLocation RHS) const;

    bool isInMainFile(SourceLocation Loc) const;

    unsigned getNumSLocEntries() const { return LocalSLocEntryTable.size(); }

    const SrcMgr::SLocEntry &getSLocEntry(FileID FID) const;

private:
    ContentCache *createContentCache(const FileEntry *FileEnt);
    ContentCache *createMemBufferContentCache(std::unique_ptr<::llvm::MemoryBuffer> Buffer);

    FileID createFileIDForSLocEntry(SrcMgr::SLocEntry Entry, unsigned Size);

    FileID getFileIDSlow(unsigned Offset) const;

    void computeLineTable(FileID FID) const;
};

}
