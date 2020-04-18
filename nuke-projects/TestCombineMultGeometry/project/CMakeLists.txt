cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(NukePractice)

set(NUKE_DEPS_PATH "D:/Work/nuke-practice/nuke-deps" CACHE PATH "path to nuke include and libs")


link_directories("${NUKE_DEPS_PATH}")
add_library(CombineMultGeometry SHARED src/CombineMultGeometry.cpp)
target_include_directories(CombineMultGeometry PRIVATE "${NUKE_DEPS_PATH}/include")
target_link_libraries(CombineMultGeometry DDImage glew32)

set_target_properties(CombineMultGeometry PROPERTIES PREFIX "")
if (APPLE)
    set_target_properties(CombineMultGeometry PROPERTIES SUFFIX ".dylib")
endif()
