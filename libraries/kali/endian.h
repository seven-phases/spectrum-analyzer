
#ifndef KALI_ENDIAN_INCLUDED
#define KALI_ENDIAN_INCLUDED

// ............................................................................

namespace endian_ {

// ............................................................................

namespace swap {

template <typename T>
T swap(const unsigned char* src)
{
    T dst = 0;
    int i = sizeof(dst);
    while (--i >= 0)
        dst |= *src++ << (i << 3);
    return dst;
}

template <typename T>
void swap(unsigned char* dst, T src)
{
    int i = sizeof(src);
    while (--i >= 0)
        *dst++ = (unsigned char)
            ((src >> (i << 3)) & 0xff);
}

} // ~ namespace swap

// ............................................................................

namespace copy {

template <typename T>
int swap(const unsigned char* src)
{
    T dst;
    memcpy(&dst, src, sizeof(dst));
    return dst;
}

template <typename T>
void swap(unsigned char* dst, T src)
{
    memcpy(dst, &src, sizeof(src));
}

} // ~ namespace copy

// ............................................................................

#if defined (__LITTLE_ENDIAN__)
    #define HOST_IS_LITTLE_ENDIAN 1
#elif defined (__BIG_ENDIAN__)
    #define HOST_IS_LITTLE_ENDIAN 0
#else
    #if defined (_M_IX86)
        #define HOST_IS_LITTLE_ENDIAN 1
    #else
        #error Unknown machine endianness.
    #endif
#endif

// ............................................................................

namespace swap_if_host_is_little
{
    #if HOST_IS_LITTLE_ENDIAN
        namespace endian = endian_::swap;
    #else
        namespace endian = endian_::copy;
    #endif
}

namespace swap_if_host_is_big
{
    #if HOST_IS_LITTLE_ENDIAN
        namespace endian = endian_::copy;
    #else
        namespace endian = endian_::swap;
    #endif
}

namespace swap_never
{
    namespace endian = endian_::copy;
}

// ............................................................................

} // ~ namespace endian_

// ............................................................................

#endif // ~ KALI_ENDIAN_INCLUDED
