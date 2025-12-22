#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include "protocol.h"
using namespace std;

string MakePipeIn(int id) { return "\\\\.\\pipe\\worker_in_" + to_string(id); }
string MakePipeOut(int id) { return "\\\\.\\pipe\\worker_out_" + to_string(id); }

int main() {
    int N = 0;
    cout << "Enter number of workers: ";
    cin >> N;

    vector<HANDLE> hIn(N);
    vector<HANDLE> hOut(N);
    vector<PROCESS_INFORMATION> workers(N);

    for (int i = 0; i < N; i++) {

        cout << "Creating pipes for worker " << i << endl;

        // --- PIPE IN ---
        hIn[i] = CreateNamedPipeA(
            MakePipeIn(i).c_str(),
            PIPE_ACCESS_OUTBOUND,
            PIPE_TYPE_BYTE | PIPE_WAIT,
            1, 1024, 1024,
            0, NULL
        );

        if (hIn[i] == INVALID_HANDLE_VALUE) {
            cout << "Failed to create IN pipe\n";
            return 1;
        }

        // --- PIPE OUT ---
        hOut[i] = CreateNamedPipeA(
            MakePipeOut(i).c_str(),
            PIPE_ACCESS_INBOUND,
            PIPE_TYPE_BYTE | PIPE_WAIT,
            1, 1024, 1024,
            0, NULL
        );

        if (hOut[i] == INVALID_HANDLE_VALUE) {
            cout << "Failed to create OUT pipe\n";
            return 1;
        }

        // --- START WORKER PROCESS ---
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
            cout << "Failed to start worker " << i << endl;
            return 1;
        }

        cout << "Worker " << i << " created.\n";
    }

    cout << "\nCommit 1: Setup complete.\n";
    return 0;
}