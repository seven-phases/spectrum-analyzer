
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "kali/platform.h"

// ............................................................................

#if 0

#define DBG 1
#include "analyzer.h"

void usage()
{
    int fs[] = {32000, 44100, 48000};
    int nfs  = sizeof(fs)/sizeof(*fs);
    int bo[] = {3, 4, 6};
    int nbo  = sizeof(bo)/sizeof(*bo);

    Analyzer a;
    for (int i = 0; i < nbo; i++)
        for (int j = 0; j < nfs; j++)
            for (int k = 0; k < 3; k++)
                a.update(fs[j] << k, bo[i]);

    /*
    Analyzer a;
    a.update(44100, 4);

    const int n = 200;
    float in[n] = {0};
    in[0] = 1;

    a.process(in, n);*/
}

// ............................................................................

#else

#define trace printf

// .... simd.h:

#include "sp/sp.h"

using namespace sp;

inline m128 inter(const m128& x, const m128& y)
{
    return _mm_sub_ps(shuffle<0, 2, 1, 2>
        (shuffle<3, 3, 0, 0>(y, x), x), x);

    // old version:
    /* return _mm_sub_ps(_mm_move_ss
        (shuffle<0, 0, 1, 2>(x, x),
         shuffle<3, 2, 1, 0>(y, y)), x); */
}

// ............................................................................

void usage()
{
    mm4 y_ = {0.f, 1.f, 2.f, 3.f};
    mm4 x_ = {4.f, 5.f, 6.f, 7.f};
    m128 y = y_;
    m128 x = x_;
    m128 z = shuffle<1, 2, 3, 0>(y, y);
    z = z;
}

#endif

// ............................................................................

int main()
{
    usage();
	system("pause");
	return 0;
}
