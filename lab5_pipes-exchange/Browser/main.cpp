#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include "protocol.h"
using namespace std;

string MakePipeIn(int id) { return "\\\\.\\pipe\\worker_in_" + to_string(id); }
string MakePipeOut(int id) { return "\\\\.\\pipe\\worker_out_" + to_string(id); }

int main() {
    int N, M;
    cout << "Enter number of workers: ";
    cin >> N;
    cout << "Enter number of tasks: ";
    cin >> M;

    vector<HANDLE> inPipes(N);
    vector<HANDLE> outPipes(N);
    vector<PROCESS_INFORMATION> workers(N);

    // --- 1. Создаём пайпы и запускаем workers ---
    for (int i = 0; i < N; i++) {

        inPipes[i] = CreateNamedPipeA(
            MakePipeIn(i).c_str(),
            PIPE_ACCESS_OUTBOUND,
            PIPE_TYPE_BYTE | PIPE_WAIT,
            1, 1024, 1024,
            0, NULL
        );

        outPipes[i] = CreateNamedPipeA(
            MakePipeOut(i).c_str(),
            PIPE_ACCESS_INBOUND,
            PIPE_TYPE_BYTE | PIPE_WAIT,
            1, 1024, 1024,
            0, NULL
        );

        STARTUPINFOA si{};
        si.cb = sizeof(si);
        string cmd = "Worker.exe " + to_string(i);

        BOOL ok = CreateProcessA(
            NULL, cmd.data(),
            NULL, NULL,
            FALSE, 0,
            NULL, NULL,
            &si, &workers[i]
        );

        if (!ok) {
            cout << "Failed to start worker " << i << endl;
            return 1;
        }
    }

 // setting tasks

    // --- 2. Отправляем M задач воркерам по очереди ---
    for (int t = 0; t < M; t++) {

        int id = t % N; 

        Task task{};
        task.type = TASK_PROCESS_DATA;
        task.size = 5;

        for (int i = 0; i < task.size; i++)
            task.data[i] = rand() % 50;

        cout << "Browser: sending task " << t << " to worker " << id << endl;

        DWORD written;
        WriteFile(inPipes[id], &task, sizeof(Task), &written, NULL);

        // --- 3. Читаем результат ---
        Result result{};
        DWORD readBytes;
        ReadFile(outPipes[id], &result, sizeof(Result), &readBytes, NULL);

        cout << "Result from worker " << id << ": ";
        for (int i = 0; i < result.size; i++)
            cout << result.data[i] << " ";
        cout << "\n";
    }

    return 0;
}