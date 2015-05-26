
#ifndef KALI_GL_EXTENSIONS_INCLUDED
#define KALI_GL_EXTENSIONS_INCLUDED

#ifndef PREPROCESS_ONLY

#include <windows.h>
#include <stddef.h>
#include <gl/gl.h>

#define   GLAPI                 typedef
#define   GL_GLEXT_PROTOTYPES   1

// ISSUE: all type definitions in glext.h go into glext_:: scope
// that WORKS FINE but may look misleading (in compilation errors for example)
// dirty workaround: remove normal namespace start below and use this instead:
// #define __int32 __int32 int32_t; namespace glext_ {const int dummy = sizeof

namespace glext_
{
    #include <gl/glext.h>
}

#include "kali/types.h"
#include "kali/app.h"

#endif // ~ PREPROCESS_ONLY

#ifndef EXT_MAP
#define EXT_MAP "extensions.map.h"
#endif

// ............................................................................

namespace gl
{
    namespace ext {struct Handler;}
    const kali::singleton <ext::Handler> extensions;
}

// ............................................................................
// extensions enumeration:

#define  E(name) name ,
#define  F(name)
#define  I(name)

enum glExtensionsEnum
{
    #include EXT_MAP
    glExtensionsEnumCount
};

// ............................................................................
// function pointers:

#undef   E
#define  E(name)
#undef   F
#define  F(name) glext_::name* name = 0;
#include EXT_MAP

// ............................................................................

namespace gl  {
namespace ext {

// ............................................................................
// extension initialization routines:

template <enum glExtensionsEnum>
inline bool initExtensionFunctions_();

template <typename T>
inline T* initFunctionPointer_(T*& ptr, const char* name)
{
    if (ptr)
        return ptr;

    #pragma warning(push)
    #pragma warning(disable: 4191)
    ptr = (T*) wglGetProcAddress(name);
    #pragma warning(pop)

    if (!ptr)
        trace("%s(\"%s\"): wglGetProcAddress failed [%i]\n",
            FUNCTION_, name, ::GetLastError());

    return ptr;
}

#undef   F
#define  F(name) && initFunctionPointer_(name, #name)
#undef   I
#define  I(name) && initExtensionFunctions_<name>()
#undef   E
#define  E(name) ;} template <> inline bool \
    initExtensionFunctions_<name>() { return true

namespace // dummy namespace just to balance brackets
{
    #include EXT_MAP
;}

// ............................................................................
// extension initialization map:

struct Tag
{
    const char* name;
    bool (*initializer)();
};

#undef   F
#define  F(name)
#undef   I
#define  I(name)
#undef   E
#define  E(name) #name " ", initExtensionFunctions_<name> ,

const Tag Map[glExtensionsEnumCount + 1] =
{
    #include EXT_MAP
    0, 0
};

// ............................................................................
// clean up:

#undef   E
#undef   F
#undef   FA

// ............................................................................

struct Handler : kali::ReleaseAny
{
    static void initialize(bool all = true)
    {
        if (gl::extensions)
            return trace("%s: already initialized\n", FUNCTION_);

        kali::app->autorelease(gl::extensions.alloc());
        if (all)
            gl::extensions->initializeAll();
    }

    bool isSupported(glExtensionsEnum index)
    {
        return available[index];
    }

private:

    bool initializeExtension(int i, bool trace_)
    {
        const Tag& e = Map[i];
        if (strstr(string, e.name))
            return available[i] = e.initializer();

        if (trace_)
            trace("%s: %sis not supported\n", FUNCTION_, e.name);
        return available[i] = false;
    }

    bool initializeAll()
    {
        bool result = true;
        for (int i = 0; i < count; i++)
            result &= initializeExtension(i, result);
        return result;
    }

    Handler() : string((const char*) glGetString(GL_EXTENSIONS))
    {
        for (int i = 0; i < count; i++)
            available[i] = false;
    }

private:
    friend kali::singleton <Handler>;
    enum {count = glExtensionsEnumCount};
    const char* string;
    bool available[count];
};

// ............................................................................

} // ~ namespace ext
} // ~ namespace gl

// ............................................................................

#endif // ~ KALI_GL_EXTENSIONS_INCLUDED
