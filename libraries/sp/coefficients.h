
#ifndef SP_COEFFICIENTS_INCLUDED
#define SP_COEFFICIENTS_INCLUDED

#include <math.h>

// ............................................................................

namespace sp {

// ............................................................................

template <typename T>
void twoPoleLPCoeffs(const T& coeffs, double fs, double f, double q, double prewrap)
{
    double w = 2 * pi * f / fs;
    double y = sin(w) / (q * (1 + cos(w * prewrap)));

    typedef T::Type V;
    double a  = 1 / (1 + y);
    coeffs[2] = V(a * (y - 1));
    coeffs[1] = V(a * 2 * cos(w));
    coeffs[0] = V((.5 / q) * (1 - coeffs[1] - coeffs[2]));
}

// ............................................................................

} // ~ namespace sp

// ............................................................................

#endif // ~ SP_COEFFICIENTS_INCLUDED
