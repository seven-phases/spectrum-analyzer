
#ifndef SA_LEGACY_INCLUDED
#define SA_LEGACY_INCLUDED

#include "sa.settings.h"

// ............................................................................

namespace kali   {
namespace legacy {

using namespace sa::config;

typedef int Preset[ParameterCount];

bool convertPreset(Preset& value)
{
    using namespace parameters;
    const int n = ParameterCount;

    #define _ ~0
    #define i SettingsIndex +
    const Preset maps[] = 
    {
        {   // v1
            _,
            _, 
            _, 
            _,
            w, 
            h,
            i inputChannel,
            i peakEnable,
            i peakDecay,
            i avrgEnable,
            i avrgTime,
            i avrgBarType,
            i avrgBarSize,
            i holdEnable,
            i holdInfinite,
            i holdTime,
            i holdDecay,
            i holdBarType,
            i holdBarSize,
            i levelCeil,
            i levelRange,
            i levelGrid,
            i freqGridType,
            i bandsPerOctave,
            i peakBarColor,
            i holdBarColor,
            i avrgBarColor,
            i gridBorderColor,
            i gridLineColor,
            i gridLabelColor,
            i bkgTopColor,
            i bkgBottomColor
        }
    };
    #undef i
    #undef _

    int ver = value[version];
    if (ver == presetVersion)
        return true;

    if (ver < 1 || ver > sizeof(maps)/sizeof(*maps))
        return false;

    const Preset& map = maps[ver - 1];
    Preset src;
    copy(src, value, n);
    copy(value, Defaults().data(), n);
    for (int i = 0; i < n; i++)
        if (map[i] > 0)
            value[map[i]] = src[i];

    #if DBG
        trace.warn("%s: version %i\n", FUNCTION_, src[version]);
        EnumNames <Index, sa::settings::Count> name;
        for (int i = SettingsIndex; i < n; i++)
            trace.warn("%20s: %4i\n", 
                name[i - SettingsIndex], value[i]);
    #endif
    return true;    
}

} // ~ namespace legacy
} // ~ namespace kali

// ............................................................................

#endif // ~ SA_LEGACY_INCLUDED
