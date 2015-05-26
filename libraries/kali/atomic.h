
#ifndef KALI_ATOMIC_INCLUDED
#define KALI_ATOMIC_INCLUDED

#include "kali/threads.h"

// ............................................................................

namespace kali   {
namespace atomic {

// ............................................................................

struct Lock
{
    void unlock() {value = 0;}
    // void lock() {while (!trylock());}
    void lock() {lock_();}
    bool trylock() {return !value && atomicTryLock_(&value);}
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
        // FIXME
        for (;;)
        {
            int spin = 0x333;
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
