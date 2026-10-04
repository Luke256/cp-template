#pragma once

#include "../core.hpp"

// SegTree
template <class Monoid, class Action = Monoid> struct SegTree {
    using MergeFunc = Monoid (*)(Monoid, Monoid);
    using ApplyFunc = Monoid (*)(Monoid, Action);
    ull n;
    vector<Monoid> tree;
    Monoid e;
    MergeFunc merge;
    ApplyFunc apply;
    SegTree(ull n_, MergeFunc mf, Monoid e_, ApplyFunc af) : e(e_), merge(mf), apply(af) {
        ull s = 1;
        while (s < n_)
            s <<= 1;
        n = s;
        tree.assign(n * 2, e);
    }
    Monoid query(ull l, ull r) {
        return query(l, r, 0, 0, n);
    }
    void update(ull idx, Action x) {
        idx += n - 1;
        tree[idx] = apply(tree[idx], x);
        while (idx) {
            idx = (idx - 1) / 2;
            tree[idx] = merge(tree[idx * 2 + 1], tree[idx * 2 + 2]);
        }
    }
    Monoid get(ull i) {
        return query(i, i + 1);
    }

private:
    Monoid query(ull ql, ull qr, ull pos, ull rl, ull rr) {
        if (qr <= rl || rr <= ql)
            return e;
        if (ql <= rl && rr <= qr)
            return tree[pos];
        return merge(query(ql, qr, pos * 2 + 1, rl, (rl + rr) >> 1),
                     query(ql, qr, pos * 2 + 2, (rl + rr) >> 1, rr));
    }
};
