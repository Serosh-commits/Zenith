#include <gtest/gtest.h>
#include "zenith/Basic/FileManager.h"

using namespace zenith;

TEST(FileManagerTest, VirtualFileCreation) {
    FileManager FM;
    FileEntry *VFile = FM.getVirtualFile("virtual_header.h", 1024, 0);

    ASSERT_NE(VFile, nullptr);
    EXPECT_EQ(VFile->Name, "virtual_header.h");
    EXPECT_EQ(VFile->getSize(), 1024u);
    EXPECT_TRUE(VFile->IsValid);

    FileEntry *Lookup = FM.getVirtualFile("virtual_header.h", 2048, 0);
    EXPECT_EQ(Lookup, VFile);
    EXPECT_EQ(VFile->getSize(), 2048u);
}

TEST(FileManagerTest, NonExistentFileLookup) {
    FileManager FM;
    FileEntry *FE = FM.getFileRef("non_existent_file_xyz123.c");

    EXPECT_EQ(FE, nullptr);
    EXPECT_EQ(FM.getNumFileLookups(), 1u);
    EXPECT_EQ(FM.getNumFileCacheMisses(), 1u);

    FileEntry *FE2 = FM.getFileRef("non_existent_file_xyz123.c");
    EXPECT_EQ(FE2, nullptr);
    EXPECT_EQ(FM.getNumFileLookups(), 2u);
    EXPECT_EQ(FM.getNumFileCacheMisses(), 1u);
}

TEST(FileManagerTest, CreateFileEntryManual) {
    FileManager FM;
    FileEntry *FE = FM.getOrCreateFileEntry("custom_file.c", 512);

    ASSERT_NE(FE, nullptr);
    EXPECT_EQ(FE->getName(), "custom_file.c");
    EXPECT_EQ(FE->getSize(), 512u);
}

TEST(FileManagerTest, MakeAbsolutePath) {
    FileManager FM;
    ::llvm::SmallVector<char, 128> Path;
    ::llvm::StringRef RelPath = "test_sample.c";
    Path.append(RelPath.begin(), RelPath.end());

    bool Success = FM.makeAbsolutePath(Path);
    EXPECT_TRUE(Success);
    EXPECT_TRUE(Path.size() > RelPath.size());
}
