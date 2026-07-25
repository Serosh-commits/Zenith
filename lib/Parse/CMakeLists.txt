add_library(ZenithParse
  Parser.cpp
  ParseDecl.cpp
  ParseExpr.cpp
  ParseStmt.cpp
)
target_link_libraries(ZenithParse PUBLIC ZenithSema ZenithLex ZenithAST)
