
#ifndef KALI_UI_NATIVE_INCLUDED
#define KALI_UI_NATIVE_INCLUDED

#include "kali/app.h"
#include "kali/ui/native/widgets.h"

// ............................................................................

namespace kali   {
namespace ui     {
namespace native {

// ............................................................................

typedef Ptr <widget::Interface> AnyWidget;

typedef Ptr <widget::LayerTabs> LayerTabs;
typedef Ptr <widget::ColorWell> ColorWell;
typedef Ptr <widget::TextRight> TextRight;
typedef Ptr <widget::TextCopy>  TextCopy;
typedef Ptr <widget::Toolbar>   Toolbar;
typedef Ptr <widget::NumEdit_>  NumEdit_; // deprecated
typedef Ptr <widget::Stepper>   Stepper;
typedef Ptr <widget::Button>    Button;
typedef Ptr <widget::Toggle>    Toggle;
typedef Ptr <widget::Fader>     Fader;
typedef Ptr <widget::Meter>     Meter;
typedef Ptr <widget::Combo>     Combo;
typedef Ptr <widget::Break>     Break;
typedef Ptr <widget::Edit>      Edit;
typedef Ptr <widget::Text>      Text;

// ............................................................................

struct LayerBase : Window, ui::Layer, widget::Parent
{
    enum style       {SysCaption, DropShadow = 0};
    Window* window() {return this;}
    bool open()      {return true;}
    void close()     {}

    // temporary, move out here:
    bool draw()                {return false;}
    void resized()             {}
    bool mouseDoubleClick()    {return false;}
    bool mouseMove(int, int)   {return false;}
    bool keyDown(int, int)     {return false;}
    bool mouse(int, int, int)  {return false;}
    bool mouseR(int, int, int) {return false;}

    bool msgHook(LRESULT&, UINT, WPARAM, LPARAM) {return false;}

    // NOTE: inherited objects are already destroyed
    // here so they won't receive their close()
    ~LayerBase() {this->destroy();}

private:
    void attach(ReleaseAny* any) {autorelease(any);}

protected:
    AutoReleasePool<> autorelease;
};

// ............................................................................

struct WindowBase : LayerBase
{
    enum style {SysCaption = 1};

    bool open()
    {
        LayerBase::open();
        centerAt();
        return true;
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
};

// ............................................................................

} // ~ namespace native
} // ~ namespace ui
} // ~ namespace kali

// ............................................................................

#endif // ~ KALI_UI_NATIVE_INCLUDED
