#pragma once

#include "../geometry/geometry2d.hpp"

// Grid
template <class T> struct Grid {
    vector<T> data;
    vector<ull> shape;
    Grid() {}
    Grid(ull h, ull w, T x = T()) : data(h * w, x), shape({h, w}) {}
    Grid(initializer_list<ull> shape_, T x = T())
        : data(accumulate(shape_.begin(), shape_.end(), 1, multiplies<ull>()), x), shape(shape_) {}
    T& operator[](ull i) {
        return data.data()[i];
    }
    template <class Container, class = decltype(begin(declval<Container>()))>
    T& operator[](const Container& idx) {
        ull i = 0;
        for (ull j = 0; j < shape.size(); ++j) {
            i = i * shape.data()[j] + idx[j];
        }
        return data.data()[i];
    }
    template <class U> T& operator[](initializer_list<U> idx) {
        ull i = 0;
        auto it = idx.begin();
        for (ull j = 0; j < shape.size(); ++j, ++it) {
            i = i * shape.data()[j] + *it;
        }
        return data.data()[i];
    }
    template <class U> T& operator[](const Geometry2D::Vector2D<U>& idx) {
        return data.data()[idx.x * shape.data()[1] + idx.y];
    }
    template <class... Args> T& operator()(Args... args) {
        ull idxs[] = {static_cast<ull>(args)...};
        ull i = 0;
        for (ull j = 0; j < sizeof...(Args); ++j) {
            i = i * shape.data()[j] + idxs[j];
        }
        return data[i];
    }
    friend istream& operator>>(istream& stream, Grid<T>& grid) {
        for (auto& i : grid.data)
            stream >> i;
        return stream;
    }
};
