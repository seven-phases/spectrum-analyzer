
#ifndef INCLUDES_INCLUDED
#define INCLUDES_INCLUDED

#include "kali/platform.h"

// ............................................................................

#if MACOSX_

#import <Cocoa/Cocoa.h>

#endif

// ............................................................................

#if WINDOWS_

#pragma warning(push, 3)

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x501
#endif

#define ISOLATION_AWARE_ENABLED 1

#if 0
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include <math.h>
#include <windows.h>
#include <commctrl.h>
#include <gdiplus.h>
#include <gl/gl.h>

#undef small

#pragma warning(pop)
#pragma warning(disable: 4127) // conditional expression is constant

// enable extra warnings:
#pragma warning(default: 4191) // unsafe conversion from 'type of expression' to 'type required'
#pragma warning(default: 4242) // (another) conversion from 'type1' to 'type2', possible loss of data
#pragma warning(default: 4254) // (another) conversion from 'type1' to 'type2', possible loss of data
#pragma warning(default: 4263) // member function does not override any base class virtual member function
#pragma warning(default: 4264) // no override available for virtual member function from base 'class'; function is hidden
#pragma warning(default: 4265) // class has virtual functions, but destructor is not virtual
#pragma warning(default: 4266) // no override available for virtual member function from base 'type'; function is hidden
#pragma warning(default: 4296) // expression is always false
#pragma warning(default: 4431) // missing type specifier - int assumed
#pragma warning(default: 4640) // construction of local static object is not thread-safe

#endif

// ............................................................................

#endif // INCLUDES_INCLUDED
