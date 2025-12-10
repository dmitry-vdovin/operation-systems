#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <format>
#include "names.hpp"

static void Fail(const char* where) {
    std::cerr << "[!] " << where << " failed, GetLastError=" << GetLastError() << "\n";
    ExitProcess(1);
}

int wmain() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    int N = 0, M = 0;
    std::cout << "Max parallel downloads N: "; std::cin >> N;
    std::cout << "Total queued files M (>N): "; std::cin >> M;
    if (N <= 0 || M <= 0 || M <= N) { std::cerr << "Require N>0 and M>N\n"; return 1; }

    HANDLE hSem = CreateSemaphoreW(nullptr, N, N, kSemName);
    HANDLE hMutex = CreateMutexW(nullptr, FALSE, kMutexName);
    HANDLE hEvt = CreateEventW(nullptr, TRUE, FALSE, kEvtName);
    if (!hSem || !hMutex || !hEvt) Fail("Create kernel objects");

    std::wstring childPath = FindDownloaderExe();
    std::vector<HANDLE> children; children.reserve(M);

    for (int i = 0; i < M; ++i) {
        std::wstring file = std::format(L"file_{:03d}.bin", i + 1);
        std::wstring cmd = L"\"" + childPath + L"\" \"" + file + L"\"";
        STARTUPINFOW si{}; si.cb = sizeof(si);
        PROCESS_INFORMATION pi{};
        if (!CreateProcessW(childPath.c_str(), cmd.data(),
            nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
            std::wcerr << L"[!] Failed to start Downloader for " << file << L"\n";
            continue;
        }
        CloseHandle(pi.hThread);
        children.push_back(pi.hProcess);
    }

    std::cout << "Browser running. Press Enter to close...\n";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::string dummy; std::getline(std::cin, dummy);

    SetEvent(hEvt); // сигнал «закрываемся»
    WaitAllProcesses(children);
    for (HANDLE p : children) CloseHandle(p);

    CloseHandle(hSem); CloseHandle(hMutex); CloseHandle(hEvt);
    std::cout << "All done.\n";
    return 0;
}