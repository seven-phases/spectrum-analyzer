
#ifndef SA_SELF_INSTALL_INCLUDED
#define SA_SELF_INSTALL_INCLUDED

#include "sa.settings.h"

// ............................................................................

namespace sa {

// ............................................................................

void installColors()
{
    using namespace settings;

    struct {const char* name; int value[ColorsCount];}
        static const scheme[] =
    {
        "7th Fashion",       0xabffffff, 0xe1f7b333, 0xabffffff, 0x54ffffff, 0x1cffffff, 0xabffffff, 0xff5a8296, 0xff1e4664,
        "Blues",             0x666699ff, 0xeb6699ff, 0x7f66cc99, 0x99f3f3f3, 0x0ff3f3f3, 0x99f3f3f3, 0xff0c0c0c, 0xff33547d,
        "Brassica Oleracea", 0x5416a085, 0xa816a085, 0xffffffff, 0x5434495e, 0x1c34495e, 0xa834495e, 0xffd3ede9, 0xffc4e7e0,
        "Contrast",          0xb300ff00, 0xb3ffff00, 0xb3ff3c3c, 0xff909090, 0x4c909090, 0xff909090, 0xff303030, 0xff202020,
        "Contrast, Not So",  0xe6669933, 0xe6c03349, 0x99ffff66, 0xff909090, 0x4c909090, 0xff909090, 0xff303030, 0xff202020,
        "Crimson Cloudy",    0x66ff6699, 0x99ff6699, 0x54ffcc33, 0x99f3f3f3, 0x0ff3f3f3, 0x99f3f3f3, 0xff0c0c0c, 0xff7d3354,
        "Dirty Milk",        0xffbdc3c7, 0xff27ae60, 0xff9b59b6, 0xff7f8c8d, 0x007f8c8d, 0xff7f8c8d, 0xffecf0f1, 0xffecf0f1,
        "Dull Day",          0xa8144664, 0xbfffac33, 0xbfffffff, 0xa8ffffff, 0x54ffffff, 0x54144664, 0xffccd8e4, 0xffffffff,
        "Fire Drill",        0xccff8033, 0x8fffffff, 0x8fffcc33, 0x5effffff, 0x21ffffff, 0x8fffffff, 0xff606060, 0xff202020,
        "Fre(a)ko",          0xe1666666, 0xa8ffffff, 0xffe0a060, 0xffc08040, 0xffe0a060, 0xff806040, 0xffffcc99, 0xffdd9966,
        "Haze",              0xbf000810, 0x99000000, 0xbff4faff, 0x7f000810, 0x14000810, 0x99000810, 0xfff4faff, 0xffd0e8ff,
        "Hominoids",         0xccebd2ce, 0xccc02020, 0xff206990, 0xff808080, 0x54808080, 0xff808080, 0xff2f2f2f, 0xff202020,
        "Ichtiandr Estate",  0x5433ccc0, 0xff33ccc0, 0xff34495e, 0x54f3f3f3, 0x1cf3f3f3, 0xa8f3f3f3, 0xff34495e, 0xff3d5a6d,
        "Immersion",         0x6633ccc0, 0xeb33ccc0, 0x9933ccff, 0x99f3f3f3, 0x0ff3f3f3, 0x99f3f3f3, 0xff0c0c0c, 0xff336666,
        "Molecule",          0xbf0688c4, 0xff7bb042, 0xffcf9a26, 0xff677b74, 0xff3a403f, 0xff677b74, 0xff1e2523, 0xff101010,
        "Tropics",           0x5466cc99, 0xa833cc66, 0xa8ccff66, 0x54ffffff, 0x0fffffff, 0xa8ffffff, 0xff111312, 0xff345846,
        "Vegetable",         0xffb2b8bc, 0xffffc53f, 0xffa5e346, 0xff828a8e, 0xff828a8e, 0xff828a8e, 0xff434c51, 0xff434c51,
        "Water In Charcoal", 0xcc2080f0, 0x99ffffff, 0xebc0c0c0, 0x5effffff, 0x21ffffff, 0x8fffffff, 0xff505050, 0xff101010,
    };

    static const int custom[] =
    {
        0x9cbc1a, 0x85a016, 0x71cc2e, 0x60ae27, 0xdb9834, 0xb98029, 0x0fc4f1, 0x129cf3,
        0x227ee6, 0x0054d3, 0x3c4ce7, 0x2b39c0, 0xf1f0ec, 0xc7c3bd, 0xa6a595, 0x5e4934
    };

    ::Settings key(config::colorsKey);
    const char* const version = "version";
    if (atof(key.get(version, "")) >= VERSION)
        return;
    tf
    key.set(version, kali::string("%.2f", VERSION)());

    const kali::EnumNames <Index, Count> name;
    for (int i = 0; i < sizeof(scheme)/sizeof(*scheme); i++)
    {
        ::Settings sub(key, scheme[i].name);
        for (int j = 0; j < ColorsCount; j++)
            sub.set(name[ColorsIndex + j], scheme[i].value[j]);
    }

    char nickname[] = ".a";
    for (int i = 0; i < sizeof(custom)/sizeof(*custom); i++)
    {
        key.set(nickname, custom[i]);
        ++nickname[1];
    }
}

// ............................................................................

} // ~ namespace sa

// ............................................................................

#endif // ~ SA_SELF_INSTALL_INCLUDED
