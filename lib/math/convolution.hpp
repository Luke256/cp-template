#pragma once

#include "modint.hpp"
#include "pow.hpp"

inline void FFT998(vector<modint<MOD2>>& a, bool invert = false) {
    size_t n = a.size();
    for (size_t i = 1, j = 0; i < n; i++) {
        size_t bit = n >> 1;
        for (; j & bit; bit >>= 1)
            j ^= bit;
        j ^= bit;
        if (i < j)
            swap(a[i], a[j]);
    }
    for (size_t len = 2; len <= n; len <<= 1) {
        modint<MOD2> wlen = modpow(3, (MOD2 - 1) / len, MOD2);
        if (invert)
            wlen = wlen.pow(MOD2 - 2);
        for (size_t i = 0; i < n; i += len) {
            modint<MOD2> w = 1;
            for (size_t j = 0; j < len / 2; j++) {
                modint<MOD2> u = a[i + j], v = a[i + j + len / 2] * w;
                a[i + j] = u + v;
                a[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }
    if (invert) {
        modint<MOD2> n_inv = modpow(n, MOD2 - 2, MOD2);
        for (modint<MOD2>& x : a)
            x *= n_inv;
    }
}
inline vector<modint<MOD2>> Convolution998(vector<modint<MOD2>> a, vector<modint<MOD2>> b) {
    size_t n = 1;
    size_t m = a.size() + b.size() - 1;
    while (n < m)
        n <<= 1;
    a.resize(n);
    b.resize(n);
    FFT998(a);
    FFT998(b);
    for (size_t i = 0; i < n; i++)
        a[i] *= b[i];
    FFT998(a, true);
    a.resize(m);
    return a;
}
