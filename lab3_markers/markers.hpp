#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <vector>

struct Shared {
    std::vector<int>  arr;          // общий массив
    CRITICAL_SECTION  csArr;        // защита массива
    CRITICAL_SECTION  csLog;        // защита логов/печати 
    HANDLE            hStart;       // общее событие "начать всем"
    std::vector<HANDLE> hPaused;    // manual-reset: marker -> main  (я остановился)
    std::vector<HANDLE> hCont;      // auto-reset:  main  -> marker (продолжить)
    std::vector<HANDLE> hTerm;      // auto-reset:  main  -> marker (завершиться)
    std::vector<HANDLE> hThreads;   // дескрипторы потоков
    std::vector<bool>   alive;      // активность потока
    int nThreads = 0;
};

struct MarkerArg { Shared* S; int id; };

unsigned __stdcall markerThread(void* p);

void init_shared(Shared& S, int arrN, int m);
void destroy_shared(Shared& S);

// запуск и базовые «контроллерные» хелперы
void start_markers(Shared& S);
void wait_all_paused(const Shared& S);
void reset_all_paused(Shared& S);
void continue_all_except(Shared& S, int exceptId /*1..N or 0*/);
void terminate_one_and_wait(Shared& S, int id /*1..N*/);