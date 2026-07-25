add_library(ZenithLex
  Token.cpp
  Lexer.cpp
  Preprocessor.cpp
  HeaderSearch.cpp
)
target_link_libraries(ZenithLex PUBLIC ZenithBasic)
