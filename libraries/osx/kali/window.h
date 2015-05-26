
#ifndef KALI_WINDOW_INCLUDED
#define KALI_WINDOW_INCLUDED

#import  <Cocoa/Cocoa.h>
#include "kali/dbgutils.h"
#include "kali/geometry.h"
#include "kali/string.h"

// ............................................................................

namespace kali {

// ............................................................................

struct Window
{
    typedef NSView* Handle;
    Handle handle;
    explicit Window(Handle handle = 0) : handle(handle) {}

public:

    void size(const Size& s)
    {
        size(s.w, s.h);
    }

    void size(int w, int h)
    {
        (handle == [[handle window] contentView])
            ? [[handle window] setContentSize:NSMakeSize(w, h)]
            : [handle setFrameSize:NSMakeSize(w, h)];
    }

    Size size() const
    {
        NSSize s = [handle frame].size;
        return Size(s.width, s.height);
    }

    Size screenSize() const
    {
        NSSize s = [[[handle window] screen] visibleFrame].size;
        return Size(s.width, s.height);
    }

    void position(const Point& p)
    {
        position(p.x, p.y);
    }

    // FIXME, fix flipped posiotion for window coords:

    void position(int x, int y)
    {
        (handle == [[handle window] contentView])
            // ? [[handle window] setFrameTopLeftPoint:NSMakePoint(x, y)]
            ? [[handle window] setFrameOrigin:NSMakePoint(x, y)]
            : [handle setFrameOrigin:NSMakePoint(x, y)];
    }

    Point position() const
    {
        NSRect r = [[handle window] frame];
        // r.origin.y = screenSize().h - r.origin.y - r.size.height;
        return Point(r.origin.x, r.origin.y);
    }

    void redraw(const Rect& r)
    {
        [handle setNeedsDisplayInRect:NSMakeRect(r.x, r.y, r.w, r.h)];
    }

    void update()
    {
        [handle displayIfNeeded];
    }

    void lockUpdate(bool lock) const
    {
        lock
            ? ::NSDisableScreenUpdates()
            : ::NSEnableScreenUpdates();
    }

    void minimize()
    {
        [[handle window] miniaturize:handle];
    }

    void destroy()
    {
        if (handle == [[handle window] contentView])
            [[handle window] close];
        else
            [handle removeFromSuperview];

    }

    void title(const char* text)
    {
        if (handle == [[handle window] contentView])
            [[handle window] setTitle:string::ns(text)];
    }

    string title() const
    {
        return [[[handle window] title]
            cStringUsingEncoding:NSMacOSRomanStringEncoding];

    }

    void icon(const char*)
    {
        // do nothing
        // currently, we set icons via nib
    }

    void clearInput()
    {
        [NSApp discardEventsMatchingMask:NSAnyEventMask beforeEvent:nil];
    }

    void roundCorners(int, Size s = Size())
    {
        // useless here. a background image
        // transparency defines the window shape
        s = s;
    }

    bool alert(const char* /* title */, const char* text, const char* comments = "") const
    {
        return !!NSRunCriticalAlertPanel(string::ns(text),
            string::ns(comments), @"OK", nil, nil);
    }

    bool alertRetryCancel(const char* /* title */, const char* text, const char* comments) const
    {
        return !!NSRunCriticalAlertPanel(string::ns(text),
            string::ns(comments), @"Retry", @"Cancel", nil);
    }

    // temporary here:
    static int OpenFileDialog(const Window*, const char* title,
        const char* filter, char* path, int size, const char* dir = 0, int* fileOffset = 0)
    {
        NSOpenPanel* panel = [NSOpenPanel openPanel];
        [panel setTitle:string::ns(title)];
        int ret = [panel runModalForDirectory:string::ns(dir)
            file:nil types:[NSArray arrayWithObject:string::ns(filter)]];
        if (ret == NSCancelButton)
            return false;

        NSString* file = [[panel filenames] objectAtIndex:0];
        [file getCString:path maxLength:size encoding:NSMacOSRomanStringEncoding];

        if (fileOffset)
            *fileOffset = [file length] - [[file lastPathComponent] length];

        return true;
    }
};

// ............................................................................

} // ~ namespace kali

// ............................................................................

#endif // ~ KALI_WINDOW_INCLUDED
