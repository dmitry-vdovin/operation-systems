cmake_minimum_required(VERSION 3.20)
project(lab4_process_sync LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(common_headers INTERFACE)
target_include_directories(common_headers INTERFACE ${CMAKE_SOURCE_DIR}/include)

add_executable(Browser browser/Browser.cpp)
target_link_libraries(Browser PRIVATE common_headers)

add_executable(Downloader downloader/Downloader.cpp)
target_link_libraries(Downloader PRIVATE common_headers)