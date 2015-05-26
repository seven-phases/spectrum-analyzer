
#ifndef KALI_ATOMIC_INCLUDED
#define KALI_ATOMIC_INCLUDED

#include <windows.h>

// ............................................................................

#if WINDOWS_

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

namespace kali {
    inline void yield_()                      {::SwitchToThread();}
    inline void sleep_(unsigned milliseconds) {::Sleep(milliseconds);}
}

#endif // ~ WINDOWS_

// ............................................................................

namespace kali   {
namespace atomic {

// ............................................................................

struct Lock
{
    void unlock()          {value = 0;}
    void lock()            {lock_();}
    bool trylock()         {return !value && atomicTryLock_(&value);}
    operator bool () const {return !!value;}

    Lock() : value(0) {}

private:
    typedef volatile atomicType_ Type;
    Type value;
    Lock(const Lock&);
    Lock& operator = (const Lock&);
    template <typename T> operator T () const;

    void lock_()
    {
        for (;;)
        {
            int spin = 0x333; // fixme
            while (--spin)
            {
                if (trylock())
                    return;
                pause_();
            }
            yield_();
        }
    }
};

// ............................................................................

} // ~ namespace atomic
} // ~ namespace kali

// ............................................................................

#endif // ~ KALI_ATOMIC_INCLUDED
