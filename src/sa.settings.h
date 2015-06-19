
#ifndef SA_OPTIONS_INCLUDED
#define SA_OPTIONS_INCLUDED

#include "kali/runtime.h"
#include "kali/function.h"
#include "kali/string.h"
#include "kali/settings.h"
#include "kali/geometry.h"
#include "analyzer.h"
#include "version.h"

// ............................................................................

namespace sa       {
namespace settings {

// ............................................................................

enum Index
{
    inputChannel,

    peakEnable,
    peakDecay,

    avrgEnable,
    avrgTime,
    avrgBarType,
    avrgBarSize,

    holdEnable,
    holdInfinite,
    holdTime,
    holdDecay,
    holdBarType,
    holdBarSize,

    levelCeil,
    levelRange,
    levelGrid,

    freqGridType,
    bandsPerOctave,

    avrgSlope,

    peakBarColor,
    holdBarColor,
    avrgBarColor,
    gridBorderColor,
    gridLineColor,
    gridLabelColor,
    bkgTopColor,
    bkgBottomColor,

    Count
};

enum
{
    ColorsIndex = peakBarColor,
    ColorsCount = Count - ColorsIndex
};

// ............................................................................

struct Descriptor
{
    Index       index;
    int         min;
    int         max;
    int         step;
    int         default_;
    const char* unit;
    const char* label;
};

const Descriptor descriptor[] =
{
    {inputChannel,  0,  3, 1,  2, "Left, Right, Both, Mix", "Channel"},

    {peakEnable,    0,  1, 1,  1,        "bool", "On"},
    {peakDecay,     1, 60, 1, 15,        "dB/s", "Decay"},

    {avrgEnable,    0,  1, 1,  1,        "bool", "On"},
    {avrgTime,    100, 20000, 100, 3000,    "s", "Time"},
    {avrgBarType,   0,  2, 1,  1, "Bars, Curve, Curve Fill", "Show As"},
    {avrgBarSize,   1,  4, 1,  3,          "px", "Size"},

    {holdEnable,    0,  1, 1,  1,        "bool", "On"},
    {holdInfinite,  0,  1, 1,  0,        "bool", "Infinite"},
    {holdTime,      0, 20000, 100, 2000,    "s", "Time"},
    {holdDecay,     1, 60, 1,  3,        "dB/s", "Decay"},
    {holdBarType,   0,  2, 1,  1, "Bars, Curve, Curve Fill", "Show As"},
    {holdBarSize,   1,  4, 1,  2,          "px", "Size"},

    {levelCeil,   -80, 20, 1,  3,          "dB", "Ceiling"},
    {levelRange,  10, 120, 1, 51,          "dB", "Range"},
    {levelGrid,     2, 12, 1,  5,          "dB", "Grid"},
    {freqGridType,   0, 1, 1,  0, "Decade, Octave", "Freq. Grid"},
    {bandsPerOctave, 0, 2, 1,  2,     "3, 4, 6", "Bands/Octave"},

    {avrgSlope,    -6, 12, 1,  0,      "dB/oct", "Slope"},

    #define _ 0x80000000, 0x7FFFFFFF, 0x202020,
    {peakBarColor,    _ 0xCC2080F0, "argb", "Peak"},
    {holdBarColor,    _ 0x99FFFFFF, "argb", "Peak Hold"},
    {avrgBarColor,    _ 0xEBC0C0C0, "argb", "Average"},
    {gridBorderColor, _ 0x5EFFFFFF, "argb", "Grid Border"},
    {gridLineColor,   _ 0x21FFFFFF, "argb", "Grid Line"},
    {gridLabelColor,  _ 0x8FFFFFFF, "argb", "Grid Label"},
    {bkgTopColor,     _ 0xFF505050,  "rgb", "Background A"},
    {bkgBottomColor,  _ 0xFF101010,  "rgb", "Background B"},
    #undef _
};

struct Depended
{
    bool use;
    int  mask;
    int  value;

    static Depended make(bool uu, bool ac = 1, int ai = -1,
        bool bc = 1, int bi = -1, bool cc = 1, int ci = -1)
    {
        Depended r =
        {
            uu,
              ((ai >= 0) << ai)
            + ((bi >= 0) << bi)
            + ((ci >= 0) << ci),
              ((ai >= 0) * ac << ai)
            + ((bi >= 0) * bc << bi)
            + ((ci >= 0) * cc << ci)
        };

        return r;
    }
};

const Depended depended[] =
{
    #define _ Depended::make
    /* inputChannel */   _ (0),
    /* peakEnable */     _ (1),
    /* peakDecay */      _ (0, 1, peakEnable),
    /* avrgEnable */     _ (1),
    /* avrgTime */       _ (0, 1, avrgEnable),
    /* avrgBarType */    _ (0, 1, avrgEnable),
    /* avrgBarSize */    _ (0, 1, avrgEnable),
    /* holdEnable */     _ (1),
    /* holdInfinite */   _ (1, 1, holdEnable),
    /* holdTime */       _ (0, 1, holdEnable, 0, holdInfinite),
    /* holdDecay */      _ (0, 1, holdEnable, 0, holdInfinite),
    /* holdBarType */    _ (0, 1, holdEnable),
    /* holdBarSize */    _ (0, 1, holdEnable),
    /* levelCeil */      _ (0),
    /* levelRange */     _ (0),
    /* levelGrid */      _ (0),
    /* freqGridType */   _ (0),
    /* bandsPerOctave */ _ (0),
    /* avrgSlope */      _ (0, 1, avrgEnable),
    #undef _
};

// ............................................................................

struct Type : kali::UsesCallback
{
    Type(int* valueArray) : value(valueArray) {}

    void defaults()
    {
        validate();
        for (int i = 0; i < Count; i++)
            value[i] = descriptor[i].default_;
    }

    static void validate()
    {
        #if DBG
        static const int InvalidDescriptorArray
            [Count == (sizeof(descriptor)
                     / sizeof(*descriptor))] = {0};

        for (int i = 0; i < Count; i++)
            if (i != descriptor[i].index)
                DBGSTOP_;
        #endif
    }

    void operator () (int i, int v, bool notify = true)
    {
        v = kali::min(descriptor[i].max,
            kali::max(descriptor[i].min, v));
        if (value[i] != v)
        {
            value[i] = v;
            if (notify)
                callback(i);
        }
    }

    int operator () (int i) const {return value[i];}
    const char* name(int i) const {return name_[i];}

    void notify() const {callback(-1);}

private:
    int* value;
    kali::EnumNames <Index, Count> name_;
};

// ............................................................................

struct WidgetAdapter
{
    typedef const Descriptor& Desc;

    int range(int i) const
    {
        Desc d = descriptor[i];
        return (d.max - d.min) / d.step;
    }

    int value(int i) const
    {
        Desc d = descriptor[i];
        return (value_(i) - d.min) / d.step;
    }

    void value(int i, int v)
    {
        Desc d = descriptor[i];
        value_(i, v * d.step + d.min);
    }

    void value(int i, const char* v_)
    {
        char* end;
        int v;
        if (i == avrgSlope)
            v = int(strtod(v_, &end) * 2);
        else
            v = (!strcmp(descriptor[i].unit, "s"))
                ? int(strtod(v_, &end) * 1000 + .5)
                : strtol(v_, &end, 10);
        if (end != v_)
            value_(i, v);
    }

    kali::string text(int i, int v) const
    {
        Desc d = descriptor[i];
        return text_(i, v * d.step + d.min);
    }

    kali::string text(int i) const
    {
        return text_(i, value_(i));
    }

    kali::string label(int i) const
    {
        using kali::string;
        Desc d = descriptor[i];

        if (!strcmp(d.unit, "bool") ||
            !strcmp(d.unit, "argb") ||
            !strcmp(d.unit,  "rgb"))
                return d.label;

        if (strstr(d.unit, ", "))
            return string("%s:", d.label);

        return string("%s (%s):", d.label, d.unit);
    }

    const char* unit(int i) const
    {
        return descriptor[i].unit;
    }

private:

    kali::string text_(int i, int v) const
    {
        using kali::string;
        Desc d = descriptor[i];

        if (i == avrgSlope)
            return string("%0.1f", .5 * v);

        if (!strcmp(d.unit, "s"))
            return string("%0.1f", .001 * v);

        if (!strstr(d.unit, ", "))
            return string("%i", v);

        const char* u = d.unit;
        while (--v >= 0)
            u = strstr(u, ", ") + 2;
        v = int(strstr(u, ", ") - u);
        return string("%.*s", abs(v), u);
    }

public:

    WidgetAdapter(Type& value_) : value_(value_) {}

private:
    Type& value_;

    WidgetAdapter(const WidgetAdapter&);
    WidgetAdapter& operator = (const WidgetAdapter&);
};

// ............................................................................

} // ~ namespace settings

// ............................................................................

namespace config {

// ............................................................................

using kali::Size;
using kali::Rect;
using namespace settings;

#define KEY "SOFTWARE\\"COMPANY"\\"NAME
const char* const prefsKey   = KEY;
const char* const colorsKey  = KEY"\\Colours";
#undef KEY

const int  presetVersion = 2;

const int  pollTime =   48;         // ms
const int  infEdge  = -200;         // dB
const int  barPad   =    2;         // px
const Rect gridPad(24, 16, 24, 16); // px

const Size displaySize(653, 261);   // px

// grid frequencies, Hz, (second value specifies if the grid line is labeled)
// "decimal" grid:
const double freqGridDec[][2] =
{
                   {25, 1},    {40, 0},    {50, 1},
        {60, 0},   {70, 0},    {80, 0},    {90, 0},
       {100, 1},  {200, 1},   {300, 0},   {400, 0},
       {500, 1},  {600, 0},   {700, 0},   {800, 0},
       {900, 0}, {1000, 1},  {2000, 1},  {3000, 0},
      {4000, 0}, {5000, 1},  {6000, 0},  {7000, 0},
      {8000, 0}, {9000, 0}, {10000, 1}, {20000, 1}
};

// plain "octave" grid:
const double freqGridLin[][2] =
{
    {31.25, 1}, {62.5, 1},  {125, 1},  {250, 1},   {500, 1},
     {1000, 1}, {2000, 1}, {4000, 1}, {8000, 1}, {16000, 1}
};

struct FreqGrid
{
    int count;
    const double (*freq)[2];
};

const FreqGrid freqGrid[] =
{
   sizeof(freqGridDec)/sizeof(*freqGridDec), freqGridDec,
   sizeof(freqGridLin)/sizeof(*freqGridLin), freqGridLin
};

enum BarType {Bar, Curve, CurveFill};

// ............................................................................

namespace parameters
{
    enum Parameters
    {
        version,
        w, h,
        unused1,
        unused2,

        Count
    };
}

enum
{
    SettingsIndex  = parameters::Count,
    ParameterCount = parameters::Count + settings::Count
};

// ............................................................................

struct Preset
{
    char name[28];
    int value[Count - ColorsCount];
};

const Preset preset[] =
{
    "Ancient",                 0, 1, 12, 0, 6000, 0, 4, 1, 0, 3000,  3, 0, 4,  0, 42, 10, 1, 0, 0,
    "Medieval",                0, 1, 18, 1, 2000, 0, 4, 1, 0, 6000,  2, 0, 3, -3, 48,  6, 1, 1, 0,
    "Modern 1",                2, 1, 15, 1, 3000, 1, 3, 1, 0, 2000,  3, 1, 2, -2, 52,  5, 0, 2, 0,
    "Modern 2",                2, 1, 20, 1,  500, 1, 3, 1, 0,    0,  2, 1, 2, -2, 52,  5, 0, 2, 0,
    ".2 Short RMS, Slow Peak", 2, 0, 18, 1,  500, 1, 2, 1, 0,    0,  2, 1, 2, -3, 48,  6, 1, 2, 0,
    ".2 Long RMS, Fast Peak",  2, 0, 15, 1, 6000, 1, 2, 1, 0,    0, 15, 1, 2, -3, 48,  6, 1, 2, 0,
    "Broken Pixels",           3, 0, 12, 1,  500, 0, 4, 1, 0, 4000, 48, 0, 4, -3, 54, 12, 1, 2, 0,
    "Modern Fill Me",          2, 0, 24, 1, 4000, 2, 2, 1, 0,    0, 12, 2, 2, -2, 52,  5, 0, 2, 3,
};

struct Defaults
{
    const char* operator () (int index, int (&dst)[ParameterCount]) const
    {
        memcpy(dst, value, sizeof(value));

        int n = sizeof(preset)/sizeof(*preset);
        if ((index > 0) && (index <= n))
        {
            memcpy(dst + SettingsIndex,
                preset[index - 1].value, sizeof(preset->value));
            return preset[index - 1].name;
        }

        return index ? ". . ." : "Default";
    }

    const int* data() const {return value;}

    Defaults()
    {
        namespace p        = parameters;
        value[p::version]  = presetVersion;
        value[p::w]        = displaySize.w;
        value[p::h]        = displaySize.h;
        value[p::unused1]  = 0;
        value[p::unused2]  = 0;

        Type v(value + SettingsIndex);
        v.defaults();
        ::Settings pkey(prefsKey);
        for (int i = 0; i < ColorsIndex; i++)
            v(i, pkey.get(v.name(i), v(i)), false);
        ::Settings ckey(colorsKey);
        for (int i = ColorsIndex; i < Count; i++)
            v(i, ckey.get(v.name(i), v(i)), false);
    }

private:
    int value[ParameterCount];
};

// ............................................................................

enum PrefIndex
{
    keepColors,
    smartDisplay,

    PrefCount
};

typedef kali::EnumNames <PrefIndex, PrefCount> PrefName;

struct Pref
{
    PrefIndex   index;
    int         default_;
    const char* label;
};

const Pref prefs[PrefCount] =
{
    keepColors,    1, "Keep current colors when loading presets",
    smartDisplay,  0, "Use presets to manage display size",
};

// ............................................................................

} // ~ namespace config

// ............................................................................
// stuff shared between plugin, editor and display:

using namespace kali;

struct Editor;
struct Display;

struct Shared
{
    Editor*        editor;
    Display*       display;
    Analyzer*      analyzer;
    settings::Type settings;
    int            parameter[config::ParameterCount];

    Shared() :
        editor   (0),
        display  (0),
        analyzer (0),
        settings (parameter + config::SettingsIndex)
    {
        config::Defaults()(0, parameter);
    }
};

// ............................................................................

} // ~ namespace sa

// ............................................................................

#endif // ~ SA_OPTIONS_INCLUDED
