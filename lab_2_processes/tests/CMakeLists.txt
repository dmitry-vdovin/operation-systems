# заглушка, чтобы CMake не ругался; реальные тесты добавим в коммите 3
add_custom_target(no_tests ALL
  COMMAND ${CMAKE_COMMAND} -E echo "Tests will be added in commit 3")