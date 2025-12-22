#include <windows.h>
#include <iostream>
#include <string>
#include "protocol.h"
using namespace std;

string MakePipeIn(int id) { return "\\\\.\\pipe\\worker_in_" + to_string(id); }
string MakePipeOut(int id) { return "\\\\.\\pipe\\worker_out_" + to_string(id); }

int main(int argc, char* argv[]) {

    if (argc < 2) {
        cout << "Worker started without ID.\n";
        return 1;
    }

    int id = atoi(argv[1]);
    cout << "Worker " << id << " starting...\n";

    HANDLE hIn = CreateFileA(
        MakePipeIn(id).c_str(),
        GENERIC_READ,
        0, NULL,
        OPEN_EXISTING,
        0, NULL
    );

    HANDLE hOut = CreateFileA(
        MakePipeOut(id).c_str(),
        GENERIC_WRITE,
        0, NULL,
        OPEN_EXISTING,
        0, NULL
    );

    cout << "Worker " << id << ": connected to pipes.\n";

    cout << "Commit 1: Worker ready.\n";
    return 0;
}