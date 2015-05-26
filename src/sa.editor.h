
#ifndef SA_EDITOR_INCLUDED
#define SA_EDITOR_INCLUDED

#include "kali/ui/native.h"
#include "sa.settings.h"

// ............................................................................

namespace sa {

// ............................................................................

using namespace kali;
using namespace kali::ui::native;

struct NullWidget : widget::Interface
{
    bool enable() const    {return 0;}
    void enable(bool)      {}
    bool visible() const   {return 0;}
    void visible(bool)     {}
    int  value() const     {return 0;}
    void value(int)        {}
    int  range() const     {return 0;}
    void range(int)        {}
    string text() const    {return string();}
    void text(const char*) {}

    Window::Handle expose() const {return 0;};
    void buddy(Window::Handle) {};
};

struct Compound : widget::Interface
{
    bool   enable() const       {return master->enable();}
    bool   visible() const      {return master->visible();}
    int    value() const        {return master->value();}
    void   value(int v)         {master->value(v);}
    int    range() const        {return master->range();}
    void   range(int v)         {master->range(v);}
    string text() const         {return text_->text();}
    void   text(const char* v)  {text_->text(v);}
    void   label(const char* v) {label_->text(v);}

    void enable(bool v)
    {
        master->enable(v);
        text_->enable(v);
        label_->enable(v);
    }

    void visible(bool v)
    {
        master->visible(v);
        text_->visible(v);
        label_->visible(v);
    }

    void ctor(AnyWidget m, AnyWidget t, AnyWidget l)
    {
        master = m;
        text_  = t;
        label_ = l;

        master->callback.to(this, &Compound::valueAction);
        text_->extCallback.to(this, &Compound::valueAction);
        text_->callback.to(this, &Compound::textAction);
    }

    /*void ctor(widget::Stepper* m, AnyWidget t, AnyWidget l)
    {
        m->buddy(t->expose());
        ctor(AnyWidget(m), t, l);
    }*/

private:
    void valueAction(int) {callback(value());}
    void textAction(int)  {textCallback(0);}

    Window::Handle expose() const {return 0;};
    void buddy(Window::Handle) {};

    Compound(const Compound&);
    Compound& operator = (const Compound&);

public:
    Callback textCallback;
    Compound() {}

private:
    AnyWidget master;
    AnyWidget text_;
    AnyWidget label_;
};

// ............................................................................

struct Editor : LayerBase
{
    typedef Editor This;
    typedef sa::settings::WidgetAdapter Settings;
    typedef const widget::ResourceCtor  Ctor;

    // ........................................................................

    void close()
    {
        tf
        saveCustomColors();
        shared.editor = 0;
        delete this;
    }

    bool open()
    {
        tf
        shared.editor = this;

        Font::Scale c(Font::main().scale());
        int x = c.x(6);
        int y = c.y(7);
        tabs = widget::Ctor<LayerTabs>(this, Rect(x, y));
        initSettingsTab();
        initColorsTab(c);
        initPrefsTab(c);

        Size s = tabs->size();

        widget::Ctor<TextRight>(this, Rect(x + s.w
            - c.x(75+7), y + 3, c.x(75), c.y(15)),
            string("v%s", VERSION_STR));

        x += c.x(7) + s.w;
        y += c.y(6) + s.h;
        y += c.y(8) + c.y(23);
        this->size(x, y);
        this->title(NAME" Settings");

        return true;
    }

    void settingsChanged(bool newColors)
    {
        updateSettingsTab();
        updateColorsTab();

        if (newColors)
        {
            scheme->text("");
            ::Settings key(config::colorsKey);
            for (int i = ColorsIndex; i < config::Count; i++)
                key.set(shared.settings.name(i), shared.settings(i));
        }
    }

    LayerBase* addLayer(const char* tag)
    {
        LayerBase* layer = autorelease(new LayerBase);
        app->loadLayer(tag, this, layer);
        tabs->add(tag, layer);
        return layer;
    }

    // ........................................................................

    static void makeEdit(This* that, Ctor& ctor, int i)
    {
        that->widget[i].ctor(ctor(i + stepperTag),
            ctor(i), ctor(i + labelTag));
    }

    static void makeCombo(This* that, Ctor& ctor, int i)
    {
        Combo master(ctor(i));
        for (int v = 0; v <= that->settings.range(i); v++)
            master->add(that->settings.text(i, v));
        that->widget[i].ctor(&*master,
            &that->nullWidget, ctor(i + labelTag));
    }

    static void makeToggle(This* that, Ctor& ctor, int i)
    {
        Toggle master(ctor(i));
        that->widget[i].ctor(&*master,
            &that->nullWidget, &*master);
    }

    // ........................................................................

    void initSettingsTab()
    {
        using namespace settings;

        struct
        {
            Index index;
            void (*make)(This*, Ctor&, int);
        }

        static const make[] =
        {
            inputChannel,   makeCombo,
            peakEnable,     makeToggle,
            peakDecay,      makeEdit,
            avrgEnable,     makeToggle,
            avrgTime,       makeEdit,
            avrgBarType,    makeCombo,
            avrgBarSize,    makeEdit,
            holdEnable,     makeToggle,
            holdInfinite,   makeToggle,
            holdTime,       makeEdit,
            holdDecay,      makeEdit,
            holdBarType,    makeCombo,
            holdBarSize,    makeEdit,
            levelCeil,      makeEdit,
            levelRange,     makeEdit,
            levelGrid,      makeEdit,
            freqGridType,   makeCombo,
            bandsPerOctave, makeCombo
        };

        Ctor ctor(addLayer("Settings"));

        for (int i = 0; i < (sizeof(make) / sizeof(*make)); i++)
            make[i].make(this, ctor, make[i].index);

        for (int i = 0; i < SettingsCount; i++)
        {
            widget[i].range(settings.range(i));
            widget[i].label(settings.label(i));
            widget[i].callback.to(this, &This::valueChanged, i);
            widget[i].textCallback.to(this, &This::textChanged, i);
        }

        updateSettingsTab();
    }

    void updateSettingsTab()
    {
        for (int i = 0; i < SettingsCount; i++)
        {
            widget[i].value(settings.value(i));
            valueChanged(settings.value(i), i);
        }
    }

    void valueChanged(int value, int index)
    {
        settings.value(index, value);
        widget[index].text(settings.text(index));
        applyDependencies(index);
    }

    void textChanged(int, int index)
    {
        int v = settings.value(index);
        settings.value(index, widget[index].text()());
        if (v != settings.value(index))
            widget[index].value(settings.value(index));
    }

    void applyDependencies(int index)
    {
        using namespace sa::settings;
        if (depended[index].use)
        {
            int value  = 0;
            for (int i = 0; i < SettingsCount; i++)
                value += !!settings.value(i) << i;

            for (int i = 0; i < SettingsCount; i++)
                widget[i].enable(depended[i].value
                    == (value & depended[i].mask));
        }
    }

    // ........................................................................

    void initColorsTab(Font::Scale& c)
    {
        LayerBase* layer = addLayer("Colors");
        Ctor ctor(layer);

        for (int i = 0; i < ColorsCount; i++)
        {
            color[i] = ctor(i + colorTag);
            color[i]->callback.to(this, &This::colorChanged, i);
            AnyWidget edit(ctor(i));
            edit->enable(*settings.unit(i + ColorsIndex) == 'a');
            opacity[i].ctor(ctor(i + stepperTag), edit, ctor(i + labelTag));
            opacity[i].range(100);
            opacity[i].label(settings.label(i + ColorsIndex));
            opacity[i].callback.to(this, &This::colorChanged, i);
            opacity[i].textCallback.to(this, &This::opacityTextChanged, i);
        }

        scheme = ctor(400);
        scheme->callback.to(this, &This::loadColorScheme);
        Rect r(scheme->position(), scheme->size());
        r.x += r.w + c.x(8);
        r.y -= 1;
        r.h += 2;
        Toolbar toolbar = widget::Ctor<Toolbar>(layer, r);
        toolbar->callback.to(this, &This::saveColorScheme);
        toolbar->add(2, "toolbar-off", "toolbar-on");

        updateColorsTab();
        loadCustomColors();
    }

    void updateColorsTab()
    {
        int index = 0;
        string name;
        string current = scheme->text();
        scheme->clear();
        ::Settings key(config::colorsKey);
        while (*(name = key.subKey(index++)))
            scheme->add(name);
        scheme->text(current());

        for (int i = 0; i < ColorsCount; i++)
        {
            color[i]->value(shared.settings(i + ColorsIndex));
            opacity[i].value(argb2opacity(shared.settings(i + ColorsIndex)));
            colorChanged(0, i);
        }
    }

    void loadColorScheme(int value)
    {
        scheme->value(value); // force current value text
        const string& name = scheme->text();
        trace.full("%s: %s\n", FUNCTION_, name());

        ::Settings key(::Settings(config::colorsKey), name);
        for (int i = ColorsIndex; i < config::Count; i++)
            shared.settings(i, key.get(shared.settings.name(i),
                shared.settings(i)), false);

        updateColorsTab();
        shared.settings.notify();
    }

    void saveColorScheme(int delete_)
    {
        Window task; // null window for app-modal alerts
        const string& name = scheme->text();

        if (!*name)
        {
            if (!delete_)
                task.alert(NAME, "Please enter a new"
                    " name for your scheme first.");
            return;
        }

        const char* question[] =
        {
            "Are you sure you want to delete the '%s' scheme?",
            "The scheme '%s' already exists.\nDo you want to overwrite it?"
        };

        ::Settings key(config::colorsKey);
        if (key.exist(name)
            && !task.alertYesNo(NAME, string
                (question[!delete_], name())))
                    return;

        trace.full("%s: %s %s\n", FUNCTION_,
            delete_ ? "delete" : "save", name());

        if (delete_)
            key.deleteKey(name);
        else
        {
            ::Settings sub(key, name);
            for (int i = ColorsIndex; i < config::Count; i++)
                sub.set(shared.settings.name(i), shared.settings(i));
        }

        updateColorsTab();
    }

    static int argb2opacity(int v)
    {
        return ((v & 0xFF000000) + 0xFFFFFF) / 0x28f5c28;
    }

    static int opacity2argb(int op, int argb)
    {
        return (argb & 0xFFFFFF)
            | ((op * 0x28f5c28) & 0xFF000000);
    }

    void colorChanged(int, int index)
    {
        colorUpdate(index, true);
    }

    void opacityTextChanged(int, int index)
    {
        string text = opacity[index].text();
        char* end;
        int v = strtol(text, &end, 10);
        v = max(0, min(100, v));
        if (end != text())
        {
            opacity[index].value(v);
            colorUpdate(index, false);
        }
    }

    void colorUpdate(int index, bool updateText)
    {
        int v = opacity[index].value();
        if (updateText)
            opacity[index].text(string("%i%%", v));
        v = opacity2argb(v, color[index]->value());
        shared.settings(index + ColorsIndex, v);

        ::Settings(config::colorsKey).set
            (shared.settings.name(index + ColorsIndex),
             shared.settings(index + ColorsIndex));
    }

    static void saveCustomColors()
    {
        ::Settings key(config::colorsKey);
        char name[4] = {'.', 0, 0, 0};
        for (int i = 0; i < 16; i++)
        {
            name[1] = char(i + 'a');
            key.set(name, ColorWell::Type::custom(i));
        }
    }

    static void loadCustomColors()
    {
        ::Settings key(config::colorsKey);
        char name[4] = {'.', 0, 0, 0};
        for (int i = 0; i < 16; i++)
        {
            name[1] = char(i + 'a');
            ColorWell::Type::custom(i) = key.get(name, 0x101010 * i);
        }
    }

    // ........................................................................

    void initPrefsTab(Font::Scale& c)
    {
        LayerBase* layer = addLayer("Preferences");

        Rect r(c.x(11) , c.x(15), c.x(300), c.y(16));
        for (int i = 0; i < PrefCount; i++)
        {
            using namespace config;
            pref[i] = widget::Ctor<Toggle>(layer, r, prefs[i].label);
            pref[i]->callback.to(this, &This::prefChanged, prefs[i].index);
            r.y += c.y(13) * 2;
        }

        updatePrefsTab();
    }

    void updatePrefsTab()
    {
        using namespace config;
        PrefName name;
        ::Settings key(prefsKey);
        for (int i = 0; i < PrefCount; i++)
            pref[i]->value(!!key.get(name[i], prefs[i].default_));
    }

    void prefChanged(int value, config::PrefIndex index)
    {
        using namespace config;
        PrefName name;
        ::Settings(prefsKey).set(name[index], value);
    }

    // ........................................................................

    Editor(Shared& shared) :
        shared(shared),
        settings(shared.settings)
    {}

    ~Editor() {tf}

private:

    enum
    {
        SettingsCount = sa::config::ColorsIndex,
        ColorsIndex   = sa::config::ColorsIndex,
        ColorsCount   = sa::config::ColorsCount,
        PrefCount     = sa::config::PrefCount
    };

    sa::Shared&  shared;
    Settings     settings;
    LayerTabs    tabs;
    Button       display;
    Combo        scheme;
    Compound     widget  [SettingsCount];
    ColorWell    color   [ColorsCount];
    Compound     opacity [ColorsCount];
    Toggle       pref    [PrefCount];
    NullWidget   nullWidget;

    enum
    {
        stepperTag = 100,
        labelTag   = 200,
        colorTag   = 300,
        otherTag   = 400,
    };
};

// ............................................................................

} // ~ namespace sa

// ............................................................................

#endif // ~ SA_EDITOR_INCLUDED
