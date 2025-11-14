#include <gtest/gtest.h>
#include "markers.hpp"

//cкрутить до "полного заполнения" (ни одного нуля)
static void round_until_full(Shared& S) {
    for (;;) {
        wait_all_paused(S);
        bool full = true;
        for (int v : S.arr) {
            if (v == 0) { full = false; break; }
        }
        reset_all_paused(S);
        if (full) break;
        // continue всем активным (0 => никого не исключаем)
        continue_all_except(S, 0);
    }
}

// ---- 1.1: один поток заполняет весь массив своими метками ----
TEST(Markers, SingleThreadFill_1_1) {
    Shared S;
    init_shared(S, /*arrN*/10, /*m*/1);
    start_markers(S);

    round_until_full(S);

    for (int v : S.arr) {
        EXPECT_EQ(v, 1);
    }

    terminate_one_and_wait(S, 1);
    destroy_shared(S);
}

// ---- 1.2: завершение очищает все следы ----
TEST(Markers, CleanupOnTerminate_1_2) {
    Shared S;
    init_shared(S, 10, 1);
    start_markers(S);

    round_until_full(S);

    terminate_one_and_wait(S, 1);

    for (int v : S.arr) {
        EXPECT_EQ(v, 0);
    }

    destroy_shared(S);
}

// ---- 2.1: отсутствие гонок c 10 потоками на массиве из 20 ----
TEST(Markers, NoRaces_2_1) {
    Shared S;
    init_shared(S, /*arrN*/20, /*m*/10);
    start_markers(S);

    round_until_full(S);

    int filled = 0;
    for (int v : S.arr) {
        // допускаются только 0 или номера потоков 1..10
        EXPECT_TRUE(v == 0 || (v >= 1 && v <= 10));
        if (v != 0) ++filled;
    }
    EXPECT_EQ(filled, 20); // должно быть заполнено ровно N ячеек

    for (int id = 1; id <= 10; ++id) {
        terminate_one_and_wait(S, id);
    }
    destroy_shared(S);
}

// ---- 2.2: поочередное завершение, промежуточные проверки ----
TEST(Markers, StepwiseTerminate_2_2) {
    Shared S;
    init_shared(S, /*arrN*/30, /*m*/5);
    start_markers(S);

    round_until_full(S);

    for (int id = 1; id <= 5; ++id) {
        // снимок до завершения
        std::vector<int> before = S.arr;

        terminate_one_and_wait(S, id);

        // исчезли только метки завершенного потока id
        for (size_t i = 0; i < S.arr.size(); ++i) {
            if (before[i] == id) {
                EXPECT_EQ(S.arr[i], 0);
            }
            else {
                EXPECT_EQ(S.arr[i], before[i]);
            }
        }

        // продолжаем остальных до нового полного заполнения
        reset_all_paused(S);
        continue_all_except(S, 0);
        round_until_full(S);
    }

    // в финале массив пуст
    for (int v : S.arr) {
        EXPECT_EQ(v, 0);
    }

    destroy_shared(S);
}