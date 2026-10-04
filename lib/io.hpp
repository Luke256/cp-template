#pragma once

#include "core.hpp"

// overload declarations
template <class T, class U> ostream& operator<<(ostream& stream, const pair<T, U>& p);
template <class T> ostream& operator<<(ostream& stream, const vector<T>& a);
template <class T> ostream& operator<<(ostream& stream, const deque<T>& a);
template <class T> ostream& operator<<(ostream& stream, const set<T>& a);
template <class K, class V> ostream& operator<<(ostream& stream, const map<K, V>& a);
template <class T, class U> istream& operator>>(istream& stream, pair<T, U>& a);
template <class T> istream& operator>>(istream& stream, vector<T>& a);

// concepts
// Sink: kind of container type (emplace_back > emplace > push_back > push > insert)
template <class C, class U>
concept EmplaceBackSink = requires(C& c, U&& x) { c.emplace_back(forward<U>(x)); };
template <class C, class U>
concept EmplaceSink = requires(C& c, U&& x) { c.emplace(forward<U>(x)); };
template <class C, class U>
concept PushBackSink = requires(C& c, U&& x) { c.push_back(forward<U>(x)); };
template <class C, class U>
concept PushSink = requires(C& c, U&& x) { c.push(forward<U>(x)); };
template <class C, class U>
concept InsertSink = requires(C& c, U&& x) { c.insert(forward<U>(x)); };
template <class C, class U>
concept Sink = EmplaceBackSink<C, U> || EmplaceSink<C, U> || PushBackSink<C, U> || PushSink<C, U> ||
               InsertSink<C, U>;
template <class T>
concept Readable = requires(istream& is, T& x) { is >> x; };

// Operator Overloading for Container Access
template <class C, class U>
    requires Sink<C, U>
constexpr C& put(C& c, U&& x) {
    if constexpr (EmplaceBackSink<C, U>)
        c.emplace_back(forward<U>(x));
    else if constexpr (EmplaceSink<C, U>)
        c.emplace(forward<U>(x));
    else if constexpr (PushBackSink<C, U>)
        c.push_back(forward<U>(x));
    else if constexpr (PushSink<C, U>)
        c.push(forward<U>(x));
    else
        c.insert(forward<U>(x));
    return c;
}
template <class C, class U>
    requires Sink<C, U>
C& operator<<(C& c, U&& x) {
    return put(c, forward<U>(x));
}
template <class C, ranges::input_range R>
    requires Sink<C, ranges::range_reference_t<R>>
void operator<<=(C& c, R&& r) {
    for (auto&& x : r)
        c << std::forward<decltype(x)>(x);
}

// inputs
template <class T, class U> ostream& operator<<(ostream& stream, const pair<T, U>& p) {
    stream << "( " << p.first << ", " << p.second << " )";
    return stream;
}
template <class T> ostream& operator<<(ostream& stream, const vector<T>& a) {
    stream << "{ ";
    for (const T& i : a)
        stream << i << " ";
    stream << "}";
    return stream;
}
template <class T> ostream& operator<<(ostream& stream, const deque<T>& a) {
    stream << "{ ";
    for (const T& i : a)
        stream << i << " ";
    stream << "}";
    return stream;
}
template <class T> ostream& operator<<(ostream& stream, const set<T>& a) {
    stream << "{ ";
    for (const T& i : a)
        stream << i << " ";
    stream << "}";
    return stream;
}
template <class K, class V> ostream& operator<<(ostream& stream, const map<K, V>& a) {
    stream << "{ ";
    for (const auto& [k, v] : a)
        stream << "{ " << k << " : " << v << " } ";
    stream << "}";
    return stream;
}

// outputs
template <class T, class U> istream& operator>>(istream& stream, pair<T, U>& a) {
    stream >> a.first >> a.second;
    return stream;
}
template <class T> istream& operator>>(istream& stream, vector<T>& a) {
    for (auto& i : a)
        stream >> i;
    return stream;
}

template <class... Args> struct PredictedInput {
    tuple<Args...> args;
    template <class T>
        requires Readable<T> && constructible_from<T, Args...>
    operator T() && {
        T x = apply(
            [](auto&&... args) -> T {
                return T(forward<decltype(args)>(args)...);
            },
            move(args));
        cin >> x;
        return x;
    }
};
template <class... Args> auto in(Args&&... args) {
    return PredictedInput<decay_t<Args>...>{{forward<Args>(args)...}};
}

template <class T> T in() {
    T x;
    cin >> x;
    return x;
}

template <class T, class U, class... Ts> auto in() {
    tuple<T, U, Ts...> x;
    apply(
        [](auto&... xs) {
            (cin >> ... >> xs);
        },
        x);
    return x;
}
