
#ifndef KALI_PLATFORM_INCLUDED
#define KALI_PLATFORM_INCLUDED

// ............................................................................

#define WINDOWS_     1 // actually means MSVC

#define API_EXPORT __declspec(dllexport)
#define FUNCTION_  __FUNCTION__
#define FUNCSIG_   __FUNCSIG__

#define inline_    __forceinline
#define restrict_  __restrict
#define noalias_   __declspec(noalias)

#define align_(x)  __declspec(align(x))

#define format__
#define vsnprintf   _vsnprintf

// ............................................................................

#endif // ~ KALI_PLATFORM_INCLUDED
