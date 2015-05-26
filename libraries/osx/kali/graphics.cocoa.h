
#ifndef KALI_GRAPHICS_COCOA_INCLUDED
#define KALI_GRAPHICS_COCOA_INCLUDED

#import  <Cocoa/Cocoa.h>
#include "kali/dbgutils.h"
#include "kali/geometry.h"
#include "kali/string.h"

// ............................................................................

namespace kali     {
namespace graphics {

// ............................................................................

struct Colour
{
    int r, g, b, a;

    Colour(int r, int g, int b, int a = 255)
        : r(r), g(g), b(b), a(a) {}

    NSColor* ns() const
    {
        const float s = 1. / 255;
        return [NSColor colorWithDeviceRed:(r * s)
            green:(g * s) blue:(b * s) alpha:(a * s)];
    }
};

// ............................................................................

struct Image : ReleaseAny
{
    Image(const char* image) : handle(ctor(image)) {}
    ~Image() {[handle release];}

    Size size() {return Size([handle size].width, [handle size].height);}

private:

    static NSImage* ctor(const char* name)
    {
        NSString* path  = [[NSBundle mainBundle]
            pathForResource:string::ns(name) ofType:@"png"];
        NSImage*  image = [[NSImage alloc] initWithContentsOfFile:path];
        if (!image)
            trace("%s: \"%s\" failed\n", FUNCTION_, name);
        [image setFlipped:YES];
        return image;
    }

private:
    typedef NSImage* Handle;
    Handle  handle;
    friend  struct Context;
};

struct Font : ReleaseAny
{
    Font(const char* face, float size, Weight weight) :
            handle(ctor(face, size, weight)) {}

private:

    static NSFont* ctor(const char* face, float size, Weight weight)
    {
        NSString* face_ = string::ns(face);
        if (weight)
            face_ = [face_ stringByAppendingString:@" Bold"];
        NSFont* handle = [NSFont fontWithName:face_ size:size];
        if (!handle)
            trace("font %s: \"%s\" style(%i) failed\n",
                FUNCTION_, face, weight);
        return handle;
    }

private:
    typedef NSFont* Handle;
    Handle  handle;
    friend  struct Context;
};

// ............................................................................

struct Context
{
    Context() {}

    void draw(Image& image, const Point& dst)
    {
        draw(image, dst, true);
    }

    void draw(Image& image, const Point& dst, bool opaque)
    {
        [image.handle
            drawAtPoint:NSMakePoint(xy.x + dst.x, xy.y + dst.y)
            fromRect:NSZeroRect operation:NSCompositeSourceOver
            fraction:.33 + opaque * .67];
    }

    template <typename FontType>
    void draw(const char* s, const FontType& f, const Rect& rect, Align align = left)
    {
        NSMutableParagraphStyle* ps =
            [[NSParagraphStyle defaultParagraphStyle] mutableCopy];
        [ps setLineBreakMode:NSLineBreakByClipping];
        NSTextAlignment a[] = {NSLeftTextAlignment,
            NSCenterTextAlignment, NSRightTextAlignment};
        [ps setAlignment:a[align]];

        NSFont* font = f.font->handle;
        NSMutableDictionary* attr = [[NSMutableDictionary alloc] initWithCapacity:8];
        [attr setValue:font forKey:NSFontAttributeName];
        [attr setValue:f.colour.ns() forKey:NSForegroundColorAttributeName];
        [attr setValue:ps forKey:NSParagraphStyleAttributeName];
        // [attr setValue:[NSNumber numberWithFloat:1.f] forKey:NSKernAttributeName];
        // fixme (link this to 'contrast' someway):
        // [attr setValue:[NSNumber numberWithFloat:-1.f] forKey:NSStrokeWidthAttributeName];

        RectF r(rect);
        r.w += .5f;
        r.h += 1.f;
        r.x += .5f;
        r.y += (r.h + [font capHeight] * f.scale.y) * .5f;
        NSRect nsrect = xformRect(r + PointF(xy),
            1.f/f.scale.x, 1.f/f.scale.y);
        NSAffineTransform* xform = [NSAffineTransform transform];
        [xform scaleXBy:f.scale.x yBy:f.scale.y];
        [xform concat];

        if (f.alias)
            [[NSGraphicsContext currentContext] setShouldAntialias:NO];

        [string::ns(s) drawWithRect:nsrect
            options:NSStringDrawingOptions(0) attributes:attr];

        if (f.alias)
            [[NSGraphicsContext currentContext] setShouldAntialias:YES];

        [xform invert];
        [xform concat];

        [attr release];
        [ps   release];
    }

    void offset(int x, int y)
    {
        xy.x += x;
        xy.y += y;
    }

    // for debugging:
    void drawFrame(const Rect& r) {drawFrame(RectF(r));}
    void drawFrame(const RectF& r)
    {
        [[NSColor magentaColor] set];
        NSFrameRect(NSMakeRect(xy.x + r.x, xy.y + r.y, r.w + 1, r.h + 1));
    }



    template <typename T>
    bool fillRect(const T* /*brush*/)
    {
        // TODO
        return true;
    }

private:

    static NSRect xformRect(const RectF& r, float x, float y)
    {
        return NSMakeRect(x * r.x, y * r.y, x * r.w, y * r.h);
    }

private:
    Point xy;
};

// ............................................................................

} // ~ namespace graphics

// ............................................................................

using graphics::Context;
using graphics::Colour;

// ............................................................................

} // ~ namespace kali

// ............................................................................

#endif // ~ KALI_GRAPHICS_COCOA_INCLUDED
