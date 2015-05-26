
#ifndef KALI_GRAPHICS_INCLUDED
#define KALI_GRAPHICS_INCLUDED

// ............................................................................

#include "kali/platform.h"

#if      WINDOWS_
#include "kali/graphics.gdiplus.h"
#elif    MACOSX_
#include "kali/graphics.cocoa.h"
#else
#error   not implemented.
#endif

// ............................................................................

#endif // ~ KALI_GRAPHICS_INCLUDED
