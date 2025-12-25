#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include "protocol.h"
#include "utils.h"

using namespace std;

int main()
{
    int N, M;
    cout << "Enter number of workers (N): ";
    cin >> N;
    cout << "Enter number of tasks (M): ";
    cin >> M;

    if (N <= 0) {
        cout << "N must be > 0\n";
        return 1;
    }
    if (M < 0) {
        cout << "M must be >= 0\n";
        return 1;
    }

    vector<HANDLE> inPipes(N, INVALID_HANDLE_VALUE);
    vector<HANDLE> outPipes(N, INVALID_HANDLE_VALUE);
    vector<PROCESS_INFORMATION> workers(N);

    // 1) Create pipes & start workers
    for (int i = 0; i < N; i++) {
        ZeroMemory(&workers[i], sizeof(PROCESS_INFORMATION));

        // канал для отправки задач worker'у (Browser -> Worker)
        inPipes[i] = CreateNamedPipeA(
            MakePipeIn(i).c_str(),
            PIPE_ACCESS_OUTBOUND,
            PIPE_TYPE_BYTE | PIPE_WAIT,
            1, 4096, 4096,
            0, NULL
        );
        if (inPipes[i] == INVALID_HANDLE_VALUE) {
            PrintLastError("CreateNamedPipe(IN)");
            return 1;
        }

        // канал для получения результатов (Worker -> Browser)
        outPipes[i] = CreateNamedPipeA(
            MakePipeOut(i).c_str(),
            PIPE_ACCESS_INBOUND,
            PIPE_TYPE_BYTE | PIPE_WAIT,
            1, 4096, 4096,
            0, NULL
        );
        if (outPipes[i] == INVALID_HANDLE_VALUE) {
            PrintLastError("CreateNamedPipe(OUT)");
            return 1;
        }

        STARTUPINFOA si{};
        si.cb = sizeof(si);

        string cmd = "Worker.exe " + to_string(i);

        BOOL ok = CreateProcessA(
            NULL,
            cmd.data(),
            NULL, NULL,
            FALSE,
            0,
            NULL, NULL,
            &si,
            &workers[i]
        );

        if (!ok) {
            PrintLastError("CreateProcess");
            return 1;
        }
    }

    for (int t = 0; t < M; t++) {
        int id = t % N;

        Task task{};
        task.type = TASK_PROCESS_DATA;

        // Для теста 2.1 (пустая задача): иногда size=0
        task.size = (t % 7 == 0) ? 0 : (rand() % 10 + 1);

        for (int i = 0; i < task.size; i++)
            task.data[i] = rand() % 100;

        DWORD written = 0;
        if (!WriteFile(inPipes[id], &task, sizeof(Task), &written, NULL)) {
            PrintLastError("WriteFile(task)");
            return 1;
        }

        Result result{};
        DWORD readBytes = 0;
        if (!ReadFile(outPipes[id], &result, sizeof(Result), &readBytes, NULL)) {
            PrintLastError("ReadFile(result)");
            return 1;
        }

        cout << "Task " << t << " -> worker " << id << " result: ";
        for (int i = 0; i < result.size; i++)
            cout << result.data[i] << " ";
        cout << "\n";
    }

    // 3) Send exit command to all workers
    cout << "\nSending TASK_EXIT to all workers...\n";
    for (int i = 0; i < N; i++) {
        Task exitTask{};
        exitTask.type = TASK_EXIT;

        DWORD written = 0;
        if (!WriteFile(inPipes[i], &exitTask, sizeof(Task), &written, NULL)) {
            PrintLastError("WriteFile(exit)");
            return 1;
        }
    }

    // 4) Wait workers & cleanup
    cout << "\nWaiting workers to terminate...\n";
    for (int i = 0; i < N; i++) {
        WaitForSingleObject(workers[i].hProcess, INFINITE);
        cout << "Worker " << i << " terminated.\n";

        CloseHandle(workers[i].hThread);
        CloseHandle(workers[i].hProcess);
        CloseHandle(inPipes[i]);
        CloseHandle(outPipes[i]);
    }

    cout << "\n=== All done. ===\n";
    return 0;
}