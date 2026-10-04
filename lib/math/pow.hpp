#pragma once

#include "../core.hpp"

inline ll fastpow(ll a, ll n) {
    ll res = 1;
    while (n > 0) {
        if (n & 1)
            res *= a;
        a *= a;
        n >>= 1;
    }
    return res;
}

inline ll modpow(ll a, ll n, ll mod) {
    ll res = 1;
    while (n > 0) {
        if (n & 1)
            res = (res * (a % mod)) % mod;
        a = ((a % mod) * (a % mod)) % mod;
        n >>= 1;
    }
    return res;
}
