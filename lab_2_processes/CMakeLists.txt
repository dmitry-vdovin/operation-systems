cmake_minimum_required(VERSION 3.20)
project(lab_2_processes LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Общие дефайны для WinAPI
add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN UNICODE _UNICODE)

# Библиотека с логикой (максимум массива) — пригодится в коммите 3, но можно собрать уже сейчас
add_library(core STATIC
  src/array_ops.cpp
)
target_include_directories(core PUBLIC ${CMAKE_SOURCE_DIR}/include)

enable_testing()
add_subdirectory(apps)
add_subdirectory(tests)