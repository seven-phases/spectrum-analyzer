
#ifndef KALI_UI_CUSTOM_WIDGETS_INCLUDED
#define KALI_UI_CUSTOM_WIDGETS_INCLUDED

#include "kali/ui/native/widgets.h"
#include "kali/ui/custom/widgets.base.h"
#include "kali/ui/custom/properties.h"

// ............................................................................

namespace kali    {
namespace ui      {
namespace custom  {
namespace widget  {

using namespace property::details;
namespace px =  property::details;

// ............................................................................

template <typename BaseType>
struct Unit_ : BaseType
{
    Rect rect() const {return drawRect();}
    void draw(Context& c, const Point& p) {c.draw(image, xy + p);}
    void draw(Context& c, const Point& p, bool t) {c.draw(image, xy + p, t);}

    template <typename T>
    void property(const T& value, Parent*)
    {
        apply(xy, value);
    }

    void property(const char* value, Parent* parent)
    {
        parent->attach(&*(image = new Image_(value)), true);
    }

protected:

    Rect drawRect() const {return Rect(xy, image->size());}
    void draw(Context& context) {context.draw(image, xy);}

private:
    typedef graphics::Image Image_;
    Ptr<Image_> image;
    Point xy;
};

typedef Unit_ <Null> Unit;
typedef Unit_ <Base> Image;

// ............................................................................

template <typename BaseType>
struct UnitPair : BaseType
{
    Rect  tooltipRect() const {return rect;}
    const Point xy() const {return rect.point();}

    template <typename T>
    void property(const T& value, Parent* parent)
    {
        u[0].property(off.unwrap(value), parent);
        u[1].property(on.unwrap(value),  parent);
        apply(rect, value);
        apply((BaseType* const&) this, value);
    }

protected:
    // void draw(Context& c) {u[this->value()].draw(c, xy());}
    void draw(Context& c) {u[this->value()].draw(c, xy(), this->enable());}
    Rect drawRect() const {return (u[0].rect() | u[1].rect()) + xy();}

protected:
    Unit u[2];
    Rect rect;
};

typedef UnitPair <Base> Lamp;

// ............................................................................

struct Font : ReleaseAny
{
    template <typename T>
    void property(const T& value, Parent*)
    {
        apply(name,      value);
        apply(size,      value);
        apply(weight,    value);
        apply(colour,    value);
        apply(scale,     value);
        apply(contrast_, value);
        apply(alias,     value);
    }

    Font() :
        cooked(0),
        name("Arial"),
        size(32.),
        weight(),
        colour(0, 0, 0),
        scale(1.0, 1.0),
        contrast_(~0),
        alias(nonalias)
    {};

    void cook_(Parent* parent)
    {
        if (cooked)
            return;
        parent->attach(&*(font = new Font_(name, size, weight)), true);
        cooked = true;
    }

protected:
    friend      struct graphics::Context;
    typedef     graphics::Font Font_;
    Ptr<Font_>  font;
    bool        cooked;
    const char* name;
    double      size;
    Weight      weight;
    Colour      colour;
    Scale       scale;
    contrast    contrast_; // win: 0...12; mac: unused
    Alias       alias;
};

// ............................................................................

struct Text : TextBase
{
    template <typename T>
    void property(const T& value, Parent* parent)
    {
        font.property(px::font.unwrap(value), parent);
        apply(rect,  value);
        apply(align, value);
    }

    void property(const Ptr<Font>& value, Parent*)
    {
        font = value;
    }

    Text() : align() {}

protected:

    Rect drawRect() const {return rect;}

    void draw(Context& context)
    {
        font.cook_(parent_);
        // context.drawFrame(rect);
        context.draw(text_, font, rect, align);
    }

protected:
    Font  font;
    Rect  rect;
    Align align;
};

// ............................................................................

struct Button : UnitPair <ActionBase>
{
protected:

    bool mouse(UpDown e, int x, int y)
    {
        if (!rect.contains(x, y))
            return false;

        value(e, !e);
        return true;
    }

    bool mouseMove(int x, int y)
    {
        if (rect.contains(x, y))
            return true;

        value(up);
        return false;
    }
};

// ............................................................................

struct Radio : UnitPair <ActionBase>
{
protected:

    bool mouse(UpDown e, int x, int y)
    {
        if (!rect.contains(x, y))
            return false;
        if (e)
            value(!value(), true);
        return true;
    }
};

typedef ExclGroup <Radio> Radios;

// ............................................................................
// Fader, TODO: add en/dis for click-inc/dec

struct Fader : ActionBase
{
    Fader() : lazy(), tweak() {}

    void inc() {value(value() + 1, true);}
    void dec() {value(value() - 1, true);}

    template <typename T>
    void property(const T& value, Parent* parent)
    {
        thumb.property(px::thumb.unwrap(value), parent);
        apply(rect,   value);
        apply(bound,  px::bound.unwrap(value));
        apply(range_, range::unwrap(value));
        apply(lazy,   value);
    }

protected:

    bool direction() const {return bound.w > bound.h;}
    Rect tooltipRect() const {return bound + rect.point();}
    void draw(Context& context) {thumb.draw(context, act().point());}

    Rect drawRect() const
    {
        Rect r(thumb.rect());
        r.x += rect.x;
        r.y += rect.y;
        r.w += bound.w - rect.w;
        r.h += bound.h - rect.h;
        return r;
    }

    bool mouse(UpDown e, int x, int y)
    {
        if (!e && tweak)
        {
            tweak = false;
            if (lazy)
                callback(value());
        }

        if (act().contains(x, y))
        {
            if (e)
            {
                tweak = true;
                refp = direction() ? x : y;
                refv = direction() ?
                    value() : range_ - value();
            }

            return true;
        }

        /*if (e && Rect(rect.point(),
            bound.size()).contains(x, y))
        {
            int v = (range_ + 9) / 10;
            if ((direction() && (x < act().x)) ||
               (!direction() && (y > act().y)))
                    v = -v;

            value(value() + v, true);
            return true;
        }*/

        return false;
    }

    bool mouseMove(int x, int y)
    {
        int v = refv;
        if (direction())
            v += ((x - refp) * range_)
                / (bound.w - rect.w);
        else
        {
            v += ((y - refp) * range_)
                / (bound.h - rect.h);
            v = range_ - v;
        }

        value(v, !lazy);
        return true;
    }

    Rect act() const
    {
        Rect r(rect);
        if (direction())
            r.x += ((bound.w - rect.w)
                * value()) / range_;
        else
            r.y += ((bound.h - rect.h)
                * (range_ - value())) / range_;

        return r;
    }

    #if 0
    bool key(UpDown e, int code)
    {
        switch (e * code)
        {
        case VK_LEFT:
        case VK_DOWN:
            dec();
            return true;
        case VK_RIGHT:
        case VK_UP:
            inc();
            return true;
        default:
            return false;
        }
    }
    #endif

protected:
    Unit thumb;
    Rect rect;
    Rect bound;
    Lazy lazy;
    bool tweak;
    int  refp;
    int  refv;
};

// ............................................................................

struct Combo : ActionBase
{
    using ActionBase::enable;
    using ActionBase::visible;
    using ActionBase::value;

    void enable(bool v)
    {
        button     .enable(v);
        text_      .enable(v);
        ActionBase::enable(v);
    }

    void visible(bool v)
    {
        button     .visible(v);
        text_      .visible(v);
        ActionBase::visible(v);
    }

    void value(int v)
    {
        if (value_ != v)
            text_.text(menu.text(value_ = v));
    }

    void text(const char* v)
    {
        value_ = ~0;
        text_.text(v);
    }

    void operator += (const char* item)
    {
        ++range_;
        menu += item;
    }

    template <typename T>
    void property(const T& value, Parent* parent)
    {
        button.property(value, parent);
        text_.property(px::text.unwrap(value), parent);
        apply(menuXY, px::menu.unwrap(value));
    }

    Combo()
    {
        value_ = ~0;
        range_ =  0;
    }

protected:

    Rect tooltipRect() const {return button.tooltipRect();}

    void show()
    {
        menu.show(parent_->window(),
            button.xy() + menuXY, value());
    }

    void ctor(Parent* p)
    {
        Base::ctor(p);
        parent_->attach(&button, false);
        parent_->attach(&text_, false);
        button.callback.to(this, &Combo::show);
        menu.callback.to(this, &ActionBase::value, true);
    }

protected:
    typedef native::Menu Menu;
    Button  button;
    Menu    menu;
    Text    text_;
    Point   menuXY;
};

// ............................................................................

// experimental:

struct Brush : Base
{
    template <typename T>
    void property(const T& value, Parent*)
    {
        apply(colour[0], off.unwrap(value));
        apply(colour[1], on.unwrap(value));
        apply(angle,     angle::unwrap(value));
        apply(rect,      value);
    }

protected:
    Rect drawRect() const {return rect;}
    void draw(Context& context) {context.fillRect(this);}

protected:
    friend struct graphics::Context;
    Colour colour[2];
    double angle;
    Rect   rect;
};

// ............................................................................

} // ~ namespace widget
} // ~ namespace custom
} // ~ namespace ui
} // ~ namespace kali

// ............................................................................

#endif // ~ KALI_UI_CUSTOM_WIDGETS_INCLUDED
