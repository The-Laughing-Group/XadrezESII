cmake_minimum_required(VERSION 3.20)
project(xadrez_engine CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Biblioteca com TODA a lógica (regras, e depois a IA). Não depende de JS nem de testes.
file(GLOB_RECURSE CORE_SRC CONFIGURE_DEPENDS src/core/*.cpp src/rules/*.cpp src/game/*.cpp src/ai/*.cpp)
add_library(chess_core STATIC ${CORE_SRC})
target_include_directories(chess_core PUBLIC include)
if(MSVC)
  target_compile_options(chess_core PRIVATE /W4)
else()
  target_compile_options(chess_core PRIVATE -Wall -Wextra)
endif()

option(COVERAGE "Gera cobertura de testes (gcc/clang)" OFF)
if(COVERAGE)
  target_compile_options(chess_core PUBLIC --coverage)
  target_link_options(chess_core PUBLIC --coverage)
endif()

# Testes (só no build nativo; o build WASM será adicionado junto com os bindings)
if(NOT EMSCRIPTEN)
  include(CTest)
  include(FetchContent)
  FetchContent_Declare(googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip)
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  file(GLOB_RECURSE TEST_SRC CONFIGURE_DEPENDS tests/*.cpp)
  add_executable(chess_tests ${TEST_SRC})
  target_link_libraries(chess_tests PRIVATE chess_core GTest::gtest_main)
  include(GoogleTest)
  gtest_discover_tests(chess_tests)
endif()
