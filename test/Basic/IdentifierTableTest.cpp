#include <gtest/gtest.h>
#include "zenith/Basic/IdentifierTable.h"
#include "zenith/Basic/LangOptions.h"

using namespace zenith;

TEST(IdentifierTableTest, StoresNamesAndMarksKeywords) {
    LangOptions LangOpts;
    LangOpts.CPlusPlus = true;

    IdentifierTable Table;
    Table.AddKeywords(LangOpts);

    IdentifierInfo &IntInfo = Table.get("int");
    EXPECT_EQ(IntInfo.getName(), "int");
    EXPECT_TRUE(IntInfo.isKeyword());
    EXPECT_EQ(IntInfo.getTokenID(), tok::kw_int);

    IdentifierInfo &ClassInfo = Table.get("class");
    EXPECT_TRUE(ClassInfo.isKeyword());
    EXPECT_EQ(ClassInfo.getTokenID(), tok::kw_class);
}

TEST(IdentifierTableTest, RespectsLanguageMode) {
    LangOptions LangOpts;
    IdentifierTable Table;
    Table.AddKeywords(LangOpts);

    EXPECT_TRUE(Table.get("int").isKeyword());
    EXPECT_FALSE(Table.get("class").isKeyword());
    EXPECT_TRUE(Table.get("bool").isKeyword());
}

TEST(IdentifierTableTest, C99OnlyKeywordsRespectOptions) {
    LangOptions LangOpts;
    LangOpts.C99 = false;
    IdentifierTable Table;
    Table.AddKeywords(LangOpts);

    EXPECT_FALSE(Table.get("restrict").isKeyword());
    EXPECT_FALSE(Table.get("bool").isKeyword());
    EXPECT_FALSE(Table.get("inline").isKeyword());
}

TEST(IdentifierTableTest, CPlusPlusKeywordCoverage) {
    LangOptions LangOpts;
    LangOpts.C99 = false;
    LangOpts.CPlusPlus = true;
    IdentifierTable Table;
    Table.AddKeywords(LangOpts);

    EXPECT_TRUE(Table.get("bool").isKeyword());
    EXPECT_TRUE(Table.get("inline").isKeyword());
    EXPECT_FALSE(Table.get("restrict").isKeyword());
}

TEST(LangOptionsTest, ExposesLanguageModeHelpers) {
    LangOptions LangOpts;
    LangOpts.CPlusPlus = true;
    LangOpts.CPlusPlus11 = true;
    LangOpts.CPlusPlus17 = true;
    LangOpts.CPlusPlus20 = true;

    EXPECT_TRUE(LangOpts.isCPlusPlus());
    EXPECT_TRUE(LangOpts.isCPlusPlus11());
    EXPECT_TRUE(LangOpts.isCPlusPlus17());
    EXPECT_TRUE(LangOpts.isCPlusPlus20());
}
