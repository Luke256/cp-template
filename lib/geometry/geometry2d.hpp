#pragma once

#include "../core.hpp"

namespace Geometry2D {
    // Vector2D
    template <class T = ll> struct Vector2D {
        T x, y;
        Vector2D() : x(0), y(0) {}
        Vector2D(T x_, T y_) : x(x_), y(y_) {}

        long double length() const {
            return sqrt((long double)x * x + y * y);
        };
        T lengthp() const {
            return x * x + y * y;
        };
        bool inrange(const Vector2D a, const Vector2D b) const {
            return (InRange(x, a.x, b.x) and InRange(y, a.y, b.y));
        }
        Vector2D yx() const {
            return Vector2D{y, x};
        }
        Vector2D rotated90() const {
            return Vector2D{-y, x};
        }
        Vector2D normalized() const {
            return *this / length();
        }
        T cross(const Vector2D a) const {
            return x * a.y - y * a.x;
        }
        Vector2D operator-(const Vector2D a) const {
            return Vector2D(*this) -= a;
        }
        Vector2D operator+(const Vector2D a) const {
            return Vector2D(*this) += a;
        }
        T operator*(const Vector2D a) const {
            return x * a.x + y * a.y;
        }
        Vector2D operator*(const T a) const {
            return Vector2D(*this) *= a;
        }
        Vector2D operator/(const T a) const {
            return Vector2D(*this) /= a;
        }
        Vector2D& operator+=(const Vector2D a) {
            x += a.x;
            y += a.y;
            return *this;
        }
        Vector2D& operator-=(const Vector2D a) {
            x -= a.x;
            y -= a.y;
            return *this;
        }
        Vector2D& operator-=(const T a) {
            x -= a;
            y -= a;
            return *this;
        }
        Vector2D& operator*=(const T a) {
            x *= a;
            y *= a;
            return *this;
        }
        Vector2D& operator/=(const T a) {
            x /= a;
            y /= a;
            return *this;
        }
        bool operator==(const Vector2D a) const {
            return (x == a.x and y == a.y);
        }
        bool operator!=(const Vector2D a) const {
            return not(x == a.x and y == a.y);
        }
        bool operator>(const Vector2D a) const {
            return a < *this;
        }
        bool operator<(const Vector2D a) const {
            return make_pair(x, y) < make_pair(a.x, a.y);
        }
        // 偏角ソート
        // bool operator<(const Vector2D a) const { bool ah = (y < 0 or (y == 0 and x < 0)); bool bh
        // = (a.y < 0 or (a.y == 0 and a.x < 0)); if (ah ^ bh) return bh; return 0 < x * a.y - y *
        // a.x; }

        friend ostream& operator<<(ostream& stream, const Vector2D<T>& x) {
            stream << "(" << x.x << ", " << x.y << ")";
            return stream;
        }
        friend istream& operator>>(istream& stream, Vector2D<T>& x) {
            stream >> x.x >> x.y;
            return stream;
        }
    };

    using Scalar = long double;
    using Vec2 = Vector2D<Scalar>;
    constexpr Scalar EPS = 1e-12L;

    // x in? [a,b]
    inline bool InRange(Scalar x, Scalar a, Scalar b) {
        return min(a, b) - EPS <= x and x <= max(a, b) + EPS;
    }

    inline int Sign(Scalar x) {
        if (x > EPS)
            return 1;
        if (x < -EPS)
            return -1;
        return 0;
    }

    struct Line {
        Vec2 p1, p2, d;
        Line(Vec2 p1, Vec2 p2) : p1(p1), p2(p2), d(p2 - p1) {}
        bool onSegment(const Vec2& p) const {
            return Sign(d.cross(p - p1)) == 0 and InRange(p.x, p1.x, p2.x) and
                   InRange(p.y, p1.y, p2.y);
        }
        bool segIntersects(const Line& line) const {
            int a = Sign(d.cross(line.p1 - p1));
            int b = Sign(d.cross(line.p2 - p1));
            int c = Sign(line.d.cross(p1 - line.p1));
            int d = Sign(line.d.cross(p2 - line.p1));
            return (a * b < 0 and c * d < 0) or (a == 0 and onSegment(line.p1)) or
                   (b == 0 and onSegment(line.p2)) or (c == 0 and line.onSegment(p1)) or
                   (d == 0 and line.onSegment(p2));
        }
        bool lineIntersects(const Line& line) const {
            return Sign(d.cross(line.d)) != 0 or Sign(d.cross(line.p1 - p1)) == 0;
        }
        Vec2 intersectsAt(const Line& line) const {
            if (!lineIntersects(line) or Sign(d.cross(line.d)) == 0)
                return Vec2{};
            return p1 + d * (line.p1 - p1).cross(line.d) / d.cross(line.d);
        }
        Vec2 projected(const Vec2& p) const {
            return Sign(d.lengthp()) ? p1 + d * ((p - p1) * d) / d.lengthp() : p1;
        }
        Scalar lineDistance(const Vec2& p) const {
            return (p - projected(p)).length();
        }
        Scalar lineDistance(const Line& l) const {
            return lineIntersects(l) ? 0 : lineDistance(l.p1);
        }
        Scalar segDistance(const Vec2& p) const {
            Scalar t = (p - p1) * d / d.lengthp();
            if (t <= 0)
                return (p - p1).length();
            else if (t >= 1)
                return (p - p2).length();
            else
                return (p1 + d * t - p).length();
        }
        Scalar segDistance(const Line& l) const {
            return segIntersects(l) ? 0
                                    : min({segDistance(l.p1), segDistance(l.p2), l.segDistance(p1),
                                           l.segDistance(p2)});
        }
    };

    struct Polygon {
        vector<Vec2> points;
        size_t n;
        // points must be counter-clockwise
        Polygon(const vector<Vec2>& points) : points(points), n(points.size()) {}
        const Vec2& vertex(size_t idx) const {
            return points[idx % n];
        }
        Vec2& vertex(size_t idx) {
            return points[idx % n];
        }
        Scalar Area() const {
            Scalar area = 0;
            for (size_t i = 0; i < n; i++) {
                area += vertex(i).cross(vertex(i + 1));
            }
            return area * Sign(area) / 2;
        }
        bool IsConvex() const {
            for (size_t i = 0; i < n; i++)
                if ((vertex(i) - vertex(i + 1)).cross(vertex(i + 1) - vertex(i + 2)) < 0)
                    return false;
            return true;
        }
        // 点が多角形に含まれるか、辺上にある
        bool Contains(const Vec2& p) {
            Scalar acc = 0;
            for (size_t i = 0; i < n; i++) {
                Vec2 a = vertex(i) - p;
                Vec2 b = vertex(i + 1) - p;
                if (Sign(a.cross(b)) == 0 and (a * b) <= EPS)
                    return true;
                acc += atan2(a.cross(b), a * b);
            }
            return abs(acc) > acosl(-1L);
        }
        // 点が多角形の辺上にある
        bool OnBoundary(const Vec2& p) const {
            for (size_t i = 0; i < n; i++) {
                if (Line(vertex(i), vertex(i + 1)).onSegment(p))
                    return true;
            }
            return false;
        }
        Scalar Diameter() const {
            if (n == 2)
                return (points[0] - points[1]).length();
            Scalar res = 0;
            size_t i = 0, j = 1;
            while (i <= n) {
                res = max(res, (vertex(i) - vertex(j)).length());
                if (Sign((vertex(i + 1) - vertex(i)).cross(vertex(j + 1) - vertex(j))) > 0)
                    j++;
                else
                    i++;
            }
            return res;
        }
        pair<Polygon, Polygon> ConvexCut(const Line& line) const {
            vector<Vec2> left, right;
            for (size_t i = 0; i < n; i++) {
                const Vec2& a = vertex(i);
                const Vec2& b = vertex(i + 1);
                int sa = Sign(line.d.cross(a - line.p1));
                int sb = Sign(line.d.cross(b - line.p1));
                if (sa >= 0)
                    left.push_back(a);
                if (sa <= 0)
                    right.push_back(a);
                if (sa * sb < 0)
                    left.push_back(line.intersectsAt(Line(a, b))),
                        right.push_back(line.intersectsAt(Line(a, b)));
            }
            return {Polygon(left), Polygon(right)};
        }
    };

    // 下から反時計回り
    inline Polygon ConvexHull(vector<Vec2> points) {
        if (points.size() < 3)
            return Polygon{{}};
        sort(points.begin(), points.end(), [](Vec2 a, Vec2 b) {
            if (Sign(a.y - b.y))
                return Sign(a.y - b.y) < 0;
            return Sign(a.x - b.x) < 0;
        });
        points.erase(unique(ALL(points)), points.end());
        auto half = [](auto begin, auto end) {
            vector<Vec2> p(begin, begin + 2);
            for (auto itr = begin + 2; itr != end; itr++) {
                while (p.size() >= 2 and
                       Sign((*itr - p.back()).cross(p[p.size() - 2] - p.back())) < 0)
                    p.pop_back();
                p.push_back(*itr);
            }
            p.pop_back();
            return p;
        };
        auto lower = half(points.begin(), points.end()),
             upper = half(points.rbegin(), points.rend());
        lower.insert(lower.end(), upper.begin(), upper.end());
        return Polygon{lower};
    }
} // namespace Geometry2D
