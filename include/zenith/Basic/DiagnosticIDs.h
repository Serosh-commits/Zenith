#pragma once
#include "llvm/ADT/StringRef.h"

namespace zenith {
namespace diag {

enum DiagID {
    err_expected,
    err_expected_semi,
    err_undeclared_identifier,
    err_redefinition,
    err_typecheck_invalid_operands,
    warn_unused_variable,
    warn_implicit_conversion,
    note_previous_definition,
    note_declared_here,
    NUM_DIAGNOSTICS
};

enum class Severity {
    Ignored,
    Note,
    Remark,
    Warning,
    Error,
    Fatal
};

class DiagnosticIDs {
public:
    Severity getSeverity(unsigned ID) const;
    ::llvm::StringRef getDescription(unsigned ID) const;
};

}
}
