#pragma once
#include <windows.h>

enum TaskType {
    TASK_PROCESS_DATA = 1,
    TASK_EXIT = 99
};

struct Task {
    TaskType type;
    int size;
    int data[256];
};

struct Result {
    int code;       // 0 = OK
    int size;
    int data[256];
};