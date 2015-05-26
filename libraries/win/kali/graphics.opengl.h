
#ifndef GL_INCLUDED
#define GL_INCLUDED

#pragma comment(lib, "opengl32.lib")

#include <malloc.h>
#include <windows.h>
#include <gl/gl.h>

#include "kali/dbgutils.h"

// ............................................................................

namespace gl {

using namespace kali;

void error(const char* id = "");

// ............................................................................

struct Context
{
    Context(HWND window)
    {
        dc = ::GetDC(window);
        setPixelFormat(dc);
        rc = ::wglCreateContext(dc);

        size(window);
        configure();
    }

    ~Context()
    {
        ::wglMakeCurrent(0, 0); // also releases dc
        ::wglDeleteContext(rc);
    }

    void begin()
    {
        // wglMakeCurrent seems to cause memory leaks
        // so let's call it as rare as possible
        if (rc != ::wglGetCurrentContext())
            if (!::wglMakeCurrent(dc, rc))
                trace("%s: wglMakeCurrent failed [%i]\n",
                    FUNCTION_, ::GetLastError());
    }

    void end()
    {
        // flush only if multiple threads
        // draw to the same window:
        // glFlush();
        ::SwapBuffers(dc);
    }

    void size(int w, int h)
    {
        begin();
        glViewport(0, 0, w, h);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, w, h, 0, -1, 1); // ordinal '2D-window' projection

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }

    static Size size()
    {
        int v[4] = {0};
        glGetIntegerv(GL_VIEWPORT, v);
        return Size(v[2], v[3]);
    }

    void size(HWND window)
    {
        RECT r;
        ::GetClientRect(window, &r);
        size(r.right, r.bottom);
    }

    void configure()
    {
        // glShadeModel(GL_SMOOTH); // <- default
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // glEnable(GL_POLYGON_SMOOTH);
        // glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

        // glEnable(GL_LINE_SMOOTH);
        // glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    }

    // copy pixels from 'src' (usually AUX) to current draw buffer
    void copyBuffer(GLenum src)
    {
        Size s = size();
        glReadBuffer(src);
        glRasterPos2i(0, s.h);
        glCopyPixels(0, 0, s.w, s.h, GL_COLOR);
    }

private:

    static void setPixelFormat(HDC dc)
    {
        static const PIXELFORMATDESCRIPTOR pfd =
        {
            sizeof(pfd),         // size of structure
            1,                   // version number
            PFD_DRAW_TO_WINDOW | // support window
            PFD_SUPPORT_OPENGL | // support OpenGL
            PFD_DOUBLEBUFFER   | // double buffered
            PFD_DEPTH_DONTCARE,  // no depth buffer is fine
            PFD_TYPE_RGBA,       // RGBA type
            32,                  // color bits
            0, 0, 0, 0, 0, 0,    // rgb bits/shift
            8, 0,                // alpha bits/shift
            0,                   // accum bits
            0, 0, 0, 0,          // accum rgba bits
            8,                   // depth buffer
            1,                   // stencil buffer
            0,                   // auxiliary buffers
            PFD_MAIN_PLANE,      // layer type (ignored)
            0,                   // reserved
            0, 0, 0              // layer masks
        };

        int pf = ::ChoosePixelFormat(dc, &pfd);
        if (!pf)
            return trace("%s: ChoosePixelFormat failed [%i]\n",
                FUNCTION_, ::GetLastError());

        #if 0
            PIXELFORMATDESCRIPTOR pfd2 = {};
            int ret = ::DescribePixelFormat(dc, pf, sizeof(pfd2), &pfd2);
            if (!ret)
                return trace("%s: DescribePixelFormat failed [%i]\n",
                    FUNCTION_, ::GetLastError());

            #define _(name, format) \
                trace.full(#name": "format" "format"\n", pfd.name, pfd2.name)
            trace.full("\nPixelFormat: %i\n", pf);
            _(dwFlags,      "0x%08x");
            _(cColorBits,   "%i");
            _(cRedBits,     "%i");
            _(cGreenBits ,  "%i");
            _(cBlueBits,    "%i");
            _(cAlphaBits,   "%i");
            _(cDepthBits,   "%i");
            _(cStencilBits, "%i");
            _(cAccumBits ,  "%i");
            #undef _
        #endif

        if (!::SetPixelFormat(dc, pf, &pfd))
            return trace("%s: SetPixelFormat failed [%i]\n",
                FUNCTION_, ::GetLastError());
    }

private:
    HDC   dc;
    HGLRC rc;

    Context(const Context&);
    Context& operator = (const Context&);
};

// ............................................................................

struct Font
{
    Font(const char* face, bool bold, int height,
        int offset = offset_, int count = count_)
            : handle(0), offset(offset), count(count)
                {ctor(face, bold, height);}

    explicit Font(int offset = offset_, int count = count_)
        : handle(0), offset(offset), count(count)
            {ctor(::GetStockObject(DEFAULT_GUI_FONT));}

    ~Font()
    {
        if (handle)
            glDeleteLists(handle, 96);
    }

private:

    enum
    {
        offset_ = ' ',
        count_  = '~' - offset_ + 1
    };

    void ctor(HGDIOBJ font)
    {
        handle  = glGenLists(count);
        HDC dc  = ::wglGetCurrentDC();
        void* old = ::SelectObject(dc, font);
        if (!::wglUseFontBitmaps(dc, offset, count, handle))
            trace("%s: wglUseFontBitmaps failed [%i]\n",
                FUNCTION_, ::GetLastError());
        ::SelectObject(dc, old);
    }

    void ctor(const char* face, bool bold, int height)
    {
        HFONT font = ::CreateFont
        (
            height,						 // Height Of Font
			0,							 // Width Of Font
			0,							 // Angle Of Escapement
			0,							 // Orientation Angle
            bold ? FW_BOLD : FW_NORMAL,	 // Font Weight
			0,						     // Italic
			0,						     // Underline
			0,						     // Strikeout
			DEFAULT_CHARSET,		     // Character Set Identifier
			OUT_TT_PRECIS,				 // Output Precision
			CLIP_DEFAULT_PRECIS,		 // Clipping Precision
			ANTIALIASED_QUALITY,		 // Output Quality
			FF_DONTCARE | DEFAULT_PITCH, // Family And Pitch
			face                         // Font Name
        );

        ctor(font);
        ::DeleteObject(font);
    }

private:
    int handle;
    int offset;
    int count;

    Font(const Font&);
    Font& operator = (const Font&);

    friend void drawText(const char*, const Font*, int, int, int);
};

inline void drawText(const char* text, const Font* font, int x, int y, int align = 0)
{
    // glPushAttrib(GL_LIST_BIT);
    int n = (int) strlen(text);
    glListBase(font->handle - font->offset_);
    glRasterPos2i(x + n * align, y);
    glCallLists(n, GL_UNSIGNED_BYTE, text);
    // glPopAttrib();
}

// ............................................................................
// some types

const int VertexSize_ = 2;
typedef double Vertex[VertexSize_];

// ............................................................................
// primitive helpers

inline int abgr_(int v)
{
    // ARGB <-> ABGR
    return (v & 0xFF00FF00)
        | ((v & 0x00FF0000) >> 16)
        | ((v & 0x000000FF) << 16);
}

inline void color(int argb)
{
    argb = abgr_(argb);
    glColor4ubv((const GLubyte*) &argb);
}

inline void drawPoints(const Vertex* p, int count, float size)
{
    glPointSize(size);
    glBegin(GL_POINTS);
        for (int i = 0; i < count; i++)
            glVertex2dv(p[i]);
    glEnd();
}

inline void rectVertices(int l, int t, int r, int b)
{
    glVertex2i(l, t);
    glVertex2i(r, t);
    glVertex2i(r, b);
    glVertex2i(l, b);
}

inline void drawRect(const Rect& r)
{
    glBegin(GL_QUADS);
        gl::rectVertices
            (r.x, r.y, r.x + r.w, r.y + r.h);
    glEnd();
}

inline void drawGradientRect(const Rect& r, const int (&color)[4])
{
    glBegin(GL_QUADS);
        gl::color(color[0]);
        glVertex2i(r.x,       r.y);
        gl::color(color[1]);
        glVertex2i(r.x + r.w, r.y);
        gl::color(color[2]);
        glVertex2i(r.x + r.w, r.y + r.h);
        gl::color(color[3]);
        glVertex2i(r.x,       r.y + r.h);
    glEnd();
}

inline void drawFrame(const Rect& r)
{
    glBegin(GL_LINE_LOOP);
        gl::rectVertices
            (r.x, r.y, r.x + r.w, r.y + r.h);
    glEnd();
}

// ............................................................................

template <typename T> noalias_ inline_
void cardinalSpline_(T dst[2], const int src[][2], double t, double s_)
{
    const double s[4] = {1, s_, s_ * s_, s_ * s_ * s_};
    const double m[4] =
    {
        -s[3] * t + 2 * (s[2] * t) - s[1] * t,
         s[3] * (2 - t) - s[2] * (3 - t) + 1,
        -s[3] * (2 - t) + s[2] * (3 - 2 * t) + s[1] * t,
         s[3] * t - s[2] * t,
    };

    for (int i = 0; i < 2; i++)
    {
        double x;

        x  = src[0][i] * m[0];
        x += src[1][i] * m[1];
        x += src[2][i] * m[2];
        x += src[3][i] * m[3];

        dst[i] = T(x);
    }
}

#ifndef GL_BRUTE_DRAW_CURVE_
#define GL_BRUTE_DRAW_CURVE_ 1
#endif

template <int segments> noalias_
void drawCurve_(const int points[][2], int count, float tension)
{
    #if GL_BRUTE_DRAW_CURVE_
    #define malloc alloca
    #define free(p)
    #endif

    // "cl /analyze" may complain here about dereferencing
    // null pointers, but it's all correct actually

    int n = count + 2;
    int (*src)[2] = (int (*)[2]) malloc(n * 2 * sizeof(int));
    memcpy(src + 1, points, (n - 2) * 2 * sizeof(int));
    src[0][0]     = src[1][0];
    src[0][1]     = src[1][1];
    src[n - 1][0] = src[n - 2][0];
    src[n - 1][1] = src[n - 2][1];

    n -= 3;
    float (*p)[segments][2] = (float (*)[segments][2])
        malloc((n * segments + 1) * 2 * sizeof(float));

    for (int i = 0; i < n; i++)
        for (int j = 0; j < segments; j++)
            cardinalSpline_(p[i][j], src + i,
                tension, (1. / segments) * j);

    p[n][0][0] = float(src[n + 1][0]);
    p[n][0][1] = float(src[n + 1][1]);

    glVertexPointer(2, GL_FLOAT, 0, p);
    glDrawArrays(GL_LINE_STRIP, 0, n * segments + 1);

    free(p);
    free(src);

    #if GL_BRUTE_DRAW_CURVE_
    #undef malloc
    #undef free
    #endif
}

// ............................................................................
// low level utility

inline void error(const char* prefix)
{
    #if DBG
    unsigned ret = glGetError();
    if (ret)
        trace("%s: %s: 0x%04X\n", prefix, FUNCTION_, ret);
    #endif
    prefix = prefix;
}

inline double getd_(GLenum param)
{
    double value = 0;
    glGetDoublev(param, &value);
    error();
    return value;
}

inline int geti_(GLenum param)
{
    int value = 0;
    glGetIntegerv(param, &value);
    error();
    return value;
}

inline bool hasAuxBuffers()
{
    if (geti_(GL_AUX_BUFFERS) < 1)
        return false;

    if (strstr((const char*) glGetString(GL_VENDOR), "Intel"))
        return false;

    return true;
}

inline void info(bool ext = 0)
{
    trace.full("\n................................................\n");
    trace.full("Vendor:   %s\n", (const char*) glGetString(GL_VENDOR));
    trace.full("Renderer: %s\n", (const char*) glGetString(GL_RENDERER));
    trace.full("Version:  %s\n", (const char*) glGetString(GL_VERSION));

    #ifdef GL_SHADING_LANGUAGE_VERSION
    trace.full("GLSL:     %s\n", (const char*) glGetString(GL_SHADING_LANGUAGE_VERSION));
    #endif

    if (!ext)
        return;
    trace.full("Extensions:\n");
    const char* s = (const char*) glGetString(GL_EXTENSIONS);
    while (s && *s)
    {
        size_t n = strcspn(s, " ");
        trace.full("    %.*s\n", n, s);
        s += n + 1;
    }
}

// ............................................................................

} // ~ namespace gl

// ............................................................................

#endif // GL_INCLUDED
