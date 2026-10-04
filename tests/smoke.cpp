#include "../lib/all.hpp"
#include "../lib/math/convolution.hpp"

ll sum(ll a, ll b) { return a + b; }
ll assign_value(ll, ll b) { return b; }
ll maximum(ll a, ll b) { return max(a, b); }
ll add(ll a, ll b) { return a + b; }

int main() {
    UnionFind uf(4);
    uf.unite(0, 1);
    assert(uf.same(0, 1) && !uf.same(0, 2) && uf.size(0) == 2);

    SegTree<ll> seg(4, sum, 0, assign_value);
    seg.update(0, 3);
    seg.update(2, 5);
    assert(seg.query(0, 3) == 8 && seg.get(2) == 5);

    LazySegTree<ll, ll, maximum, add, add> lazy(4, -INF, 0, {1, 2, 3, 4});
    lazy.update(1, 3, 10);
    assert(lazy.query(0, 4) == 13 && lazy.get(1) == 12);

    assert((ZAlgorithm(string("ababa")) == vector<size_t>{5, 0, 3, 0, 1}));
    AhoCorasick patterns({"abc", "bc"});
    assert(patterns.match("zabc") && !patterns.match("zzz"));

    using mint = modint<MOD2>;
    auto product = Convolution998({mint(1), mint(2)}, {mint(3), mint(4)});
    assert(product.size() == 3 && product[0] == mint(3)
           && product[1] == mint(10) && product[2] == mint(8));

    Geometry2D::Line line({0, 0}, {2, 0});
    assert(line.onSegment({1, 0}) && !line.onSegment({3, 0}));
    Grid<ll> grid(2, 3, 0);
    grid(1, 2) = 7;
    Geometry2D::Vector2D<ll> cell(1, 2);
    assert(grid[cell] == 7);

    vector<ll> values(2);
    istringstream input("4 5");
    input >> values;
    assert((values == vector<ll>{4, 5}));
    ostringstream output;
    output << values;
    assert(output.str() == "{ 4 5 }");
    assert(Debugger::toStr(values) == "{ 4 5 }");
    ll best = 5;
    chmin(best, 3LL).then([&] { assert(best == 3); });
    assert(fastpow(2, 10) == 1024 && modpow(2, 10, 1000) == 24);
}
