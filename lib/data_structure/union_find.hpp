#pragma once

#include "../core.hpp"

// Union Find
struct UnionFind {
    vector<ll> tree;
    UnionFind(ll x) : tree(x, -1) {}
    ll root(ll x) {
        if (tree[x] < 0)
            return x;
        return tree[x] = root(tree[x]);
    }
    bool same(ll x, ll y) {
        return root(x) == root(y);
    }
    ll size(ll x) {
        return -tree[root(x)];
    }
    void unite(ll x, ll y) {
        x = root(x), y = root(y);
        if (x == y)
            return;
        if (size(x) < size(y))
            swap(x, y);
        tree[x] += tree[y];
        tree[y] = x;
    }
};
