add_library(ZenithBasic
  SourceLocation.cpp
  FileManager.cpp
  SourceManager.cpp
  Diagnostic.cpp
  DiagnosticIDs.cpp
  IdentifierTable.cpp
  TargetInfo.cpp
)
target_link_libraries(ZenithBasic PUBLIC ${LLVM_LIBS})
