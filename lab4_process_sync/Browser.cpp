#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <iostream>
#include <string>
#include <limits>
#include "names.hpp"

static void Fail(const char* where) {
    std::cerr << "[!] " << where << " failed, GetLastError=" << GetLastError() << "\n";
    ExitProcess(1);
}

int wmain() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    int N = 0; // макс. параллельных слотов
    std::cout << "Max parallel downloads N: ";
    std::cin >> N;
    if (N <= 0) { std::cerr << "N>0 required\n"; return 1; }

    HANDLE hSem = CreateSemaphoreW(nullptr, N, N, kSemName);
    HANDLE hMutex = CreateMutexW(nullptr, FALSE, kMutexName);
    HANDLE hEvt = CreateEventW(nullptr, TRUE, FALSE, kEvtName);
    if (!hSem || !hMutex || !hEvt) Fail("Create kernel objects");

    std::wstring childPath = FindDownloaderExe();
    std::wstring file = L"file_001.bin";
    std::wstring cmd = L"\"" + childPath + L"\" \"" + file + L"\"";

    STARTUPINFOW si{}; si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};
    BOOL ok = CreateProcessW(childPath.c_str(), cmd.data(),
        nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi);
    if (!ok) Fail("CreateProcessW(Downloader)");

    CloseHandle(pi.hThread);

    std::cout << "Browser running. Press Enter to close...\n";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::string dummy; std::getline(std::cin, dummy);

    SetEvent(hEvt); // сообщаем всем «закрываемся»

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);

    CloseHandle(hSem); CloseHandle(hMutex); CloseHandle(hEvt);
    std::cout << "Done.\n";
    return 0;
}