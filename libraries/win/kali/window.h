
#ifndef KALI_WINDOW_INCLUDED
#define KALI_WINDOW_INCLUDED

#include <windows.h>
#include "kali/geometry.h"
#include "kali/string.h"
#include "kali/dbgutils.h"

#ifndef KALI_APP_INCLUDED
#include "kali/app.mini.h"
#endif

// ............................................................................

namespace kali {

// ............................................................................

struct Window
{
    typedef HWND Handle;
    Handle handle;

    Window() : handle(0) {}
    explicit Window(Handle handle) : handle(handle) {}

    // ........................................................................

    void size(const Size& s)
    {
        size(s.w, s.h);
    }

    void size(int w, int h)
    {
        WINDOWPLACEMENT wp = {sizeof(wp)};
        ::GetWindowPlacement(handle, &wp);
        const Rect& m  = metrics();
        RECT& r        = wp.rcNormalPosition;
        r.right        = r.left + w + m.w;
        r.bottom       = r.top  + h + m.h;
        /*if (!::IsWindowVisible(handle))
            wp.showCmd = SW_HIDE;*/
        ::SetWindowPlacement(handle, &wp);
    }

    Size size() const
    {
        if (::GetWindowLong(handle, GWL_STYLE) & WS_CHILD)
        {
            RECT r;
            // note: not ClientRect as it may have border!
            ::GetWindowRect(handle, &r);
            return Size(r.right - r.left,
                        r.bottom - r.top);
        }

        WINDOWPLACEMENT wp = {sizeof(wp)};
        ::GetWindowPlacement(handle, &wp);
        RECT& r = wp.rcNormalPosition;
        if (wp.showCmd == SW_MAXIMIZE)
            ::GetWindowRect(handle, &r);
        const Rect& m = metrics();
        return Size(r.right  - r.left - m.w,
                    r.bottom - r.top  - m.h);
    }

    Size screenSize() const
    {
        RECT r;
        ::GetClientRect(::GetDesktopWindow(), &r);
        return Size(r.right, r.bottom);
    }

    void position(const Point& p)
    {
        position(p.x, p.y);
    }

    void position(int x, int y)
    {
        WINDOWPLACEMENT wp = {sizeof(wp)};
        ::GetWindowPlacement(handle, &wp);
        RECT& r   = wp.rcNormalPosition;
        r.right  += x - r.left;
        r.bottom += y - r.top;
        r.left    = x;
        r.top     = y;
        /*if (!::IsWindowVisible(handle))
            wp.showCmd = SW_HIDE;*/
        ::SetWindowPlacement(handle, &wp);
    }

    Point position() const
    {
        WINDOWPLACEMENT wp = {sizeof(wp)};
        ::GetWindowPlacement(handle, &wp);
        const RECT& r = wp.rcNormalPosition;
        return Point(r.left, r.top);
    }

    void redraw(const Rect& r)
    {
        const RECT rr = {r.x, r.y, r.x + r.w, r.y + r.h};
        ::InvalidateRect(handle, &rr, 0);
    }

    void redraw()
    {
        ::InvalidateRect(handle, 0, 0);
    }

    void update()
    {
        ::UpdateWindow(handle);
    }

    void lockUpdate(bool lock)
    {
        if (!::LockWindowUpdate(lock ? handle : 0))
            trace.warn("%s: LockWindowUpdate failed\n", FUNCTION_);
    }

    void minimize()
    {
        update();
        ::ShowWindow(handle, SW_MINIMIZE);
    }

    void destroy()
    {
        update();
        ::DestroyWindow(handle);
    }

    void title(const char* text)
    {
        ::SetWindowText(handle, text);
    }

    string title() const
    {
        string text;
        ::GetWindowText(handle, text(), text.size);
        return text;
    }

    void icon(const char* name)
    {
        if (name && *name)
            for (int i = 0; i < 2; i++)
                ::SendMessage(handle, WM_SETICON, i, (LPARAM)
                    ::LoadImage(app->module(), (LPSTR) name, IMAGE_ICON,
                    ::GetSystemMetrics(i ? SM_CXICON : SM_CXSMSIZE),
                    ::GetSystemMetrics(i ? SM_CYICON : SM_CYSMSIZE), LR_SHARED));
    }

    void clearInput()
    {
        MSG msg;
        if (::GetInputState())
            while (::PeekMessage(&msg, handle,
                0, 0, PM_QS_INPUT | PM_REMOVE));
    }

    void roundCorners(int n, Size s = Size())
    {
        if (s.empty())
            s = size();
        ::SetWindowRgn(handle, ::CreateRoundRectRgn
            (0, 0, s.w + 1, s.h + 1, n, n), 0);
    }

    bool alert(const char* title, const char* text, const char* comments = 0) const
    {
        const string& s = !comments ? text
            : string("%s    \n%s    ", text, comments);
        return ::MessageBox(handle, s, title,
            MB_TASKMODAL | MB_ICONWARNING | MB_OK) == IDOK;
    }

    bool alertYesNo(const char* title, const char* text, const char* comments = 0) const
    {
        const string& s = !comments ? text
            : string("%s    \n%s    ", text, comments);
        return ::MessageBox(handle, s, title,
            MB_TASKMODAL | MB_ICONQUESTION | MB_YESNO) == IDYES;
    }

    bool alertRetryCancel(const char* title, const char* text, const char* comments = 0) const
    {
        const string& s = !comments ? text
            : string("%s    \n%s    ", text, comments);
        return ::MessageBox(handle, s, title,
            MB_TASKMODAL | MB_ICONWARNING | MB_RETRYCANCEL) == IDRETRY;
    }

    template <typename T>
    void object(T* obj) const
    {
        ::SetWindowLongPtr(handle, GWLP_USERDATA, (__int3264) (LONG_PTR) obj);
    }

    template <typename T>
    T* object() const
    {
        return (T*) (LONG_PTR) ::GetWindowLongPtr(handle, GWLP_USERDATA);
    }

    // ........................................................................

private:

    Rect metrics() const
    {
        if (!(::GetWindowLong(handle, GWL_STYLE) & WS_CAPTION))
            return Rect();

        int x = ::GetSystemMetrics(SM_CXFIXEDFRAME);
        int y = ::GetSystemMetrics(SM_CYFIXEDFRAME);
        int c = ::GetSystemMetrics(SM_CYCAPTION);
        return Rect(x, c + y, 2 * x, c + 2 * y);
    }
};

// ............................................................................

} // ~ namespace kali

// ............................................................................

#endif // ~ KALI_WINDOW_INCLUDED
