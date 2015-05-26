
#ifndef KALI_UI_NATIVE_DETAILS_INCLUDED
#define KALI_UI_NATIVE_DETAILS_INCLUDED

#include <windows.h>
#include <commctrl.h>
#include "kali/dbgutils.h"

// ............................................................................

namespace kali    {
namespace ui      {
namespace native  {
namespace details {

// ............................................................................

inline Window::Handle cloneWindow
    (Window::Handle window, int styleRemove = 0, int styleAdd = 0, bool destroy = false)
{
    string class_, text;
    ::GetClassName(window, class_(), class_.size);
    ::GetWindowText(window, text(), text.size);
    int style = ::GetWindowLong(window, GWL_STYLE);
    style = (style & ~styleRemove) | styleAdd;
    int styleEx = ::GetWindowLong(window, GWL_EXSTYLE);

    RECT r;
    Window::Handle parent = ::GetParent(window);
    ::GetWindowRect(window, &r);
    ::MapWindowPoints(0, parent, (POINT*) &r, 2);

    // workaround for a Combo to get exact clone:
    if (style & CBS_DROPDOWNLIST)
    {
        RECT rr;
        ::SendMessage(window, CB_GETDROPPEDCONTROLRECT, 0, (LPARAM) &rr);
        r.bottom = r.top - rr.top + rr.bottom;
        style |= ~styleRemove & WS_VSCROLL;
    }

    Window::Handle handle = CreateWindowEx
        (styleEx, class_, text, style,
        r.left, r.top, r.right - r.left,
        r.bottom - r.top, parent, 0, app->module(), 0);

    if (handle)
    {
        ::SendMessage(handle, WM_SETFONT,
            ::SendMessage(window, WM_GETFONT, 0, 0), 0);

        if (destroy)
            ::DestroyWindow(window);

        return handle;
    }

    trace("%s: CreateWindow failed [%i]\n", FUNCTION_, ::GetLastError());
    return 0;
}

// ............................................................................
// temporary here:

template <typename F>
struct DynamicApi
{
    operator F*()   const {return func;}
    operator bool() const {return !!func;}

    DynamicApi(const char* lib, const char* f) {ctor(lib, f);}
    ~DynamicApi() {::FreeLibrary(module);}

private:
    HMODULE module;
    F*      func;

    DynamicApi(const DynamicApi&);
    DynamicApi& operator = (const DynamicApi&);
    template <typename T> operator T () const;

    void ctor(const char* lib, const char* f)
    {
        func   = 0;
        module = ::LoadLibrary(lib);
        if (!module)
            return trace("%s: LoadLibrary(%s) failed [%i]\n",
                FUNCTION_, lib, ::GetLastError());

        #pragma warning(push)
        #pragma warning(disable: 4191)
        func = (F*) ::GetProcAddress(module, f);
        if (!func)
            trace("%s: GetProcAddress(%s) failed [%i]\n",
                FUNCTION_, f, ::GetLastError());
        #pragma warning(pop)
    }
};

// ............................................................................

} // ~ namespace details
} // ~ namespace native
} // ~ namespace ui
} // ~ namespace kali

// ............................................................................

#endif // ~ KALI_UI_NATIVE_WIDGETS_INCLUDED
