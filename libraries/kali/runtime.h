
#ifndef KALI_RUNTIME_INCLUDED
#define KALI_RUNTIME_INCLUDED

#include "kali/platform.h"
#include "kali/dbgutils.h"

// ............................................................................

namespace kali {

// ............................................................................

#ifdef min
#undef min
#undef max
#endif

template <typename T> T min(T a, T b) {return a < b ? a : b;}
template <typename T> T max(T a, T b) {return a > b ? a : b;}

template <typename T>
void swap(T& a, T& b)
{
    T temp(a);
    a = b;
    b = temp;
}

template <typename T>
void copy(T* restrict_ dst, const T* restrict_ src, int size)
{
    while (--size >= 0)
        *dst++= *src++;
}

template <typename T>
void sort(T* data, int count)
{
    struct aux {
        static int compare(const void* a, const void* b)
            {return strcmp(((const T*) a)->name, ((const T*) b)->name);}};

    qsort(data, count, sizeof(T), aux::compare);
}

inline bool inlist(const char* const list[], const char* value)
{
    while (*list)
        if (!strcmp(*list++, value))
            return true;

    return false;
}

template <typename T>
T nextPowOf2(T v)
{
    v--;
    for (int i = 1; i < sizeof(T) * 8; i <<= 1)
        v = v | v >> i;
    return v + 1;
}

// ............................................................................

#if WINDOWS_ // fixme: MSVC actually

template <typename T>
const char* typeString()
{
    static char aux[] = FUNCSIG_;
    aux[sizeof(aux) - sizeof(">(void)")] = 0;
    const char* p = aux;
    while (*p++ != '<');
    return p;
}

#endif

#if MACOSX_ // fixme: GCC actually

template <typename T>
const char* typeString()
{
    static char aux[sizeof(FUNCSIG_)];
    memcpy(aux, FUNCSIG_, sizeof(aux));
    char* p = aux + sizeof(aux) - sizeof("]");
    *p = 0;
    while (*--p != '=');
    return p + 2;
}

#endif

template <typename T>
const char* typeString(const T&) {return typeString<T>();}

// ............................................................................

#if WINDOWS_ // fixme: MSVC actually

template <typename E, E N>
struct EnumNames
{
    const char* operator [] (int i) const {return names(i);}

    EnumNames() {ctor();}

private:

    enum {Size = N};

    static const char*& names(int i)
    {
        static const char* aux[Size] = {0};
        return aux[i];
    }

    static void ctor()
    {
        if (!names(0))
            _<E(0)>(sizeof(FUNCSIG_) + sizeof("_")
                - sizeof("ctor(void)"));
    }

    template <E I> inline_ static void _(int offset)
    {
        static char aux[] = FUNCSIG_;
        names(I) = aux + offset;
        _<E(I + 1)>(offset);
        aux[sizeof(aux) - sizeof(">(int)")] = 0;
    }

    template <> static void _ <Size> (int) {}
};

#endif

// ............................................................................

#if MACOSX_ // fixme: GCC actually

template <typename E, E N, E I> struct EnumNames_
{
    static void _(const char** dst, int offset)
    {
        static char aux[sizeof(FUNCSIG_)];
        memcpy(aux, FUNCSIG_, sizeof(aux));
        aux[sizeof(aux) - 2] = 0;
        dst[I] = aux + offset;
        EnumNames_<E, N, E(I + 1)>::_(dst, offset);
    }
};

template <typename E, E N> struct EnumNames_ <E, N, N>
{
    static void _(const char**, int) {}
};

template <typename E, E N>
struct EnumNames
{
    const char* operator [] (int i) const {return names(i);}

    EnumNames()
    {
        if (!names(0))
            __(&names(0), 0);
    }

private:

    static const char*& names(int i)
    {
        static const char* aux[N] = {0};
        return aux[i];
    }

    static void __(const char** dst, int)
    {
        const int offset = sizeof(FUNCSIG_)
            + sizeof(", E I" " I = ") - 2;
        EnumNames_<E, N, E(0)>::_(dst, offset);
    }
};

#endif

// ............................................................................

} // ~ namespace kali

// ............................................................................

#endif // KALI_RUNTIME_INCLUDED
