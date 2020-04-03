cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(NukePractice)

set(NUKE_DEPS_PATH "" CACHE PATH "path to nuke include and libs")


link_directories("${NUKE_DEPS_PATH}")
add_library(TestPlugin SHARED src/TestPlugin.cpp)
target_include_directories(TestPlugin PRIVATE "${NUKE_DEPS_PATH}/include")
target_link_libraries(TestPlugin DDImage glew32)

set_target_properties(TestPlugin PROPERTIES PREFIX "")
if (APPLE)
    set_target_properties(TestPlugin PROPERTIES SUFFIX ".dylib")
endif()
