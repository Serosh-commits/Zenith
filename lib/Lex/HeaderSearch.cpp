#include "zenith/Lex/HeaderSearch.h"
#include "zenith/Basic/FileManager.h"

namespace zenith {

void HeaderSearch::AddSearchPath(const std::string &Path, bool IsSystem) {
    if (IsSystem) {
        SystemDirs.push_back(Path);
    } else {
        UserDirs.push_back(Path);
    }
}

std::optional<std::string> HeaderSearch::LookupFile(::llvm::StringRef Filename, bool IsAngled) {
    auto It = LookupCache.find(Filename);
    if (It != LookupCache.end()) {
        return It->second;
    }
    
    std::string FilenameStr = Filename.str();

    if (!IsAngled) {
        for (const auto &Dir : UserDirs) {
            std::string Path = Dir + "/" + FilenameStr;
            if (FM.exists(Path)) {
                LookupCache[Filename] = Path;
                return Path;
            }
        }
    }
    
    for (const auto &Dir : SystemDirs) {
        std::string Path = Dir + "/" + FilenameStr;
        if (FM.exists(Path)) {
            LookupCache[Filename] = Path;
            return Path;
        }
    }
    
    LookupCache[Filename] = std::nullopt;
    return std::nullopt;
}

}
