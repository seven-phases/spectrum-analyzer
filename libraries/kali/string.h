
#ifndef KALI_STRING_INCLUDED
#define KALI_STRING_INCLUDED

#include <string.h>
#include <stdarg.h>
#include "kali/platform.h"

// ............................................................................

namespace kali    {
namespace details {

// ............................................................................

template <unsigned Size>
struct String
{
    String(const char* src) {copy(data, src, size);}

    explicit String(const char* format, ...) format__
	{
        if (format)
        {
		    va_list arglist;
		    va_start(arglist, format);
		    vsnprintf(data, size, format, arglist);
            va_end(arglist);
            data[size - 1] = 0;
        }
        else
            *data = 0;
	}

    String() {*data = 0;}
    char* operator () () {return data;}
    operator const char* () const {return data;}
    const char* operator () () const {return data;}
    enum {size = Size};

private:
    char data[size];
    template <typename T> String(T);
    template <typename T> operator T () const;

    // ........................................................................
    // extra helpers:

public:

    const char* append(const char* src)
    {
        // meet Schlemiel
        size_t n = strlen(data);
        copy(data + n, src, int(size - n));
        return data;
    }

    template <int n>
    static wchar_t* a2w(wchar_t (&dst)[n], const char* src)   {return copy(dst, src, n);}
    static wchar_t* a2w(wchar_t* dst, const char* src, int n) {return copy(dst, src, n);}

    template <int n>
    static char* w2a(char (&dst)[n], const wchar_t* src)      {return copy(dst, src, n);}
    static char* w2a(char* dst, const wchar_t* src, int n)    {return copy(dst, src, n);}


    #if MACOSX_

    static NSString* ns(const char* src)
    {
        return !src ? nil :
            [NSString stringWithCString:src
                encoding:NSMacOSRomanStringEncoding];
    }

    #endif

private:

    template <typename A, typename B>
    static A* copy(A* restrict_ dst, const B* restrict_ src, int n)
    {
        A* p = dst;
        while ((--n > 0) && *src)
            *p++ = A(*src++);
        *p = 0;
        return dst;
    }

    // ........................................................................

}; // ~ struct String

// ............................................................................

} // ~ namespace details

// ............................................................................

typedef details::String <256> string;

// ............................................................................

} // ~ namespace kali

// ............................................................................

#endif // ~ KALI_STRING_INCLUDED
