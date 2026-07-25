add_library(ZenithSema
  Sema.cpp
  SemaDecl.cpp
  SemaExpr.cpp
  SemaType.cpp
)
target_link_libraries(ZenithSema PUBLIC ZenithAST ZenithLex)
