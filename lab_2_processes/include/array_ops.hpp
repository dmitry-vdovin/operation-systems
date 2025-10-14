#pragma once
#include <cstdint>
#include <vector>

// Ћогика варианта c) є4 Ч максимум массива.
// ќтдельно от процессов/каналов, чтобы соблюдать DRY и писать юнит-тесты.
int32_t max_element(const std::vector<int32_t>& a);