
#ifndef KALI_DBG_UTILS_INCLUDED
#define KALI_DBG_UTILS_INCLUDED

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "kali/platform.h"
#include "kali/details/noop.h"

// ............................................................................

#ifndef DBG
    #define DBG (defined(_DEBUG))
#endif

#ifdef WINDOWS_
    #ifndef OutputDebugString
    extern "C" __declspec(dllimport)
        void __stdcall OutputDebugStringA(const char*);
    #endif
    #if !DBG
        #define GetLastError NoOp
    #endif
#else
    #include <syslog.h>
#endif

#define DBGSTOP_ *((int*) 0) = 0

// ............................................................................
// trace

struct trace

#if !DBG

: NoOp
{
    NoOp
        warn,
        full,
        setLevel,
        setOutput;

#else // DBG

{
    #define out(L, F)   \
        va_list A;      \
        va_start(A, F); \
        out(L, F, A);   \
        va_end(A);

    void operator () (const char* format, ...) const format__ {out(Error, format);}
    void warn        (const char* format, ...) const format__ {out(Warning, format);}
    void full        (const char* format, ...) const format__ {out(Full, format);}

    #undef out

    static void out(int level, const char* format, va_list args)
    {
        if (level > options().level)
            return;

        char msg[512];
        int m = options().prefixSize;
        int n = m;
        const int size = sizeof(msg) - n - sizeof("!!");
        memcpy(msg, options().prefix, options().prefixSize);
        n += vsnprintf(msg + n, size, format, args);

        if (n == (m - 1))
            n += size;

        if (level < Warning)
            (msg[n - 1] == '\n')
                ? strcpy(msg + n - 1, "!!\n")
                    : strcpy(msg + n, "!!");

        switch (options().output)
        {
            case Std: fputs(msg, stdout); return;
            case Err: fputs(msg, stderr); return;
            default: sysout(msg); return;
        }
    }

    static void setPrefix(const char* prefix)
    {
        char* p = options().prefix;
        int   n = sizeof(options().prefix);
        while ((--n > 0) && *prefix)
            *p++ = *prefix++;
        *p = 0;
        options().prefixSize = int(p - options().prefix);
    }

    static void setLevel(int level)   {options().level  = level;}
    static void setOutput(int output) {options().output = output;}

    static void sysout(const char* msg)
    {
        #ifdef WINDOWS_
            OutputDebugStringA(msg);
        #else
            syslog(4, msg);
        #endif
    }

private:

    struct Options
    {
        int  level;
        int  output;
        char prefix[16];
        int  prefixSize;
    };

    static Options& options()
    {
        static Options aux
            = {Warning, Default, "", 0};
        return aux;
    }

#endif // ~ DBG

public:

    enum
    {
        None     = 0,
        Error,
        Warning,
        Full,
        Killing  = None
    };

    enum
    {
        Sys = 0,
        Std,
        Err,

        #if MACOSX_
        #ifdef TRACE_OUTPUT
            Default = TRACE_OUTPUT
        #else
            Default = Err
        #endif
        #else
            Default = Sys
        #endif
    };

    trace() {}

} const trace;

// ............................................................................

#endif // ~ KALI_DBG_UTILS_INCLUDED
