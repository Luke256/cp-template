#pragma once

#include "../core.hpp"

// LazySegTree
template <class Monoid, class Action, auto merge, auto apply, auto comp> struct LazySegTree {
    static_assert(std::is_convertible_v<decltype(merge), std::function<Monoid(Monoid, Monoid)>>,
                  "merge must work as Monoid(Monoid, Monoid)");
    static_assert(std::is_convertible_v<decltype(apply), std::function<Monoid(Monoid, Action)>>,
                  "mapping must work as Monoid(Monoid, Action)");
    static_assert(std::is_convertible_v<decltype(comp), std::function<Action(Action, Action)>>,
                  "composition must work as Action(Action, Action)");
    ull n;
    vector<Monoid> tree;
    vector<Action> lazy;
    Monoid e;
    Action id;
    LazySegTree(ull n_, Monoid e_, Action id_, const vector<Monoid>& initial_values = {})
        : e(e_), id(id_) {
        ull s = 1;
        while (s < n_)
            s <<= 1;
        n = s;
        tree.assign(n * 2, e);
        lazy.assign(n * 2, id);
        for (ull i = 0; i < initial_values.size(); i++)
        {
            tree[i + n - 1] = initial_values[i];
        }
        for (ull i = n - 2; i < n - 1; i--)
        {
            tree[i] = merge(tree[(i << 1) + 1], tree[(i << 1) + 2]);
        }
    }
    Monoid query(ull l, ull r) {
        return query(l, r, 0, 0, n);
    }
    void update(ull idx, Action x) {
        update(idx, idx + 1, x);
    }
    void update(ull l, ull r, Action x) {
        update(l, r, 0, 0, n, x);
    }
    Monoid get(ull i) {
        return query(i, i + 1);
    }
    void clear() {
        fill(tree.begin(), tree.end(), e);
        fill(lazy.begin(), lazy.end(), id);
    }
    void flush() {
        for (int i = 0; i < n * 2; i++)
        {
            fetch(i);
        }
    }
    const Monoid& operator[](ull idx) {
        return tree[idx + n - 1];
    }

private:
    void fetch(ull pos) {
        if (lazy[pos] == id)
            return;
        if (pos < n - 1) {
            lazy[(pos << 1) + 1] = comp(lazy[(pos << 1) + 1], lazy[pos]);
            lazy[(pos << 1) + 2] = comp(lazy[(pos << 1) + 2], lazy[pos]);
        }
        tree[pos] = apply(tree[pos], lazy[pos]);
        lazy[pos] = id;
    }
    Monoid query(ull ql, ull qr, ull pos, ull rl, ull rr) {
        if (rr <= ql || qr <= rl)
            return e;
        fetch(pos);
        if (ql <= rl && rr <= qr)
            return tree[pos];
        return merge(query(ql, qr, (pos << 1) + 1, rl, (rl + rr) >> 1),
                     query(ql, qr, (pos << 1) + 2, (rl + rr) >> 1, rr));
    }
    void update(ull ql, ull qr, ull pos, ull rl, ull rr, Action x) {
        fetch(pos);
        if (rr <= ql || qr <= rl)
            return;
        if (ql <= rl && rr <= qr) {
            lazy[pos] = comp(lazy[pos], x);
            fetch(pos);
            return;
        }
        update(ql, qr, (pos << 1) + 1, rl, (rl + rr) >> 1, x);
        update(ql, qr, (pos << 1) + 2, (rl + rr) >> 1, rr, x);
        tree[pos] = merge(tree[(pos << 1) + 1], tree[(pos << 1) + 2]);
    }
};
