
#ifndef KALI_UI_BASE_INCLUDED
#define KALI_UI_BASE_INCLUDED

// #include "kali/graphics.h"

// ............................................................................

namespace kali {
namespace ui   {

// ............................................................................

struct Layer : ReleaseAny
{
    virtual bool open()  = 0;
    virtual void close() = 0;

    enum style
    {
        UsesGraphics = 0,
        SysCaption   = 1,
        DropShadow   = 0,
    };
};

namespace widget
{
    struct Interface : ReleaseAny
    {
        virtual bool enable() const  = 0;
        virtual void enable(bool)    = 0;
        virtual bool visible() const = 0;
        virtual void visible(bool)   = 0;
        virtual int  value() const   = 0;
        virtual void value(int v)    = 0;
    };
}

// ............................................................................

namespace custom {

// ............................................................................

struct Respondent
{
    virtual bool key(UpDown, int)        = 0;
    virtual bool mouse(UpDown, int, int) = 0;
    virtual bool mouseMove(int, int)     = 0;
    virtual Rect drawRect() const        = 0;

#if defined KALI_GRAPHICS_INCLUDED
    virtual void draw(Context&)          = 0;
#endif

protected:

    virtual ~Respondent() {}
};

struct Layer : ui::Layer, Respondent
{
    virtual Rect titlebar() const = 0;
};

// ............................................................................

} // ~ namespace custom
} // ~ namespace ui
} // ~ namespace kali

// ............................................................................

#endif // ~ KALI_UI_BASE_INCLUDED
