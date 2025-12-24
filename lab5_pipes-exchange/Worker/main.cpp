#include <windows.h>
#include <iostream>
#include <algorithm>
#include "protocol.h"
using namespace std;

string MakePipeIn(int id) { return "\\\\.\\pipe\\worker_in_" + to_string(id); }
string MakePipeOut(int id) { return "\\\\.\\pipe\\worker_out_" + to_string(id); }

int main(int argc, char* argv[]) {

    int id = atoi(argv[1]);
    cout << "Worker " << id << " started.\n";

    HANDLE hIn = CreateFileA(MakePipeIn(id).c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    HANDLE hOut = CreateFileA(MakePipeOut(id).c_str(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

    while (true) {

        Task task{};
        DWORD readBytes = 0;

        BOOL ok = ReadFile(hIn, &task, sizeof(Task), &readBytes, NULL);
        if (!ok || readBytes == 0) continue;

        // --- Exit command ---
        if (task.type == TASK_EXIT) {
            cout << "Worker " << id << " exiting...\n";
            break;
        }

        // --- Processing (simple: sort array) ---
        Result res{};
        res.code = 0;
        res.size = task.size;

        for (int i = 0; i < task.size; i++)
            res.data[i] = task.data[i];

        sort(res.data, res.data + res.size);

        DWORD written;
        WriteFile(hOut, &res, sizeof(Result), &written, NULL);
    }

    CloseHandle(hIn);
    CloseHandle(hOut);
    return 0;
}