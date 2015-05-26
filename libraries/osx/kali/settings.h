
#ifndef KALI_SETTINGS_INCLUDED
#define KALI_SETTINGS_INCLUDED

#include <CoreFoundation/CoreFoundation.h>

// ............................................................................

struct Settings
{
    typedef const CFStringRef KeyType;

    Settings(KeyType key) : handle(key) {}
    ~Settings() {::CFPreferencesAppSynchronize(handle);}

    void set(const char* name, int value) const
	{
        CFStringRef k = ::CFStringCreateWithCString
            (kCFAllocatorDefault, name, kCFStringEncodingMacRoman);
        CFNumberRef v = ::CFNumberCreate
            (kCFAllocatorDefault, kCFNumberSInt32Type, &value);
        ::CFPreferencesSetAppValue(k, v, handle);
        ::CFRelease(k);
        ::CFRelease(v);
	}

    int get(const char* name, int default_) const
	{
        CFStringRef k = ::CFStringCreateWithCString
            (kCFAllocatorDefault, name, kCFStringEncodingMacRoman);
        CFNumberRef v = (CFNumberRef) ::CFPreferencesCopyAppValue(k, handle);
        if (v)
        {
            CFNumberGetValue(v, kCFNumberSInt32Type, &default_);
            CFRelease(v);
        }

        ::CFRelease(k);
		return default_;
	}

private:
    KeyType handle;
    Settings(const Settings&);
    Settings& operator = (const Settings&);
};

// ............................................................................

#endif // ~ KALI_SETTINGS_INCLUDED
