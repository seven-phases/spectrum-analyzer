
#ifndef SP_BASE_INCLUDED
#define SP_BASE_INCLUDED

#include <math.h>

// ............................................................................

namespace sp {

// ............................................................................

const double pi     = 3.14159265358979323846;
const double ln2    = 0.69314718055994530942; // log(2)
const double ln10   = 2.30258509299404568402; // log(10)
const double sqrt_5 = 0.70710678118654752440; // sqrt(.5)

const double adn    = 1e-10;                  // anti-denormal, -200dB

// ............................................................................

inline double g2dB(double v) {return (20 / ln10) * log(v);}
inline double dB2g(double v) {return exp((ln10 / 20) * v);}

// inline double sinh(double v) {return .5 * (exp(v) - 1 / exp(v));}

// ............................................................................

template <typename T>
T set(T& a, T b)
{
    T ret(a);
    a = b;
    return ret;
}

// ............................................................................

} // ~ namespace sp

// ............................................................................

#endif // ~ SP_BASE_INCLUDED
