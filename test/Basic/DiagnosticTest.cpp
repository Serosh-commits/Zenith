#include <gtest/gtest.h>
#include "zenith/Basic/Diagnostic.h"

using namespace zenith;

namespace {
class CapturingConsumer : public DiagnosticConsumer {
public:
    void HandleDiagnostic(diag::Severity Level, SourceLocation Loc, ::llvm::StringRef Message) override {
        LastLevel = Level;
        LastLoc = Loc;
        LastMessage = Message.str();
    }

    diag::Severity LastLevel = diag::Severity::Ignored;
    SourceLocation LastLoc;
    std::string LastMessage;
};
}

TEST(DiagnosticTest, ReportsAndFormats) {
    DiagnosticsEngine Diags;
    CapturingConsumer Consumer;
    Diags.setConsumer(&Consumer);

    auto Builder = Diags.Report(diag::err_expected_semi);
    Builder << "if";
    Builder.emit();

    EXPECT_EQ(Consumer.LastLevel, diag::Severity::Error);
    EXPECT_EQ(Consumer.LastMessage, "expected ';'");

    auto Builder2 = Diags.Report(SourceLocation(42), diag::err_undeclared_identifier);
    Builder2 << "foo";
    Builder2.emit();

    EXPECT_EQ(Consumer.LastLevel, diag::Severity::Error);
    EXPECT_EQ(Consumer.LastMessage, "use of undeclared identifier 'foo'");
    EXPECT_TRUE(Consumer.LastLoc.isValid());
    EXPECT_EQ(Consumer.LastLoc.getRawEncoding(), 42u);
}
