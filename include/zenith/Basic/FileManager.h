#pragma once
#include "zenith/Basic/FileEntry.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/ErrorOr.h"
#include "llvm/Support/MemoryBuffer.h"
#include <memory>

namespace zenith {

class FileManager {
    ::llvm::StringMap<::llvm::ErrorOr<FileEntry *>, ::llvm::BumpPtrAllocator> SeenFileEntries;
    ::llvm::StringMap<bool> SeenDirEntries;

    ::llvm::DenseMap<UniqueID, FileEntry *> UniqueRealFiles;

    ::llvm::BumpPtrAllocator FilesAlloc;
    ::llvm::SmallVector<std::unique_ptr<FileEntry>, 16> OwnedFiles;

    unsigned NextFileUID = 0;

    unsigned NumFileLookups = 0;
    unsigned NumFileCacheMisses = 0;

public:
    FileManager() = default;

    FileEntry *getFileRef(::llvm::StringRef Filename);

    FileEntry *getOrCreateFileEntry(::llvm::StringRef Name, uint64_t Size);

    FileEntry *getVirtualFile(::llvm::StringRef Filename, uint64_t Size,
                              time_t ModTime);

    ::llvm::ErrorOr<std::unique_ptr<::llvm::MemoryBuffer>>
    getBufferForFile(::llvm::StringRef Filename, bool IsVolatile = false);

    ::llvm::ErrorOr<std::unique_ptr<::llvm::MemoryBuffer>>
    getBufferForFile(const FileEntry &Entry, bool IsVolatile = false);

    bool exists(::llvm::StringRef Path);

    bool makeAbsolutePath(::llvm::SmallVectorImpl<char> &Path) const;

    void PrintStats() const;

    unsigned getNumFileLookups() const { return NumFileLookups; }
    unsigned getNumFileCacheMisses() const { return NumFileCacheMisses; }
};

}
