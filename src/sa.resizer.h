
#ifndef SA_RESIZER_INCLUDED
#define SA_RESIZER_INCLUDED

#include "kali/ui/native.h"

// ............................................................................

namespace sa {

// ............................................................................

using namespace kali;

/*  The method used here is sort of one big kludge. It (a simplified version) was
    tempting initially, but now it turns out to be too Host/OS versions/settings
    depended, and what's most important, it brings too many barely ever fixable
    side-effects/issues in certain hosts. It becomes a real nightmare to test/
    improve/maintain this method, so I'm about to give it up in favour of
    something more safe and simple.
    (Trivial "resize-triangle"/"near-edges-trigger" with in-Editor-mouse-tracking
    should be enough and will be equally usable from a user point of view.)
*/

struct Resizer : ui::native::LayerBase
{
    int poll(HWND editor_)
    {
        if (!editor)
            ctor(editor_);

        if (!parent || !::IsWindowVisible(parent))
            return ::ShowWindow(handle, SW_HIDE);

        if (!resizingNow) {
            Rect r = rect(parent);
            if (curRect != r)
            {
                tf
                curRect  = r;
                return ::SetWindowPos(handle, 0, r.x, r.y, r.w, r.h,
                    SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_SHOWWINDOW);
            }
        }

        return ::IsWindowVisible(handle) ? ~0
            : ::ShowWindow(handle, SW_SHOWNA);
    }

    bool msgHook(LRESULT& result, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        switch (msg)
        {
        case WM_ERASEBKGND:
            result = ~0;
            break;

        case WM_ENTERSIZEMOVE:
            resizingNow = 1;
            break;

        case WM_EXITSIZEMOVE:
            trace.full("msgHook: WM_EXITSIZEMOVE\n");
            resizingNow = 0;
            // always return focus to the parent:
            ::SwitchToThisWindow(parent, 0);
            break;

        case WM_NCHITTEST:
            result = ::DefWindowProc(handle, msg, wparam, lparam);
            // return HTTRANSPARENT for anything except right and bottom edges:
            // (can't resize by left/top edges since some (rare) hosts get plugin
            // window screwed if resized w/o SWP_NOMOVE)
            switch (result)
            {
            case HTRIGHT:
            case HTBOTTOM:
            case HTBOTTOMRIGHT:
                break;
            default:
                result = HTTRANSPARENT;
            }
            break;
        }

        return true;
    }

    void resized()
    {
        if (resizingNow)
            resizeEditor(rect(handle));
    }

private:

    static Rect rect(HWND handle)
    {
        RECT r;
        ::GetWindowRect(handle, &r);
        return Rect(r.left, r.top,
            r.right - r.left, r.bottom - r.top);
    }

    void resizeEditor(const Rect& newRect)
    {
        int w = newRect.w - curRect.w;
        int h = newRect.h - curRect.h;
        curRect = newRect;
        Size s = Window(editor).size();
        Window(editor).size(s.w += w, s.h += h);

        if (plugin->canHostDo("sizeWindow") > 0) {
            trace.full("%s: hostCanDoSizeWindow\n", FUNCTION_);
            if (plugin->sizeWindow(s.w, s.h))
                return trace.full("%s: sizeWindow(%i/%+i, %i/%+i) accepted\n",
                    FUNCTION_, s.w, w, s.h, h); // fixme, do something else?
        }

        trace.full("%s: plugin->sizeWindow() not supported\n", FUNCTION_);

        // todo, actually need to loop through all ancestors
        // until `parent` one (it's not always just one)
        HWND ancestor = ::GetAncestor(editor, GA_PARENT);
        Rect r = rect(ancestor);
        trace.full("%s: SetWindowPos(ancestor, 0, 0, %i, %i)\n",
            FUNCTION_, r.w + w, r.h + h);
        ::SetWindowPos(ancestor, 0, 0, 0,
            r.w + w, r.h + h, SWP_NOMOVE | SWP_NOZORDER);

        r = newRect;
        trace.full("%s: SetWindowPos(parent, %i, %i, %i, %i)\n",
                FUNCTION_, r.x, r.y, r.w, r.h);
        ::SetWindowPos(parent, 0, r.x, r.y, r.w, r.h, SWP_NOMOVE | SWP_NOZORDER);
    }

    void ctor(HWND editor_)
    {
        editor = editor_;
        parent = editor_;
        string name;
        do
        {
            parent = ::GetAncestor(parent, GA_PARENT);
            name   = Window(parent).title();
            trace.full("%s: %p %s\n", FUNCTION_, parent, name());
        }
        while (!strstr(name, NAME)
            && (::GetWindowLong(parent, GWL_STYLE) & WS_CHILD));

        Window p(parent);
        kali::app->createWindow(&p, this);
        ::ShowWindow(handle, SW_HIDE);

        // WS_EX_TRANSPARENT does not work in Win7 the way
        // it did in XP (and Vista?), so one more hack :(
        if (LOBYTE(::GetVersion()) > 5) {
            ::SetWindowLong(handle, GWL_EXSTYLE,
                ::GetWindowLong(handle, GWL_EXSTYLE)
                    & ~WS_EX_TRANSPARENT | WS_EX_LAYERED);
            ::SetLayeredWindowAttributes(handle, 0, 1, LWA_ALPHA);
        }
    }

public:
    Resizer(AudioEffectX* plugin) :
        plugin(plugin),
        editor(0),
        parent(0),
        resizingNow(0)
    {}

    void close() {tf}
    ~Resizer()   {this->destroy(); tf}

private:
    AudioEffectX* plugin;
    HWND          editor;
    HWND          parent;
    Rect          curRect;
    bool          resizingNow;
};

// ............................................................................

} // ~ namespace sa

// ............................................................................

namespace kali    {
namespace details {

template <>
struct Traits <sa::Resizer> : TraitsBase <sa::Resizer>
{
    enum
    {
        styleEx = WS_EX_TRANSPARENT,
        style   = WS_POPUP | WS_SIZEBOX
    };

    static bool create(const Window* parent, sa::Resizer* window)
    {
        return createWindow<Traits>(parent, window);
    }
};

} // ~ namespace details
} // ~ namespace kali

// ............................................................................

#endif // ~ SA_RESIZER_INCLUDED
