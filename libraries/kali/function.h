
#ifndef KALI_FUNCTION_INCLUDED
#define KALI_FUNCTION_INCLUDED

// ............................................................................

namespace kali    {
namespace details {

// ............................................................................

template <typename R, typename T>
struct FuncBase
{
    virtual R operator () (T) const {return R();}
    // virtual FuncBase* clone() const {return this;}
    virtual ~FuncBase() {}
};

template <typename T, typename F> struct FuncAux;

template <typename R, typename T, typename U>
struct FuncAux <T, R (*)(T, U)> : FuncBase <R, T>
{
    typedef R (*F)(T, U);
    FuncAux(F func, U user) : func(func), user(user) {}
    R operator () (T value) const {return (*func)(value, user);}
    // FuncBase* clone() const {return new FuncAux(*this);}
    F func;
    U user;
};

template <typename R, typename T, typename A>
struct FuncAux <T, R (A::*)()> : FuncBase <R, T>
{
    typedef R (A::*F)();
    FuncAux(A* obj, F func) : obj(obj), func(func) {}
    R operator () (T) const {return (obj->*func)();}
    // FuncBase* clone() const {return new FuncAux(*this);}
    A* obj;
    F func;
};

template <typename R, typename T, typename A>
struct FuncAux <T, R (A::*)(T)> : FuncBase <R, T>
{
    typedef R (A::*F)(T);
    FuncAux(A* obj, F func) : obj(obj), func(func) {}
    R operator () (T value) const {return (obj->*func)(value);}
    // FuncBase* clone() const {return new FuncAux(*this);}
    A* obj;
    F func;
};

template <typename R, typename T, typename A, typename U>
struct FuncAux <T, R (A::*)(T, U)> : FuncBase <R, T>
{
    typedef R (A::*F)(T, U);
    FuncAux(A* obj, F func, U user)
        : obj(obj), func(func), user(user) {}
    R operator () (T value) const {return (obj->*func)(value, user);}
    // FuncBase* clone() const {return new FuncAux(*this);}
    A* obj;
    F func;
    U user;
};

// ............................................................................

template <typename R, typename T>
struct Function
{
    R operator () (T value) const {return (*func)(value);}

    typedef FuncBase <R, T> Base;

    void to(Base* f)
    {
        clear();
        if (f)
            func = f;
    }

    void clear()
    {
        if (func != &null)
        {
            Base* f = func;
            func = &null;
            delete f;
        }
    }

    Function() : func(&null) {}
    // Function(const Function& f) : func(f.func->clone()) {}
    ~Function() {clear();}

    /* Function& operator = (const Function& f)
    {
        func = f.func->clone();
        return *this;
    }; */

    template <typename U> void
        to(R (*f)(T, U), U u) {to(f ? new FuncAux<T, R (*)(T, U)>(f, u) : 0);}
    template <typename A, typename B> void
        to(A* a, R (B::*f)()) {to(new FuncAux<T, R (B::*)()>(a, f));}
    template <typename A, typename B> void
        to(A* a, R (B::*f)(T)) {to(new FuncAux<T, R (B::*)(T)>(a, f));}
    template <typename A, typename B, typename U> void
        to(A* a, R (B::*f)(T, U), U u) {to(new FuncAux<T, R (B::*)(T, U)>(a, f, u));}

protected:
    Base* func;
    Base  null; // cannot be static if used within dll :(

private:
    Function(const Function&);
    Function& operator = (const Function&);
};

// template <typename R, typename T>
// typename Function<R, T>::Base Function<R, T>::null;

// ............................................................................

template <typename R, typename T>
struct Ftor : Function <R, T>
{
    Ftor() {}
    using Function <R, T> :: to;
    template <typename U> Ftor(R (*f)(T, U), U u)  {to(f, u);}
    template <typename A> Ftor(A* a, R (A::*f)())  {to(a, f);}
    template <typename A> Ftor(A* a, R (A::*f)(T)) {to(a, f);}
    template <typename A, typename U>
        Ftor(A* a, R (A::*f)(T, U), U u) {to(a, f, u);}
};

// ............................................................................

} // ~ namespace details

// ............................................................................

typedef details::Function <void, int> Callback;
typedef details::Ftor     <void, int> Ftor;

struct  UsesCallback {Callback callback;};

// ............................................................................

} // ~ namespace kali

// ............................................................................

#endif // ~ KALI_FUNCTION_INCLUDED
