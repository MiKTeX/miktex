// -*- related-file-name: "../../liblcdf/point.cc" -*-
#ifndef LCDF_POINT_HH
#define LCDF_POINT_HH
#include <math.h>

struct Point {

    double x;
    double y;

    Point()				{ }
    Point(double xx, double yy)		: x(xx), y(yy) { }
    // Point(const Point &)		use compiler default
    Point(const Point &p, double dx, double dy) : x(p.x + dx), y(p.y + dy) { }
    // ~Point()				use compiler default

    inline double squared_length() const noexcept;
    inline double length() const noexcept;
    inline double magnitude() const noexcept;
    static inline double distance(const Point &, const Point &) noexcept;
    static inline double dot(const Point &, const Point &) noexcept;
    static Point midpoint(const Point &, const Point &) noexcept;

    inline double angle() const noexcept;

    void shift(double dx, double dy)	{ x += dx; y += dy; }

    inline Point shifted(double dx, double dy) const noexcept;
    Point rotated(double) const noexcept;
    inline Point normal() const noexcept;

    bool on_line(const Point &, const Point &, double) const noexcept;
    bool on_segment(const Point &, const Point &, double) const noexcept;

    inline Point &operator+=(const Point &) noexcept;
    inline Point &operator-=(const Point &) noexcept;
    inline Point &operator*=(double) noexcept;
    inline Point &operator/=(double) noexcept;

    // Point operator+(Point, const Point &);
    // Point operator-(Point, const Point &);
    // Point operator*(Point, double);
    // Point operator/(Point, double);
    // Point operator-(const Point &);

    // bool operator==(const Point &, const Point &);
    // bool operator!=(const Point &, const Point &);

};

inline double
Point::squared_length() const noexcept
{
    return x*x + y*y;
}

inline double
Point::length() const noexcept
{
    return sqrt(x*x + y*y);
}

inline double
Point::magnitude() const noexcept
{
    return length();
}

inline double
Point::angle() const noexcept
{
    return atan2(y, x);
}

inline Point
Point::shifted(double dx, double dy) const noexcept
{
    return Point(x + dx, y + dy);
}

inline Point
Point::normal() const noexcept
{
    double l = length();
    return (l ? Point(x/l, y/l) : *this);
}

inline Point &
Point::operator+=(const Point &p) noexcept
{
    x += p.x;
    y += p.y;
    return *this;
}

inline Point &
Point::operator-=(const Point &p) noexcept
{
    x -= p.x;
    y -= p.y;
    return *this;
}

inline Point &
Point::operator*=(double d) noexcept
{
    x *= d;
    y *= d;
    return *this;
}

inline Point &
Point::operator/=(double d) noexcept
{
    x /= d;
    y /= d;
    return *this;
}

inline bool
operator==(const Point &a, const Point &b) noexcept
{
    return a.x == b.x && a.y == b.y;
}

inline bool
operator!=(const Point &a, const Point &b) noexcept
{
    return a.x != b.x || a.y != b.y;
}

inline Point
operator+(Point a, const Point &b) noexcept
{
    a += b;
    return a;
}

inline Point
operator-(Point a, const Point &b) noexcept
{
    a -= b;
    return a;
}

inline Point
operator-(const Point &a) noexcept
{
    return Point(-a.x, -a.y);
}

inline Point
operator*(Point a, double scale) noexcept
{
    a *= scale;
    return a;
}

inline Point
operator*(double scale, Point a) noexcept
{
    a *= scale;
    return a;
}

inline Point
operator/(Point a, double scale) noexcept
{
    a /= scale;
    return a;
}

inline double
Point::distance(const Point &a, const Point &b) noexcept
{
    return (a - b).length();
}

inline double
Point::dot(const Point &a, const Point &b) noexcept
{
    return a.x*b.x + a.y*b.y;
}

#endif
