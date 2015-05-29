
#ifndef SA_DISPLAY_INCLUDED
#define SA_DISPLAY_INCLUDED

#include "kali/ui/native.h"
#include "kali/graphics.opengl.h"
#include "analyzer.h"
#include "sa.editor.h"
#include "sa.resizer.h"

// ............................................................................

namespace sa {

using namespace kali;

// ............................................................................

struct DrawData
{
    DrawData() :
        freqMin(0),
        freqMax(0),
        nBands (0)
    {}

    typedef sp::Meter
        <config::pollTime, config::infEdge>
            Meter;

    enum {MaxBands = Analyzer::MaxBands};

    Meter  peak[MaxBands];
    double avrg[MaxBands];
    double freqMin;
    double freqMax;
    int    nBands;
};

// ............................................................................

struct Display : ui::native::LayerBase, DrawData
{
    template <bool H, settings::Index Color, typename T>
    void drawBars(int p[][4][2], int h, const T& level) const
    {
        using namespace config;
        const int n = data->nBands;

        for (int i = 0; i < n; i++)
        {
            int y = int(gridLevelScale
                * (settings(levelCeil) - level[i]));
            p[i][1][1] = p[i][2][1] = y;
            p[i][0][1] = p[i][3][1] = y * H + h;
        }

        gl::color(settings(Color));
        glDrawArrays(GL_QUADS, 0, n * 4);
    }

    template <settings::Index Color, settings::Index Width, typename T>
    void drawCurve(int p[][2], const T& level) const
    {
        using namespace config;
        const int n = data->nBands;

        for (int i = 0; i < n; i++)
            p[i + 1][1] = int(gridLevelScale
                * (settings(levelCeil) - level[i]));

        p[0][1] = p[1][1] * 2 - p[2][1];
        if (p[n - 1][1] < p[n][1])
            p[n + 1][1] = p[n][1] * 2 - p[n - 1][1];
        else
            p[n + 1][1] = p[n][1];

        gl::color(settings(Color));
        glLineWidth(.667f * (.5f + settings(Width)));
        gl::drawCurve_<16>(p, n + 2, .5f);
    }

    // ........................................................................

    void drawPeaks() const
    {
        using namespace config;

        int x = gridRect.x + barPad + 1;
        int y = gridRect.y + barPad + 1;
        int h = gridRect.h - barPad * 2 - 1;
        int w = gridRect.w - barPad * 2 - 1;

        // begin

        glPushMatrix();
        glTranslated(0 - .5, y - .5, 0);

        // set clipping

        glEnable(GL_SCISSOR_TEST);
        glEnableClientState(GL_VERTEX_ARRAY);
        // note: -1 for x/y because of the -.5 translate:
        glScissor(x - barPad - 1, context->size().h
            - (y + h - 1), w + barPad * 2, h);

        // bars

        int p[MaxBands][4][2];
        const int n = data->nBands;
        glVertexPointer(2, GL_INT, 0, p);

        w = max(barWidth, (barWidth + barPad + 1) >> 1);
        int v = barWidth + barPad - w;
        for (int i = 0; i < n; i++)
        {
            p[i][0][0] = p[i][1][0] = x; x += w;
            p[i][2][0] = p[i][3][0] = x; x += v;
        }

        if (settings(peakEnable))
            drawBars<0, peakBarColor>(p, h,
                sp::Iter<const Meter, double, &Meter::peak>(data->peak));

        if (settings(avrgEnable)  && !settings(avrgBarType))
            drawBars<1, avrgBarColor>(p,
                settings(avrgBarSize), data->avrg);

        if (settings(holdEnable) && !settings(holdBarType))
            drawBars<1, holdBarColor>(p, settings(holdBarSize),
                sp::Iter<const Meter, double, &Meter::hold>(data->peak));

        // curves

        int pp[MaxBands + 2][2];
        w = barWidth;
        x = gridRect.x + barPad + 1 + w/2;
        for (int i = 0; i < n; i++)
        {
            pp[i + 1][0] = x;
            x += w + barPad;
        }
        pp[0][0] = pp[1][0] - (w + barPad);
        pp[n + 1][0] = pp[n][0] + (w + barPad);

        glEnable(GL_LINE_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

        if (settings(avrgEnable) && settings(avrgBarType))
            drawCurve<avrgBarColor, avrgBarSize>(pp, data->avrg);

        if (settings(holdEnable) && settings(holdBarType))
            drawCurve<holdBarColor, holdBarSize>(pp,
                sp::Iter<const Meter, double, &Meter::hold>(data->peak));

        glDisable(GL_LINE_SMOOTH);

        // mouse position (TODO: clean up!)
        if (gridRect.contains(mousePos))
        {
            gl::color(settings(gridLabelColor));
            glBegin(GL_QUADS);
            const int pd = 2;
            gl::rectVertices(gridRect.right() - 85 - pd,
                -barPad + pd, gridRect.right() - pd, 15 + pd);
            glEnd();

            int ca = 0xFF000000 | settings(bkgTopColor);
            int cb = 0xFF000000 | settings(bkgBottomColor);
            int cc = ((ca & 0xFEFEFEFEu) >> 1)
                   + ((cb & 0xFEFEFEFEu) >> 1);
            gl::color(cc);

            int q = mousePos.x - (gridRect.x + barPad + barWidth / 2);
            int qw = gridRect.w - (barPad + barWidth / 2) * 2 - 2;
            double freq = data->freqMin * exp(log(data->freqMax/data->freqMin)
                * q / qw);
            double level = settings(levelCeil)
                - (mousePos.y - gridRect.y - 1) / gridLevelScale;
            if (level > 0)
                level += 1 / gridLevelScale;
            gl::drawText(string("%iHz, %.01fdB",
                int(freq + .51), .5 * int(2. * level)),
                font, gridRect.right() + 4 - pd, 9 + pd, -5);
        }

        // end

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisable(GL_SCISSOR_TEST);
        glPopMatrix();
    }

    void drawBackground(bool cache)
    {
        // copy background if cached
        if (cache)
        {
            if (bkgCached)
                return context->copyBuffer(GL_AUX0);
            glDrawBuffer(GL_AUX0);
        }

        // begin
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        glPushMatrix();

        using namespace config;
        const int n = data->nBands;
        gridRect = Rect(context->size());
        Rect& r  = gridRect;

        // background itself
        // (snippet: diagonal gradient fill)
        int ca = 0xFF000000 | settings(bkgTopColor);
        int cb = 0xFF000000 | settings(bkgBottomColor);
        int cc = ((ca & 0xFEFEFEFEu) >> 1)
               + ((cb & 0xFEFEFEFEu) >> 1);
        glBegin(GL_QUADS);
            gl::color(cc);
            glVertex2i(0, r.h);
            gl::color(ca);
            glVertex2i(0, 0);
            gl::color(cc);
            glVertex2i(r.w, 0);
            gl::color(cb);
            glVertex2i(r.w, r.h);
        glEnd();

        // calculate 'the best fit' grid rectangle & peak bar width
        int width = r.w - gridPad.x - gridPad.w;
        barWidth  = (width - barPad - 1) / n - barPad;
        width     = 1 + barPad + n * (barWidth + barPad);
        r.x      += (r.w - width) / 2;
        r.w       = width;
        r.y      += gridPad.y;
        r.h      -= gridPad.y + gridPad.h;

        // vertical scale, px/dB
        gridLevelScale = (r.h - barPad
            * 2 - 2) / double(settings(levelRange));

        // grid border
        glTranslated(r.x - .5, r.y - .5, 0);
        glLineWidth(1);
        gl::color(settings(gridBorderColor));
        glBegin(GL_LINE_LOOP);
            glVertex2i(0, r.h);
            glVertex2i(0, 0);
            glVertex2i(r.w, 0);
            glVertex2i(r.w, r.h);
        glEnd();

        // glEnable(GL_LINE_STIPPLE);
        // glLineStipple(1, 0x1111);

        // vertical grid lines & labels
        const double fRatio1 = 1. / data->freqMin;
        const double fRatio2 = 1. / log(fRatio1 * data->freqMax);
        const int x = barPad + barWidth / 2;
        const int w = r.w - x * 2 - 2;

        const FreqGrid& g = freqGrid[settings(freqGridType)];
        for (int i = 0; i < g.count; i++)
        {
            int xx = 1 + x + int(w * fRatio2
                * log(g.freq[i][0] * fRatio1) + .5);

            gl::color(settings(gridLineColor));
            glBegin(GL_LINES);
                glVertex2i(xx, 1);
                glVertex2i(xx, r.h);
            glEnd();

            if (g.freq[i][1] > 0)
            {
                gl::color(settings(gridLabelColor));
                gl::drawText(freqString(g.freq[i][0]),
                    font, xx + 2, r.h + 11, -3);
            }
        }

        // horizontal grid lines & labels
        const double scale = gridLevelScale;
        const int    top   = settings(levelCeil);
        const int    grid  = settings(levelGrid);
        const int    range = settings(levelRange);
        int level = top - top % grid;
        if (level > top)
            level -= grid;
        while (level > (top - range))
        {
            int y = barPad + 1
                + int(scale * (top - level));

            if (level < top)
            {
                gl::color(settings(gridLineColor));
                glBegin(GL_LINES);
                    glVertex2i(1, y);
                    glVertex2i(r.w - 0, y);
                glEnd();
            }

            gl::color(settings(gridLabelColor));
            gl::drawText(string("%- i", level),
                    font, 0, y + 2, -5);
            gl::drawText(string("%- i", level),
                    font, r.w + 4, y + 2);

            level -= grid;
        }

        // glDisable(GL_LINE_STIPPLE);

        // end
        glPopMatrix();

        // save cache
        if (cache)
        {
            glDrawBuffer(GL_BACK);
            context->copyBuffer(GL_AUX0);
            bkgCached = true;
        }
    }

    static string freqString(double value)
    {
        if (value < 1000)
            return string("%.f", value);

        int rem = int(.01 * fmod(value, 1000));
        return string(rem ? "%ik%i"
            : "%ik", int(.001 * value), rem);
    }

    bool draw()
    {
        if (context && !IsIconic(handle))
        {
            context->begin();
            drawBackground(0 /*bkgCacheEnable*/);
            gl::error(" bkg:");
            drawPeaks();
            gl::error(" peaks:");
            context->end();
        }

        return true;
    }

    void poll()
    {
        resizer.poll(this->handle);

        Analyzer::Peak::Out p;
        const double scale = 1.
            / shared.analyzer->readPeaks(p);

        using namespace config;

        const double inf_ = sp::dB2g(infEdge);
        const double inf  = inf_ * inf_;

        const Meter::Options mo =
        {
            settings(peakDecay),
            settings(holdInfinite),
            settings(holdTime),
            settings(holdDecay),
        };

        const int    size   = settings(avrgTime) / pollTime;
        const double slope  = 10. / nBands * settings(avrgSlope);
        const double outset = 6 - .5 * nBands * slope;

        for (int i = 0; i < nBands; i++)
        {
            peak[i].tick(.5 * sp::g2dB(p.p[i] + inf), mo);
            double a = p.a[i] * scale;
            a = sp::g2dB(avrf[i].tick(a, inf, size));
            avrg[i] = .5 * (outset + a + slope * i);
        }

        if (!freeze)
            draw();
    }

    void settingsChanged()
    {
        freqMin     = shared.analyzer->freqMin;
        freqMax     = shared.analyzer->freqMax;
        const int n = shared.analyzer->nBands;

        if (n != nBands)
        {
            nBands = n;
            reset();
        }

        bkgCached = false;
        ::InvalidateRect(handle, 0, 0);
    }

    void reset()
    {
        for (int i = 0; i < nBands; i++)
        {
            peak[i].reset();
            avrf[i].clear();
        }
    }

    bool toggleFreeze()
    {
        freeze = !freeze;

        if (freeze)
        {
            frozen = *this;
            data = &frozen;
        }
        else
        {
            data = this;
            bkgCached = false;
        }

        return true;
    }

    void openEditor()
    {
        if (shared.editor)
        {
            ::ShowWindow(shared.editor->handle, SW_RESTORE);
            ::SetForegroundWindow(shared.editor->handle);
        }
        else
            kali::app->createWindow(this, new Editor(shared));
    }

    bool mouseDoubleClick()
    {
        reset();
        return true;
    }

    bool mouseR(int e, int, int)
    {
        if (!e)
            openEditor();
        return true;
    }

    bool mouseMove(int x, int y)
    {
        if (!mousePos.x && !mousePos.y)
            enableMouseLeave();
        mousePos = Point(x, y);
        // ::SetCursor(::LoadCursor(0, IDC_SIZENS));
        return true;
    }

    void enableMouseLeave() const
    {
        TRACKMOUSEEVENT tme;
        tme.cbSize      = sizeof(tme);
        tme.dwFlags     = TME_LEAVE;
        tme.hwndTrack   = this->handle;
        tme.dwHoverTime = HOVER_DEFAULT;
        ::TrackMouseEvent(&tme);
    }

    bool keyDown(int key, int flags)
    {
        enum
        {
            Ctrl   = 0x0100,
            Shift  = 0x0200,
            Repeat = 0x1000
        };

        int code = key
            + Ctrl   * (0 > ::GetKeyState(VK_CONTROL))
            + Shift  * (0 > ::GetKeyState(VK_SHIFT))
            + Repeat * !!(flags & KF_REPEAT);

        switch (code)
        {
        case 'R':
        case Ctrl+'R':
            return mouseDoubleClick();

        case 'F':
        case Ctrl+'F':
            return toggleFreeze();
        }

        return false;
    }

    void resized()
    {
        if (::IsIconic(handle) || !context)
            return;

        namespace n = config::parameters;

        Size s = size();
        shared.parameter[n::w] = s.w;
        shared.parameter[n::h] = s.h;

        ::Settings key(config::prefsKey);
        key.set("w", s.w);
        key.set("h", s.h);

        settings.notify();

        context->size(handle);
        settingsChanged();
    }

    bool msgHook(LRESULT& result, UINT msg, WPARAM, LPARAM)
    {
        if (msg != WM_ERASEBKGND)
            return false;

        result = ~0;
        return true;
    }

    // ........................................................................

    void applyParameters()
    {
        using namespace config;
        namespace n = parameters;
        int* v = shared.parameter;

        ::Settings key(prefsKey);
        if (!key.get(PrefName()[smartDisplay],
            prefs[smartDisplay].default_))
        {
            v[n::w] = key.get("w", displaySize.w);
            v[n::h] = key.get("h", displaySize.h);
        }

        this->size(Size(v[n::w], v[n::h]));
    }

    bool open()
    {
        tf
        shared.display = this;

        applyParameters();

        context = new gl::Context(handle);
        font    = new gl::Font("Tahoma", false, -9);
        timer.callback.to(this, &Display::poll);
        timer.start(this, config::pollTime);

        bkgCacheEnable = !!::Settings(config::prefsKey)
            .get("fastGraphics", gl::hasAuxBuffers());

        return true;
    }

    void close()
    {
        tf
        // ! must select context before Font::dtor
        context->begin();
        delete font;
        delete context;
        if (shared.editor)
            shared.editor->destroy();
        shared.editor  = 0;
        shared.display = 0;
    }

    ~Display() {tf}

    template <typename Plugin>
    Display(Plugin* plugin) :
        plugin(plugin),
        shared(plugin->shared),
        settings(shared.settings),
        resizer(plugin),
        context(0),
        font(0),
        barWidth(1),
        bkgCached(0),
        bkgCacheEnable(0),
        freeze(0)
    {
        data = this;
    }

private:

    typedef const settings::Type Settings;
    typedef sp::Averager
        <(20000 / config::pollTime) + 1> AvrgFilter;

private:
    AudioEffectX*   plugin;
    Shared&         shared;
    Settings&       settings;
    Resizer         resizer;
    const DrawData* data;
    AvrgFilter      avrf[MaxBands];
    DrawData        frozen;
    gl::Context*    context;
    gl::Font*       font;
    Point           mousePos;
    Rect            gridRect;
    double          gridLevelScale;
    int             barWidth;
    bool            bkgCached;
    bool            bkgCacheEnable;
    bool            freeze;
    Timer           timer;
};

// ............................................................................

} // ~ namespace sa

// ............................................................................

namespace kali    {
namespace details {

template <>
struct LayerTraits <sa::Display> : TraitsBase <sa::Display>
{
    enum
    {
        classStyle = CS_OWNDC | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW,
        style      = WS_CHILD | WS_CLIPSIBLINGS
    };

    static bool create(const Window* parent, sa::Display* window)
    {
        return createWindow<LayerTraits>(parent, window);
    }
};

template <>
struct Traits <sa::Editor> : TraitsBase <sa::Editor>
{
    enum
    {
        classStyle = 0,
        styleEx    = WS_EX_CONTROLPARENT | WS_EX_TOOLWINDOW,
        style      = WS_SYSMENU | WS_THICKFRAME
    };

    static bool create(const Window* parent, sa::Editor* window)
    {
        return createWindow<Traits>(parent, window);
    }
};

} // ~ namespace details
} // ~ namespace kali

// ............................................................................

#endif // ~ SA_DISPLAY_INCLUDED
