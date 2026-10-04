#pragma once

#include "../core.hpp"

// Modint
template <std::uint_fast64_t Modulus> class modint {
    using u64 = std::uint_fast64_t;

public:
    u64 a;
    constexpr modint(const u64 x = 0) noexcept : a(((x % Modulus) + Modulus) % Modulus) {}
    constexpr u64& value() noexcept {
        return a;
    }
    constexpr const u64& value() const noexcept {
        return a;
    }
    constexpr modint operator+(const modint rhs) const noexcept {
        return modint(*this) += rhs;
    }
    constexpr modint operator-(const modint rhs) const noexcept {
        return modint(*this) -= rhs;
    }
    constexpr modint operator*(const modint rhs) const noexcept {
        return modint(*this) *= rhs;
    }
    constexpr modint operator/(const modint rhs) const noexcept {
        return modint(*this) /= rhs;
    }
    constexpr modint& operator+=(const modint rhs) noexcept {
        a += rhs.a;
        if (a >= Modulus) {
            a -= Modulus;
        }
        return *this;
    }
    constexpr modint& operator-=(const modint rhs) noexcept {
        if (a < rhs.a) {
            a += Modulus;
        }
        a -= rhs.a;
        return *this;
    }
    constexpr modint& operator*=(const modint rhs) noexcept {
        a = a * rhs.a % Modulus;
        return *this;
    }
    constexpr modint& operator/=(modint rhs) noexcept {
        u64 exp = Modulus - 2;
        while (exp) {
            if (exp % 2) {
                *this *= rhs;
            }
            rhs *= rhs;
            exp /= 2;
        }
        return *this;
    }
    constexpr modint pow(u64 exp) noexcept {
        modint res = 1, rhs = *this;
        while (exp) {
            if (exp & 1) {
                res *= rhs;
            }
            rhs *= rhs;
            exp >>= 1;
        }
        return res;
    }
    constexpr modint& operator=(u64 x) {
        a = x % Modulus;
        return *this;
    }
    constexpr bool operator==(const modint x) const {
        return a == x.a;
    }
    friend istream& operator>>(istream& stream, modint<Modulus>& a) {
        u64 x;
        stream >> x;
        a = ((x % Modulus) + Modulus) % Modulus;
        return stream;
    }
    friend ostream& operator<<(ostream& stream, const modint<Modulus> a) {
        stream << a.a;
        return stream;
    }
};
