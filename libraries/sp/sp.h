
#ifndef SP_INCLUDED
#define SP_INCLUDED

#include "kali/platform.h"

#include "sp/base.h"
#include "sp/core.h"
#include "sp/coefficients.h"
#include "sp/more.h"

// ............................................................................
// for now assuming SSE::ps *only*:

inline m128 operator + (const m128& a, const m128& x) {return _mm_add_ps(a, x);}
inline m128 operator * (const m128& x, const m128& y) {return _mm_mul_ps(x, y);}
inline m128         max(const m128& x, const m128& y) {return _mm_max_ps(x, y);}

template <int a, int b, int c, int d>
inline m128 shuffle(const m128& x, const m128& y)
{
    return _mm_shuffle_ps(x, y, _MM_SHUFFLE(d, c, b, a));
}

inline m128 hsum(const m128& x)
{
    m128 r = _mm_add_ps(x, _mm_movehl_ps(x, x));
    return _mm_add_ss(r, _mm_shuffle_ps(r, r, 1));
}

// ............................................................................

namespace sp {

// ............................................................................

struct TwoPoleLP
{
    enum
    {
        State = 2,
        Coeff = 3
    };

    static inline_ m128 tick(float in, m4f (&z)[State], const m4f (&k)[Coeff])
    {
        m128 out = _mm_set_ps1(in) * k[0]
             + z[0] * k[1]
             + z[1] * k[2];
        z[1] = z[0];
        z[0] = out;
        return out;
    }
};

struct TwoPoleLPSAx : TwoPoleLP // useful only for SA due to delayed output
{
    static inline_ m128 tick(float in, m4f (&z)[State], const m4f (&k)[Coeff])
    {
        m128 out = z[0];
        z[0] = _mm_set_ps1(in) * k[0]
             + z[0] * k[1]
             + z[1] * k[2];
        z[1] = out;
        return out;
    }
};

// ............................................................................

struct ZeroLP
{
    enum {State = 1};

    template <typename T> static inline_
    T tick(T in, T (&z)[State])
    {
        T out = in + z[0];
        z[0]  = in;
        return out;
    }
};

// ............................................................................

} // ~ namespace sp

// ............................................................................

#endif // ~ SP_INCLUDED
