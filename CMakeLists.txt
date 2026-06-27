cmake_minimum_required(VERSION 3.14)
project(PokerEngine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(POKER_ENGINE_BUILD_TESTS "Build unit tests" ON)

set(BUILD_TESTS OFF CACHE BOOL "Disable PokerHandEvaluator tests" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_PLO4 OFF CACHE BOOL "" FORCE)
set(BUILD_PLO5 OFF CACHE BOOL "" FORCE)
set(BUILD_PLO6 OFF CACHE BOOL "" FORCE)
set(BUILD_CARD5 OFF CACHE BOOL "" FORCE)
set(BUILD_CARD6 OFF CACHE BOOL "" FORCE)
set(BUILD_CARD7 OFF CACHE BOOL "" FORCE)

add_subdirectory(third_party/PokerHandEvaluator/cpp)

add_library(poker_engine
  src/card.cpp
  src/deck.cpp
  src/game.cpp
  src/showdown.cpp
  src/session.cpp
)
target_include_directories(poker_engine
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(poker_engine PUBLIC pheval)
target_compile_options(poker_engine PRIVATE -Wall -Wextra -Wpedantic)

add_executable(play tools/play.cpp)
target_link_libraries(play PRIVATE poker_engine)
target_compile_options(play PRIVATE -Wall -Wextra -Wpedantic)

if(POKER_ENGINE_BUILD_TESTS)
  enable_testing()
  include(FetchContent)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
  )
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  add_executable(poker_engine_tests
    tests/card_test.cpp
    tests/deck_test.cpp
    tests/game_test.cpp
    tests/showdown_test.cpp
  )
  target_link_libraries(poker_engine_tests PRIVATE poker_engine GTest::gtest_main)
  target_compile_options(poker_engine_tests PRIVATE -Wall -Wextra -Wpedantic)

  include(GoogleTest)
  gtest_discover_tests(poker_engine_tests)
endif()
