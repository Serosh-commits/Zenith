find_package(GTest QUIET)

if(GTest_FOUND)
  include(GoogleTest)

  add_executable(ZenithBasicTests
    Basic/SourceLocationTest.cpp
    Basic/FileManagerTest.cpp
    Basic/SourceManagerTest.cpp
    Basic/DiagnosticTest.cpp
    Basic/IdentifierTableTest.cpp
  )
  target_link_libraries(ZenithBasicTests PRIVATE ZenithBasic GTest::gtest_main)
  gtest_discover_tests(ZenithBasicTests)

  add_executable(ZenithLexTests
    Lex/TokenTest.cpp
    Lex/LexerTest.cpp
    Lex/PreprocessorTest.cpp
  )
  target_link_libraries(ZenithLexTests PRIVATE ZenithLex ZenithBasic GTest::gtest_main)
  gtest_discover_tests(ZenithLexTests)

  add_executable(ZenithASTTests
    AST/ASTTest.cpp
  )
  target_link_libraries(ZenithASTTests PRIVATE ZenithAST ZenithBasic GTest::gtest_main)
  gtest_discover_tests(ZenithASTTests)

  add_executable(ZenithSemaTests
    Sema/SemaTest.cpp
  )
  target_link_libraries(ZenithSemaTests PRIVATE ZenithSema ZenithAST ZenithLex ZenithBasic GTest::gtest_main)
  gtest_discover_tests(ZenithSemaTests)

  add_executable(ZenithParseTests
    Parse/ParserTest.cpp
  )
  target_link_libraries(ZenithParseTests PRIVATE ZenithParse ZenithSema ZenithLex ZenithAST ZenithBasic GTest::gtest_main)
  gtest_discover_tests(ZenithParseTests)
else()
  message(STATUS "GTest not found, skipping tests")
endif()
