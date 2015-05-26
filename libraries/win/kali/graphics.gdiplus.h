
#ifndef KALI_GRAPHICS_GDIPLUS_INCLUDED
#define KALI_GRAPHICS_GDIPLUS_INCLUDED

#include <gdiplus.h>
#include "kali/types.h"
#include "kali/geometry.h"
#include "kali/string.h"
#include "kali/resources.h"

// ............................................................................

namespace kali     {
namespace graphics {

// ............................................................................

typedef Gdiplus::Color Colour;

// ............................................................................

struct Image : ReleaseAny
{
    Image(const char* name) : stream("images", name), handle(stream) {}
    Size size() {return Size(handle.GetWidth(), handle.GetHeight());}

private:
    resource::Stream stream;
    Gdiplus::Bitmap handle;
    friend struct Context;

    // Note: stream has to be open for advanced image effects to work
    // (otherwise we could close it right in c-tor)
};

// ............................................................................

struct Font : ReleaseAny
{
    typedef Gdiplus::Font Handle;

    Font(const char* face, double size, Weight weight)
        : handle(ctor(face, float(size), weight)) {}
    ~Font() {delete handle;}

private:

    static Handle* ctor(const char* face, float size, Weight weight)
    {
        wchar_t s[64];
        return new Handle(string::a2w(s, face), size,
            weight ? Gdiplus::FontStyleBold
            : Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
    }

private:
    Handle* handle;
    friend struct Context;
};

// ............................................................................

struct Context
{
    Context(HDC dc) : handle(dc) {}

    void draw(Image& image, const Point& dst)
    {
        handle.DrawImage(&image.handle,
            xy.x + dst.x, xy.y + dst.y);
    }

    void draw(Image& image, const Point& dst, bool opaque)
    {
        if (opaque)
            return draw(image, dst);

        using namespace Gdiplus;

        ImageAttributes attr;
        attr.SetColorMatrix(transparencyMatrix(),
            ColorMatrixFlagsDefault, ColorAdjustTypeBitmap);
        Gdiplus::Rect r(xy.x + dst.x, xy.y + dst.y,
            image.handle.GetWidth(), image.handle.GetHeight());
        handle.DrawImage(&image.handle, r, 0, 0,
            r.Width, r.Height, UnitPixel, &attr);
    }

    template <typename FontType>
    void draw(const char* s, const FontType& f, const Rect& r, Align align = left)
    {
        using namespace Gdiplus;

        StringFormat format(StringFormat::GenericTypographic());
        format.SetFormatFlags(StringFormatFlagsNoWrap | (format.GetFormatFlags()
            & ~(StringFormatFlagsLineLimit | StringFormatFlagsNoClip)));
        format.SetLineAlignment(StringAlignmentCenter);
        StringAlignment a[] = {StringAlignmentNear,
            StringAlignmentCenter, StringAlignmentFar};
        format.SetAlignment(a[align]);

        handle.SetTextRenderingHint(f.alias
            ? TextRenderingHintAntiAliasGridFit : TextRenderingHintAntiAlias);
        unsigned contrast = handle.GetTextContrast();
        if (!f.alias && (f.contrast_ >= 0))
            handle.SetTextContrast(f.contrast_);
        handle.ScaleTransform(float(f.scale.x), float(f.scale.y));

        wchar_t s_[256];
        SolidBrush b(f.colour);
        Gdiplus::RectF rect = scaleRect(kali::Rect(r).offset(xy),
            1./f.scale.x, 1./f.scale.y);

        handle.DrawString(string::a2w(s_, s),
            -1, f.font->handle, rect, &format, &b);

        handle.ResetTransform();
        handle.SetTextContrast(contrast);
    }

    void offset(int x, int y)
    {
        xy.x += x;
        xy.y += y;
    }

    // for debugging
    void drawFrame(const Rect& r)
    {
        using namespace Gdiplus;
        Pen pen(Color(128, 255, 0, 255), 1);
        handle.DrawRectangle(&pen,
            xy.x + r.x, xy.y + r.y, r.w, r.h);
    }

    // experimental
    template <typename T>
    void fillRect(const T* brush)
    {
        const Gdiplus::RectF rect
            = scaleRect(brush->rect + xy, 1., 1.);
        Gdiplus::LinearGradientBrush lgb(rect,
            brush->colour[0], brush->colour[1],
            (float) brush->angle, 0);
        handle.FillRectangle(&lgb, rect);
    }

private:
    typedef Gdiplus::Graphics Handle;
public: // FIXME!!
    Handle handle;
    Point  xy;
private:
    friend struct BufferedContext;

    Context(Gdiplus::Image* image, int x, int y)
        : handle(image), xy(x, y)
    {}

    static Gdiplus::RectF scaleRect(const Rect& r, double x, double y)
    {
        return Gdiplus::RectF
           ((float) (r.x * x), (float) (r.y * y),
            (float) (r.w * x), (float) (r.h * y));
    }

    typedef Gdiplus::ColorMatrix Matrix;
    static const Matrix* transparencyMatrix()
    {
        static const Matrix aux =
        {
            1.f, 0.f, 0.f,  0.f, 0.f,
            0.f, 1.f, 0.f,  0.f, 0.f,
            0.f, 0.f, 1.f,  0.f, 0.f,
            0.f, 0.f, 0.f, .33f, 0.f,
            0.f, 0.f, 0.f,  0.f, 1.f
        };

        return &aux;
    }
};

struct BufferedContext
{
    BufferedContext(HDC dc, const Rect& r) :
        context(dc),
        image(r.w, r.h, &context.handle),
        this_(&image, -r.x, -r.y)
    {}

    ~BufferedContext()
    {
        context.handle.DrawImage(&image,
            -this_.xy.x, -this_.xy.y);
    }

    operator Context& () {return this_;}

private:
    typedef Gdiplus::Bitmap Image;
    Context context;
    Image   image;
    Context this_;
};

// ............................................................................

struct Initializer : ReleaseAny
{
    Initializer() : token(0)
    {
        Gdiplus::GdiplusStartupInput input;
        Gdiplus::GdiplusStartup(&token, &input, 0);
    }

    ~Initializer()
    {
        Gdiplus::GdiplusShutdown(token);
    }

private:
    ULONG_PTR token;
};

// ............................................................................

} // ~ namespace graphics

// ............................................................................

using graphics::Context;
using graphics::Colour;

// ............................................................................

} // ~ namespace kali

// ............................................................................

#endif // ~ KALI_GRAPHICS_GDIPLUS_INCLUDED
