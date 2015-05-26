

#ifndef KALI_UI_CUSTOM_WIDGETS_BASE_INCLUDED
#define KALI_UI_CUSTOM_WIDGETS_BASE_INCLUDED

#include "kali/function.h"
#include "kali/containers.h"
#include "kali/ui/base.h"

// ............................................................................

namespace kali   {
namespace ui     {
namespace custom {
namespace widget {

// ............................................................................

struct Interface : ui::widget::Interface, Respondent
{
    virtual void ctor(struct Parent*) = 0;
    virtual void tooltip(const char*) = 0;
};

struct Parent
{
    virtual Window* window()                          = 0;
    virtual void attach(Interface*, bool)             = 0;
    virtual void attach(ReleaseAny*, bool)            = 0;
    virtual void addToolTip(const Rect&, const char*) = 0;
    virtual ~Parent() {}
};

// ............................................................................

struct Base : Interface
{
    void ctor(Parent* parent)    {parent_ = parent;}

    bool enable() const  {return get(enable_);}
    void enable(bool v)  {return set(enable_, v);}
    bool visible() const {return get(visible_);}
    void visible(bool v) {return set(visible_, v);}
    int  value() const   {return get(value_);}
    void value(int v)    {return set(value_, v);}

    template <typename T>
    void property(const T&, Parent*) {}

    void tooltip(const char* s)
    {
        parent_->addToolTip(tooltipRect(), s);
    }

protected:

    bool key(UpDown, int)        {return false;}
    bool mouse(UpDown, int, int) {return false;}
    bool mouseMove(int, int)     {return false;}
    Rect drawRect() const        {return Rect();}
    void draw(Context&)          {}

    virtual Rect tooltipRect() const {return drawRect();}

    template <typename T>
    void redraw(T) {parent_->window()->redraw(drawRect());}

    template <typename T>
    static const T& get(const T& ref) {return ref;}

    template <typename T>
    void set(T& ref, const T& v)
    {
        if (ref != v)
            redraw(ref = v);
    }

    Base() :
        enable_  (1),
        visible_ (1),
        value_   (0),
        range_   (1),
        parent_  (0)
    {}

protected:
    bool     enable_;
    bool     visible_;
    int      value_;
    int      range_;
    Parent*  parent_;
};

// ............................................................................

struct ActionBase : Base, UsesCallback
{
    using Base::value;

    void value(int v, bool callback_)
    {
        v = min(max(v, 0), range_);
        if (v != value())
        {
            value(v);
            if (callback_)
                callback(value());
        }
    }
};

// ............................................................................

struct TextBase : Base
{
    const char* text() const {return text_;}
    void text(const char* v) {redraw(text_ = v);}

protected:
    string text_;
};

// ............................................................................

template <typename BaseType>
struct List : BaseType
{
    void add(Interface* widget) {list.add(widget);}

    bool key(UpDown e, int code)
    {
        return keyFocus_ && keyFocus_->key(e, code);
    }

    bool mouse(UpDown e, int x, int y)
    {
        Ptr<Interface> aux;
        for (iterator i = list.end(); i != list.begin();)
            if ((*--i)->visible() && (*i)->enable() && (*i)->mouse(e, x, y))
                return (e ? mouseFocus_ : aux) = keyFocus_ = *i;
        return mouseFocus_ = 0;
    }

    bool mouseMove(int x, int y)
    {
        if (mouseFocus_
            && mouseFocus_->mouseMove(x, y))
                return true;
        return mouseFocus_ = 0;
    }

    void draw(Context& context)
    {
        // const Rect r(context.rect());
        for (iterator i = list.begin(); i != list.end(); ++i)
            if ((*i)->visible() /*&& (*i)->drawRect().intersects(r)*/)
                (*i)->draw(context);
    }

    Rect drawRect() const
    {
        Rect r;
        for (iterator i = list.begin(); i != list.end(); ++i)
            r |= (*i)->drawRect();
        return r;
    }

    void clear()
    {
        keyFocus_   = 0;
        mouseFocus_ = 0;
        list.clear();
    }

private:
    Ptr<Interface> keyFocus_;
    Ptr<Interface> mouseFocus_;
    kali::List<Interface*> list;

    typedef kali::List<Interface*>
        ::iterator iterator;
};

// ............................................................................

template <class T>
struct ExclGroup : ActionBase
{
    using ActionBase::value;
    using ActionBase::enable;

    void operator += (Ptr<T> widget) {add(widget);}

    void add(Ptr<T> widget)
    {
        widget->callback.to(this,
            &ExclGroup::changed, range_++);
        list.add(&*widget);
    }

    void value(int v)
    {
        value_ = v;
        int j  = 0;
        for (iterator i = list.begin(); i != list.end(); ++i)
            (*i)->value(v == j++);
    }

    void enable(bool v)
    {
        for (iterator i = list.begin(); i != list.end(); ++i)
            (*i)->enable(v);
    }

    ExclGroup()
    {
        value_ = ~0;
        range_ =  0;
    }

private:

    void changed(int v, int index)
    {
        v ? value(index, true)
            : list[index]->value(1);
    }

private:
    kali::List<T*> list;

    typedef typename kali::List<T*>
        ::iterator iterator;
};

// ............................................................................

struct Font;
struct Text;

// ............................................................................

} // ~ namespace widget
} // ~ namespace custom
} // ~ namespace ui
} // ~ namespace kali

// ............................................................................

#endif // ~ KALI_UI_CUSTOM_WIDGETS_BASE_INCLUDED
