
#ifndef KALI_CONTAINERS_INCLUDED
#define KALI_CONTAINERS_INCLUDED

#include "kali/types.h"
#include "kali/runtime.h"

// ............................................................................

namespace kali {

// ............................................................................

template <typename T>
struct List
{
    typedef T* iterator;

    iterator begin() const {return data;}
    iterator end()   const {return data + size_;}
    int      size()  const {return size_;}
    void     clear()       {size_ = 0;}

    T& operator [] (int i) const {return data[i];}
    T& at(int i)           const {return data[i];}

    void add(const T& value)
    {
        if (size_ == capacity)
            grow();
        data[size_++] = value;
    }

    explicit List(int n = 16)
        : data(n ? new T[n] : 0), size_(0), capacity(n) {}

    List(const List& list) :
        data(list.capacity ? new T[list.capacity] : 0),
        size_(list.size_),
        capacity(list.capacity) {copy(data, list.data, size_);}

    List& operator = (const List& list)
    {
        List temp(list);
        swap(temp);
        return *this;
    }

    ~List() {delete [] data;}

private:

    void grow()
    {
        capacity = (capacity + 8) * 2;
        List temp(*this);
        swap(temp);
    }

    void swap(List& list)
    {
        kali::swap(data,     list.data);
        kali::swap(size_,    list.size_);
        kali::swap(capacity, list.capacity);
    }

private:
    iterator data;
    int      size_;
    int      capacity;
};

// ............................................................................

template <typename T = void>
struct AutoReleasePool // FIXME: rename to AutoReleaseList
{
    typedef List<T*> Pool;
    typedef typename Pool::iterator iterator;
    AutoReleasePool() : pool(128) {}
    explicit AutoReleasePool(int n) : pool(n) {}
    ~AutoReleasePool() {release();}

    template <typename U> U* operator () (U* ptr)
    {
        pool.add(ptr);
        return ptr;
    }

    void release()
    {
        for (iterator i = pool.end(); i != pool.begin();)
            delete *--i;
        pool.clear();
    }

private:
    Pool pool;
    AutoReleasePool(const AutoReleasePool&);
    AutoReleasePool& operator = (const AutoReleasePool&);
};

template <>
struct AutoReleasePool <void>
{
    AutoReleasePool() {}
    explicit AutoReleasePool(int n) : pool(n) {}

    template <typename T> T* operator () (T* ptr)
    {
        add(ptr, ptr);
        return ptr;
    }

    void release() {pool.release();}

private:

    template <typename T> void add(T* ptr, ReleaseAny*) {pool(ptr);}

    #if 1

    template <typename T> void add(T* ptr, void*)
    {
        struct Aux : ReleaseAny
        {
            T* p;
            Aux(T* p) : p(p) {}
            ~Aux() {delete p;}
        };

        pool(new Aux(ptr));
    }

    #endif

private:
    AutoReleasePool <ReleaseAny> pool;
};

// ............................................................................

} // ~ namespace kali

// ............................................................................

#endif // ~ KALI_CONTAINERS_INCLUDED
