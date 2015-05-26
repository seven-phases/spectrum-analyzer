
#ifndef KALI_UI_CUSTOM_INCLUDED
#define KALI_UI_CUSTOM_INCLUDED

// #include "kali/app.h"
#include "kali/settings.h"
#include "kali/ui/custom/widgets.h"

// ............................................................................

namespace kali   {
namespace ui     {
namespace custom {

// ............................................................................

typedef Ptr <widget::Interface> AnyWidget;

typedef Ptr <widget::Button>    Button;
typedef Ptr <widget::Radios>    Radios;
typedef Ptr <widget::Radio>     Radio;
typedef Ptr <widget::Fader>     Fader;
typedef Ptr <widget::Combo>     Combo;
typedef Ptr <widget::Image>     Image;
typedef Ptr <widget::Lamp>      Lamp;
typedef Ptr <widget::Font>      Font;
typedef Ptr <widget::Text>      Text;

// ............................................................................

struct LayerBase :
    Window,
    widget::Parent,
    widget::List <Layer>
{
    enum style {DropShadow, UsesGraphics};
    Rect titlebar() const {return Rect();}
    void close() {}

    // fixme, inherited objects are already destroyed
    // here so they won't receive their close()
    ~LayerBase() {this->destroy();}

    bool open()
    {
        Window::size(this->drawRect().size());
        return true;
    }

    void clear()
    {
        tooltips.clear(this);
        widget::List<Layer>::clear();
        autorelease.release();
    }

private:

    Window* window() {return this;}
    void attach(ReleaseAny* any, bool) {autorelease(any);}

    void attach(widget::Interface* widget, bool autorelease_)
    {
        this->add(widget);
        widget->ctor(this);
        if (autorelease_)
            autorelease(widget);
    }

    void addToolTip(const Rect& rect, const char* text)
    {
        tooltips.attach(this, rect, text);
    }

protected:
    AutoReleasePool<> autorelease;

private:
    native::TooltipSupport tooltips;
};

// ............................................................................

struct WindowBase : LayerBase
{
    enum style {SysCaption, DropShadow = 1};

    bool open()
    {
        LayerBase::open();
        centerAt();
        return true;
    }

    Rect titlebar() const {return titlebar_;}

    void titlebar(int x, int y, int w, int h)
    {
        titlebar_ = Rect(x, y, w, h);
    }

    void centerAt(const Window* at = 0)
    {
        Rect r = !at ? Rect(screenSize())
            : Rect(at->position(), at->size());
        Size s = Window::size();
        r.x += (r.w - s.w) / 2;
        r.y += (r.h - s.h) / 2;
        Window::position(r.x, r.y);
    }

    void restorePosition(const Settings& settings,
        const char* prefix = "", Align align = left, bool exact = 0)
    {
        Size s = screenSize();
        int  x = s.w;
        int  y = s.h;
        int  a = 1 + (4 >> align);
        if (exact)
        {
            s = Window::size();
            x = 2 * (x - s.w);
            y = 2 * (y - s.h);
            a = a + 1;
        }

        x = settings.get(string("%sx", prefix), x / a);
        y = settings.get(string("%sy", prefix), y / a);
        Window::position(x, y);
    }

    void storePosition(const Settings& settings,
        const char* prefix = "") const
    {
        Point p = Window::position();
        settings.set(string("%sx", prefix), p.x);
        settings.set(string("%sy", prefix), p.y);
    }

private:
    Rect titlebar_;
};

// ............................................................................

} // ~ namespace custom
} // ~ namespace ui
} // ~ namespace kali

// ............................................................................

#endif // ~ KALI_UI_CUSTOM_INCLUDED
