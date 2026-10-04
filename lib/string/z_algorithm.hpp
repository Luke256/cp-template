#pragma once

#include "../core.hpp"

// Z-algorithm
template <ArrayLike T> vector<size_t> ZAlgorithm(const T& x) {
    const size_t n = ranges::size(x);
    if (n == 0)
        return {};
    vector<size_t> z(n);
    z[0] = n;
    size_t l = 0, r = 0;
    for (size_t i = 1; i < n; ++i) {
        if (i < r)
            z[i] = min(r - i, z[i - l]);
        while (i + z[i] < n && x[z[i]] == x[i + z[i]])
            ++z[i];
        if (i + z[i] > r)
            l = i, r = i + z[i];
    }
    return z;
}
