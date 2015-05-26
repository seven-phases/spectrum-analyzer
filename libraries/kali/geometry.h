
#ifndef KALI_GEOMETRY_INCLUDED
#define KALI_GEOMETRY_INCLUDED

#include "kali/runtime.h"

// ............................................................................

namespace kali     {
namespace geometry {

// ............................................................................

template <typename T, int Unique = 0>
struct Size
{
    T w, h;
    template <typename V, int U>
    explicit Size(const Size<V, U>& s) : w(T(s.w)), h(T(s.h)) {}
    Size(T w, T h) : w(w), h(h) {}
    Size() : w(0), h(0) {}

    T    width()  const {return w;}
    T    height() const {return h;}
    bool empty()  const {return !((w > 0) && (h > 0));}
};

// ............................................................................

template <typename T, int Unique = 0>
struct Point
{
    T x, y;
    template <typename V, int U>
    explicit Point(const Point<V, U>& p) : x(T(p.x)), y(T(p.y)) {}
    Point(T x, T y) : x(x), y(y) {}
    Point() : x(0), y(0) {}

    T left() const {return x;}
    T top()  const {return y;}

    Point& offset(const Point& p)
    {
        x += p.x;
        y += p.y;
        return *this;
    }
};

// ............................................................................

template <template <typename, int> class T, typename V, int U>
T<V, U> offset(const T<V, U>& a, const Point<V, U>& b)
{
    T<V, U> r(a);
    r.x += b.x;
    r.y += b.y;
    return r;
}

template <template <typename, int> class T, typename V, int U>
T<V, U> expand(const T<V, U>& a, const Size<V, U>& b)
{
    T<V, U> r(a);
    r.x -= b.w;
    r.y -= b.h;
    r.w += b.w * 2;
    r.h += b.h * 2;
    return r;
}

template <typename T>
T unite(const T& a, const T& b)
{
    T r(min(a.x, b.x), min(a.y, b.y));
    r.w = max(a.right(),  b.right())  - r.x;
    r.h = max(a.bottom(), b.bottom()) - r.y;
    return r;
}

template <typename T>
T intersect(const T& a, const T& b)
{
    T r(max(a.x, b.x), max(a.y, b.y));
    r.w = min(a.right(),  b.right())  - r.x;
    r.h = min(a.bottom(), b.bottom()) - r.y;
    return r;
}

template <typename T>
bool equal(const T& a, const T& b)
{
    return a.x == b.x
        && a.y == b.y
        && a.w == b.w
        && a.h == b.h;
}

template <typename T>
bool intersects(const T& a, const T& b)
{
    return a.x < b.right()
        && a.y < b.bottom()
        && a.right()  > b.x
        && a.bottom() > b.y;
}

template <template <typename, int> class T, typename V, int U> T<V, U>
operator +  (const T<V, U>& a, const Point<V, U>& b) {return offset(a, b);}
template <typename T> T
operator |  (const T& a, const T& b) {return unite(a, b);}
template <typename T> T
operator &  (const T& a, const T& b) {return intersect(a, b);}

// ............................................................................

template <typename T, int Unique = 0>
struct Rect
{
    T       x, y, w, h;
    typedef Point <T, Unique> Point;
    typedef Size  <T, Unique> Size;

    template <typename V, int U>
    explicit Rect(const Rect<V, U>& r) : x(T(r.x)), y(T(r.y)), w(T(r.w)), h(T(r.h)) {}
    explicit Rect(const Point& p, const Size& s = Size()) : x(p.x), y(p.y), w(s.w), h(s.h) {}
    explicit Rect(const Size& s) : x(0), y(0), w(s.w), h(s.h) {}
    Rect(T x, T y, T w = 0, T h = 0) : x(x), y(y), w(w), h(h) {}
    Rect() : x(0), y(0), w(0), h(0) {}

    T     left()   const {return x;}
    T     top()    const {return y;}
    T     width()  const {return w;}
    T     height() const {return h;}
    T     right()  const {return x + w;}
    T     bottom() const {return y + h;}
    Point point()  const {return Point(x, y);}
    Size  size()   const {return Size(w, h);}
    bool  empty()  const {return !((w > 0) && (h > 0));}

    void point(int x_, int y_) {x = x_, y = y_;}
    void point(const Point& p) {point(p.x, p.y);}
    void size(int w_, int h_)  {w = w_, y = h_;}
    void size(const Size& s)   {size(s.w, s.h);}

    bool contains(const Point& p) const {return contains(p.x, p.y);}

    bool contains(T x_, T y_) const
    {
        return x_ >= x
            && x_ < right()
            && y_ >= y
            && y_ < bottom();
    }

    bool  equal(const Rect& r)      const {return geometry::equal(*this, r);}
    bool  intersects(const Rect& r) const {return geometry::intersects(*this, r);}
    Rect& unite(const Rect& r)            {return *this = geometry::unite(*this, r);}
    Rect& offset(const Point& p)          {return *this = geometry::offset(*this, p);}
    Rect& expand(const Size& s)           {return *this = geometry::expand(*this, s);}

    bool  operator == (const Rect& r) const {return  equal(r);}
    bool  operator != (const Rect& r) const {return !equal(r);}
    Rect& operator |= (const Rect& r)       {return  unite(r);}
    Rect& operator += (const Point& p)      {return  offset(p);}
};

// ............................................................................

} // ~ namespace geometry

// ............................................................................

typedef geometry::Point <int>   Point;
typedef geometry::Size  <int>   Size;
typedef geometry::Rect  <int>   Rect;
typedef geometry::Point <float> PointF;
typedef geometry::Size  <float> SizeF;
typedef geometry::Rect  <float> RectF;

typedef geometry::Rect  <int,    1> Margin;
typedef geometry::Point <double, 2> Scale;

// ............................................................................

} // ~ namespace kali

// ............................................................................

#endif // KALI_GEOMETRY_INCLUDED
