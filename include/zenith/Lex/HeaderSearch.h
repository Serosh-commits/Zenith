#pragma once

#include <vector>
#include <string>
#include <optional>
#include <llvm/ADT/StringMap.h>
#include <llvm/ADT/StringRef.h>

namespace zenith {

class FileManager;

class HeaderSearch {
    std::vector<std::string> UserDirs;
    std::vector<std::string> SystemDirs;
    ::llvm::StringMap<std::optional<std::string>> LookupCache;
    FileManager &FM;

public:
    explicit HeaderSearch(FileManager &FM) : FM(FM) {}

    void AddSearchPath(const std::string &Path, bool IsSystem);
    std::optional<std::string> LookupFile(::llvm::StringRef Filename, bool IsAngled);
};

}
