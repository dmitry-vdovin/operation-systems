#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <format>
#include "names.hpp"
#include "stats.hpp"

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

    // ∆дЄм либо сигнал закрыти€ браузера, либо слот семафора
    HANDLE waits[2] = { hEvt, hSem };
    DWORD wr = WaitForMultipleObjects(2, waits, FALSE, INFINITE);
    if (wr == WAIT_OBJECT_0) {
        LogLine(hMutex, "[child] Browser closing, aborting: "
            + std::string(fileName.begin(), fileName.end()));
        CloseHandle(hSem); CloseHandle(hMutex); CloseHandle(hEvt);
        return 0;
    }

    LogLine(hMutex, "[child] Connected. Start: " + std::string(fileName.begin(), fileName.end()));

    // ¬ариант 4: stddev(200). √енерируем 200 значений [0,100),
    // seed = PID ^ 0x9E3779B9 ^ 4.
    std::mt19937 rng(static_cast<uint32_t>(GetCurrentProcessId()) ^ 0x9E3779B9u ^ 4u);
    std::uniform_real_distribution<double> dist(0.0, 100.0);
    std::vector<double> data(200);
    for (double& x : data) x = dist(rng);

    double sigma = stddev_population_welford(data.data(), data.size());

    // Ђ¬рем€ обработкиї: 1Ц3 секунды
    std::uniform_int_distribution<int> dms(1000, 3000);
    Sleep(dms(rng));

    LogLine(hMutex, std::format("[child] Done: {} | stddev(pop) = {:.6f}",
        std::string(fileName.begin(), fileName.end()), sigma));

    ReleaseSemaphore(hSem, 1, nullptr);
    CloseHandle(hSem); CloseHandle(hMutex); CloseHandle(hEvt);
    return 0;
}