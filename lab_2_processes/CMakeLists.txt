# CMakeList.txt: проект CMake для lab_2_processes; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#

# Добавьте источник в исполняемый файл этого проекта.
add_executable (lab_2_processes "lab_2_processes.cpp" "lab_2_processes.h" "src/array_ops.cpp" "tests/test_array_ops.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET lab_2_processes PROPERTY CXX_STANDARD 20)
endif()

# TODO: Добавьте тесты и целевые объекты, если это необходимо.


add_subdirectory(lab_2_processes)

add_library(core STATIC
  src/array_ops.cpp
)

target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)