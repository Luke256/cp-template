#pragma once

#include <bits/stdc++.h>

#pragma GCC optimize("O3,unroll-loops")
#pragma GCC target("avx2,bmi,bmi2,lzcnt,popcnt")

#define rep(i, n) for (ll i = 0; i < (n); ++i)
#define ALL(x) x.begin(), x.end()
#define BACK(x) x.rbegin(), x.rend()
#define MOD1 1000000007
#define MOD1_BASE 131
#define MOD2 998244353
#define INF (LLONG_MAX / 2)
#define FLOAT_ANS fixed << setprecision(20)
#define TORAD(x) (x * acosl(-1) / 180.0)
#define TODEG(x) (x * 180.0 / acosl(-1))

// Templates ----------------------------------------

using namespace std;
using ll = long long;
using LL = __int128_t;
using ull = unsigned long long;
using ld = long double;

// ArrayLike: kind of array-like type (size() / [] / ==)
template <class T>
concept ArrayLike = requires(const T& x, size_t i, size_t j) {
    { ranges::size(x) } -> convertible_to<size_t>;
    { x[i] == x[j] } -> convertible_to<bool>;
};
template <class T, class U>
concept ArrayLikeT = ArrayLike<T> && is_same_v<ranges::range_value_t<T>, U>;

template <typename T> // T:重み
using p_que = priority_queue<T, vector<T>, greater<T>>;

struct Condition {
    bool state;
    constexpr Condition(bool v) noexcept : state(v) {}
    constexpr explicit operator bool() const {
        return state;
    }
    template <class F>
    constexpr Condition then(F&& func) const noexcept(is_nothrow_invocable_v<F>) {
        if (state) {
            invoke(forward<F>(func));
        }
        return *this;
    }
    template <class F>
    constexpr Condition otherwise(F&& func) const noexcept(is_nothrow_invocable_v<F>) {
        if (!state) {
            invoke(forward<F>(func));
        }
        return *this;
    }
};

template <typename T> Condition chmin(T& a, T b) {
    if (a > b) {
        a = b;
        return true;
    }
    return false;
}

template <typename T> Condition chmax(T& a, T b) {
    if (a < b) {
        a = b;
        return true;
    }
    return false;
}

inline auto range(size_t n) {
    return views::iota(0u, n);
}

template <typename T> void RotateVec2(vector<vector<T>>& v) {
    ll h = v.size();
    ll w = v[0].size();
    vector<vector<T>> t(w, vector<T>(h));
    rep(i, h) {
        rep(j, w) {
            t[j][h - i - 1] = v[i][j];
        }
    }
    v = t;
}

template <class T> bool InRange(T x, T mn, T mx) {
    return (mn <= x && x <= mx);
}

template <typename T> vector<T>& merged(vector<T>& a, vector<T>& b) {
    vector<T> res;
    merge(a.begin(), a.end(), b.begin(), b.end(), back_inserter(res));
    return res;
}

inline ll popcount(ll x) {
    ll res = 0;
    while (x) {
        res += x % 2;
        x >>= 1;
    }
    return res;
}

inline ll isqrt(ll n) {
    if (n <= 0)
        return 0;
    ll x = sqrt(n);
    while ((x + 1) * (x + 1) <= n)
        x++;
    while (x * x > n)
        x--;
    return x;
}
