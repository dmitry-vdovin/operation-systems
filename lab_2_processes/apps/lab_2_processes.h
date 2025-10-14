#pragma once
#include <windows.h>
#include <string>

void fail(const char* where);
bool write_all(HANDLE h, const void* buf, DWORD bytes);
bool read_exact(HANDLE h, void* buf, DWORD bytes);

int child_main();   // режим потомка (stdin→stdout эхо)
int parent_main();  // режим родителя (создание каналов, запуск child, ping→pong)