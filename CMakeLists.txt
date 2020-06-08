cmake_minimum_required(VERSION 3.1)

if(${CMAKE_VERSION} VERSION_LESS 3.15)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
else()
    cmake_policy(VERSION 3.15)
endif()

project(StringPoolExample VERSION 1.0
                  DESCRIPTION "String Pool Example"
                  LANGUAGES CXX)


set(HEADER_FILES include/Block.h include/Pool.h include/PoolString.h)
add_executable(StringPoolExample src/StringPoolTest.cpp ${HEADER_FILES})
set_target_properties(StringPoolExample PROPERTIES LINKER_LANGUAGE CXX)
target_compile_features(StringPoolExample PRIVATE cxx_std_17)
target_include_directories(StringPoolExample PRIVATE include)
