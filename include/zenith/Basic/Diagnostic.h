#pragma once
#include "zenith/Basic/DiagnosticIDs.h"
#include "zenith/Basic/SourceLocation.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"
#include <vector>
#include <string>
#include <memory>

namespace zenith {

class DiagnosticBuilder;

class DiagnosticConsumer {
public:
    virtual ~DiagnosticConsumer() = default;
    virtual void HandleDiagnostic(diag::Severity Level, SourceLocation Loc, ::llvm::StringRef Message) = 0;
};

class DiagnosticsEngine {
    std::unique_ptr<diag::DiagnosticIDs> DiagIDs;
    DiagnosticConsumer* Consumer = nullptr;
    unsigned ErrorCount = 0;
    unsigned WarningCount = 0;

public:
    DiagnosticsEngine();
    
    void setConsumer(DiagnosticConsumer* C) { Consumer = C; }
    DiagnosticConsumer* getConsumer() const { return Consumer; }
    
    diag::DiagnosticIDs& getDiagnosticIDs() { return *DiagIDs; }
    
    DiagnosticBuilder Report(SourceLocation Loc, unsigned DiagID);
    
    bool hasErrorOccurred() const { return ErrorCount > 0; }
    unsigned getNumErrors() const { return ErrorCount; }
    unsigned getNumWarnings() const { return WarningCount; }
    
    void EmitDiag(SourceLocation Loc, unsigned DiagID, const std::vector<std::string>& Args);
};

class DiagnosticBuilder {
    DiagnosticsEngine* Engine;
    SourceLocation Loc;
    unsigned DiagID;
    std::vector<std::string> Args;
    bool Emitted = false;

public:
    DiagnosticBuilder(DiagnosticsEngine* engine, SourceLocation loc, unsigned diagID)
        : Engine(engine), Loc(loc), DiagID(diagID) {}

    DiagnosticBuilder(const DiagnosticBuilder&) = delete;
    DiagnosticBuilder& operator=(const DiagnosticBuilder&) = delete;
    
    DiagnosticBuilder(DiagnosticBuilder&& Other) noexcept
        : Engine(Other.Engine), Loc(Other.Loc), DiagID(Other.DiagID), Args(std::move(Other.Args)), Emitted(Other.Emitted) {
        Other.Emitted = true;
    }

    ~DiagnosticBuilder() {
        if (!Emitted && Engine) {
            Engine->EmitDiag(Loc, DiagID, Args);
            Emitted = true;
        }
    }

    DiagnosticBuilder& operator<<(::llvm::StringRef Str) {
        Args.push_back(Str.str());
        return *this;
    }

    DiagnosticBuilder& operator<<(int Val) {
        Args.push_back(std::to_string(Val));
        return *this;
    }
};

class TextDiagnosticPrinter : public DiagnosticConsumer {
    ::llvm::raw_ostream& OS;

public:
    TextDiagnosticPrinter(::llvm::raw_ostream& os);
    void HandleDiagnostic(diag::Severity Level, SourceLocation Loc, ::llvm::StringRef Message) override;
};

}
