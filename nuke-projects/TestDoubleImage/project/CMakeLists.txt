cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(NukePractice)

set(NUKE_DEPS_PATH "" CACHE PATH "path to nuke include and libs")


link_directories("${NUKE_DEPS_PATH}")
add_library(DoubleImage SHARED src/DoubleImage.cpp)
target_include_directories(DoubleImage PRIVATE "${NUKE_DEPS_PATH}/include")
target_link_libraries(DoubleImage DDImage glew32)

set_target_properties(DoubleImage PROPERTIES PREFIX "")
if (APPLE)
    set_target_properties(DoubleImage PROPERTIES SUFFIX ".dylib")
endif()
