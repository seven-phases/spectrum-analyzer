
#ifndef KALI_THREADS_INCLUDED
#define KALI_THREADS_INCLUDED

#include <windows.h>
#include "kali/dbgutils.h"
#include "kali/containers.h"

// ............................................................................

namespace kali {

// ............................................................................

inline void yield_()                      {::SwitchToThread();}
inline void sleep_(unsigned milliseconds) {::Sleep(milliseconds);}

inline int  cpuCount()
{
    SYSTEM_INFO si;
    ::GetSystemInfo(&si);
    return si.dwNumberOfProcessors;
}

// ............................................................................
// debug stuff

#define CHKH chkHandle_(FUNCTION_, handle)
#define CHKR(EXPR) chkReturn_(FUNCTION_, (EXPR))

template <typename T>
inline void chkHandle_(const char* prefix, T handle)
{
    if (handle == 0)
        trace("%s: failed [%i]\n", prefix, ::GetLastError());
}

inline int chkReturn_(const char* prefix, int ret)
{
    if (ret)
        trace("%s: failed [%i]\n", prefix, ret);
    return ret;
}

// ............................................................................
// Thread

struct Thread
{
    template <typename T>
    bool begin(T* obj, void (T::*func)())
    {
        struct Func
        {
            typedef void (T::*F)();
            Func(T* obj, F func) : obj(obj), func(func) {}
            T* obj;
            F func;

            static DWORD WINAPI thunk(void* ptr)
            {
                Func* func = (Func*) ptr;
                (func->obj->*func->func)(); // :)
                delete func;
                return 0;
            }
        };

        if (handle)
        {
            trace("%s: already running\n", FUNCTION_);
            return false;
        }

        handle = ::CreateThread(0, 0, Func::thunk,
            (void*) new Func(obj, func), 0, 0); CHKH;
        return *this;
    }

    int wait(int t = INFINITE) const
    {
        return CHKR(::WaitForSingleObject(handle, t));
    }

    operator bool() const {return !!handle;}
    Thread() : handle(0) {}
    ~Thread() {end();}

    void end(int t = 1000)
    {
        if (handle)
        {
            wait(t);
            ::CloseHandle(handle);
		    handle = 0;
        }
    }

private:
    typedef HANDLE Handle;
    Handle handle;

    Thread(const Thread&);
    Thread& operator = (const Thread&);
    template <typename T> operator T ();
};

// ............................................................................
// Mutex

struct Mutex
{
    void unlock()       {::ReleaseMutex(handle);}
    void lock()         {CHKR(waitfor_(handle, INFINITE));}
    bool trylock()      {return !CHKR(waitfor_(handle, 0));}
    bool trylock(int t) {return !CHKR(waitfor_(handle, t));}
    operator bool() const {return !!handle;}

    Mutex() : handle(::CreateMutex(0, 0, 0)) {CHKH;}
    explicit Mutex(const char* name)
        : handle(::CreateMutex(0, 0, name)) {CHKH;}

    ~Mutex()
    {
        ::CloseHandle(handle);
        handle = 0;
    }

private:

    static int waitfor_(HANDLE handle, int t)
    {
        int ret = ::WaitForSingleObject(handle, t);
        // return 'success' on WAIT_ABANDONED as we HAVE gained the ownership:
        return (ret == WAIT_ABANDONED) ? 0 : ret;
    }

private:
    typedef HANDLE Handle;
    Handle handle;

    Mutex(const Mutex&);
    Mutex& operator = (const Mutex&);
    template <typename T> operator T ();
};

// ............................................................................
// Event

struct Event
{
    void wait() const {CHKR(::WaitForSingleObject(handle, INFINITE));}
    void signal()     {::SetEvent(handle);}
    void reset()      {::ResetEvent(handle);}
    operator bool() const {return !!handle;}

    HANDLE operator * () const {return handle;}

    explicit Event(const char* name, bool autoReset = true)
        : handle(::CreateEvent(NULL, !autoReset, 0, name)) {CHKH;}
    explicit Event(bool autoReset = true)
        : handle(::CreateEvent(NULL, !autoReset, 0, 0)) {CHKH;}

    ~Event()
    {
        ::CloseHandle(handle);
        handle = 0;
    }

private:
    typedef HANDLE Handle;
    Handle handle;

    Event(const Event&);
    Event& operator = (const Event&);
    template <typename T> operator T ();
};

// ............................................................................
// multiple wait objects

template <typename T, int Capacity>
struct MultipleObjects
{
    int wait(int t = INFINITE) const
    {
        return ::WaitForMultipleObjects
            (list.size(), handles, 0, t);
    }

    T* operator [] (int index) const
    {
        return list[index];
    }

    void add(T* object)
    {
        if (list.size() >= Capacity)
            return trace("%s: no more space - %i\n",
                FUNCTION_, Capacity);

        handles[list.size()] = **object;
        list.add(object);
    }

    void release()
    {
        for (iterator i = list.end(); i != list.begin();)
            delete *--i;
    }

    MultipleObjects() : list(Capacity) {}

private:
    typedef ::HANDLE Handle;
    Handle   handles[Capacity];
    List<T*> list;

    typedef typename List<T*>
        ::iterator iterator;
};

// ............................................................................

#undef CHKH
#undef CHKR

// ............................................................................

} // ~ namespace kali

// ............................................................................
// Atomics

extern "C" {

typedef long atomicType_;

#define pause_                 _mm_pause
#define atomicOr_              _InterlockedOr
#define atomicExchange_        _InterlockedExchange
#define atomicCompareExchange_ _InterlockedCompareExchange

#ifndef _WIN64 // winnt.h declares these functions

void pause_();
long atomicOr_(volatile long*, long);
long atomicExchange_(volatile long*, long);
long atomicCompareExchange_(volatile long*, long, long);

#pragma intrinsic(pause_)
#pragma intrinsic(atomicOr_)
#pragma intrinsic(atomicExchange_)
#pragma intrinsic(atomicCompareExchange_)

#endif // ~ _WIN64

inline bool atomicTryLock_(volatile long* value)
{
    return !atomicCompareExchange_(value, 1, 0);
}

} // ~ extern "C"

// ............................................................................

#endif // ~ KALI_THREADS_INCLUDED
