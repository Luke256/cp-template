#pragma once

#include "../core.hpp"

// Aho-Corasick
template <class T = char> struct AhoCorasick {
    struct Node {
        unordered_map<T, size_t> next;
        size_t fail = 0;
        bool matches = false;
    };
    vector<Node> nodes;
    AhoCorasick(initializer_list<string_view> patterns)
        requires same_as<T, char>
        : AhoCorasick(views::all(patterns)) {}
    AhoCorasick(initializer_list<initializer_list<T>> patterns)
        requires(!same_as<T, char>)
        : AhoCorasick(views::all(patterns)) {}
    template <ranges::input_range R> AhoCorasick(R&& patterns) : nodes(1) {
        for (const auto& pattern : patterns) {
            size_t node = 0;
            for (const T& ch : [&] {
                     if constexpr (same_as<T, char> &&
                                   convertible_to<decltype(pattern), string_view>)
                         return string_view(pattern);
                     else
                         return views::all(pattern);
                 }()) {
                const auto [it, inserted] = nodes[node].next.try_emplace(ch, nodes.size());
                node = it->second;
                if (inserted)
                    nodes.emplace_back();
            }
            nodes[node].matches = true;
        }
        queue<size_t> q;
        for (const auto& [ch, node] : nodes[0].next) {
            nodes[node].matches |= nodes[0].matches;
            q.push(node);
        }
        while (!q.empty()) {
            const size_t node = q.front();
            q.pop();
            for (const auto& [ch, next] : nodes[node].next) {
                const size_t fail = step(nodes[node].fail, ch);
                nodes[next].fail = fail;
                nodes[next].matches |= nodes[fail].matches;
                q.push(next);
            }
        }
    }
    bool match(string_view text) const
        requires same_as<T, char>
    {
        return match(ranges::subrange(text.begin(), text.end()));
    }
    template <ranges::input_range R>
        requires(!same_as<T, char> || !convertible_to<R, string_view>)
    bool match(R&& text) const {
        if (nodes[0].matches)
            return true;
        size_t node = 0;
        for (const T& ch : text) {
            node = step(node, ch);
            if (nodes[node].matches)
                return true;
        }
        return false;
    }
    size_t step(size_t node, const T& ch) const {
        while (true) {
            const auto it = nodes[node].next.find(ch);
            if (it != nodes[node].next.end())
                return it->second;
            if (!node)
                return 0;
            node = nodes[node].fail;
        }
    }
};
template <class T> AhoCorasick(initializer_list<initializer_list<T>>) -> AhoCorasick<T>;
template <ranges::input_range R>
    requires convertible_to<ranges::range_reference_t<R>, string_view>
AhoCorasick(R&&) -> AhoCorasick<char>;
template <ranges::input_range R>
    requires(!convertible_to<ranges::range_reference_t<R>, string_view>)
AhoCorasick(R&&) -> AhoCorasick<ranges::range_value_t<ranges::range_reference_t<R>>>;
