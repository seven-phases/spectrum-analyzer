
#include "includes.h"
#include "main.h"
#include "kali/app.dll.h"

// ............................................................................

extern "C" AEffect* VSTPluginMain(audioMasterCallback audioMaster)
{
    #ifdef _CRTDBG_MAP_ALLOC
        _CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF);
    #endif

    #if DBG > 1
        trace.setLevel(trace.Full);
    #endif
    tf

	if (audioMaster(0, audioMasterVersion, 0, 0, 0, 0))
	    return (new Plugin(audioMaster))->getAeffect();

    trace("%s: failed, wrong master version.\n", FUNCTION_);
	return 0;
}

// ............................................................................
