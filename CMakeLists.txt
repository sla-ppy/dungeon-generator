cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(dun-gen)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost 1.75 REQUIRED COMPONENTS system)

add_subdirectory(deps/doctest)
add_subdirectory(deps/fmt)

set(DUN_GEN_SRCS 
    src/Common.h
    src/Generation.h src/Generation.cpp
    src/Rendering.h src/Rendering.cpp
    src/Log.h)
set(DUN_GEN_LIBS Boost::boost doctest fmt)
set(DUN_GEN_INCLUDE_DIRS deps/stb)

add_executable(dun-gen ${DUN_GEN_SRCS} src/main.cpp)
target_link_libraries(dun-gen ${DUN_GEN_LIBS})
target_include_directories(dun-gen PRIVATE ${DUN_GEN_INCLUDE_DIRS})
target_compile_definitions(dun-gen PRIVATE
    DOCTEST_CONFIG_DISABLE # removes test code
)

add_executable(dun-gen-tests ${DUN_GEN_SRCS} tests/tests_main.cpp)
target_link_libraries(dun-gen-tests ${DUN_GEN_LIBS})
target_include_directories(dun-gen-tests PRIVATE ${DUN_GEN_INCLUDE_DIRS})
