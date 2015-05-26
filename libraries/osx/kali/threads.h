
#ifndef KALI_THREADS_INCLUDED
#define KALI_THREADS_INCLUDED

#include <unistd.h>
#include <pthread.h>
#include <mach/mach.h>
#include <sys/sem.h>
#include <sys/errno.h>
#include <libkern/OSAtomic.h>

#include "kali/dbgutils.h"

// ............................................................................

namespace kali {

inline void sleep_(int milliseconds) {usleep(milliseconds * 1000);}

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
            Func(T* obj, F func) : obj(obj), func(func){}
            T* obj;
            F func;

            static void* thunk(void* ptr)
            {
                Func* func = (Func*) ptr;
                (func->obj->*func->func)(); // :)
                delete func;
                return 0;
            }
        };

        if (handle)
        {
            trace("%s: already opened\n", FUNCTION_);
            return false;
        }

        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        int ret = pthread_create(&handle,
            &attr, Func::thunk, new Func(obj, func));
        pthread_attr_destroy(&attr);
        if (ret)
            trace("pthread_create failed [%x]\n", ret);

        sleep_(0);
        return !!handle;
    }

    void end(int t = 1000)
    {
        if (handle)
        {
            wait(t);
		    handle = 0;
        }
    }

    int wait(int t = -1) const
    {
        if (!handle)
            return 1; // fixme: return invalid handle code

        enum {quant = 10}; // 10ms
        t = (t + quant) / quant;

        do
        {
            if (!running())
                return 0;
            sleep_(quant);
        } while (--t);

        return 2; // fixme: return timeout code
    }

    operator bool() const {return !!handle;}
    Thread() : handle(0) {}
    ~Thread() {end();}

private:

    bool running() const
    {
        // a sort of hack, but there's no other (easy) way
        return thread_resume(pthread_mach_thread_np(handle)) < 6;
    }

private:
    typedef pthread_t Handle;
    Handle handle;
    Thread(const Thread&);
    Thread& operator = (const Thread&);
    template <typename T> operator T ();
};

// ............................................................................

struct Semaphore
{
    Semaphore() : handle(-1) {}

    bool open(int key, int initial_value)
    {
        handle = semget(key, 1, 0);
        if (handle != -1)
            return true;

        handle = semget(key, 1, IPC_CREAT | ALL);
        if (handle != -1)
        {
            value(initial_value);
            return true;
        }

        trace("%s: semget failed [%i]\n", FUNCTION_, errno);
        return false;
    }

    int  value()      {return ctl(FUNCTION_, GETVAL);}
    int  value(int v) {return ctl(FUNCTION_, SETVAL, &v);}
    void remove()     {ctl(FUNCTION_, IPC_RMID);}
    // ~Semaphore() {remove();}

protected:

    int op(const char* prefix, int cmd, int flags = 0)
    {
        sembuf cmd_ = {0, cmd, flags};
        int ret = semop(handle, &cmd_, 1);
        if (ret)
            trace("%s: semop failed [%i]\n", prefix, errno);

        return ret;
    }

    int ctl(const char* prefix, int cmd, void* data = 0)
    {
        int ret = data
            ? semctl(handle, 0, cmd, *(semun_t*) data)
            : semctl(handle, 0, cmd);
        if (ret == -1)
            trace("%s: semctl failed [%i]\n", prefix, errno);

        return ret;
    }

private:

    enum
    {
        X_     = SEM_A | SEM_R,
        ALL = X_ | (X_ >> 3) | (X_ >> 6),
        EACCES_ = EACCES
    };

private:
    int handle;
};

// ............................................................................
// these are very limited and unsafe representatives of the corresponding
// substances for Windows (kali::Mutex and kali::Event respectively)
// Use only for IPC communications and be careful
// (use kali::atomic::Lock as a 'single process' mutex)

struct Mu : public Semaphore
{
    Mu(int key) {Semaphore::open(key, 1);}

    bool lock()    {return !op(FUNCTION_, -1, SEM_UNDO);}
    bool trylock() {return !op(FUNCTION_, -1, SEM_UNDO | IPC_NOWAIT);}
    bool unlock()  {return !op(FUNCTION_, +1, SEM_UNDO);} // note: never unlock when it's not locked by _you_!

    bool trylock(int t)
    {
        enum {quant = 10}; // 10ms
        t = (t + quant) / quant;

        do
        {
            if (trylock())
                return true;
            sleep_(quant);
        } while (--t);

        return false;
    }
};

struct Ev : public Semaphore
{
    Ev(int key) {Semaphore::open(key, 0);}

    void wait()   {op(FUNCTION_, -1);} // note: the only listener for event!
    void signal() {value(1);}
    void reset()  {value(0);}
};

// ............................................................................
// Atomics

typedef int atomicType_;

inline int  atomicOr_(int volatile* value, int mask) {return OSAtomicOr32(mask, (unsigned*) value);}
inline bool atomicTryLock_(int volatile* value) {return OSAtomicCompareAndSwap32(0, 1, (int*) value);}

inline int  atomicExchange_(int* value, int new_)
{
    // FIXME: there's __sync_lock_test_and_set but it's available only since gcc 4.2.1
    #if 0
        return __sync_lock_test_and_set(value, new_);
    #else
        int ret(*value);
        *value = new_;
        return ret;
    #endif
}

// ............................................................................

} // ~ namespace kali

// ............................................................................

#endif // ~ KALI_THREADS_INCLUDED
