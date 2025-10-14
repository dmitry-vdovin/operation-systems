add_executable(lab_2_processes
  lab_2_processes.cpp
  lab_2_processes.h
)

# привяжем Core (пока не используем, но пусть будет готово к Коммиту 3)
target_link_libraries(lab_2_processes PRIVATE core)