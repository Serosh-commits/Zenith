#include "zenith/Basic/SourceManager.h"
#include "zenith/Basic/FileManager.h"
#include "zenith/Basic/Diagnostic.h"
#include "llvm/Support/MemoryBuffer.h"
#include <algorithm>

namespace zenith {

SourceManager::SourceManager(FileManager &FM, DiagnosticsEngine &Diags)
    : FileMgr(FM), Diag(Diags) {
    SrcMgr::SLocEntry Sentinel;
    Sentinel.Offset = 0;
    Sentinel.IsExpansion = false;
    Sentinel.File.Content = nullptr;
    LocalSLocEntryTable.push_back(Sentinel);
    NextLocalOffset = 1;
}

ContentCache *SourceManager::createContentCache(const FileEntry *FileEnt) {
    auto CC = std::make_unique<ContentCache>(FileEnt);
    ContentCache *Ret = CC.get();
    ContentCaches.push_back(std::move(CC));
    return Ret;
}

ContentCache *SourceManager::createMemBufferContentCache(
    std::unique_ptr<::llvm::MemoryBuffer> Buffer) {
    auto CC = std::make_unique<ContentCache>();
    CC->setBuffer(std::move(Buffer));
    ContentCache *Ret = CC.get();
    ContentCaches.push_back(std::move(CC));
    return Ret;
}

FileID SourceManager::createFileIDForSLocEntry(SrcMgr::SLocEntry Entry,
                                                unsigned Size) {
    Entry.Offset = NextLocalOffset;
    LocalSLocEntryTable.push_back(Entry);
    NextLocalOffset += Size + 1;
    return FileID(static_cast<int>(LocalSLocEntryTable.size()) - 1);
}

FileID SourceManager::createFileID(const FileEntry *SourceFile,
                                    SourceLocation IncludeLoc,
                                    SrcMgr::CharacteristicKind FileCharacter) {
    ContentCache *CC = createContentCache(SourceFile);

    auto BufOrErr = FileMgr.getBufferForFile(SourceFile->Name);
    unsigned Size = 0;
    if (BufOrErr) {
        Size = (*BufOrErr)->getBufferSize();
        CC->setBuffer(std::move(*BufOrErr));
    }

    SrcMgr::SLocEntry Entry;
    Entry.IsExpansion = false;
    Entry.File.Content = CC;
    Entry.File.IncludeLoc = IncludeLoc;
    Entry.File.FileCharacteristic = FileCharacter;

    return createFileIDForSLocEntry(Entry, Size);
}

FileID SourceManager::createFileID(std::unique_ptr<::llvm::MemoryBuffer> Buffer,
                                    SrcMgr::CharacteristicKind FileCharacter,
                                    SourceLocation IncludeLoc) {
    unsigned Size = Buffer->getBufferSize();
    ContentCache *CC = createMemBufferContentCache(std::move(Buffer));

    SrcMgr::SLocEntry Entry;
    Entry.IsExpansion = false;
    Entry.File.Content = CC;
    Entry.File.IncludeLoc = IncludeLoc;
    Entry.File.FileCharacteristic = FileCharacter;

    return createFileIDForSLocEntry(Entry, Size);
}

SourceLocation SourceManager::createExpansionLoc(SourceLocation SpellingLoc,
                                                  SourceLocation ExpansionLocStart,
                                                  SourceLocation ExpansionLocEnd,
                                                  unsigned Length,
                                                  bool ExpansionIsTokenRange) {
    SrcMgr::SLocEntry Entry;
    Entry.IsExpansion = true;
    Entry.Expansion = SrcMgr::ExpansionInfo::create(
        SpellingLoc, ExpansionLocStart, ExpansionLocEnd, ExpansionIsTokenRange);
    Entry.Offset = NextLocalOffset;
    LocalSLocEntryTable.push_back(Entry);

    SourceLocation Loc = SourceLocation::getMacroLoc(NextLocalOffset);
    NextLocalOffset += Length + 1;
    return Loc;
}

const SrcMgr::SLocEntry &SourceManager::getSLocEntry(FileID FID) const {
    unsigned ID = FID.getID();
    if (ID < LocalSLocEntryTable.size())
        return LocalSLocEntryTable[ID];
    return LocalSLocEntryTable[0];
}

FileID SourceManager::getFileID(SourceLocation SpellingLoc) const {
    unsigned Offset = SpellingLoc.getOffset();

    if (LastFileIDLookup.isValid()) {
        unsigned Idx = LastFileIDLookup.getID();
        if (Idx < LocalSLocEntryTable.size()) {
            const auto &Entry = LocalSLocEntryTable[Idx];
            unsigned NextOff = (Idx + 1 < LocalSLocEntryTable.size())
                                   ? LocalSLocEntryTable[Idx + 1].Offset
                                   : NextLocalOffset;
            if (Entry.Offset <= Offset && Offset < NextOff)
                return LastFileIDLookup;
        }
    }

    return getFileIDSlow(Offset);
}

FileID SourceManager::getFileIDSlow(unsigned Offset) const {
    int Lo = 0;
    int Hi = static_cast<int>(LocalSLocEntryTable.size()) - 1;
    FileID Result;

    while (Lo <= Hi) {
        int Mid = Lo + (Hi - Lo) / 2;
        if (LocalSLocEntryTable[Mid].Offset <= Offset) {
            Result = FileID(Mid);
            Lo = Mid + 1;
        } else {
            Hi = Mid - 1;
        }
    }

    LastFileIDLookup = Result;
    return Result;
}

SourceLocation SourceManager::getLocForStartOfFile(FileID FID) const {
    if (FID.isInvalid() ||
        static_cast<unsigned>(FID.getID()) >= LocalSLocEntryTable.size())
        return SourceLocation();
    return SourceLocation(LocalSLocEntryTable[FID.getID()].Offset);
}

SourceLocation SourceManager::getLocForEndOfFile(FileID FID) const {
    if (FID.isInvalid())
        return SourceLocation();
    const auto &Entry = getSLocEntry(FID);
    if (Entry.IsExpansion || !Entry.File.Content)
        return SourceLocation();
    unsigned Size = Entry.File.Content->getSize();
    return SourceLocation(Entry.Offset + Size);
}

std::pair<FileID, unsigned>
SourceManager::getDecomposedLoc(SourceLocation Loc) const {
    FileID FID = getFileID(Loc);
    unsigned Offset = Loc.getOffset() - getSLocEntry(FID).Offset;
    return {FID, Offset};
}

std::pair<FileID, unsigned>
SourceManager::getDecomposedExpansionLoc(SourceLocation Loc) const {
    SourceLocation ExpLoc = getExpansionLoc(Loc);
    return getDecomposedLoc(ExpLoc);
}

std::pair<FileID, unsigned>
SourceManager::getDecomposedSpellingLoc(SourceLocation Loc) const {
    SourceLocation SpLoc = getSpellingLoc(Loc);
    return getDecomposedLoc(SpLoc);
}

SourceLocation SourceManager::getSpellingLoc(SourceLocation Loc) const {
    while (Loc.isMacroID()) {
        FileID FID = getFileID(Loc);
        const auto &Entry = getSLocEntry(FID);
        if (!Entry.IsExpansion)
            break;
        Loc = Entry.Expansion.getSpellingLoc();
    }
    return Loc;
}

SourceLocation SourceManager::getExpansionLoc(SourceLocation Loc) const {
    while (Loc.isMacroID()) {
        FileID FID = getFileID(Loc);
        const auto &Entry = getSLocEntry(FID);
        if (!Entry.IsExpansion)
            break;
        Loc = Entry.Expansion.getExpansionLocStart();
    }
    return Loc;
}

SourceLocation SourceManager::getFileLoc(SourceLocation Loc) const {
    while (true) {
        if (Loc.isFileID())
            return Loc;
        FileID FID = getFileID(Loc);
        const auto &Entry = getSLocEntry(FID);
        if (!Entry.IsExpansion)
            return Loc;
        if (Entry.Expansion.isMacroArgExpansion())
            Loc = Entry.Expansion.getSpellingLoc();
        else
            Loc = Entry.Expansion.getExpansionLocStart();
    }
}

void SourceManager::computeLineTable(FileID FID) const {
    const auto &Entry = getSLocEntry(FID);
    if (Entry.IsExpansion || !Entry.File.Content)
        return;

    ContentCache *CC = Entry.File.Content;
    if (CC->SourceLineCache)
        return;

    const ::llvm::MemoryBuffer *Buf = CC->getBufferOrNone();
    if (!Buf)
        return;

    LineOffsetMapping::compute(Buf->getBufferStart(), Buf->getBufferEnd(),
                               CC->SourceLineCache, CC->NumLines);
}

unsigned SourceManager::getLineNumber(FileID FID, unsigned FilePos) const {
    const auto &Entry = getSLocEntry(FID);
    if (Entry.IsExpansion || !Entry.File.Content)
        return 1;

    ContentCache *CC = Entry.File.Content;
    if (!CC->SourceLineCache)
        computeLineTable(FID);

    if (!CC->SourceLineCache)
        return 1;

    unsigned *Table = CC->SourceLineCache;
    unsigned NumLines = CC->NumLines;

    unsigned Lo = 0, Hi = NumLines;
    while (Lo < Hi) {
        unsigned Mid = Lo + (Hi - Lo) / 2;
        if (Table[Mid] <= FilePos)
            Lo = Mid + 1;
        else
            Hi = Mid;
    }
    return Lo;
}

unsigned SourceManager::getColumnNumber(FileID FID, unsigned FilePos) const {
    unsigned LineNo = getLineNumber(FID, FilePos);
    if (LineNo == 0)
        return 1;

    const auto &Entry = getSLocEntry(FID);
    if (Entry.IsExpansion || !Entry.File.Content)
        return 1;

    ContentCache *CC = Entry.File.Content;
    if (!CC->SourceLineCache)
        return 1;

    unsigned LineStart = CC->SourceLineCache[LineNo - 1];
    return FilePos - LineStart + 1;
}

unsigned SourceManager::getSpellingLineNumber(SourceLocation Loc) const {
    auto [FID, Offset] = getDecomposedSpellingLoc(Loc);
    return getLineNumber(FID, Offset);
}

unsigned SourceManager::getSpellingColumnNumber(SourceLocation Loc) const {
    auto [FID, Offset] = getDecomposedSpellingLoc(Loc);
    return getColumnNumber(FID, Offset);
}

unsigned SourceManager::getExpansionLineNumber(SourceLocation Loc) const {
    auto [FID, Offset] = getDecomposedExpansionLoc(Loc);
    return getLineNumber(FID, Offset);
}

unsigned SourceManager::getExpansionColumnNumber(SourceLocation Loc) const {
    auto [FID, Offset] = getDecomposedExpansionLoc(Loc);
    return getColumnNumber(FID, Offset);
}

PresumedLoc SourceManager::getPresumedLoc(SourceLocation Loc) const {
    if (Loc.isInvalid())
        return PresumedLoc();

    auto [FID, Offset] = getDecomposedExpansionLoc(Loc);
    const auto &Entry = getSLocEntry(FID);
    if (Entry.IsExpansion)
        return PresumedLoc();

    const char *Filename = "";
    if (Entry.File.Content && Entry.File.Content->OrigEntry)
        Filename = Entry.File.Content->OrigEntry->Name.c_str();

    unsigned Line = getLineNumber(FID, Offset);
    unsigned Col = getColumnNumber(FID, Offset);

    return PresumedLoc(Filename, FID, Line, Col, Entry.File.IncludeLoc);
}

const char *SourceManager::getCharacterData(SourceLocation SL) const {
    auto [FID, Offset] = getDecomposedSpellingLoc(SL);
    const auto &Entry = getSLocEntry(FID);
    if (Entry.IsExpansion || !Entry.File.Content)
        return nullptr;

    const ::llvm::MemoryBuffer *Buf = Entry.File.Content->getBufferOrNone();
    if (!Buf)
        return nullptr;

    return Buf->getBufferStart() + Offset;
}

::llvm::StringRef SourceManager::getBufferData(FileID FID) const {
    const auto &Entry = getSLocEntry(FID);
    if (Entry.IsExpansion || !Entry.File.Content)
        return "";
    return Entry.File.Content->getBufferData();
}

const ::llvm::MemoryBuffer *SourceManager::getBuffer(FileID FID) const {
    const auto &Entry = getSLocEntry(FID);
    if (Entry.IsExpansion || !Entry.File.Content)
        return nullptr;
    return Entry.File.Content->getBufferOrNone();
}

bool SourceManager::isBeforeInTranslationUnit(SourceLocation LHS,
                                               SourceLocation RHS) const {
    if (LHS == RHS)
        return false;

    auto [LHSFID, LHSOffset] = getDecomposedLoc(LHS);
    auto [RHSFID, RHSOffset] = getDecomposedLoc(RHS);

    if (LHSFID == RHSFID)
        return LHSOffset < RHSOffset;

    ::llvm::SmallVector<FileID, 16> LHSChain, RHSChain;

    FileID CurFID = LHSFID;
    while (CurFID.isValid()) {
        LHSChain.push_back(CurFID);
        const auto &Entry = getSLocEntry(CurFID);
        if (!Entry.isFile() || Entry.File.IncludeLoc.isInvalid())
            break;
        CurFID = getFileID(Entry.File.IncludeLoc);
    }

    CurFID = RHSFID;
    while (CurFID.isValid()) {
        RHSChain.push_back(CurFID);
        const auto &Entry = getSLocEntry(CurFID);
        if (!Entry.isFile() || Entry.File.IncludeLoc.isInvalid())
            break;
        CurFID = getFileID(Entry.File.IncludeLoc);
    }

    ::llvm::DenseMap<int, unsigned> LHSMap;
    for (unsigned I = 0; I < LHSChain.size(); ++I)
        LHSMap[LHSChain[I].getID()] = I;

    for (unsigned I = 0; I < RHSChain.size(); ++I) {
        auto It = LHSMap.find(RHSChain[I].getID());
        if (It != LHSMap.end()) {
            FileID CommonFID = RHSChain[I];
            unsigned LHSIdx = It->second;
            unsigned RHSIdx = I;

            if (LHSIdx == 0 && RHSIdx == 0)
                return LHSOffset < RHSOffset;

            SourceLocation LHSIncLoc = (LHSIdx > 0)
                ? getSLocEntry(LHSChain[LHSIdx - 1]).File.IncludeLoc
                : SourceLocation(getSLocEntry(LHSFID).Offset + LHSOffset);
            SourceLocation RHSIncLoc = (RHSIdx > 0)
                ? getSLocEntry(RHSChain[RHSIdx - 1]).File.IncludeLoc
                : SourceLocation(getSLocEntry(RHSFID).Offset + RHSOffset);

            return LHSIncLoc.getRawEncoding() < RHSIncLoc.getRawEncoding();
        }
    }

    return LHS.getRawEncoding() < RHS.getRawEncoding();
}

bool SourceManager::isInMainFile(SourceLocation Loc) const {
    if (Loc.isInvalid() || MainFileID.isInvalid())
        return false;
    auto [FID, Offset] = getDecomposedExpansionLoc(Loc);
    return FID == MainFileID;
}

}
