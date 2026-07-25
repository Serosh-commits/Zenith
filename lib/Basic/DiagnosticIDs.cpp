#include "zenith/Basic/DiagnosticIDs.h"

namespace zenith {
namespace diag {

struct DiagnosticInfo {
    Severity Level;
    const char* FormatStr;
};

static const DiagnosticInfo DiagInfoTable[] = {
    {Severity::Error, "expected '%0'"},
    {Severity::Error, "expected ';'"},
    {Severity::Error, "use of undeclared identifier '%0'"},
    {Severity::Error, "redefinition of '%0'"},
    {Severity::Error, "invalid operands to binary expression"},
    {Severity::Warning, "unused variable '%0'"},
    {Severity::Warning, "implicit conversion from '%0' to '%1'"},
    {Severity::Note, "previous definition is here"},
    {Severity::Note, "declared here"}
};

Severity DiagnosticIDs::getSeverity(unsigned ID) const {
    if (ID >= NUM_DIAGNOSTICS) return Severity::Ignored;
    return DiagInfoTable[ID].Level;
}

::llvm::StringRef DiagnosticIDs::getDescription(unsigned ID) const {
    if (ID >= NUM_DIAGNOSTICS) return "";
    return DiagInfoTable[ID].FormatStr;
}

}
}
