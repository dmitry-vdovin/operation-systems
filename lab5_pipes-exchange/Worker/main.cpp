#include <windows.h>
#include <iostream>
#include <algorithm>
#include "protocol.h"
#include "utils.h"

using namespace std;

int main(int argc, char* argv[])
{
    if (argc < 2) {
        cout << "Worker started without ID.\n";
        return 1;
    }

    int id = atoi(argv[1]);
    cout << "Worker " << id << " online.\n";

    HANDLE hIn = CreateFileA(
        MakePipeIn(id).c_str(),
        GENERIC_READ,
        0, NULL,
        OPEN_EXISTING,
        0, NULL
    );
    if (hIn == INVALID_HANDLE_VALUE) {
        PrintLastError("CreateFile(IN)");
        return 1;
    }

    HANDLE hOut = CreateFileA(
        MakePipeOut(id).c_str(),
        GENERIC_WRITE,
        0, NULL,
        OPEN_EXISTING,
        0, NULL
    );
    if (hOut == INVALID_HANDLE_VALUE) {
        PrintLastError("CreateFile(OUT)");
        return 1;
    }

    while (true) {
        Task task{};
        DWORD readBytes = 0;

        BOOL ok = ReadFile(hIn, &task, sizeof(Task), &readBytes, NULL);
        if (!ok) {
            PrintLastError("ReadFile(task)");
            break;
        }
        if (readBytes == 0)
            continue;

        if (task.type == TASK_EXIT) {
            cout << "Worker " << id << " shutting down.\n";
            break;
        }

        // ===== Processing (пример: сортировка массива) =====
        Result res{};
        res.code = 0;
        res.size = task.size;

        for (int i = 0; i < task.size; i++)
            res.data[i] = task.data[i];

        // пустая задача (size=0) — сортировка не падает, просто ничего не делает
        if (res.size > 0)
            sort(res.data, res.data + res.size);

        DWORD written = 0;
        if (!WriteFile(hOut, &res, sizeof(Result), &written, NULL)) {
            PrintLastError("WriteFile(result)");
            break;
        }
    }

    CloseHandle(hIn);
    CloseHandle(hOut);
    return 0;
}