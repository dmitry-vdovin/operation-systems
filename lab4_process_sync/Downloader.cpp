#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <iostream>
#include <string>
#include "names.hpp"

static void Fail(const char* where) {
    std::cerr << "[!] " << where << " failed, GetLastError=" << GetLastError() << "\n";
    ExitProcess(2);
}

int wmain(int argc, wchar_t* argv[]) {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    std::wstring fileName = (argc >= 2) ? argv[1] : L"unknown.bin";

    HANDLE hSem = OpenSemaphoreW(SYNCHRONIZE | SEMAPHORE_MODIFY_STATE, FALSE, kSemName);
    HANDLE hMutex = OpenMutexW(MUTEX_ALL_ACCESS, FALSE, kMutexName);
    HANDLE hEvt = OpenEventW(SYNCHRONIZE, FALSE, kEvtName);
    if (!hSem || !hMutex || !hEvt) Fail("Open kernel objects");

    HANDLE waits[2] = { hEvt, hSem };
    DWORD wr = WaitForMultipleObjects(2, waits, FALSE, INFINITE);
    if (wr == WAIT_OBJECT_0) {
        LogLine(hMutex, "[child] Browser closing, aborting: "
            + std::string(fileName.begin(), fileName.end()));
        CloseHandle(hSem); CloseHandle(hMutex); CloseHandle(hEvt);
        return 0;
    }

    LogLine(hMutex, "[child] Started: " + std::string(fileName.begin(), fileName.end()));
    Sleep(500); // заглушка «работы»
    LogLine(hMutex, "[child] Finished: " + std::string(fileName.begin(), fileName.end()));

    ReleaseSemaphore(hSem, 1, nullptr);
    CloseHandle(hSem); CloseHandle(hMutex); CloseHandle(hEvt);
    return 0;
}