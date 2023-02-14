cmake_minimum_required(VERSION 3.12)
project(Taskmgr)
set(CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED True)
# add_subdirectory(sqlite)
add_subdirectory(sha256)
add_executable(Taskmgr main.cpp)
find_package(OpenSSL REQUIRED)
target_link_libraries(Taskmgr PUBLIC crypto)
# find_package(sqlite3 REQUIRED)
target_link_libraries(Taskmgr PUBLIC sqlite3)
target_link_libraries(Taskmgr PUBLIC sha256)
target_include_directories(Taskmgr PUBLIC
"${.}"
"${.}/sha256"
)
# target_link_libraries(Taskmgr PUBLIC sqlite)
# target_include_directories(Taskmgr PUBLIC 
# "${.}"
# "${.}/sqlite"
# )
