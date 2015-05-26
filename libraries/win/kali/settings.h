
#ifndef KALI_SETTINGS_INCLUDED
#define KALI_SETTINGS_INCLUDED

#include <windows.h>
#include "kali/dbgutils.h"

// ............................................................................

struct Settings
{
    void set(const char* name, int value) const
	{
		::RegSetValueEx(handle, name, 0, REG_DWORD,
            (BYTE*) &value, sizeof(value));
	}

    int get(const char* name, int default_) const
	{
		DWORD size = sizeof(default_);
		::RegQueryValueEx(handle, name,
            0, 0, (BYTE*) &default_, &size);
		return default_;
	}

    void set(const char* name, const char* value) const
	{
		::RegSetValueEx(handle, name, 0, REG_SZ,
            (const BYTE*) value, (DWORD) strlen(value) + 1);
	}

    #ifdef KALI_STRING_INCLUDED

    kali::string get(const char* name, const char* default_) const
	{
        kali::string value = default_;
		DWORD size = value.size;
		::RegQueryValueEx(handle, name,
            0, 0, (BYTE*) value(), &size);
		return value;
	}

    kali::string subKey(int index) const
    {
        FILETIME unused;
        kali::string key;
        DWORD size = key.size;
        ::RegEnumKeyEx(handle, index,
            key(), &size, 0, 0, 0, &unused);
        return key;
    }

    #endif

    // ........................................................................
    // some extensions (Windows only)

    bool exist(const char* key) // test for a subkey
    {
        Handle h;
        int ret = ::RegOpenKeyEx(handle, key, 0, KEY_READ, &h);
        if (ret == 0)
            ::RegCloseKey(h);
        return !ret;
    }

    void deleteKey(const char* key) // delete subkey
    {
        ::RegDeleteKey(handle, key);
    }

    // ........................................................................

    ~Settings() {::RegCloseKey(handle);}

    /*explicit*/ Settings(const char* key)
        : handle(ctor(HKEY_CURRENT_USER, key)) {}

    Settings(const Settings& root, const char* key)
        : handle(ctor(root.handle, key)) {}

private:
    typedef ::HKEY Handle;
    Handle handle;
    Settings(const Settings&);
    Settings& operator = (const Settings&);

    static Handle ctor(Handle root, const char* key)
    {
        Handle handle = 0;
        REGSAM access = KEY_READ | KEY_WRITE
            | KEY_CREATE_SUB_KEY | KEY_ENUMERATE_SUB_KEYS;
        if (::RegCreateKeyEx(root, key, 0, 0, 0, access, 0, &handle, 0))
            trace("%s: - cannot open %x\\%s\n", FUNCTION_, root, key);
        return handle;
	}
};

// ............................................................................

#endif // ~ KALI_SETTINGS_INCLUDED
