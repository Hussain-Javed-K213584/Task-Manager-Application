cmake_minimum_required(VERSION 3.12)
project(Taskmgr)
add_subdirectory(sqlite)
add_executable(Taskmgr main.cpp)
target_link_libraries(Taskmgr PUBLIC sqlite)
target_include_directories(Taskmgr PUBLIC 
"${.}"
"${.}/sqlite"
)
