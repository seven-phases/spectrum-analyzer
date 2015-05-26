
#ifndef KALI_UI_NATIVE_WIDGETS_INCLUDED
#define KALI_UI_NATIVE_WIDGETS_INCLUDED

#include <windows.h>
#include <commctrl.h>
#include "kali/ui/native/widgets.base.h"
#include "kali/ui/native/details.h"

// ............................................................................

namespace kali   {
namespace ui     {
namespace native {

// ............................................................................

struct Menu : UsesCallback
{
    void operator += (const char* item)
    {
        ::AppendMenu(handle, MF_STRING,
            ::GetMenuItemCount(handle) + 1, item);
    }

    string text(int index) const
    {
        string s = "? ? ?";
        ::GetMenuString(handle,
            index + 1, s(), s.size, MF_BYCOMMAND);
        return s;
    }

    void show(Window* window, const Point& p, int value = ~0)
    {
        show(window, p.x, p.y, value);
    }

    void show(Window* window, int x, int y, int value = ~0)
    {
        if (value < 0)
            --value;
        mark_(value, true);

        int align = TPM_RIGHTALIGN | TPM_TOPALIGN;

        POINT p = {x, y};
        ::ClientToScreen(window->handle, &p);
        int v = ::TrackPopupMenu(handle,
            TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTBUTTON |
            align, p.x, p.y, 0, window->handle, 0);

        if (v)
            callback(v - 1);

        mark_(value, false);
    }

    Menu(const Menu& m) : handle(::CreatePopupMenu())
    {
        string s;
        int n = ::GetMenuItemCount(m.handle) + 1;
        for (int i = 1; i < n; i++)
        {
            ::GetMenuString(m.handle,
                i, s(), s.size, MF_BYCOMMAND);
            ::AppendMenu(handle, MF_STRING, i, s);
        }
    }

    Menu() : handle(::CreatePopupMenu()) {}
    ~Menu() {::DestroyMenu(handle);}

private:

    void mark_(int index, bool m)
    {
        ::CheckMenuItem(handle, index + 1, MF_BYCOMMAND
            | (m ? MF_CHECKED : MF_UNCHECKED));
    }

private:
    HMENU handle;
};

// ............................................................................

struct Font : ReleaseAny
{
    struct Scale
    {
        enum {refx = 6, refy = 13};
        int x(int v) {return (v * x_ + refx/2) / refx;}
        int y(int v) {return (v * y_) / refy;}
        Scale(int x_, int y_) : x_(x_), y_(y_) {}

    private:
        int x_, y_;
    };

    static const Font& main()
    {
        static Font* aux = 0;
        if (!aux)
            aux = app->autorelease(new Font);
        return *aux;
    }

    Scale    scale() const {return scale(handle);}
    operator HFONT() const {return handle;}

    Font() : handle(ctor()) {}
    ~Font() {::DeleteObject(handle);}

private:
    HFONT handle;

    Font(const Font&);
    Font& operator = (const Font&);
    template <typename T> operator T () const;

private:

    HFONT static ctor()
    {
        NONCLIENTMETRICS ncm = {sizeof(ncm)};
		::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);
        return ::CreateFontIndirect(&ncm.lfMessageFont);
    }

    static Scale scale(HFONT handle)
    {
        HDC dc  = ::CreateCompatibleDC(0);
        HGDIOBJ f = ::SelectObject(dc, handle);

        TEXTMETRIC tm;
        ::GetTextMetrics(dc, &tm);
        int y = tm.tmHeight;

        const char a[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        const int n = sizeof(a) - 1;
        SIZE size;
        ::GetTextExtentPoint32(dc, a, n, &size);
        int x = (size.cx + (n / 2)) / n;

        ::SelectObject(dc, f);
        ::DeleteDC(dc);

        return Scale(x, y);
    }
};

// ............................................................................

template <typename T, typename R>
inline void EnumerateFonts(T* obj, R (T::*func)(const char*))
{
    struct Func
    {
        typedef R (T::*F)(const char*);
        Func(T* obj, F func) : obj(obj), func(func) {}
        T* obj;
        F func;

        static int CALLBACK thunk(const LOGFONT* lf,
            const TEXTMETRIC* /*tm*/, DWORD /*type*/, LPARAM ptr)
        {
            /* if ((type == RASTER_FONTTYPE) ||
                (type == DEVICE_FONTTYPE))
                return ~0;*/

            Func* func = (Func*) ptr;
            (func->obj->*func->func) // :)
                (lf->lfFaceName);
            return ~0;
        }
    };

    HDC dc = ::GetDC(0);
    Func f(obj, func);
    ::EnumFontFamilies(dc, 0, &Func::thunk, (LPARAM) &f);
    ::ReleaseDC(0, dc);
}

// ............................................................................

struct TooltipSupport
{
    TooltipSupport() : handle(0) {}
    ~TooltipSupport() {clear(0);}

    void attach(Window* window, const Rect& r, const char* text)
    {
        if (!this || r.empty())
            return;

        if (!handle)
            handle = ctor(window);

        TOOLINFO ti;
        RECT rect   = {r.x, r.y, r.right(), r.bottom()};
        ti.cbSize   = sizeof(TOOLINFO);
        ti.uFlags   = TTF_SUBCLASS;
        ti.hwnd     = window->handle;
        ti.uId      = 1;
        ti.rect     = rect;
        ti.hinst    = 0;
        ti.lpszText = (char*) (text);
        ti.lParam   = 0;

        if (!::SendMessage(handle, TTM_ADDTOOL, 0, (LPARAM) &ti))
            trace("%s: TTM_ADDTOOL failed [%i]\n", FUNCTION_, ::GetLastError());
    }

    void clear(Window*)
    {
        ::DestroyWindow(handle);
        handle = 0;
    }

private:

    static HWND ctor(Window* window)
    {
        HWND handle = ::CreateWindowEx(WS_EX_TOPMOST,
            TOOLTIPS_CLASS, 0, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
            window->handle, 0, app->module(), 0);

        if (!handle)
            trace("%s: CreateWindowEx failed [%i]\n", FUNCTION_, ::GetLastError());

        return handle;
    }

private:
    HWND handle;
};

// ............................................................................

struct Timer : UsesCallback
{
    void start(Window* window, unsigned time)
    {
        stop();
        window_ = window;
        ::SetTimer(window_->handle, (UINT_PTR) this, time, thunk);
    }

    void stop()
    {
        if (window_)
            ::KillTimer(window_->handle, (UINT_PTR) this);
        window_ = 0;
        counter = 0;
    }

    Timer() : window_(0) {}
    ~Timer() {stop();}

private:

    static VOID CALLBACK thunk(HWND, UINT, UINT_PTR ptr, DWORD)
    {
        Timer* timer = (Timer*) ptr;
        if (timer)
            timer->callback(++timer->counter);
    }

private:
    Window* window_;
    int     counter;
};

// ............................................................................

//struct Cursor
//{
//    typedef LPSTR Handle;
//    Cursor() : value_(IDC_ARROW) {}
//    Handle value() const {return value_;}
//
//    void value(Handle v)
//    {
//        if (value_ != v)
//            ::SetCursor(::LoadCursor(0, value_ = v));
//    }
//
//private:
//    Handle value_;
//};

// ............................................................................

struct WaitCursor
{
    WaitCursor()  {::SetCursor(::LoadCursor(0, IDC_WAIT));}
    ~WaitCursor() {::SetCursor(::LoadCursor(0, IDC_ARROW));}

    explicit WaitCursor(bool enable)
    {
        if (enable)
            ::SetCursor(::LoadCursor(0, IDC_WAIT));
    }
};

// ............................................................................

namespace widget {

// ............................................................................

struct Base : Interface
{
    typedef Window::Handle Handle;

    bool enable() const  {return !!::IsWindowEnabled(handle);}
    void enable(bool v)  {::EnableWindow(handle, v);}
    bool visible() const {return !!::IsWindowVisible(handle);}
    void visible(bool v) {::ShowWindow(handle, SW_SHOW * v);}
    int  value() const   {return 0;}
    void value(int)      {}
    int  range() const   {return 1;}
    void range(int)      {}

    void text(const char* v) {::SetWindowText(handle, v);}

    string text() const
    {
        string v;
        ::GetWindowText(handle, v(), v.size);
        return v;
    }

    Point position()
    {
        RECT r;
        ::GetWindowRect(handle, &r);
        ::MapWindowPoints(0,
            ::GetParent(handle), (POINT*) &r, 2);
        return Point(r.left, r.top);
    }

    void position(int x, int y)
    {
        ::SetWindowPos(handle, 0, x, y, 0, 0,
            SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW);
    }

    Size size()
    {
        RECT r;
        ::GetClientRect(handle, &r);
        return Size(r.right, r.bottom);
    }

    void size(int w, int h)
    {
        ::SetWindowPos(handle, 0, 0, 0, w, h,
            SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW);
    }

    virtual void ctor(Parent* /*parent*/, Handle h)
    {
        handle = h;
        Window(handle).object(this);
    }

    static void thunk_(Handle src, int flags)
    {
        Base* widget = object(src);
        if (widget)
            widget->action(flags);
    }

    static void drawThunk_(DRAWITEMSTRUCT* ds)
    {
        Base* widget = object(ds->hwndItem);
        if (widget)
            widget->draw(ds);
    }

    Window::Handle expose() const {return handle;};

    enum
    {
        // for indirect Ctor:
        style_       = 0,
        styleEx_     = 0,
        // for resource Ctor:
        styleRC_     = 0,
        styleMaskRC_ = 0
    };

    Base() : handle(0) {}

protected:

    int msg(UINT m, WPARAM w, LPARAM l) const
    {
        return (int) ::SendMessage(handle, m, w, l);
    }

    static Base* object(Handle handle)
    {
        return Window(handle).object<Base>();
    }

    virtual void action(int) {callback(value());}

    virtual void draw(DRAWITEMSTRUCT*) {}

    void changeStyle(int index, int remove, int add, bool recreate = false)
    {
        if (recreate)
            return ctor(0, details::cloneWindow
                (handle, remove, add, true));

        int v = ::GetWindowLong(handle, index);
        v = (~remove & v) | add;
        ::SetWindowLong(handle, index, v);
        ::SetWindowPos(handle, 0, 0, 0, 0, 0,
            SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED);
    }

public: // protected:
    Handle handle;
    Base(const Base&);
    Base& operator = (const Base&);
};

// ............................................................................

struct Null : Interface
{
    bool enable() const    {return 0;}
    void enable(bool)      {}
    bool visible() const   {return 0;}
    void visible(bool)     {}
    int  value() const     {return 0;}
    void value(int)        {}
    int  range() const     {return 0;}
    void range(int)        {}
    string text() const    {return string();}
    void text(const char*) {}

    Window::Handle expose() const {return 0;};
};

// ............................................................................

template <typename T>
typename T::Type* Ctor(Parent*, const Rect&, const char* text = 0);

// ............................................................................

struct Text : Base
{
    static const char* class_() {return WC_STATIC;}

    enum {style_ = SS_CENTERIMAGE};
};

struct TextRight : Text
{
    enum {style_ = SS_RIGHT | SS_CENTERIMAGE/* | SS_BLACKFRAME*/};
};

struct Break : Text
{
    enum {style_ = SS_ETCHEDHORZ};
};

// ............................................................................

struct Edit : Base
{
    static const char* class_() {return WC_EDIT;}

    enum
    {
        style_   = ES_AUTOHSCROLL | WS_TABSTOP,
        styleEx_ = WS_EX_CLIENTEDGE
    };

    void append(const char* v)
    {
        msg(EM_SETSEL, (WPARAM) -1, -1);
        msg(EM_REPLACESEL, 0, (LPARAM) v);
    }

    void border(bool v)
    {
        changeStyle(GWL_EXSTYLE,
            WS_EX_CLIENTEDGE, WS_EX_CLIENTEDGE * v);
    }

    void align(Align v)
    {
        changeStyle(GWL_STYLE,
            (left | center | right), v, true);
    }

protected:

    void action(int flags)
    {
        (flags == EN_KILLFOCUS)
            ? extCallback(value())
            : callback(0);
    }
};

struct TextCopy : Edit
{
    enum
    {
        style_ = ES_READONLY
               | ES_MULTILINE
               | ES_AUTOVSCROLL
               | WS_TABSTOP,
    };
};

// ............................................................................

struct Button : Base
{
    static const char* class_() {return WC_BUTTON;}

    enum
    {
        styleRC_     = BS_PUSHBUTTON,
        styleMaskRC_ = BS_TYPEMASK,
        style_       = styleRC_ | WS_TABSTOP
    };
};

struct Toggle : Button
{
    enum
    {
        styleRC_ = BS_AUTOCHECKBOX,
        style_   = styleRC_ | WS_TABSTOP
    };

    int  value() const {return msg(BM_GETCHECK, 0, 0);}
    void value(int v)  {msg(BM_SETCHECK, v, 0);}
};

struct Radio : Toggle
{
    enum
    {
        styleRC_ = BS_AUTORADIOBUTTON,
        style_   = styleRC_ | WS_TABSTOP
    };
};

// ............................................................................

struct Fader : Base
{
    static const char* class_() {return TRACKBAR_CLASS;}

    enum {style_ = TBS_DOWNISLEFT | WS_TABSTOP};

    int  value() const {return msg(TBM_GETPOS, 0, 0);}
    void value(int v)  {msg(TBM_SETPOS, TRUE, v);}
    int  range() const {return msg(TBM_GETRANGEMAX, 0, 0);}
    void range(int v)  {msg(TBM_SETRANGEMAX, TRUE, v);}
};

// ............................................................................

struct Stepper : Base
{
    static const char* class_() {return UPDOWN_CLASS;}

    enum {style_ = UDS_ALIGNRIGHT | UDS_ARROWKEYS};

    int  value() const {return msg(UDM_GETPOS32, 0, 0);}
    void value(int v)  {msg(UDM_SETPOS32, 0, v);}
    int  range() const {return msg(UDM_GETRANGE, 0, 0);}
    void range(int v)  {msg(UDM_SETRANGE32, 0, v);}

    void buddy(Handle h)
    {
        msg(UDM_SETBUDDY, (WPARAM) h, 0);
        ::SetWindowPos(handle, h, 0, 0,
            0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }

protected:

    void action(int)
    {
        callback(value());
        ::SendMessage((Handle) ::SendMessage(handle,
            UDM_GETBUDDY, 0, 0), EM_SETSEL, 0, -1);
    }
};

// ............................................................................

struct Meter : Base
{
    static const char* class_() {return PROGRESS_CLASS;}

    int  value() const {return msg(PBM_GETPOS, 0, 0);}
    void value(int v)  {msg(PBM_SETPOS, v, 0);}
    int  range() const {return msg(PBM_GETRANGE, 0, 0);}
    void range(int v)  {msg(PBM_SETRANGE32, 0, v);}
};

// ............................................................................

struct Combo : Base
{
    static const char* class_() {return WC_COMBOBOX;}

    enum {style_ = CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP};

    int  value() const {return msg(CB_GETCURSEL, 0, 0);}
    void value(int v)  {msg(CB_SETCURSEL, v, 0);}
    int  range() const {return msg(CB_GETCOUNT, 0, 0);}
    void range(int)    {}

    void   text(const char* v) {msg(CB_SELECTSTRING, (WPARAM) -1, (LPARAM) v);}
    string text() const        {return Base::text();}

    void add(const char* text, int v) {data(add(text), v);}
    int  add(const char* text) {return msg(CB_ADDSTRING, 0, (LPARAM) text);}
    void data(int i, int v) const {msg(CB_SETITEMDATA, i, v);}
    int  data(int i) {return msg(CB_GETITEMDATA, i, 0);}
    void clear() {msg(CB_RESETCONTENT, 0, 0);}

    void valueByData(int d, int default_ = ~0)
    {
        int n = range();
        for (int i = 0; i < n; i++)
            if (data(i) == d)
                return value(i);
        value(default_);
    }

    // enables list sorting (must be set *before* adding strings)
    void sort(bool v = true)
    {
        // changeStyle(GWL_STYLE, CBS_SORT, CBS_SORT * v); // does not work :(
        // well, let's burn it then:
        changeStyle(GWL_STYLE, CBS_SORT, CBS_SORT * v, true);
    }
};

// ............................................................................

struct Group : Base
{
    typedef Radio Widget;

    static const char* class_() {return Widget::class_();}
    enum
    {
        styleRC_     = WS_GROUP | Widget::styleRC_,
        styleMaskRC_ = WS_GROUP | Widget::styleMaskRC_
    };

    int value() const
    {
        int j = 0;
        for (iterator i = list.begin(); i != list.end(); ++j, ++i)
            if (::SendMessage(*i, BM_GETCHECK, 0, 0))
                return j;
        return ~0;
    }

    void value(int v)
    {
        int j = 0;
        for (iterator i = list.begin(); i != list.end(); ++i)
            ::SendMessage(*i, BM_SETCHECK, (v == j++), 0);
    }

    void ctor(Parent* parent, Handle h)
    {
        handle   = h;
        Handle w = handle;
        Handle p = parent->window()->handle;
        do
        {
            Window(w).object((Base*) this);
            list.add(w);
            w = ::FindWindowEx(p, h, WC_BUTTON, 0);
        } while (h
            && ((::GetWindowLong(h, GWL_STYLE)
                & styleMaskRC_) == Widget::styleRC_));
    }

private:
    typedef List<Handle>   List;
    typedef List::iterator iterator;
    List list;
};

// ............................................................................

struct ColorWell : Base
{
    static const char* class_() {return WC_STATIC;}

    enum
    {
        styleRC_     = SS_NOTIFY
                     | SS_GRAYRECT
                     | SS_OWNERDRAW,
        styleMaskRC_ = styleRC_ & ~WS_TABSTOP,
        style_       = styleRC_ | WS_TABSTOP
    };

    int value() const {return value_;}

    void value(int v)
    {
        value_ = v;
        ::InvalidateRect(handle, 0, 0);
    }

    static COLORREF& custom(int index)
    {
        static COLORREF aux[16] = {0};
        return aux[index];
    }

    ColorWell() : value_(0xC0DE) {}

protected:

    void action(int)
    {
        CHOOSECOLOR cc;
        cc.lStructSize    = sizeof(cc);
        cc.hwndOwner      = handle;
        cc.hInstance      = 0;
        cc.rgbResult      = bgr(value());
        cc.lpCustColors   = &custom(0);
        cc.Flags          = CC_RGBINIT | CC_FULLOPEN;
        cc.lCustData      = 0;
        cc.lpfnHook       = 0;
        cc.lpTemplateName = 0;

        ::ChooseColor(&cc);
        int v = abgr(cc.rgbResult) | 0xFF000000;
        if (v != value())
        {
            value(v);
            callback(v);
        }
    }

    void draw(DRAWITEMSTRUCT* ds)
    {
        HBRUSH brush
            = ::CreateSolidBrush(bgr(value()));
        ::DrawEdge(ds->hDC, &ds->rcItem,
            BDR_SUNKENOUTER, BF_RECT | BF_ADJUST);
        ::FillRect(ds->hDC, &ds->rcItem, brush);
        ::DeleteObject(brush);
    }

    static int abgr(int v) // ARGB <-> ABGR
    {
        return (v & 0xFF00FF00)
            | ((v & 0x00FF0000) >> 16)
            | ((v & 0x000000FF) << 16);
    }

    static int bgr(int v) // (A)RGB -> BGR
    {
        return abgr(v) & 0x00FFFFFF;
    }

protected:
    int value_;
};

// ............................................................................

struct Tabs : Base
{
    static const char* class_() {return WC_TABCONTROL;}

    enum {style_ = WS_TABSTOP};

    int  value() const {return msg(TCM_GETCURSEL, 0, 0);}
    void value(int v)  {msg(TCM_SETCURSEL, v, 0);}
    int  range() const {return msg(TCM_GETITEMCOUNT, 0, 0);}
    void range(int)    {}

    int  add(const char* text) {return add(text, 0);}
    int  data(int i) const     {return data<int>(i);}

protected:

    template <typename T>
    int add(const char* text, const T& v)
    {
        TCITEM ti  = {0};
        ti.mask    = TCIF_TEXT | TCIF_PARAM;
        ti.pszText = (char*) text;
        ti.lParam  = LPARAM(v);
        // fixme, not sure about win64 (MSDN says *must* be 4 bytes)
        // typedef char DataShouldBe4Bytes[sizeof(T) == 4];
        return msg(TCM_INSERTITEM, range(), (LPARAM) &ti);
    }

    template <typename T>
    T data(int i) const
    {
        TCITEM ti = {0};
        ti.mask   = TCIF_PARAM;
        msg(TCM_GETITEM, i, (LPARAM) &ti);
        return T(ti.lParam);
    }
};

// ............................................................................

struct LayerTabs : Tabs
{
    int value() const         {return Tabs::value();}
    int add(const char* text) {return Tabs::add(text, 0);}

    void value(int v)
    {
        Tabs::value(v);
        selectWindow();
    }

    int add(const char* text, Window* window)
    {
        int ret = Tabs::add(text, window->handle);
        alignWindow(window);
        ::ShowWindow(window->handle,
            ret ? SW_HIDE : SW_SHOW);
        return ret;
    }

    LayerTabs() :
        IsAppThemed("uxtheme", "IsAppThemed"),
        EnableThemeDialogTexture("uxtheme", "EnableThemeDialogTexture")
    {}

protected:

    void action(int)
    {
        selectWindow();
        callback(value());
    }

    void selectWindow() const
    {
        for (int i = 0; i < range(); i++)
            ::ShowWindow(data<HWND>(i),
                (i == value()) ? SW_SHOW : SW_HIDE);
    }

    void alignWindow(Window* window)
    {
        RECT  r = {0, 0, 0, 0};
        msg(TCM_ADJUSTRECT, FALSE, (LPARAM) &r);
        Point p = position();
        window->position(p.x + r.left, p.y + r.top);
        Size  s = window->size();
        Size  z = size();
        size(max<int>(z.w, s.w + r.left - r.right),
             max<int>(z.h, s.h + r.top - r.bottom));

        if (IsAppThemed && IsAppThemed())
            EnableThemeDialogTexture(window->handle, 6);
    }

private:
    typedef BOOL    WINAPI F1(VOID);
    typedef HRESULT WINAPI F2(HWND, DWORD);
    details::DynamicApi<F1> IsAppThemed;
    details::DynamicApi<F2> EnableThemeDialogTexture;
};

// ............................................................................

struct ImageList
{
    ImageList(int n, const char* rc)
    {
        BITMAP bi;
        HANDLE image = ::LoadImage(app->module(),
            rc, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
        if (!image)
            trace("%s: LoadImage(\"%s\") failed [%i]\n",
                FUNCTION_, rc, ::GetLastError());

        ::GetObject(image, sizeof(bi), &bi);
        handle = ::ImageList_Create(bi.bmWidth / n,
            bi.bmHeight, ILC_COLOR32, n, 0);
        ::ImageList_Add(handle, (HBITMAP) image, 0);
        ::DeleteObject(image);
    }

    ~ImageList() {::ImageList_Destroy(handle);}

public:
    HIMAGELIST handle;
};

// ............................................................................

struct Toolbar : Base
{
    static const char* class_() {return TOOLBARCLASSNAME;}

    enum
    {
        styleRC_     = BTNS_BUTTON,
        styleMaskRC_ = styleRC_,
        style_       = CCS_NOPARENTALIGN | TBSTYLE_TRANSPARENT
                     | CCS_NODIVIDER | CCS_NORESIZE | TBSTYLE_FLAT
    };

    int  range() const {return msg(TB_BUTTONCOUNT, 0, 0);}
    void range(int)    {}

    void add(int n, const char* off, const char* on = 0)
    {
        delete imgOff;
        imgOff = new ImageList(n, off);
        msg(TB_SETIMAGELIST, 0, (LPARAM) imgOff->handle);

        if (on)
        {
            delete imgOn;
            imgOn = new ImageList(n, on);
            msg(TB_SETHOTIMAGELIST, 0, (LPARAM) imgOn->handle);
        }

        int x = size().h;
        msg(TB_SETBUTTONSIZE, 0, MAKELONG(x + 1, x));

        TBBUTTON b;
        b.fsState = TBSTATE_ENABLED;
        b.fsStyle = BTNS_BUTTON;
        b.iString = 0;
        b.dwData  = 0;

        for (int i = 0; i < n; i++)
        {
            b.iBitmap = b.idCommand = i;
            msg(TB_ADDBUTTONS, 1, (LPARAM) &b);
        }
    }

    void ctor(Parent* parent, Handle h)
    {
        Base::ctor(parent, h);
        msg(TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
    }

    ~Toolbar()
    {
        delete imgOff;
        delete imgOn;
    }

    Toolbar() : imgOff(0), imgOn(0) {}

protected:
    ImageList* imgOff;
    ImageList* imgOn;

protected:

    void action(int index) {callback(index);}
};

// ............................................................................

struct ResourceCtor
{
private:

    struct Aux
    {
        Aux(Parent* parent, int tag) : parent(parent), tag(tag)
        {
            handle  = ::GetDlgItem(parent->window()->handle, tag);
            if (!handle)
                trace("kali::ResourceCtor: failed"
                    " to find widget with tag %i\n", tag);

            *class_ = 0;
            ::GetClassName(handle, class_, sizeof(class_));
            style = ::GetWindowLong(handle, GWL_STYLE);
        }

        operator Ptr<Interface> () const
        {
            static const Make make[] =
            {
                make_<ColorWell>,
                make_<Stepper>,
                make_<Button>,
                make_<Toggle>,
                make_<Group>,
                make_<Fader>,
                make_<Meter>,
                make_<Combo>,
                make_<Edit>,
                make_<Text>,
                make_ // Base
            };

            int   i      = 0;
            Base* widget = 0;
            while (!widget)
                widget = make[i++](this);

            widget->ctor(parent, handle);
            parent->attach(widget);
            return widget;
        }

        template <typename T>
        operator Ptr<T> () const
        {
            if (!equal<T>())
                trace("kali::ResourceCtor: widget(%i) type (class/style)"
                    " mismatch: [%s, %x] vs. [%s, %x]\n", tag, T::class_(),
                    T::styleRC_, class_, style & T::styleMaskRC_);

            T* widget = new T();
            widget->ctor(parent, handle);
            parent->attach(widget);
            return widget;
        }

    private:

        typedef Base* (*Make)(const Aux*);
        template <typename T> operator T () const;

        template <typename T>
        static Base* make_(const Aux* a) {return a->equal<T>() ? new T() : 0;}
        static Base* make_(const Aux*)   {return new Base;}

        template <typename T>
        bool equal() const
        {
            return ((style & T::styleMaskRC_) == T::styleRC_)
                && !strcmp(T::class_(), class_);
        }

    private:
        Base::Handle handle;
        Parent*      parent;
        int          tag;
        int          style;
        char         class_[64];
    };

public:
    explicit ResourceCtor(Parent* parent) : parent(parent) {}
    Aux operator() (int tag) const {return Aux(parent, tag);}

private:
    Parent* parent;
};

// ............................................................................
// todo: rework both so they won't conflict:

template <typename T> inline
typename T::Type* Ctor(Parent* parent, const Rect& r, const char* text)
{
    typedef T::Type Widget;

    Widget::Handle handle = CreateWindowEx
       (Widget::styleEx_, Widget::class_(), text,
        WS_CHILD | WS_VISIBLE | Widget::style_,
        r.x, r.y, r.w, r.h, parent->window()->handle,
        0, ::GetModuleHandle(0), 0);

    if (handle)
    {
        ::SendMessage(handle, WM_SETFONT,
            (WPARAM) (HFONT) Font::main(), 0);
        Widget* widget = new Widget();
        widget->ctor(parent, handle);
        parent->attach(widget);
        return widget;
    }

    trace("%s: CreateWindow failed [%i]\n", FUNCTION_, ::GetLastError());
    return 0;
}

#if KALI_UI_NATIVE_WIDGETS_BRUTE_CTOR

template <typename T> inline
typename T::Type* Ctor(const Window* parent, const Rect& r, const char* text = 0)
{
    typedef T::Type Widget;

    Widget::Handle handle = CreateWindow(Widget::class_(),
        text, WS_CHILD | WS_VISIBLE | Widget::style_,
        r.x, r.y, r.w, r.h, parent->handle,
        (HMENU) 343, app->module(), 0);

    if (handle)
    {
        ::SendMessage(handle, WM_SETFONT,
            (WPARAM) (HFONT) Font::main(), 0);
        Widget* widget = new Widget();
        widget->ctor(0 /* fixme */, handle);
        return widget;
    }

    trace("%s: CreateWindow failed [%i]\n", FUNCTION_, ::GetLastError());
    return 0;
}

#endif // ~ KALI_UI_NATIVE_WIDGETS_BRUTE_CTOR

// ............................................................................

} // ~ namespace widget
} // ~ namespace native
} // ~ namespace ui

// ............................................................................

using ui::native::Timer;
using ui::native::WaitCursor;

// ............................................................................

} // ~ namespace kali

// ............................................................................

#endif // ~ KALI_UI_NATIVE_WIDGETS_INCLUDED
