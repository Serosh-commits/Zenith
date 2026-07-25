add_library(ZenithAST
  ASTContext.cpp
  Decl.cpp
  Stmt.cpp
  Expr.cpp
  Type.cpp
)
target_link_libraries(ZenithAST PUBLIC ZenithBasic)
