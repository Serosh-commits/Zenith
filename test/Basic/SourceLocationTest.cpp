#include <gtest/gtest.h>
#include "zenith/Basic/SourceLocation.h"
#include "zenith/Basic/SourceManager.h"
#include "zenith/Basic/FileManager.h"
#include "zenith/Basic/Diagnostic.h"
#include "llvm/ADT/DenseMap.h"

using namespace zenith;

TEST(SourceLocationTest, DefaultAndInvalid) {
    SourceLocation Loc;
    EXPECT_FALSE(Loc.isValid());
    EXPECT_TRUE(Loc.isInvalid());
    EXPECT_EQ(Loc.getRawEncoding(), 0u);

    FileID FID;
    EXPECT_FALSE(FID.isValid());
    EXPECT_TRUE(FID.isInvalid());
    EXPECT_EQ(FID.getID(), 0);
}

TEST(SourceLocationTest, FileAndMacroLocations) {
    SourceLocation FileLoc(100);
    EXPECT_TRUE(FileLoc.isValid());
    EXPECT_FALSE(FileLoc.isInvalid());
    EXPECT_TRUE(FileLoc.isFileID());
    EXPECT_FALSE(FileLoc.isMacroID());
    EXPECT_EQ(FileLoc.getOffset(), 100u);

    SourceLocation MacroLoc(100 | (1U << 31));
    EXPECT_TRUE(MacroLoc.isValid());
    EXPECT_FALSE(MacroLoc.isFileID());
    EXPECT_TRUE(MacroLoc.isMacroID());
    EXPECT_EQ(MacroLoc.getOffset(), 100u);
}

TEST(SourceLocationTest, OffsetsAndArithmetic) {
    SourceLocation Loc(500);
    SourceLocation OffLoc = Loc.getLocWithOffset(25);
    EXPECT_EQ(OffLoc.getRawEncoding(), 525u);
    EXPECT_EQ(OffLoc.getOffset(), 525u);
}

TEST(SourceLocationTest, RawAndPointerEncoding) {
    SourceLocation Loc(0x12345678);
    EXPECT_EQ(Loc.getRawEncoding(), 0x12345678u);

    void *Ptr = Loc.getPtrEncoding();
    SourceLocation Decoded = SourceLocation::getFromPtrEncoding(Ptr);
    EXPECT_EQ(Decoded, Loc);
}

TEST(SourceLocationTest, OffsetOverflowIsRejected) {
    SourceLocation Loc(0x7fffffffu);
    EXPECT_DEATH({ (void)Loc.getLocWithOffset(1); }, "offset overflow");
}

TEST(SourceLocationTest, PairOfFileLocations) {
    SourceLocation L1(100);
    SourceLocation L2(200);
    SourceLocation M1(100 | (1U << 31));

    EXPECT_TRUE(SourceLocation::isPairOfFileLocations(L1, L2));
    EXPECT_FALSE(SourceLocation::isPairOfFileLocations(L1, M1));
    EXPECT_FALSE(SourceLocation::isPairOfFileLocations(SourceLocation(), L2));
}

TEST(SourceLocationTest, SourceRangeAndCharSourceRange) {
    SourceLocation B(10);
    SourceLocation E(20);
    SourceRange R(B, E);

    EXPECT_TRUE(R.isValid());
    EXPECT_EQ(R.getBegin(), B);
    EXPECT_EQ(R.getEnd(), E);

    SourceRange SubRange(SourceLocation(12), SourceLocation(18));
    EXPECT_TRUE(R.fullyContains(SubRange));

    CharSourceRange TR = CharSourceRange::getTokenRange(R);
    EXPECT_TRUE(TR.isTokenRange());
    EXPECT_FALSE(TR.isCharRange());

    CharSourceRange CR = CharSourceRange::getCharRange(R);
    EXPECT_FALSE(CR.isTokenRange());
    EXPECT_TRUE(CR.isCharRange());
}

TEST(SourceLocationTest, PresumedLocValidation) {
    PresumedLoc Invalid;
    EXPECT_TRUE(Invalid.isInvalid());
    EXPECT_FALSE(Invalid.isValid());

    FileID FID(1);
    PresumedLoc Valid("test.c", FID, 42, 10, SourceLocation(5));
    EXPECT_TRUE(Valid.isValid());
    EXPECT_STREQ(Valid.getFilename(), "test.c");
    EXPECT_EQ(Valid.getFileID(), FID);
    EXPECT_EQ(Valid.getLine(), 42u);
    EXPECT_EQ(Valid.getColumn(), 10u);
    EXPECT_EQ(Valid.getIncludeLoc(), SourceLocation(5));
}

TEST(SourceLocationTest, DenseMapInfoSpecializations) {
    ::llvm::DenseMap<SourceLocation, int> LocMap;
    SourceLocation L1(10);
    SourceLocation L2(20);
    LocMap[L1] = 100;
    LocMap[L2] = 200;

    EXPECT_EQ(LocMap[L1], 100);
    EXPECT_EQ(LocMap[L2], 200);
    EXPECT_EQ(LocMap.count(SourceLocation(30)), 0u);

    ::llvm::DenseMap<FileID, int> FIDMap;
    FileID F1(1);
    FileID F2(2);
    FIDMap[F1] = 111;
    FIDMap[F2] = 222;

    EXPECT_EQ(FIDMap[F1], 111);
    EXPECT_EQ(FIDMap[F2], 222);
}
