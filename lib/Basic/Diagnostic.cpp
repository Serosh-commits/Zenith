#include "zenith/Basic/Diagnostic.h"
#include "llvm/Support/raw_ostream.h"

namespace zenith {

DiagnosticsEngine::DiagnosticsEngine() 
    : DiagIDs(std::make_unique<diag::DiagnosticIDs>()) {}

DiagnosticBuilder DiagnosticsEngine::Report(SourceLocation Loc, unsigned DiagID) {
    return DiagnosticBuilder(this, Loc, DiagID);
}

DiagnosticBuilder DiagnosticsEngine::Report(unsigned DiagID) {
    return Report(SourceLocation(), DiagID);
}

void DiagnosticsEngine::EmitDiag(SourceLocation Loc, unsigned DiagID, const std::vector<std::string>& Args) {
    diag::Severity Level = DiagIDs->getSeverity(DiagID);
    if (Level == diag::Severity::Error || Level == diag::Severity::Fatal) {
        ++ErrorCount;
    } else if (Level == diag::Severity::Warning) {
        ++WarningCount;
    }

    if (!Consumer) return;

    ::llvm::StringRef FormatStr = DiagIDs->getDescription(DiagID);
    if (FormatStr.empty()) {
        Consumer->HandleDiagnostic(Level, Loc, "<unknown diagnostic>");
        return;
    }
    std::string Message;
    Message.reserve(FormatStr.size() + 32);

    size_t i = 0;
    while (i < FormatStr.size()) {
        if (FormatStr[i] == '%' && i + 1 < FormatStr.size() && std::isdigit(FormatStr[i+1])) {
            unsigned ArgIdx = FormatStr[i+1] - '0';
            if (ArgIdx < Args.size()) {
                Message += Args[ArgIdx];
            }
            i += 2;
        } else {
            Message += FormatStr[i];
            i++;
        }
    }

    Consumer->HandleDiagnostic(Level, Loc, Message);
}

TextDiagnosticPrinter::TextDiagnosticPrinter(::llvm::raw_ostream& os) : OS(os) {}

void TextDiagnosticPrinter::HandleDiagnostic(diag::Severity Level, SourceLocation Loc, ::llvm::StringRef Message) {
    OS << (Loc.isValid() ? std::to_string(Loc.getRawEncoding()) : "<unknown>") << ": ";
    switch (Level) {
        case diag::Severity::Ignored: OS << "ignored: "; break;
        case diag::Severity::Note: OS << "note: "; break;
        case diag::Severity::Remark: OS << "remark: "; break;
        case diag::Severity::Warning: OS << "warning: "; break;
        case diag::Severity::Error: OS << "error: "; break;
        case diag::Severity::Fatal: OS << "fatal error: "; break;
    }
    OS << Message << "\n";
}

}
