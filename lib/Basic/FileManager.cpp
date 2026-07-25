#include "zenith/Basic/FileManager.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"

namespace zenith {

::llvm::StringRef FileEntry::getName() const {
    return Name;
}

FileEntry *FileManager::getFileRef(::llvm::StringRef Filename) {
    ++NumFileLookups;

    auto It = SeenFileEntries.find(Filename);
    if (It != SeenFileEntries.end()) {
        if (It->second)
            return *It->second;
        return nullptr;
    }

    ++NumFileCacheMisses;

    ::llvm::sys::fs::file_status Status;
    auto EC = ::llvm::sys::fs::status(Filename, Status);
    if (EC) {
        SeenFileEntries.insert({Filename, std::error_code(EC)});
        return nullptr;
    }

    UniqueID UID(Status.getUniqueID().getDevice(),
                 Status.getUniqueID().getFile());

    FileEntry *&UFE = UniqueRealFiles[UID];
    if (!UFE) {
        auto Entry = std::make_unique<FileEntry>();
        Entry->Name = Filename.str();
        Entry->Size = Status.getSize();
        Entry->ModTime = ::llvm::sys::toTimeT(Status.getLastModificationTime());
        Entry->UID = UID;
        Entry->FileUID = NextFileUID++;
        Entry->IsNamedPipe = (Status.type() == ::llvm::sys::fs::file_type::fifo_file);
        Entry->IsDeviceFile = (Status.type() == ::llvm::sys::fs::file_type::character_file);
        Entry->IsValid = true;
        UFE = Entry.get();
        OwnedFiles.push_back(std::move(Entry));
    }

    SeenFileEntries.insert({Filename, UFE});
    return UFE;
}

FileEntry *FileManager::getOrCreateFileEntry(::llvm::StringRef Name,
                                              uint64_t Size) {
    auto It = SeenFileEntries.find(Name);
    if (It != SeenFileEntries.end() && It->second)
        return *It->second;

    auto Entry = std::make_unique<FileEntry>();
    Entry->Name = Name.str();
    Entry->Size = Size;
    Entry->FileUID = NextFileUID++;
    Entry->IsValid = true;
    FileEntry *Ptr = Entry.get();
    OwnedFiles.push_back(std::move(Entry));
    SeenFileEntries.insert({Name, Ptr});
    return Ptr;
}

FileEntry *FileManager::getVirtualFile(::llvm::StringRef Filename,
                                        uint64_t Size, time_t ModTime) {
    auto It = SeenFileEntries.find(Filename);
    if (It != SeenFileEntries.end() && It->second) {
        FileEntry *FE = *It->second;
        FE->Size = Size;
        FE->ModTime = ModTime;
        return FE;
    }

    auto Entry = std::make_unique<FileEntry>();
    Entry->Name = Filename.str();
    Entry->Size = Size;
    Entry->ModTime = ModTime;
    Entry->FileUID = NextFileUID++;
    Entry->IsValid = true;
    FileEntry *Ptr = Entry.get();
    OwnedFiles.push_back(std::move(Entry));
    SeenFileEntries.insert({Filename, Ptr});
    return Ptr;
}

::llvm::ErrorOr<std::unique_ptr<::llvm::MemoryBuffer>>
FileManager::getBufferForFile(::llvm::StringRef Filename, bool IsVolatile) {
    return ::llvm::MemoryBuffer::getFile(Filename);
}

::llvm::ErrorOr<std::unique_ptr<::llvm::MemoryBuffer>>
FileManager::getBufferForFile(const FileEntry &Entry, bool IsVolatile) {
    return ::llvm::MemoryBuffer::getFile(Entry.Name);
}

bool FileManager::exists(::llvm::StringRef Path) {
    if (SeenFileEntries.count(Path))
        return true;
    return ::llvm::sys::fs::exists(Path);
}

bool FileManager::makeAbsolutePath(::llvm::SmallVectorImpl<char> &Path) const {
    return ::llvm::sys::fs::make_absolute(Path) == std::error_code();
}

void FileManager::PrintStats() const {
    ::llvm::errs() << "\n*** File Manager Stats:\n";
    ::llvm::errs() << UniqueRealFiles.size() << " real files found.\n";
    ::llvm::errs() << NumFileLookups << " file lookups, "
                   << NumFileCacheMisses << " file cache misses.\n";
}

}
