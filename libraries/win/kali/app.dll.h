
#ifndef KALI_APP_DLL_INCLUDED
#define KALI_APP_DLL_INCLUDED
#if defined(_DLL) | defined(_WINDLL)

// ............................................................................

BOOL WINAPI DllMain(HMODULE module, DWORD reason, LPVOID)
{
    using namespace kali;

    switch (reason)
    {
        case DLL_PROCESS_ATTACH:
            app.alloc();
            app->details_ = app->autorelease(new AppDetails);
            app->details_->module_ = module;
            break;

        case DLL_PROCESS_DETACH:
            app.release();
            break;
    }

	return TRUE;
}

// ............................................................................

#endif // ~ _DLL | _WINDLL
#endif // ~ KALI_APP_DLL_INCLUDED
