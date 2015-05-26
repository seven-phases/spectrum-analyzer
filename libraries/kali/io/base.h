
#ifndef IO_BASE_INCLUDED
#define IO_BASE_INCLUDED

// ............................................................................

namespace io {

// ............................................................................

template <typename T>
struct protected_ : protected T
{
    typedef T Base;
};

// ............................................................................
// yak, TODO - rename, formalize

namespace samplerate
{
    const int list[] =
    {
         32000,
         44100,
         48000,
         64000,
         88200,
         96000,
        176400,
        192000,
        352800,
        384000
    };
}

// ............................................................................

// This wrapper represents sample rate by 'index' instead of the actual rate value.
// It is useful for a bit more straight-forward sample rate control within GUI code.

template <typename T>
struct SampleRateAlt : T
{
    typedef SampleRateAlt Base;

    SampleRateAlt() {memset(rate, 0, sizeof(rate));}
    int  supportedRates(int index) const {return rate[index];}
    void samplerate(int index) {T::samplerate(rate[index]);}

    int samplerate() const
    {
        int i = 0;
        int v = T::samplerate();
        while (rate[i])
            if (rate[i++] == v)
                return --i;
        return ~0;
    }

    void cook() // TODO - hide it somehow
    {
        int mask = T::supportedRates();
        for (int i = 0, j = 0; i < MaxRates; i++)
            if ((mask >> i) & 1)
                rate[j++] = samplerate::list[i];
    }

private:

    enum
    {
        MaxRates = sizeof(samplerate::list)
            / sizeof(*samplerate::list)
    };

private:
    int rate[MaxRates];
};

// ............................................................................

namespace asio {

// ............................................................................

enum Properties
{
    buffer_size,
    driver_active,

    PropertyCount
};

static const int defaults[PropertyCount] =
{
    50, // buffer_size
    0,  // driver_active
};

// ............................................................................

} // ~ namespace asio
} // ~ namespace io

// ............................................................................

#endif // ~ IO_BASE_INCLUDED
