
#include "includes.h"
#include "main.h"

// ............................................................................

#if PERF

void Plugin::processReplacing(float** in, float** out, int n)
{
    process(in, out, n);
}

#endif

//void Plugin::processDoubleReplacing(double** in, double** out, int n)
//{
//    process(in, out, n);
//}

// ............................................................................
