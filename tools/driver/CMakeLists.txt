add_executable(zenithc main.cpp)
target_link_libraries(zenithc PRIVATE ZenithParse ZenithSema ZenithLex ZenithAST ZenithBasic)
