
#ifndef KALI_APP_DETAILS_INCLUDED
#define KALI_APP_DETAILS_INCLUDED

#include <windows.h>
#include <commctrl.h>
#include "kali/window.h"
#include "kali/ui/native/widgets.h"
#include "kali/resources.h"

// ............................................................................

namespace kali {

// ............................................................................

struct AppDetails : ReleaseAny
{
    Module* module_;
    Window* mainWindow_;
    bool    graphics_;

    AppDetails() :
        module_(0),
        mainWindow_(0),
        graphics_(0)
    {}
};

inline Module* app::module()     const {return details_->module_;}
inline Window* app::mainWindow() const {return details_->mainWindow_;}
inline void    app::useThreads()       {}

inline void app::initGraphics()
{
#if defined KALI_GRAPHICS_INCLUDED
    if (details_->graphics_)
        return;
    autorelease(new graphics::Initializer);
    details_->graphics_ = true;
#endif
}

// ............................................................................

namespace details {

// ............................................................................

template <typename Traits>
bool preinitWindow()
{
    WNDCLASSEX wcx;
    wcx.cbSize		  = sizeof(wcx);
    wcx.style	      = Traits::classStyle;
    wcx.hInstance     = app->module();
    wcx.lpfnWndProc   = Traits::thunk;
    wcx.lpszMenuName  = Traits::name();
    wcx.lpszClassName = Traits::name();
    wcx.cbClsExtra    = 0;
    wcx.cbWndExtra    = 0;
    wcx.hIcon		  = 0;
    wcx.hIconSm		  = 0;
    wcx.hCursor       = ::LoadCursor(0, IDC_ARROW);
    wcx.hbrBackground = ::GetSysColorBrush(COLOR_3DFACE);

    return !!::RegisterClassEx(&wcx);
}

// ............................................................................

template <typename Traits, typename T>
bool createWindow(const Window* parent, T* window)
{
    Traits::initializer();
    preinitWindow<Traits>();
    const int c = CW_USEDEFAULT;
    HWND handle = ::CreateWindowEx((DWORD) Traits::styleEx,
        Traits::name(), Traits::name(), (DWORD) Traits::style,
        c, c, c, c, parent->handle, 0, app->module(), window);
    if (!handle)
        trace("%s: CreateWindowEx failed [%i]\n", FUNCTION_, ::GetLastError());

    if (!::IsWindowVisible(handle))
        ::ShowWindow(handle, SW_SHOWDEFAULT);
    return !!handle;
}

template <typename Traits, typename T>
static bool loadWindow(const char* tag, const Window* parent, T* window)
{
    resource::Raw <char> rc(RT_DIALOG, tag);
    resource::details::patchDialogFont(rc.data(), rc.size());

    HWND handle = ::CreateDialogIndirectParam(app->module(),
        (DLGTEMPLATE*) rc.data(), parent->handle,
        Traits::thunk, (LPARAM) window);

    if (!handle)
        trace("%s: CreateDialogParam failed [%i]\n", FUNCTION_, ::GetLastError());

    if (!::IsWindowVisible(handle))
        ::ShowWindow(handle, SW_SHOWDEFAULT);
    return !!handle;
}

// ............................................................................

#pragma warning(push)
#pragma warning(disable: 4706)

template <typename T, bool IsNative> struct Dispatch;

// ............................................................................

#define split(v) short(LOWORD(v)), short(HIWORD(v))

template <typename T>
struct Dispatch <T, false>
{
    static void initializer()
    {
        app->initGraphics();
    }

    static LRESULT CALLBACK thunk(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        T* window;

        if (msg == WM_NCCREATE
            && lparam
            && (window = (T*) ((CREATESTRUCT*) lparam)->lpCreateParams))
        {
            window->handle = handle;
		    window->object(window);
        }
        else
            window = Window(handle).object<T>();

        return (window)
            ? dispatch(window, msg, wparam, lparam)
            : ::DefWindowProc(handle, msg, wparam, lparam);
    }

    static LRESULT dispatch(T* window, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        HWND handle = window->handle;

        switch (msg)
        {
        case WM_CREATE:
            return window->open() ? 0 : -1;

        case WM_DESTROY:
            window->close();
            if (window == app->mainWindow())
                ::PostQuitMessage(0);
            return 0;

        case WM_CLOSE:
            ::DestroyWindow(handle);
            return 0;

        case WM_LBUTTONDOWN:
            if (window->mouse(down, split(lparam)))
                ::SetCapture(handle);
            return 0;

        case WM_LBUTTONUP:
            window->mouse(up, split(lparam));
            ::ReleaseCapture();
            return 0;

        case WM_MOUSEMOVE:
            if ((wparam & MK_LBUTTON)
                && !window->mouseMove(split(lparam)))
                    ::ReleaseCapture();
            return 0;

        case WM_NCHITTEST:
        {
            POINT p = {split(lparam)};
            ::ScreenToClient(handle, &p);
            if (window->titlebar().contains(p.x, p.y))
                return HTCAPTION;
            break;
        }

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC dc = ::BeginPaint(handle, &ps);
            Rect r(ps.rcPaint.left, ps.rcPaint.top,
                ps.rcPaint.right  - ps.rcPaint.left,
                ps.rcPaint.bottom - ps.rcPaint.top);
            if (!r.empty())
            {
                graphics::BufferedContext c(dc, r);
                window->draw(c);
            }

            ::EndPaint(handle, &ps);
            return 0;
        }

        case WM_ERASEBKGND:
            return TRUE;

        #if 0

        // keyboard input is sort of implemented
        // but let's disable it for now since none of
        // current kali-based apps do actually show 'key focus'
        // (hence key control would look mysterious)

        case WM_GETDLGCODE:
            return DLGC_WANTARROWS;

        case WM_KEYDOWN:
            window->key(down, int(wparam));
            return 0;

        case WM_KEYUP:
            window->key(up, int(wparam));
            return 0;

        #endif

        #if 1

        // temporary hack, native widgets are not supposed
        // to be used directly with custom windows.
        // in future it should be a native
        // layer placed in between

        case WM_CTLCOLORSTATIC:
            return (LRESULT) ::GetSysColorBrush(COLOR_WINDOW);

        #endif

        }

        return ::DefWindowProc(handle, msg, wparam, lparam);
    }
};

// ............................................................................

template <typename T>
struct Dispatch <T, true>
{
    static void initializer() {}

    static LRESULT CALLBACK thunk(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        T* window;

        if (msg == WM_NCCREATE
            && lparam
            && (window = (T*) ((CREATESTRUCT*) lparam)->lpCreateParams))
        {
            window->handle = handle;
		    window->object(window);
        }
        else
            window = Window(handle).object<T>();

        return (window)
            ? dispatch(window, msg, wparam, lparam)
            : ::DefWindowProc(handle, msg, wparam, lparam);
    }

    static LRESULT dispatch(T* window, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        HWND handle = window->handle;

        switch (msg)
        {
        case WM_CREATE:
            return window->open() ? 0 : -1;

        case WM_DESTROY:
            window->close();
            if (window == app->mainWindow())
                ::PostQuitMessage(0);
            return 0;

        case WM_GETDLGCODE:
            return DLGC_WANTARROWS | DLGC_WANTTAB;

        case WM_COMMAND:
            switch (HIWORD(wparam))
            {
            case EN_CHANGE:
            case BN_CLICKED:
            case CBN_SELCHANGE:
                widget::Base::thunk_((HWND) lparam, LOWORD(wparam));
                return 0;

            case EN_KILLFOCUS:
                // todo: ? this actually conflicts with above thunk_ call
                widget::Base::thunk_((HWND) lparam, HIWORD(wparam));
                return true;
            }
            break;

        case WM_NOTIFY:
            switch (((NMHDR*) lparam)->code)
            {
            case TCN_SELCHANGE:
                widget::Base::thunk_
                    (((NMHDR*) lparam)->hwndFrom, 0);
                return 0;
            }
            break;

        case WM_VSCROLL:
		case WM_HSCROLL:
            switch (LOWORD(wparam))
            {
                case SB_LINEUP:
                case SB_PAGEUP:
                case SB_LINEDOWN:
                case SB_PAGEDOWN:
                case SB_THUMBTRACK:
                case SB_THUMBPOSITION:
                    widget::Base::thunk_((HWND) lparam, 0);
                    return 0;
            }
			return 0;

        case WM_DRAWITEM:
            widget::Base::drawThunk_((DRAWITEMSTRUCT*) lparam);
            return TRUE;

        #if GL_TRUE // temporary extensions:

        case WM_PAINT:
            if (!window->draw())
                break;
            ::ValidateRect(window->handle, 0);
            return 0;

        case WM_MOVE:
        case WM_SIZE:
            window->resized();
            return 0;

        case WM_MOUSELEAVE:
        case WM_MOUSEMOVE:
            if (!window->mouseMove((short)
                LOWORD(lparam), (short) HIWORD(lparam)))
                    break;
            return 0;

        case WM_LBUTTONDBLCLK:
            if (!window->mouseDoubleClick())
                break;
            return 0;

        case WM_LBUTTONDOWN:
            if (!window->mouse(down, split(lparam)))
                break;
            return 0;

        case WM_LBUTTONUP:
            if (!window->mouse(up, split(lparam)))
                break;
            return 0;

        case WM_RBUTTONUP:
            if (!window->mouseR(up, split(lparam)))
                break;
            return 0;

        case WM_KEYDOWN:
            if (!window->keyDown(int(wparam), int(lparam)))
                break;
            return 0;

        case WM_ENTERSIZEMOVE:
        case WM_EXITSIZEMOVE:
        case WM_ERASEBKGND:
        case WM_NCHITTEST:
        {
            LRESULT result = 0;
            if (window->msgHook(result, msg, wparam, lparam))
                return result;
            break;
        }

        #endif

        }

        return ::DefWindowProc(handle, msg, wparam, lparam);
    }
};

// ............................................................................

template <typename T>
struct DispatchLoaded
{
    static INT_PTR CALLBACK thunk(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        T* window;

        if (msg == WM_INITDIALOG
            && (window = (T*) (lparam)))
        {
            window->handle = handle;
			window->object(window);
        }
        else
            window = Window(handle).object<T>();

        if (window)
            return dispatch(window, msg, wparam, lparam);

        return FALSE;
    }

    static INT_PTR dispatch(T* window, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        using namespace ui::native;

        switch (msg)
        {
        case WM_INITDIALOG:
            if (!window->open())
                window->destroy();
            return true;

        case WM_DESTROY:
            window->close();
            if (window == app->mainWindow())
                ::PostQuitMessage(0);
            return true;

        case WM_GETDLGCODE:
            return DLGC_WANTARROWS | DLGC_WANTTAB;

        case WM_COMMAND:
            switch (HIWORD(wparam))
            {
            case EN_CHANGE:
            case BN_CLICKED:
            case CBN_SELCHANGE:
                widget::Base::thunk_((HWND) lparam, LOWORD(wparam));
                return true;

            case EN_KILLFOCUS:
                // todo: this actually conflicts with above thunk_ call
                widget::Base::thunk_((HWND) lparam, HIWORD(wparam));
                return true;
            }
            break;

        case WM_VSCROLL:
		case WM_HSCROLL:
            if ((LOWORD(wparam) == SB_LINEUP) ||
                (LOWORD(wparam) == SB_LINEDOWN) ||
                (LOWORD(wparam) == SB_THUMBTRACK) ||
                (LOWORD(wparam) == SB_THUMBPOSITION))
                    widget::Base::thunk_((HWND) lparam, 0);
			return true;

        case WM_DRAWITEM:
            widget::Base::drawThunk_((DRAWITEMSTRUCT*) lparam);
            return true;
        }

        return false;
    }
};

#undef split

// ............................................................................

#pragma warning(pop)

// ............................................................................

template <typename T>
struct TraitsBase : Dispatch <T, !T::UsesGraphics>
{
    enum
    {
        classStyle = CS_DROPSHADOW * !!T::DropShadow,
        styleEx    = WS_EX_DLGMODALFRAME * !!T::SysCaption,
        style      = T::SysCaption
                   ? WS_POPUP | WS_CAPTION | WS_SYSMENU
                   : WS_POPUP
    };

    static const char* name()
    {
        return typeString<T>();
    }

    static bool create(const Window* parent, T* window)
    {
        return createWindow<TraitsBase>(parent, window);
    }
};

template <typename T>
struct Traits : TraitsBase <T>
{

};

template <typename T>
struct AppTraits : Traits <T>
{
    enum
    {
        styleEx = 0,
        style   = T::SysCaption
                ? WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX
                : WS_POPUP   | WS_SYSMENU | WS_MINIMIZEBOX
    };

    static bool create(const Window* parent, T* window)
    {
        return createWindow<AppTraits>(parent, window);
    }
};

template <typename T>
struct LayerTraits : Traits <T>
{
    enum
    {
        // Note: WS_EX_CONTROLPARENT is a must!
        // (otherwise endless WM_GETDLGCODE loop
        // arises under certain conditions)
        classStyle = 0,
        styleEx    = WS_EX_CONTROLPARENT,
        style      = WS_CHILD
    };

    static bool create(const Window* parent, T* window)
    {
        return createWindow<LayerTraits>(parent, window);
    }
};

// ............................................................................

inline bool switchTo(const char* name)
{
    HWND handle = ::FindWindow(name, name);
    if (handle)
    {
        ::ShowWindow(handle, SW_RESTORE);
        ::SetForegroundWindow(handle);
    }

    return !!handle;
}

inline int loop(HWND handle)
{
    MSG msg;
    while (::GetMessage(&msg, NULL, 0, 0))
    {
	    if (!::IsDialogMessage(handle, &msg))
	    {
		    ::TranslateMessage(&msg);
		    ::DispatchMessage(&msg);
	    }
    }

    return (int) msg.wParam;
}

inline void fixSystemMenu(HWND handle)
{
    if (::GetWindowLong(handle, GWL_STYLE)
        & WS_MINIMIZEBOX)
            return;

    HMENU menu = ::GetSystemMenu(handle, FALSE);
    ::DeleteMenu(menu, SC_RESTORE,  MF_BYCOMMAND);
    ::DeleteMenu(menu, SC_MINIMIZE, MF_BYCOMMAND);
    ::DeleteMenu(menu, SC_MAXIMIZE, MF_BYCOMMAND);
}

// ............................................................................

}  // ~ namespace details

// ............................................................................

template <typename T>
int app::run(bool ItCouldBeOnlyOne)
{
    if (ItCouldBeOnlyOne
        && details::switchTo
            (details::AppTraits<T>::name()))
                return 0;

    INITCOMMONCONTROLSEX icc = {sizeof(icc), ICC_WIN95_CLASSES};
    ::InitCommonControlsEx(&icc);

    T*     window;
    Window parent;

    using kali::app;
    app.alloc();
    app->details_ = app->autorelease(new AppDetails);
    app->details_->module_     = ::GetModuleHandle(0);
    app->details_->mainWindow_ = window = new T;

    if (!details::AppTraits<T>::create(&parent, window))
        return ::MessageBox(0, "Sorry, I can't live here.",
            details::AppTraits<T>::name(), MB_ICONERROR) & 0;
    window->icon(resource::details::firstIcon());
    app->autorelease(window); // should go *after* create()
    int ret = details::loop(window->handle);
    app.release();
    return ret;
}

// ............................................................................

template <typename T>
bool app::createWindow(const Window* parent, T* window)
{
    return details::Traits<T>::create(parent, window);
}

// ............................................................................

template <typename T>
bool app::createLayer(const Window* parent, T* window)
{
    return details::LayerTraits<T>::create(parent, window);
}

template <typename T>
bool app::loadLayer(const char* tag, const Window* parent, T* window)
{
    using namespace details;
    return loadWindow
        <DispatchLoaded<T>>(tag, parent, window);
}

// ............................................................................

}  // ~ namespace kali

// ............................................................................

#endif // ~ KALI_APP_DETAILS_INCLUDED
