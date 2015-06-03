
#ifndef SA_DISPLAY_INCLUDED
#define SA_DISPLAY_INCLUDED

#include "kali/ui/native.h"
#include "kali/graphics.opengl.h"
#include "analyzer.h"
#include "sa.editor.h"
#include "sa.widgets.h"

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

struct Display : DrawData,
    TrackMousePosition <ui::native::LayerBase>
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
        glVertexPointer(2, GL_INT, 0, p);
        glDrawArrays(GL_QUADS, 0, n * 4);
    }

    template <settings::Index Color, settings::Index Width, typename T>
    void drawCurve(float p[][2], int n, const T& level, bool fill) const
    {
        using namespace config;

        n -= 2;
        const double ceil = settings(levelCeil) * gridLevelScale;
        for (int i = 0; i < n; i++)
            p[i + 1][1] = float(ceil - level[i] * gridLevelScale);

        p[0][1] = p[1][1] * 2 - p[2][1];
        if (p[n - 1][1] < p[n][1])
            p[n + 1][1] = p[n][1] * 2 - p[n - 1][1];
        else
            p[n + 1][1] = p[n][1];

        unsigned color = settings(Color);
        const float (*fillRect)[2] = 0;
        
        Rect r = gridRect;
        const float pp[4][2] = {
            float(r.x + r.w), float(-1 + r.h),
            float(r.x),       float(-1 + r.h),
            float(r.x),       float(-1),
            float(r.x + r.w), float(-1),
        };

        if (fill) {
            fillRect = pp;
            color = (color & 0xffffff)
                  + (color >> 1u & 0xff000000);
        }

        gl::color(color);
        float width = .667f * (.5f + settings(Width));
        gl::drawCurve_<16>(p, n + 2, fillRect, width);
    }

    // ........................................................................

    void drawForeground() const
    {
        using namespace config;

        int x = gridRect.x + barPad + 1;
        int y = gridRect.y + barPad + 1;
        int h = gridRect.h - barPad * 2 - 1;
        int w = gridRect.w - barPad * 2 - 1;

        // clipping

        glEnable(GL_SCISSOR_TEST);
        glScissor(x - 1, y, w, h);

        // begin
        glPushMatrix();
        glTranslatef(0 - .5f, y - .5f, 0);
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnable(GL_LINE_SMOOTH);

        // bar x points

        const int n = data->nBands;
        int p[MaxBands][4][2];
        w = max(barWidth, (barWidth + barPad + 1) >> 1);
        int v = barWidth + barPad - w;
        for (int i = 0; i < n; i++)
        {
            p[i][0][0] = p[i][1][0] = x; x += w;
            p[i][2][0] = p[i][3][0] = x; x += v;
        }

        // curve x points

        float pp[MaxBands + 2][2];
        float xx = gridRect.x + barPad + (w + 1) * .5f;
        w = barWidth + barPad;
        for (int i = 0; i < n; i++)
        {
            pp[i + 1][0] = xx; xx += w;
        }
        pp[0][0]     = pp[1][0] - (w + barPad);
        pp[n + 1][0] = pp[n][0] + (w + barPad);

        // render

        const sp::Iter<const Meter, double, &Meter::peak> peakIter(data->peak);
        const sp::Iter<const Meter, double, &Meter::hold> holdIter(data->peak);

        const int holdType = settings(holdBarType);
        const int avrgType = settings(avrgBarType);

        // better to draw peak on top of hold if the latter is curve-fill
        if ((holdType != CurveFill) && settings(peakEnable))
            drawBars<0, peakBarColor>(p, h, peakIter);

        if (settings(holdEnable)) holdType
            ? drawCurve<holdBarColor, holdBarSize>
                (pp, n + 2, holdIter, holdType == CurveFill)
            : drawBars<1, holdBarColor>(p, settings(holdBarSize), holdIter);

        if ((holdType == CurveFill) && settings(peakEnable))
            drawBars<0, peakBarColor>(p, h, peakIter);
        
        if (settings(avrgEnable)) avrgType
            ? drawCurve<avrgBarColor, avrgBarSize>
                (pp, n + 2, data->avrg, avrgType == CurveFill)
            : drawBars<1, avrgBarColor>(p, settings(avrgBarSize), data->avrg);

        drawPointerInfo();

        // end

        glDisable(GL_LINE_SMOOTH);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisable(GL_SCISSOR_TEST);
        glPopMatrix();
    }

    void drawBackground()
    {
        // begin
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        glPushMatrix();

        using namespace config;
        const int n = data->nBands;
        gridRect = Rect(context->size());
        Rect& r  = gridRect;

        // background itself
        gl::drawRectDiagonalGradient(Rect(0, 0, r.w, r.h),
            0xFF000000 | settings(bkgTopColor),
            0xFF000000 | settings(bkgBottomColor));

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
        glTranslatef(r.x - .5f, r.y - .5f, 0);
        glLineWidth(1);
        gl::color(settings(gridBorderColor));
        gl::drawRectFrame(Rect(0, 0, r.w, r.h));

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

        // end
        glPopMatrix();
    }

    void drawPointerInfo() const  // TODO: clean up!
    {
        if (!gridRect.contains(mousePos))
            return;

        using namespace config;

        const Rect& r = gridRect; 
        const int   x = r.right(); 
        const int bar = barPad + barWidth / 2;

        gl::color(settings(gridLabelColor));
        glBegin(GL_QUADS);
        gl::rectVertices(x - 85, -1, x - barPad, 18 - barPad);
        glEnd();

        int ca = 0xFF000000 | settings(bkgTopColor);
        int cb = 0xFF000000 | settings(bkgBottomColor);
        int cc = ((ca & 0xFEFEFEFEu) >> 1)
               + ((cb & 0xFEFEFEFEu) >> 1);
        gl::color(cc);

        int mx = mousePos.x - r.x + bar;
        int gx = r.w - bar * 2 - 2;
        double freq = freqMin * exp
            (log(freqMax / freqMin) * mx / gx);
        double level = settings(levelCeil)
            - (mousePos.y - r.y - 1) / gridLevelScale;
        if (level > 0)
            level += 1 / gridLevelScale;
        gl::drawText(string("%iHz, %.01fdB",
            int(freq + .51), .5 * int(2. * level)),
            font, x + 3, 11, -5);
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
            drawBackground();
            gl::error(" back:");
            drawForeground();
            gl::error(" fore:");
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
            // copy current data to frozen
            frozen = *this; 
            data = &frozen;
        }
        else
            data = this;

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

    bool vstKeyDown(int key)
    {
        using namespace vst;

        switch (key)
        {
        case      'R':
        case Ctrl+'R':
            return mouseDoubleClick();

        case      'F':
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
    Rect            gridRect;
    double          gridLevelScale;
    int             barWidth;
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
