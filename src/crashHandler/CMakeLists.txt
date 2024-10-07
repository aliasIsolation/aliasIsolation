cmake_minimum_required(VERSION 3.29)

project(aliasIsolation)

add_library(crashHandler STATIC crashHandler.cpp)
target_link_libraries(crashHandler dbghelp.lib psapi.lib)