
#ifndef SP_MORE_INCLUDED
#define SP_MORE_INCLUDED

// ............................................................................

namespace sp {

// ............................................................................

template <typename T, int N>
struct AveragerDelay
{
    enum {capacity = N};

    T tick(T in)
    {
        incSize(in);
        return decSize();
    }

    void incSize(T in) // push
    {
        if (--in_ < data)
            in_ += capacity;
        *in_ = in;
    }

    T decSize() // pop
    {
        if (--out_ < data)
            out_ += capacity;
        return *out_;
    }

    void size(int size_)
    {
        in_   = data;
        out_  = data + size_;
        clear();
    }

    void clear()
    {
        for (int i = 0; i < capacity; i++)
            data[i] = T();
    }

    AveragerDelay() {size(0);}

private:
    T*  in_;
    T*  out_;
    T   data[N];
};

// ............................................................................

template <int N>
struct Averager // rectangular window average for RMS
{
    double tick(double in, double inf, int size_)
    {
        resize(size_);
        value += in;
        value -= delay.tick(in);
        return (value > (inf * size * 10000))
            ? (value / size) : inf;
    }

    void resize(int size_)
    {
        if (size_ > delay.capacity)
            size_ = delay.capacity;

        int n = size_ - size;
        if (n < 0)
            while (++n <= 0)
                value -= delay.decSize();
        else
        {
            double current = (size)
                ? (value / size) : 0;
            while (--n >= 0)
            {
                value += current;
                delay.incSize(current);
            }
        }

        size = size_;
    }

    void clear()
    {
        value = 0;
        delay.clear();
    }

    Averager() : size(0), value(0) {}

private:
    int    size;
    double value;
    AveragerDelay <double, N> delay;
};

// ............................................................................

template <int PollTime, int InfEdge, typename T = double>
struct Meter
{
    // all time variables in ms:

    struct Options
    {
        int pdecay;
        int hinf;
        int hhold;
        int hdecay;
    };

    inline_ void tick(T value, const Options& opt)
    {
        peak -= opt.pdecay * (.001 * PollTime);
        if (peak < value)
            peak = value;

        if (holdTime > 0)
            holdTime -= PollTime;
        else
            hold -= !opt.hinf
                * opt.hdecay * (.001 * PollTime);

        if (hold < value)
        {
            hold = value;
            holdTime = opt.hhold;
        }
    }

    void reset()     {hold = peak = InfEdge;}
    void resetHold() {hold = peak;}

    Meter() :
        peak(InfEdge),
        hold(InfEdge),
        holdTime(0)
    {}

public:
    T peak;
    T hold;

private:
    int holdTime;
};

// ............................................................................

} // ~ namespace sp

// ............................................................................

#endif // ~ SP_MORE_INCLUDED
