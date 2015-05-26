
#ifndef KALI_PLATFORM_INCLUDED
#define KALI_PLATFORM_INCLUDED

// ............................................................................

#define MACOSX_      1 // actually GCC

#define API_EXPORT __attribute__ ((visibility("default")))
#define FUNCTION_  __FUNCTION__
#define FUNCSIG_   __PRETTY_FUNCTION__

#define inline_    __attribute__ ((always_inline))
#define restrict_  __restrict

#define format__   __attribute__ ((format(printf, 2, 3)))

// ............................................................................

#endif // ~ KALI_PLATFORM_INCLUDED
