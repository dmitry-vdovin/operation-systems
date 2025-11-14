cmake_minimum_required(VERSION 3.20)
project(lab3_markers LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Библиотека с логикой потоков
add_library(markers STATIC
    src/markers.cpp
)
target_include_directories(markers PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Приложение (интерактивный контроллер из Коммита 2)
add_executable(markers_app
    apps/markers_app.cpp
)
target_link_libraries(markers_app PRIVATE markers)

enable_testing()

# -------- GoogleTest через FetchContent --------
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
)
# Для корректной линковки CRT в MSVC
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Тесты
add_executable(tests
    tests/test_markers.cpp
)
target_link_libraries(tests PRIVATE markers gtest_main)

include(GoogleTest)
gtest_discover_tests(tests)