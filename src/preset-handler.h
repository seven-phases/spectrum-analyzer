
#ifndef PRESET_HANDLER_INCLUDED
#define PRESET_HANDLER_INCLUDED

#include "includes.h"
#include "kali/dbgutils.h"
#include "kali/runtime.h"
#include "vst/vst.h"

// ............................................................................

template <int nPresets, int nParameters>
struct PresetBank
{
    int  count;
    int  index;
    char name  [nPresets][28];
    int  value [nPresets][nParameters];
};

template <typename Plugin, int nPresets, int nParameters>
struct PresetHandler :
    vst::PluginBase <Plugin>,
    PresetBank <nPresets, nParameters>
{
    enum
    {
        PresetCount    = nPresets,
        ParameterCount = 1
    };

    virtual const int (&getPreset() const)[nParameters] = 0;
    virtual void setPreset(int (&)[nParameters])  = 0;

    // following 5 funs are workarounds for certain weird hosts
    // (Audition for example) not supporting `n parameters = 0`.
    // Plugin must call invalidatePreset() whenever parameters
    // change (some hosts still ignore it sometimes though :(

    void  invalidatePreset() {setParameterAutomated(0, 0);}

private:

	float getParameter(VstInt32) {return doom;}
    void  setParameter(VstInt32, float v) {doom = v;}
    bool  canParameterBeAutomated(VstInt32) {return false;}
    void  getParameterName(VstInt32, char* v) {copy(v, "None", 5);}

    // ........................................................................

    typedef vst::PluginBase <Plugin>           Base;
    typedef PresetBank <nPresets, nParameters> Bank;

    VstInt32 getProgram()
    {
        // trace.full("%s: %i\n", FUNCTION_, index);
        return index;
    }

	void setProgram(VstInt32 i)
    {
        trace.full("%s(%i)\n", FUNCTION_, i);
        index = i;
        setPreset(value[i]);
    }

    void setProgramName(char* text)
    {
        // trace.full("%s(\"%s\")\n", FUNCTION_, text);
        copy(name[index], text, kVstMaxProgNameLen);
    }

    void getProgramName(char* text)
    {
        // trace.full("%s: \"%s\"\n", FUNCTION_, name[index]);
        copy(text, name[index], kVstMaxProgNameLen);
    }

    bool getProgramNameIndexed(VstInt32, VstInt32 i, char* text)
    {
        // trace.full("%s(%i)\n", FUNCTION_, i);
        return (i < PresetCount)
            ? !!copy(text, name[i], kVstMaxProgNameLen)
            : false;
    }

    VstInt32 getChunk(void** data, bool isPreset)
    {
        trace.full("%s: %s (%p)\n", FUNCTION_,
            isPreset ? "Preset" : "Bank", value[index]);

        memcpy(value[index], getPreset(), sizeof(*value));

        if (isPreset)
        {
            *data = value[index];
            return sizeof(*value);
        }

        *data = &count;
        return sizeof(Bank);
    }

	VstInt32 setChunk(void* data_, VstInt32 size_, bool isPreset)
    {
        trace.full("%s: %s, (%p) size %i\n", FUNCTION_,
            isPreset ? "Preset" : "Bank", data_, size_);

        if (isPreset)
        {
            int n = size_ / sizeof(int);
            n = kali::min(n, nParameters);
            const int* v = (const int*) data_;
            for (int j = 0; j < n; j++)
                value[index][j] = v[j];
            setPreset(value[index]);
            return 1;
        }

        int size = 0;
        const char* data = (const char*) data_;
        int m = *(const int*) data;
        size += sizeof(count);
        index = *(const int*) (data + size);
        size += sizeof(count);
        const char* text = data + size;
        size += m * sizeof(*name);
        int n = (size_ - size) / (m * sizeof(**value));
        const int* v = (const int*) (data + size);

        // trace.full("%s: m %i, n %i\n", FUNCTION_, m, n);

        m = kali::min(m, nPresets);
        n = kali::min(n, nParameters);

        for (int i = 0; i < m; i++)
        {
            copy(name[i], text, sizeof(*name));
            text += sizeof(*name);
            for (int j = 0; j < n; j++)
                value[i][j] = *v++;
        }

        if (index < 0 || index >= nPresets)
            index = 0;
        setPreset(value[index]);

        return 1;
    }

    /* void tracy(const int* v, const char* prefix) const
    {
        typedef kali::details::String <1024> string;
        string s("%s:", prefix);
        for (int i = 0; i < nParameters; i++)
            s.append(string("%s %4i,", 
                (i & 3) ? "" : "\n ", v[i]));
        trace.warn(s.append("\n"));
    } */

    // ........................................................................

public:

    template <typename Defaults>
    PresetHandler(audioMasterCallback master, const Defaults& defaults)
        : Base(master), doom(0.f)
    {
        this->programsAreChunks();

        count = nPresets;
        index = 0;
        for (int i = 0; i < nPresets; i++)
            copy(name[i], defaults(i, value[i]), sizeof(*name));
    }

private:
    float doom;
};

// ............................................................................

#endif // ~ PRESET_HANDLER_INCLUDED
