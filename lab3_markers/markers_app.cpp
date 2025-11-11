#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <iostream>
#include "markers.hpp"

static void print_array(const Shared& S, const char* title) {
    std::cout << title << " [";
    for (size_t i = 0; i < S.arr.size(); ++i) {
        if (i) std::cout << ' ';
        std::cout << S.arr[i];
    }
    std::cout << "]\n";
}

int main() {
   
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    int n = 0, m = 0;
    std::cout << "Размер массива: ";
    std::cin >> n;
    std::cout << "Количество потоков marker: ";
    std::cin >> m;

    if (n <= 0 || m <= 0) {
        std::cerr << "n и m должны быть > 0\n";
        return 1;
    }

    Shared S;
    init_shared(S, n, m);
    start_markers(S);

   
    wait_all_paused(S);
    print_array(S, "Снимок (все на паузе)");

    for (int id = 1; id <= m; ++id) {
        terminate_one_and_wait(S, id);
    }

    print_array(S, "После завершения всех потоков (очистка выполнена)");
    destroy_shared(S);
    return 0;
}