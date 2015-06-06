
#ifndef SP_CORE_INCLUDED
#define SP_CORE_INCLUDED

#include <math.h>
#include <xmmintrin.h>
#include "kali/platform.h"

typedef __m128 m128;

// ............................................................................

namespace sp {

// ............................................................................

struct Undefined;
struct SSE;

template <typename T, int N, typename Traits = Undefined>
struct array
{
    T value[N];

    enum     {size = N};
    typedef T Type;
    typedef T CppType[N];

    T  operator [] (int i) const {return value[i];}
    T& operator [] (int i)       {return value[i];}

    // operator       CppType& ()       {return value;}
    // operator const CppType& () const {return value;}
};

template <>
struct array <float, 4, SSE>
{
    typedef float T;
    static const int N = 4;

    T align_(16) value[N];

    enum     {size = N};
    typedef T Type;

    T  operator [] (int i) const {return value[i];}
    T& operator [] (int i)       {return value[i];}

    array& operator = (m128 v) {_mm_store_ps(value, v); return *this;}
    array& operator = (const array& v) {return *this = m128(v);}
    operator m128 () const {return _mm_load_ps(value);}
};

typedef array <float, 4, SSE> m4f;

// ............................................................................

template <typename T, typename U, U T::* value>
struct Iter // FIXME: name better
{
    T* a;
    typedef T Type;
    Iter(T* a) : a(a) {}
    U& operator [] (int i) {return a[i].*value;}
    const U& operator [] (int i) const {return a[i].*value;}
};

template <typename T, typename U>
struct IterA // FIXME: name better
{
    T*  a;
    int j;
    typedef U Type;
    IterA(T* a, int j) : a(a), j(j) {}
    U& operator [] (int i) const {return a[i][j];}
};

// ............................................................................

template <size_t A>
struct AlignedNew
{
    static void* operator new(size_t size) {return _aligned_malloc(size, A);}
    static void operator delete(void* ptr) {_aligned_free(ptr);}

protected:
    ~AlignedNew() {}
};

// ............................................................................

} // ~ namespace sp

// ............................................................................

#endif // ~ SP_CORE_INCLUDED
