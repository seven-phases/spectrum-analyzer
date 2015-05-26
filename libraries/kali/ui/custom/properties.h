
#ifndef KALI_UI_CUSTOM_PROPERTIES_INCLUDED
#define KALI_UI_CUSTOM_PROPERTIES_INCLUDED

#include "kali/ui/custom/widgets.h"

#if 0
#define Prefix Px
#define Null _
struct  Null {};
#endif

// ............................................................................

namespace kali     {
namespace ui       {
namespace custom   {
namespace property {

// ............................................................................

template <typename Unique, typename T = Null>
struct Prefix
{
    template <typename U>
    Prefix <Prefix, U> operator / (const U& v) const {return Prefix<Prefix, U>(v);}

    template <typename U> static
    Null unwrap(const U&) {return Null();}

    template <typename U> static
    const U& unwrap(const Prefix<Prefix, U>& v) {return v.v_;}

    Prefix() : v_(v_) {}

private:
    const T& v_;
    Prefix(const T& v) : v_(v) {}
    Prefix& operator = (const Prefix&);
    template <typename, typename> friend struct Prefix;
};

// ............................................................................

template <typename T, typename Unique = Undefined>
struct Value
{
    T value;
    Value() : value() {}
    Value(const T& value) : value(value) {}
    operator const T& () const {return value;}

    template <typename U>
    static Null unwrap(const U&) {return Null();}
    static const T& unwrap(const Value& v) {return v;}
};

// ............................................................................

template <typename Widget>
struct Ctor : Prefix <Widget>
{
private:

    typedef widget::Parent Parent;

    struct Aux
    {
        Aux(Parent* parent, Widget* widget)
            : parent(parent), widget(widget) {}

        template <typename T>
        operator Ptr<T> () const {return widget;}

        template <typename T>
        const Aux& operator - (const T& value) const
        {
            widget->property(value, parent);
            return *this;
        }

    private:
        Parent* parent;
        Widget* widget;
    };

    Aux aux() const
    {
        Widget* widget = new Widget;
        parent->attach(widget, true);
        return Aux(parent, widget);
    }

public:

    template <typename T>
    Aux operator - (const T& value) const {return aux() - value;}

    template <typename A> Aux
    operator () (const A& a) const {return aux() - a;}
    template <typename A, typename B> Aux
    operator () (const A& a, const B& b) const {return aux() - a - b;}
    template <typename A, typename B, typename C> Aux
    operator () (const A& a, const B& b, const C& c) const {return aux() - a - b - c;}

    template <typename T> operator Ptr<T> () const {return aux();}

    Ctor() : parent(0) {}
    explicit Ctor(Parent* parent) : parent(parent) {}

    // temporary:
    Aux clone(Widget* w) const
    {
        Widget* widget = new Widget(*w);
        parent->attach(widget, true);
        return Aux(parent, widget);
    }

private:
    Parent* parent;
};

// ............................................................................
// properties:

const Prefix <struct on>    on;
const Prefix <struct off>   off;
const Prefix <struct bound> bound;
const Prefix <struct thumb> thumb;
const Prefix <struct menu>  menu;

enum Lazy  {nonlazy,  lazy};
enum Alias {nonalias, alias};

typedef Value <double, struct angle_>    angle;
typedef Value <int,    struct range_>    range;
typedef Value <int,    struct contrast_> contrast;

// ............................................................................

namespace details {

// ............................................................................

using namespace property;

const Ctor <widget::Font> font;
const Ctor <widget::Text> text;

// ............................................................................

#define tf // trace("%s\n", FUNCSIG_);

template <typename U, typename T> void apply(U&, const T&) {}
template <typename T> void apply(T& ref, const T& value) {tf ref = value;}

// "specializations" for common property conversions/aliases
inline void apply(const char*& ref, const char* value) {tf ref = value;}
inline void apply(Rect& ref, const Size& value)        {tf ref.size(value);}
inline void apply(Rect& ref, const Point& value)       {tf ref.point(value);}
inline void apply(Point& ref, const Rect& value)       {tf ref = value.point();}

#if 0
// disabled due to internal XCode3/GCC4.0 error in Function::$Aux::clone()
// (but it does compile fine with XCode2.x/GCC4.0 and XCode3/GCC4.2)
inline void apply(widget::ActionBase* ptr, const Ftor& value) {tf ptr->callback = value;}
#endif

// apply-to-function "specializations" (experimental):
/*
template <typename W, typename T>
void apply(W* widget, void (W::*f)(T), const T& value) {(widget->*f)(value);}

template <typename W, typename F, typename T>
void apply(W*, F, const T&) {}
*/

#undef tf

// ............................................................................

} // ~ namespace details
} // ~ namespace property
} // ~ namespace custom
} // ~ namespace ui
} // ~ namespace kali

// ............................................................................

#ifdef Prefix
#undef Prefix
#undef Null
#endif

#endif // KALI_UI_CUSTOM_PROPERTIES_INCLUDED
