# CMakeList.txt: проект CMake для lab3_markers; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#

# Добавьте источник в исполняемый файл этого проекта.
add_executable (lab3_markers "src/markers.cpp" "include/lab3_markers.hpp" "apps/markers_app.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET lab3_markers PROPERTY CXX_STANDARD 20)
endif()

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
