// lab_2_processes.cpp: определяет точку входа для приложения.
//

#include "lab_2_processes.h"

// lab2_pipes_max.cpp — Шаг 2: полноценный обмен массивом + max()
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <random>
#include <algorithm>
#include <cstdint>

static void Fail(const char* where) {
    DWORD e = GetLastError();
    std::cerr << "[!] " << where << " failed, GetLastError=" << e << "\n";
    ExitProcess(1);
}

static bool WriteAll(HANDLE h, const void* buf, DWORD bytes) {
    const BYTE* p = static_cast<const BYTE*>(buf);
    DWORD done = 0;
    while (done < bytes) {
        DWORD w = 0;
        if (!WriteFile(h, p + done, bytes - done, &w, nullptr)) return false;
        done += w;
    }
    return true;
}

static bool ReadExact(HANDLE h, void* buf, DWORD bytes) {
    BYTE* p = static_cast<BYTE*>(buf);
    DWORD done = 0;
    while (done < bytes) {
        DWORD r = 0;
        if (!ReadFile(h, p + done, bytes - done, &r, nullptr)) return false; // EOF
        if (r == 0) return false; // EOF
        done += r;
    }
    return true;
}

static int readInt(const char* prompt) {
    while (true) {
        std::cout << prompt;
        long long v;
        if (std::cin >> v) return static_cast<int>(v);
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "[!] Некорректный ввод, попробуйте еще раз.\n";
    }
}

// ----------------- CHILD MODE -----------------
int childMain() {
    HANDLE hin = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE hout = GetStdHandle(STD_OUTPUT_HANDLE);

    DWORD n = 0;
    if (!ReadExact(hin, &n, sizeof(n))) return 0; // нет данных — выходим спокойно

    std::vector<int32_t> a(n);
    if (n > 0) {
        if (!ReadExact(hin, a.data(), n * sizeof(int32_t))) return 0; // ранний EOF
        // Вариант c) №4: максимальный элемент массива
        int32_t mx = a[0];
        for (DWORD i = 1; i < n; ++i) if (a[i] > mx) mx = a[i];
        WriteAll(hout, &mx, sizeof(mx));
    }
    else {
        // n == 0: договоримся возвращать INT32_MIN
        int32_t mx = INT32_MIN;
        WriteAll(hout, &mx, sizeof(mx));
    }
    return 0;
}

// ----------------- PARENT MODE -----------------
int parentMain() {
    // 1) Создание каналов
    SECURITY_ATTRIBUTES sa{ sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };
    HANDLE cStdinRd = nullptr, cStdinWr = nullptr;
    HANDLE cStdoutRd = nullptr, cStdoutWr = nullptr;
    if (!CreatePipe(&cStdinRd, &cStdinWr, &sa, 0)) Fail("CreatePipe(stdin)");
    if (!CreatePipe(&cStdoutRd, &cStdoutWr, &sa, 0)) Fail("CreatePipe(stdout)");

    SetHandleInformation(cStdinWr, HANDLE_FLAG_INHERIT, 0); // оставляем у родителя
    SetHandleInformation(cStdoutRd, HANDLE_FLAG_INHERIT, 0); // оставляем у родителя

    // 2) STARTUPINFO с перенаправлением стандартных дескрипторов ребёнка
    STARTUPINFOA si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = cStdinRd;    // child: stdin <- Pipe1(READ)
    si.hStdOutput = cStdoutWr;   // child: stdout -> Pipe2(WRITE)
    si.hStdError = cStdoutWr;

    PROCESS_INFORMATION pi{};
    char exePath[512];
    GetModuleFileNameA(nullptr, exePath, sizeof(exePath));
    std::string cl = std::string("\"") + exePath + "\" child";

    if (!CreateProcessA(nullptr, cl.data(), nullptr, nullptr, TRUE, 0, nullptr, nullptr, &si, &pi)) {
        Fail("CreateProcess(child)");
    }

    // Эти концы нам больше не нужны в родителе:
    CloseHandle(cStdinRd);
    CloseHandle(cStdoutWr);

    // -------- Ввод/генерация массива в родителе --------
    int n = readInt("Введите размер массива n: ");
    if (n < 0) n = 0;
    std::vector<int32_t> a;
    a.reserve(n);

    std::cout << "Способ заполнения: 1 — вручную, 2 — случайно? ";
    int mode = 1; std::cin >> mode;

    if (mode == 2) {
        int lo = readInt("минимум: ");
        int hi = readInt("максимум: ");
        if (lo > hi) std::swap(lo, hi);
        std::mt19937 rng{ std::random_device{}() };
        std::uniform_int_distribution<int32_t> dist(lo, hi);
        for (int i = 0; i < n; ++i) a.push_back(dist(rng));
    }
    else {
        std::cout << "Введите " << n << " целых: ";
        for (int i = 0; i < n; ++i) { long long x; std::cin >> x; a.push_back((int32_t)x); }
    }

    // -------- Передаём данные ребёнку по Pipe1 --------
    DWORD N = static_cast<DWORD>(n);
    if (!WriteAll(cStdinWr, &N, sizeof(N))) Fail("WriteAll(N)");
    if (N > 0) {
        if (!WriteAll(cStdinWr, a.data(), N * sizeof(int32_t))) Fail("WriteAll(array)");
    }
    // ВАЖНО: закрыть запись => ребёнок увидит EOF после чтения всех данных
    CloseHandle(cStdinWr);

    // -------- Читаем результат из Pipe2 --------
    int32_t mx = INT32_MIN;
    if (!ReadExact(cStdoutRd, &mx, sizeof(mx))) Fail("ReadExact(result)");
    CloseHandle(cStdoutRd);

    // -------- Ожидание завершения потомка и вывод --------
    WaitForSingleObject(pi.hProcess, INFINITE); // синхронизация по заданию (лаба_2_ОС.pdf)
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    std::cout << "Максимальный элемент массива: " << mx << "\n";
    return 0;
}

int main(int argc, char** argv) {
    setlocale(LC_ALL, "rus");
    // два режима (родитель/потомок), запуск child через CreateProcess с перенаправлением stdin/stdout
    // см. требования ЛР и конспект по процессам/ожиданию. (лаба_2_ОС.pdf) (03-Процессы.pdf)
    if (argc > 1 && std::string(argv[1]) == "child") return childMain();
    return parentMain();
}