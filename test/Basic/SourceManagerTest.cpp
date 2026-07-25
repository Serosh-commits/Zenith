#include <gtest/gtest.h>
#include "zenith/Basic/SourceManager.h"
#include "zenith/Basic/FileManager.h"
#include "zenith/Basic/Diagnostic.h"
#include "llvm/Support/MemoryBuffer.h"

using namespace zenith;

TEST(SourceManagerTest, LineAndColumnNumbers) {
    FileManager FM;
    DiagnosticsEngine Diags;
    SourceManager SM(FM, Diags);

    const char *Source = "int main() {\n  return 0;\n}\n";
    auto Buf = ::llvm::MemoryBuffer::getMemBuffer(Source);
    FileID FID = SM.createFileID(std::move(Buf));
    SM.setMainFileID(FID);

    EXPECT_EQ(SM.getMainFileID(), FID);

    EXPECT_EQ(SM.getLineNumber(FID, 0), 1u);
    EXPECT_EQ(SM.getColumnNumber(FID, 0), 1u);

    EXPECT_EQ(SM.getLineNumber(FID, 12), 1u);
    EXPECT_EQ(SM.getColumnNumber(FID, 12), 13u);

    EXPECT_EQ(SM.getLineNumber(FID, 13), 2u);
    EXPECT_EQ(SM.getColumnNumber(FID, 13), 1u);

    EXPECT_EQ(SM.getLineNumber(FID, 15), 2u);
    EXPECT_EQ(SM.getColumnNumber(FID, 15), 3u);
}

TEST(SourceManagerTest, DecomposedLocations) {
    FileManager FM;
    DiagnosticsEngine Diags;
    SourceManager SM(FM, Diags);

    const char *Source = "hello world";
    auto Buf = ::llvm::MemoryBuffer::getMemBuffer(Source);
    FileID FID = SM.createFileID(std::move(Buf));

    SourceLocation StartLoc = SM.getLocForStartOfFile(FID);
    SourceLocation WorldLoc = StartLoc.getLocWithOffset(6);

    auto [DecompFID, Offset] = SM.getDecomposedLoc(WorldLoc);
    EXPECT_EQ(DecompFID, FID);
    EXPECT_EQ(Offset, 6u);
}

TEST(SourceManagerTest, ExpansionLocations) {
    FileManager FM;
    DiagnosticsEngine Diags;
    SourceManager SM(FM, Diags);

    const char *Source = "#define FOO 42\nFOO\n";
    auto Buf = ::llvm::MemoryBuffer::getMemBuffer(Source);
    FileID FID = SM.createFileID(std::move(Buf));

    SourceLocation SpellingLoc = SM.getLocForStartOfFile(FID).getLocWithOffset(12);
    SourceLocation ExpansionLocStart = SM.getLocForStartOfFile(FID).getLocWithOffset(15);
    SourceLocation ExpansionLocEnd = ExpansionLocStart.getLocWithOffset(3);

    SourceLocation MacroLoc = SM.createExpansionLoc(SpellingLoc, ExpansionLocStart, ExpansionLocEnd, 3);
    EXPECT_TRUE(MacroLoc.isMacroID());

    EXPECT_EQ(SM.getSpellingLoc(MacroLoc), SpellingLoc);
    EXPECT_EQ(SM.getExpansionLoc(MacroLoc), ExpansionLocStart);
}

TEST(SourceManagerTest, IsBeforeInTranslationUnit) {
    FileManager FM;
    DiagnosticsEngine Diags;
    SourceManager SM(FM, Diags);

    auto Buf1 = ::llvm::MemoryBuffer::getMemBuffer("file1 content");
    FileID FID1 = SM.createFileID(std::move(Buf1));

    SourceLocation Loc1 = SM.getLocForStartOfFile(FID1);
    SourceLocation Loc2 = Loc1.getLocWithOffset(5);

    EXPECT_TRUE(SM.isBeforeInTranslationUnit(Loc1, Loc2));
    EXPECT_FALSE(SM.isBeforeInTranslationUnit(Loc2, Loc1));
    EXPECT_FALSE(SM.isBeforeInTranslationUnit(Loc1, Loc1));
}
