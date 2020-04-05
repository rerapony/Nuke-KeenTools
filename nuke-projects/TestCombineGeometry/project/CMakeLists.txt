cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(NukePractice)

set(NUKE_DEPS_PATH "" CACHE PATH "path to nuke include and libs")


link_directories("${NUKE_DEPS_PATH}")
add_library(CombineGeometry SHARED src/CombineGeometry.cpp)
target_include_directories(CombineGeometry PRIVATE "${NUKE_DEPS_PATH}/include")
target_link_libraries(CombineGeometry DDImage glew32)

set_target_properties(CombineGeometry PROPERTIES PREFIX "")
if (APPLE)
    set_target_properties(CombineGeometry PROPERTIES SUFFIX ".dylib")
endif()
