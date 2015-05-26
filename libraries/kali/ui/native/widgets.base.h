
#ifndef KALI_UI_NATIVE_WIDGETS_BASE_INCLUDED
#define KALI_UI_NATIVE_WIDGETS_BASE_INCLUDED

#include "kali/function.h"
#include "kali/containers.h"
#include "kali/ui/base.h"

// ............................................................................

namespace kali   {
namespace ui     {
namespace native {
namespace widget {

// ............................................................................

struct Parent
{
    virtual Window* window()         = 0;
    virtual void attach(ReleaseAny*) = 0;
    virtual ~Parent() {}
};

struct Interface : ui::widget::Interface, UsesCallback
{
    virtual int  range() const = 0;
    virtual void range(int v)  = 0;

    virtual string text() const    = 0;
    virtual void text(const char*) = 0;

    virtual Window::Handle expose() const = 0;

    Callback extCallback; // FIXME, todo
};

// ............................................................................

} // ~ namespace widget
} // ~ namespace native
} // ~ namespace ui
} // ~ namespace kali

// ............................................................................

#endif // ~ KALI_UI_NATIVE_WIDGETS_BASE_INCLUDED
