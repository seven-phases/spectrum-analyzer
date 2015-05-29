
#ifndef PLUGIN_INCLUDED
#define PLUGIN_INCLUDED

// #define  DBG  1
#define  PERF 0
#define  PROCESS_DBL 1

#include "preset-handler.h"
#include "sa.legacy.h"
#include "sa.display.h"
#include "version.h"

// ............................................................................

struct Plugin :
    sp::AlignedNew <16>,
    PresetHandler <Plugin, 8, sa::config::ParameterCount>
{
    typedef PresetHandler <Plugin, 8, sa::config::ParameterCount> Base;

    void suspend()
    {
        tf
        analyzerUpdate();
    }

    void setSampleRate(float rate)
    {
        Base::setSampleRate(rate);
        analyzerUpdate();
    }

    void settingsChanged(int index)
    {
        // trace.full("%s: %i\n", FUNCTION_, index);

        if (index == sa::settings::bandsPerOctave)
            analyzerUpdate();

        if (shared.display)
            shared.display->settingsChanged();

        this->invalidatePreset();
    }

    void analyzerUpdate()
    {
        using namespace sa::settings;

        const int bpo[] = {3, 4, 6};
        analyzer.update(sampleRate, bpo
            [shared.settings(bandsPerOctave)]);

        trace.full("%s(%.0f): nBands %i"
            ", [%.2f %.2f]\n", FUNCTION_,
            sampleRate, analyzer.nBands,
            analyzer.freqMin, analyzer.freqMax);
    }

    template <typename T> inline_
    void process(const T* const* in, T* const* out, int n)
    {
        bypass(in, out, n);
        int ch = shared.settings
            (sa::settings::inputChannel);
        analyzer.process(in, n, ch);
    }

    template <typename T> inline_
    void bypass(const T* const* in, T* const* out, int n) const
    {
        const T* in0  =  in[0];
        const T* in1  =  in[1];
              T* out0 = out[0];
              T* out1 = out[1];

        while (--n >= 0)
        {
            *out0++ = *in0++;
            *out1++ = *in1++;
        }
    }

#if PERF
    void processReplacing(float** in, float** out, int n);
    void processDoubleReplacing(double** in, double** out, int n);
#else
    void processReplacing(float** in, float** out, int n)         {process(in, out, n);}
    #if PROCESS_DBL
    void processDoubleReplacing(double** in, double** out, int n) {process(in, out, n);}
    #endif
#endif

    const int (&getPreset() const)[sa::config::ParameterCount] {return shared.parameter;}

    void setPreset(int (&value)[sa::config::ParameterCount])
    {
        using namespace sa::config;
        using sa::config::ParameterCount;
        namespace p = parameters;

        if (!sa::legacy::convertPreset(value))
            return trace("%s: unsupported preset version %i\n",
                FUNCTION_, value[p::version]);

        bool applyColors = !Settings(prefsKey).get
            (PrefName()[keepColors], prefs[keepColors].default_);
        int n = applyColors ? Count : ColorsIndex;
        for (int i = 0; i < n; i++)
            shared.settings(i, value[i + SettingsIndex], false);

        analyzerUpdate();

        if (shared.editor)
            shared.editor->settingsChanged(applyColors);

        if (shared.display)
            shared.display->settingsChanged();
        else
        {
            // apply display settings only if it's not yet on screen
            if (Settings(prefsKey).get(PrefName()
                [smartDisplay], prefs[smartDisplay].default_))
            {
                for (int i = p::w; i <= p::h; i++)
                    shared.parameter[i] = value[i];
            }
        }

        this->invalidatePreset();
    }

    enum
    {
        UniqueID = 'SPhA',
        Version  = int(VERSION * 1000),
    };

    static const char* name()   {return NAME;}
    static const char* vendor() {return COMPANY;}

    ~Plugin()
    {
        // just in case host forgets to call Editor::close before deleting
        // plugin, get rid of all windows *before* AudioEffect dtor!
        if (editor)
        {
            delete editor;
            editor = 0;
        }

        tf
    }

    Plugin(audioMasterCallback master)
        : Base(master, sa::config::Defaults())
    {
        tf
        this->setNumInputs(2);
	    this->setNumOutputs(2);
        #if PROCESS_DBL
            this->canDoubleReplacing();
        #endif

        shared.analyzer = &analyzer;
        this->setEditor(new vst::Editor
            <Plugin, sa::Display>(this));
        shared.settings.callback.to
            (this, &Plugin::settingsChanged);

        analyzerUpdate(); // must init analyzer before editor/display open
    }

public:
    sa::Shared shared;

private:
    Analyzer analyzer;
};

// ............................................................................

#endif // ~ PLUGIN_INCLUDED
