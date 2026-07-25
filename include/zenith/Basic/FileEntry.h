#pragma once
#include "llvm/ADT/DenseMapInfo.h"
#include "llvm/ADT/StringRef.h"
#include <cstdint>
#include <string>
#include <utility>

namespace zenith {

struct UniqueID {
    uint64_t Device = 0;
    uint64_t File = 0;

    UniqueID() = default;
    UniqueID(uint64_t Dev, uint64_t F) : Device(Dev), File(F) {}

    uint64_t getDevice() const { return Device; }
    uint64_t getFile() const { return File; }

    bool operator==(const UniqueID &Other) const {
        return Device == Other.Device && File == Other.File;
    }

    bool operator!=(const UniqueID &Other) const { return !(*this == Other); }

    bool operator<(const UniqueID &Other) const {
        if (Device < Other.Device) return true;
        if (Other.Device < Device) return false;
        return File < Other.File;
    }
};

struct FileEntry {
    std::string Name;
    std::string RealPathName;
    uint64_t Size = 0;
    time_t ModTime = 0;
    UniqueID UID;
    unsigned FileUID = 0;
    bool IsNamedPipe = false;
    bool IsDeviceFile = false;
    bool IsValid = false;

    FileEntry() = default;

    ::llvm::StringRef getName() const;
    uint64_t getSize() const { return Size; }
    unsigned getFileUID() const { return FileUID; }
    time_t getModificationTime() const { return ModTime; }
    const UniqueID &getUniqueID() const { return UID; }
    bool isNamedPipe() const { return IsNamedPipe; }
};

}

namespace llvm {
template <> struct DenseMapInfo<zenith::UniqueID> {
    static inline zenith::UniqueID getEmptyKey() {
        return {~uint64_t(0), ~uint64_t(0)};
    }
    static inline zenith::UniqueID getTombstoneKey() {
        return {~uint64_t(0) - 1, ~uint64_t(0) - 1};
    }
    static unsigned getHashValue(const zenith::UniqueID &Val) {
        return llvm::detail::combineHashValue(
            DenseMapInfo<uint64_t>::getHashValue(Val.getDevice()),
            DenseMapInfo<uint64_t>::getHashValue(Val.getFile()));
    }
    static bool isEqual(const zenith::UniqueID &LHS,
                        const zenith::UniqueID &RHS) {
        return LHS == RHS;
    }
};
}
