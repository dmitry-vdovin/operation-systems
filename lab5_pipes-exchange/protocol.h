#pragma once
#include <windows.h>

enum TaskType {
    TASK_PROCESS_DATA = 1,
    TASK_EXIT = 99
};

// Структура задачи
struct Task {
    TaskType type;
    int size;
    int data[256];  // ограничение для примера
};

// Структура результата
struct Result {
    int code;
    int size;
    int data[256];
};