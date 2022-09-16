cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(game)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost 1.75 REQUIRED COMPONENTS system)

set(GAME_SRCS )
set(GAME_LIBS Boost::boost)

add_subdirectory(deps/doctest)

add_executable(game ${GAME_SRCS} src/main.cpp)
target_link_libraries(game ${GAME_LIBS})
target_compile_definitions(game PRIVATE
    DOCTEST_CONFIG_DISABLE # removes test code
)

add_executable(game-tests ${GAME_SRCS} tests/tests_main.cpp)
target_link_libraries(game-tests ${GAME_LIBS})
