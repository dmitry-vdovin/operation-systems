// lab1_threads_variant4.cpp
// Вариант 4: вывести элементы массива, попадающие в [a, b].

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>   // _beginthreadex
#include <iostream>
#include <vector>
#include <string>
#include <limits>

struct WorkerInput {
    std::vector<int> data;
    int a;
    int b;
    // Результат работы потока:
    std::vector<int> result;
    // флаг завершения (для демонстрации варианта без WaitForSingleObject)
    volatile bool done = false;
};

// Реализация worker для CreateThread
DWORD WINAPI WorkerProc_CreateThread(LPVOID p) {
    WorkerInput* in = static_cast<WorkerInput*>(p);
    // отобрать элементы из [a, b]
    for (int x : in->data) {
        if (x >= in->a && x <= in->b) {
            in->result.push_back(x);
        }
        Sleep(10);
    }
    in->done = true;
    return 0;
}

// Реализация worker для _beginthreadex
unsigned __stdcall WorkerProc_BeginThreadEx(void* p) {
    WorkerInput* in = static_cast<WorkerInput*>(p);
    for (int x : in->data) {
        if (x >= in->a && x <= in->b) {
            in->result.push_back(x);
        }
        Sleep(10);
    }
    in->done = true;
    return 0;
}

// обработка ввода целого числа
int readInt(const char* prompt) {
    while (true) {
        std::cout << prompt;
        long long v;
        if (std::cin >> v) return static_cast<int>(v);
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "[!] Некорректный ввод, попробуйте еще раз.\n";
    }
}

int main() {
    setlocale(LC_ALL, "Russian");
    std::ios::sync_with_stdio(false);

    std::cout << "Вариант 4: вывести элементы массива, попадающие в [a, b]\n\n";

    // 1) Ввод массива
    int n = readInt("Введите размер массива n: ");
    std::vector<int> arr;
    arr.reserve(n);
    std::cout << "Выберите способ заполнения:\n"
        << " 1 - ввод вручную\n"
        << " 2 - сгенерировать случайно\n> ";
    int fillMode = 1;
    std::cin >> fillMode;

    if (fillMode == 2) {
        int seed = readInt("seed: ");
        int lo = readInt("минимум: ");
        int hi = readInt("максимум: ");
        if (lo > hi) std::swap(lo, hi);
        // генератор случайных чисел
        unsigned int s = static_cast<unsigned int>(seed);
        auto rnd = [&]() {
            s = 1664525u * s + 1013904223u;
            return lo + static_cast<int>(s % (unsigned)(hi - lo + 1));
            };
        for (int i = 0; i < n; ++i) arr.push_back(rnd());
    }
    else {
        std::cout << "Введите " << n << " целых: ";
        for (int i = 0; i < n; ++i) {
            int x; std::cin >> x; arr.push_back(x);
        }
    }

    // Интервал [a, b]
    int a = readInt("Введите a: ");
    int b = readInt("Введите b: ");
    if (a > b) std::swap(a, b);

    // Пауза для Suspend/Resume
    int pauseMs = readInt("Пауза перед возобновлением worker (мс): ");
    if (pauseMs < 0) pauseMs = 0;


    std::cout << "\nВыберите способ создания потока:\n"
        << " 1 - CreateThread\n"
        << " 2 - _beginthreadex\n> ";
    int method = 1;
    std::cin >> method;

    // Способ ожидания завершения
    std::cout << "\nОжидать завершение:\n"
        << " 1 - WaitForSingleObject(INFINITE)\n"
        << " 2 - БЕЗ WaitForSingleObject (опрос GetExitCodeThread)\n> ";
    int waitMode = 1;
    std::cin >> waitMode;

    // Упаковка параметров
    WorkerInput in;
    in.data = std::move(arr);
    in.a = a;
    in.b = b;

    // 3) Создание потока
    HANDLE hThread = nullptr;
    DWORD threadId = 0;
    uintptr_t thrd = 0;

    if (method == 2) {
        unsigned threadIdEx = 0;
        // Создаём "сразу запущенным"
        thrd = _beginthreadex(
            /*security*/ nullptr,
            /*stack*/ 0,
            /*start*/ &WorkerProc_BeginThreadEx,
            /*arg*/ &in,
            /*initflag*/ 0,         // 0 => сразу запустится
            &threadIdEx
        );
        if (!thrd) {
            std::cerr << "Ошибка: _beginthreadex вернул 0\n";
            return 1;
        }
        hThread = reinterpret_cast<HANDLE>(thrd);
        threadId = static_cast<DWORD>(threadIdEx);
    }
    else {
        // CreateThread
        hThread = CreateThread(
            /*lpThreadAttributes*/ nullptr,
            /*dwStackSize*/ 0,
            /*lpStartAddress*/ &WorkerProc_CreateThread,
            /*lpParameter*/ &in,
            /*dwCreationFlags*/ 0,  // создаём и сразу запускаем
            /*lpThreadId*/ &threadId
        );
        if (!hThread) {
            std::cerr << "Ошибка: CreateThread вернул NULL, GetLastError=" << GetLastError() << "\n";
            return 1;
        }
    }

    std::cout << "Поток создан. ID=" << threadId << "\n";

    // 4) Приостановить, подождать pauseMs, возобновить
    Sleep(1);
    DWORD suspendCount = SuspendThread(hThread); // увеличит счетчик приостановок
    if (suspendCount == (DWORD)-1) {
        std::cerr << "SuspendThread() ошибка, GetLastError=" << GetLastError() << "\n";
    }
    else {
        std::cout << "Worker приостановлен. SuspendCount=" << suspendCount + 1 << "\n";
    }

    std::cout << "Ждём " << pauseMs << " мс...\n";
    Sleep(pauseMs);

    DWORD resumeCount = ResumeThread(hThread); // уменьшит счётчик
    if (resumeCount == (DWORD)-1) {
        std::cerr << "ResumeThread() ошибка, GetLastError=" << GetLastError() << "\n";
    }
    else {
        std::cout << "Worker возобновлён. Новый SuspendCount=" << resumeCount - 1 << "\n";
    }

    // 6) дождаться завершения Worker (2 способа)
    if (waitMode == 1) {
        // ждём бесконечно
        WaitForSingleObject(hThread, INFINITE);
    }
    else {
        // Без WaitForSingleObject:
        // опрашиваем код завершения, пока STILL_ACTIVE
        DWORD code = STILL_ACTIVE;
        while (true) {
            if (!GetExitCodeThread(hThread, &code)) {
                std::cerr << "GetExitCodeThread() ошибка, GetLastError=" << GetLastError() << "\n";
                break;
            }
            if (code != STILL_ACTIVE) break; // поток завершился
           
            Sleep(20);
        }
    }

    // 7) вывожу результат
    std::cout << "\nЭлементы из отрезка [" << in.a << ", " << in.b << "]: ";
    if (in.result.empty()) {
        std::cout << "(нет)\n";
    }
    else {
        for (size_t i = 0; i < in.result.size(); ++i) {
            if (i) std::cout << ' ';
            std::cout << in.result[i];
        }
        std::cout << "\n";
    }

    // 8) закрыть дескриптор и выйти
    CloseHandle(hThread);
    std::cout << "Готово.\n";
    return 0;
}