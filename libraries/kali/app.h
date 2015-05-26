
#ifndef KALI_APP_INCLUDED
#define KALI_APP_INCLUDED

#include "kali/containers.h"

// ............................................................................

namespace kali {

// ............................................................................

struct Window;

// temporary here:
#if WINDOWS_
typedef HINSTANCE__ Module;
#else
typedef void Module;
#endif

struct app
{
    template <typename T>
    static int run(bool ItCouldBeOnlyOne = 1);

    template <typename T>
    static bool createWindow(const Window* parent, T* window);

    template <typename T>
    static bool createLayer(const Window* parent, T* window);

    template <typename T>
    static bool loadLayer(const char* tag, const Window* parent, T* window);

    Module* module()     const;
    Window* mainWindow() const;
    void    useThreads();
    void    initGraphics();

    AutoReleasePool<> autorelease;

    struct AppDetails* details_;
};

const singleton <app> app;

// ............................................................................

}  // ~ namespace kali

#include "kali/app.details.h"

// ............................................................................

#endif // ~ KALI_APP_INCLUDED
