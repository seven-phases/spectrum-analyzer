
#ifndef VST_INCLUDED
#define VST_INCLUDED

#pragma warning(push, 3)

#include "audioeffectx.h"
#include "aeffeditor.h"

#pragma warning(pop)

#include "kali/app.h"

#if DBG
    #define tf trace.full("%s(%08x)\n", FUNCTION_, ::GetCurrentThreadId());
#else
    #define tf
#endif

// ............................................................................

namespace vst {

// ............................................................................

#define strcpy  dont_use_strcpy_with_vst
#define strncpy dont_use_strncpy_with_vst
// use PluginBase::copy instead

//.............................................................................

template <typename Plugin>
struct PluginBase : AudioEffectX
{
    enum
    {
        Category       = kPlugCategEffect,
        Version        = 1000,
        UniqueID       = 0,
        PresetCount    = 0,
        ParameterCount = 0
    };

    noalias_ static char* copy(char* dst, const char* src, int n)
    {
        char* p = dst;
        while ((--n > 0) && *src)
            *p++ = *src++;
        *p = 0;
        return dst;
    }

private:

    bool getEffectName(char* text)    {return !!copy(text, Plugin::name(), kVstMaxEffectNameLen);}
    bool getVendorString(char* text)  {return !!copy(text, Plugin::vendor(), kVstMaxVendorStrLen);}
    bool getProductString(char* text) {return !!copy(text, Plugin::name(), kVstMaxProductStrLen);}

    VstInt32 getVendorVersion()       {return Plugin::Version;}
    VstPlugCategory getPlugCategory() {return VstPlugCategory(Plugin::Category);}

public:

    PluginBase(audioMasterCallback master) : AudioEffectX
        (master, Plugin::PresetCount, Plugin::ParameterCount)
            {setUniqueID(Plugin::UniqueID);}

private:
    PluginBase(const PluginBase&);
    PluginBase& operator = (const PluginBase&);
};

// ............................................................................

template <typename Plugin, typename Window>
struct Editor : AEffEditor
{
    bool open(void* ref)
    {
        tf
        AEffEditor::open(ref);

        if (window)
        {
            trace("%s: window already open\n", FUNCTION_);
            return true;
        }

        kali::Window parent((kali::Window::Handle) ref);
        window = new Window(plugin);
        kali::app->createLayer(&parent, window);
        updateRect();

        return !!window->handle;
    }

    void close()
    {
        if (window)
        {
            AEffEditor::close();
            window->destroy();
            delete window;
            window = 0;
            tf
        }
    }

    bool onKeyDown(VstKeyCode& key)
    {
        /* trace.full("%s: %c, %i, %i\n", FUNCTION_,
            key.character, key.virt, key.modifier); */
        int code = (key.character & 0xff - 0x20)
                 + (key.modifier << 8);
        return window->vstKeyDown(code);
    }

    bool getRect(ERect** r)
    {
        tf
        updateRect();
        *r = &rect;
        return true;
    }

    void updateRect()
    {
        if (window)
        {
            kali::Size s = window->size();
            rect.left    = 0;
            rect.top     = 0;
            rect.right   = VstInt16(s.w);
            rect.bottom  = VstInt16(s.h);
        }
    }

    ~Editor() {close(); tf}

    Editor(Plugin* plugin) :
        AEffEditor(plugin),
        plugin(plugin),
        window(0)
    {tf}

private:
    Plugin* plugin;
    Window* window;
    ERect   rect;

    Editor(const Editor&);
    Editor& operator = (const Editor&);
};

// ............................................................................

enum KeyModifier
{
    #define _(M) MODIFIER_##M << 8
    Alt   = _(ALTERNATE),
    Cmd   = _(COMMAND),
    Ctrl  = _(CONTROL),
    Shift = _(SHIFT),
    #undef  _
};

// ............................................................................

} // ~ namespace vst

// ............................................................................

#endif // ~ VST_INCLUDED
