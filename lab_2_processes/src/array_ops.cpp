#include "array_ops.hpp"
#include <stdexcept>

int32_t max_element(const std::vector<int32_t>& a) {
    if (a.empty()) throw std::invalid_argument("array is empty");
    int32_t m = a[0];
    for (size_t i = 1; i < a.size(); ++i) if (a[i] > m) m = a[i];
    return m;
}