
#ifndef KALI_TYPES_INCLUDED
#define KALI_TYPES_INCLUDED

#include "kali/platform.h"

// ............................................................................

namespace kali {

// ............................................................................

enum Align  {left, center, right};
enum UpDown {up, down};
enum Weight {regular, bold};

// ............................................................................

struct Null {};
struct Undefined;

// ............................................................................

struct ReleaseAny
{
    virtual ~ReleaseAny() {}
};

// ............................................................................

template <typename T>
struct Ptr
{
    typedef T Type;

    Ptr(Type* ptr = 0) : ptr(ptr) {}
    operator Type*    () const {return   ptr;}
    operator Type&    () const {return  *ptr;}
    operator bool     () const {return !!ptr;}
    Type& operator *  () const {return  *ptr;}
    Type* operator -> () const {return   ptr;}

    #if 0
    // hmm, well, weird, but TOO sweet syntax sugar:
    template <typename U>
    void operator += (const U& u) const {*ptr += u;}
    #endif

private:
    Type* ptr;
    template <typename U> operator U () const;
};

// ............................................................................

template <typename T, int Unique = 0>
struct singleton
{
    typedef T Type;

    singleton() {}
    Type* operator -> () const {return   ptr;}
    operator bool ()     const {return !!ptr;}

    static Type* alloc()
    {
        ptr = new Type;
        return ptr;
    }

    static void release()
    {
        delete ptr;
        ptr = 0;
    }

private:
    static Type* ptr;
    template <typename U> operator U () const;
};

template <typename T, int Unique>
T* singleton<T, Unique>::ptr = 0;

// ............................................................................

} // ~ namespace kali

// ............................................................................

#endif // KALI_TYPES_INCLUDED
