cmake_minimum_required(VERSION 3.16)
project(terminal_pong CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_compile_options(-Wall -Wextra)

add_library(ponglib
  src/game.cpp
  src/terminal.cpp
)
target_include_directories(ponglib PUBLIC src)

add_executable(pong src/main.cpp)
target_link_libraries(pong PRIVATE ponglib)

enable_testing()
add_executable(test_game tests/test_game.cpp)
target_link_libraries(test_game PRIVATE ponglib)
target_compile_definitions(test_game PRIVATE PONG_ENABLE_TEST_HOOKS)
add_test(NAME test_game COMMAND test_game)
