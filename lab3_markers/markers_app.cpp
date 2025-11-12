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
    // корректный вывод UTF-8 в консоль Windows
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
    init_shared(S, n, m);   // массив, критические секции, события, запуск потоков
    start_markers(S);       // общий старт всем потокам

    int alive = m;

    while (alive > 0) {
        // 6a) дождаться, пока ВСЕ активные потоки сообщат о паузе
        wait_all_paused(S);

        // 6b) вывести массив
        print_array(S, "Снимок (все на паузе)");

        // 6c) спросить, кого завершить (0 — завершить всех)
        int killId = 0;
        for (;;) {
            std::cout << "Кого завершить (1.." << m << ", 0 = завершить всех): ";
            std::cin >> killId;
            if (!std::cin) return 1;
            if (killId == 0) break;
            if (1 <= killId && killId <= m && S.alive[killId - 1]) break;
            std::cout << "Некорректный id, попробуйте снова.\n";
        }

        if (killId == 0) {
            // завершить всех оставшихся
            for (int id = 1; id <= m; ++id) {
                if (S.alive[id - 1]) {
                    terminate_one_and_wait(S, id); // 6d–6e
                    --alive;
                }
            }
            break;
        }

        // 6d–6e) завершить выбранный поток и дождаться его
        terminate_one_and_wait(S, killId);
        --alive;

        // 6f) показать, что следы завершённого потока очищены
        print_array(S, "После очистки завершённого");

        // 6g) новый раунд: сбросить paused и продолжить остальных
        reset_all_paused(S);
        continue_all_except(S, killId);
    }

    destroy_shared(S); // освободить ресурсы
    std::cout << "Все потоки завершены. Ресурсы освобождены.\n";
    return 0;
}