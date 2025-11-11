#include "markers.hpp"
#include <process.h>
#include <iostream>

static void fail(const char* where) {
    DWORD e = GetLastError();
    std::cerr << "[!] " << where << " failed, GetLastError=" << e << "\n";
    ExitProcess(1);
}

unsigned __stdcall markerThread(void* p) {
    auto* A = static_cast<MarkerArg*>(p);
    Shared& S = *A->S;
    const int id = A->id;

    // общий старт
    WaitForSingleObject(S.hStart, INFINITE);

    // (поток #4 → srand(4))
    srand(id);

    int marks = 0;
    for (;;) {
        // 3a) случайный индекс
        size_t idx = static_cast<size_t>(rand()) % S.arr.size();

        // 3b) критсекция вокруг ресурса
        EnterCriticalSection(&S.csArr);

        if (S.arr[idx] == 0) {
            // 3c) «длинная» операция + запись
            Sleep(5);
            S.arr[idx] = id;
            Sleep(5);
            LeaveCriticalSection(&S.csArr);
            ++marks;
            continue; // к следующей попытке
        }

        // занято — выходим из цикла на «паузу»
        LeaveCriticalSection(&S.csArr);

        // сообщение main о паузе
        EnterCriticalSection(&S.csLog);
        std::cout << "[marker " << id << "] paused: marks=" << marks
            << ", blocked_index=" << idx << "\n";
        LeaveCriticalSection(&S.csLog);

        SetEvent(S.hPaused[id - 1]); // marker -> main

        // ждём: продолжить или завершиться
        HANDLE waits[2] = { S.hCont[id - 1], S.hTerm[id - 1] };
        DWORD w = WaitForMultipleObjects(2, waits, FALSE, INFINITE);

        if (w == WAIT_OBJECT_0) {
            // continue: вернуться к циклу
            continue;
        }
        else {
            // terminate: очистить свои следы и завершиться
            EnterCriticalSection(&S.csArr);
            for (size_t j = 0; j < S.arr.size(); ++j) {
                if (S.arr[j] == id) S.arr[j] = 0;
            }
            LeaveCriticalSection(&S.csArr);

            EnterCriticalSection(&S.csLog);
            std::cout << "[marker " << id << "] terminated and cleaned marks\n";
            LeaveCriticalSection(&S.csLog);
            return 0;
        }
    }
}

// утилита создания событий
static HANDLE make_event(BOOL manual, BOOL initial) {
    HANDLE h = CreateEvent(nullptr, manual, initial, nullptr);
    if (!h) fail("CreateEvent");
    return h;
}

void init_shared(Shared& S, int arrN, int m) {
    InitializeCriticalSection(&S.csArr);
    InitializeCriticalSection(&S.csLog);

    S.arr.assign(arrN, 0);
    S.nThreads = m;
    S.hStart = make_event(TRUE, FALSE);     // manual-reset

    S.hPaused.resize(m);
    S.hCont.resize(m);
    S.hTerm.resize(m);
    S.hThreads.resize(m);
    S.alive.assign(m, true);

    for (int i = 0; i < m; ++i) {
        S.hPaused[i] = make_event(TRUE, FALSE); // manual-reset (main сам сбросит)
        S.hCont[i] = make_event(FALSE, FALSE); // auto-reset
        S.hTerm[i] = make_event(FALSE, FALSE); // auto-reset
    }

    // запуск потоков
    for (int i = 0; i < m; ++i) {
        auto* A = new MarkerArg{ &S, i + 1 };
        uintptr_t h = _beginthreadex(nullptr, 0, &markerThread, A, 0, nullptr);
        if (!h) fail("_beginthreadex");
        S.hThreads[i] = reinterpret_cast<HANDLE>(h);
    }
}

void start_markers(Shared& S) {
    SetEvent(S.hStart);
}

void wait_all_paused(const Shared& S) {
    std::vector<HANDLE> waits;
    waits.reserve(S.nThreads);
    for (int i = 0; i < S.nThreads; ++i)
        if (S.alive[i]) waits.push_back(S.hPaused[i]);

    if (waits.empty()) return;

    DWORD r = WaitForMultipleObjects(
        static_cast<DWORD>(waits.size()), waits.data(), TRUE, INFINITE
    );
    if (r == WAIT_FAILED) fail("WaitForMultipleObjects(wait_all_paused)");
}

void reset_all_paused(Shared& S) {
    for (int i = 0; i < S.nThreads; ++i)
        if (S.alive[i]) ResetEvent(S.hPaused[i]);
}

void continue_all_except(Shared& S, int exceptId) {
    for (int i = 0; i < S.nThreads; ++i)
        if (S.alive[i] && (i + 1) != exceptId)
            SetEvent(S.hCont[i]); // auto-reset
}

void terminate_one_and_wait(Shared& S, int id) {
    if (id < 1 || id > S.nThreads || !S.alive[id - 1]) return;
    SetEvent(S.hTerm[id - 1]);
    WaitForSingleObject(S.hThreads[id - 1], INFINITE);
    S.alive[id - 1] = false;
}

void destroy_shared(Shared& S) {
    for (int i = 0; i < S.nThreads; ++i) {
        if (S.hThreads[i]) CloseHandle(S.hThreads[i]);
        if (S.hPaused[i])  CloseHandle(S.hPaused[i]);
        if (S.hCont[i])    CloseHandle(S.hCont[i]);
        if (S.hTerm[i])    CloseHandle(S.hTerm[i]);
    }
    if (S.hStart) CloseHandle(S.hStart);

    DeleteCriticalSection(&S.csArr);
    DeleteCriticalSection(&S.csLog);
}