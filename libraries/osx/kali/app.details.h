
#ifndef KALI_APP_DETAILS_INCLUDED
#define KALI_APP_DETAILS_INCLUDED

#include "kali/window.h"
#include "kali/ui/native/widgets.h"
#include "kali/ui/custom.h"

// ............................................................................

namespace kali {

// ............................................................................

struct AppDetails : ReleaseAny
{
    void (*entry)();
    Window* mainWindow_;
};

inline Module* app::module()     const {return 0;}
inline Window* app::mainWindow() const {return details_->mainWindow_;}

inline void app::useThreads()
{
    [NSThread detachNewThreadSelector:@selector(isHidden)
        toTarget:NSApp withObject:nil];
}

// ............................................................................

}  // ~ namespace kali

// ............................................................................

@interface      kaliCustomWindow : NSWindow {} @end
@implementation kaliCustomWindow

// - (BOOL) canBecomeMainWindow {return YES;}
- (BOOL) canBecomeKeyWindow {return YES;}

- (void) dealloc
{
    trace.full("%s\n", FUNCTION_);
    [super dealloc];
}

@end

// ............................................................................

@interface      kaliNonDraggableArea : NSView {} @end
@implementation kaliNonDraggableArea
- (void) close {}
- (BOOL) isFlipped              {return YES;}
- (BOOL) mouseDownCanMoveWindow {return NO;}
@end

// ............................................................................

@interface kaliCustomView : NSView
{
    @public kali::ui::custom::LayerBase* layer;
    @public const char* name;
}
@end

@implementation kaliCustomView

- (void) drawRect: (NSRect) rect
{
    rect = rect; // unused
    kali::Context context;
    if (layer)
        layer->draw(context);
}

- (void) mouseUp: (NSEvent*) event
{
    NSPoint p = [self convertPoint:[event locationInWindow] fromView:nil];
    if (layer)
        layer->mouse(kali::up, (int) p.x, (int) p.y);
}

- (void) mouseDown: (NSEvent*) event
{
    NSPoint p = [self convertPoint:[event locationInWindow] fromView:nil];
    if (layer)
        layer->mouse(kali::down, (int) p.x, (int) p.y);
}

- (void) mouseDragged: (NSEvent*) event
{
    NSPoint p = [self convertPoint:[event locationInWindow] fromView:nil];
    if (layer)
        layer->mouseMove((int) p.x, (int) p.y);
}

- (NSString*) view: (NSView*) view stringForToolTip: (NSToolTipTag) tag
    point: (NSPoint) point userData: (void*) userData
{
    view  = view;
    tag   = tag;
    point = point;
    return kali::string::ns((const char*) userData);
}

- (void) dealloc
{
    // trace.warn("%s: %s\n", name, FUNCTION_);
    [super dealloc];
}

- (void) close
{
    // trace.warn("%s: %s\n", name, FUNCTION_);
    [[self subviews] makeObjectsPerformSelector:@selector(close)];
    if (layer)
    {
        layer->close();
        layer->handle = 0;
        layer = 0;
    }
}

- (void) windowWillClose: (NSNotification*) notification
{
    trace.full("%s: %s\n", name, FUNCTION_);

    // (assuming the first NSApp window is the main app window)
    // close all other windows if any:
    NSArray* windows = [NSApp windows];
    if ([windows indexOfObject:[self window]] == 0)
        for (int i = 1; i < [windows count]; i++)
            [[windows objectAtIndex:i] close];

    notification = notification; // unused
    [self close];
}

- (BOOL) isFlipped              {return YES;}
- (BOOL) mouseDownCanMoveWindow {return (self == [[self window] contentView]);}

- (void) menuThunk:  (NSMenuItem*) src {kali::ui::Menu::thunk_(src);}
- (void) timerThunk: (NSTimer*)    src {kali::ui::Timer::thunk_(src);}

@end

// ............................................................................

@interface      AppDelegate : NSObject {} @end
@implementation AppDelegate

- (void) applicationDidFinishLaunching: (NSNotification*) notification
{
    notification = notification; // unused
    kali::app->details_->entry();
}

- (void) applicationWillTerminate: (NSNotification*) notification
{
    trace.full("%s\n", FUNCTION_);
    notification = notification; // unused

    // force windows to close if 'Quit'ting by app or dock menus
    [[NSApp windows] makeObjectsPerformSelector:@selector(close)];

    kali::app.release();
    trace.full("%s [end]\n", FUNCTION_);
}

/* - (NSApplicationTerminateReply) applicationShouldTerminate: (NSApplication*) app
{
    app = app;
    trace.warn("%s\n", FUNCTION_);
    return NSTerminateNow;
} */

- (BOOL) applicationShouldTerminateAfterLastWindowClosed: (NSApplication*) app
{
    app = app;
    return YES;
}

@end

// ............................................................................

@interface      AutoMenuTitles : NSObject {} @end
@implementation AutoMenuTitles
- (NSString*) name  {return [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleName"];}
- (NSString*) help  {return [NSString stringWithFormat:@"%@ Help",  [self name]];}
- (NSString*) hide  {return [NSString stringWithFormat:@"Hide %@",  [self name]];}
- (NSString*) quit  {return [NSString stringWithFormat:@"Quit %@",  [self name]];}
- (NSString*) about {return [NSString stringWithFormat:@"About %@", [self name]];}
@end

// ............................................................................

namespace kali    {
namespace details {

// ............................................................................

inline void applyTitlebarRect(NSWindow* window, const Rect& r)
{
    // TODO: (incomplete impl.)
    NSSize s = [window frame].size;
    NSView* view = [[[kaliNonDraggableArea alloc]
        initWithFrame:NSMakeRect(0, r.h, s.width, s.height)]
        autorelease];
    [[window contentView] addSubview:view];
}

template <typename T>
inline void appEntry()
{
    T* window = app->autorelease(new T);
    app->details_->mainWindow_ = window;
    app->createWindow(0, window, true);
}

// ............................................................................

}  // ~ namespace details

// ............................................................................

template <typename T>
int app::run(bool)
{
    // NOTE: NSApplicationMain may never return actually,
    // so do not allocate any objects on the stack here!

    using kali::app;
    app.alloc();
    app->details_ = app->autorelease(new AppDetails);
    app->details_->entry       = details::appEntry<T>;
    app->details_->mainWindow_ = 0;

    return NSApplicationMain(0, 0);
}

// ............................................................................

template <typename T>
bool app::createWindow(Window* /*parent*/, T* window, bool app_)
{
    unsigned style = !T::SysCaption
        ? NSBorderlessWindowMask
        : NSTitledWindowMask | NSClosableWindowMask
            | (NSMiniaturizableWindowMask * app_);

    kaliCustomWindow* handle = [[kaliCustomWindow alloc]
        initWithContentRect:NSMakeRect(450, 400, 400, 300)
		styleMask:style backing:NSBackingStoreBuffered defer:YES];

    if (!T::SysCaption)
    {
        [handle setMovableByWindowBackground:YES];
        [handle setBackgroundColor:[NSColor clearColor]];
        [handle setOpaque:NO];
    }

    if (T::DropShadow)
        [handle setHasShadow:YES];

    if (!app_)
    {
        [handle setLevel:NSFloatingWindowLevel];
        [handle setHidesOnDeactivate:YES];
    }

    kaliCustomView* view = [[[kaliCustomView alloc]
        initWithFrame:NSMakeRect(0, 0, 200, 150)]
        autorelease];
    [handle setContentView:view];
    [handle setDelegate:view];

    window->handle = view;
    view->name = typeString<T>();
    if (!window->open())
    {
        [handle close];
        return false;
    }

    view->layer = window;
    if (window->titlebar().w != -333)
        details::applyTitlebarRect(handle, window->titlebar());
    [handle makeKeyAndOrderFront:handle];
    return true;
}

// ............................................................................

template <typename T>
bool app::createLayer(Window* parent, T* window)
{
    kaliCustomView* handle = [[[kaliCustomView alloc]
        initWithFrame:NSMakeRect(0, 0, 200, 150)]
        autorelease];
    [parent->handle addSubview:handle];

    window->handle = handle;
    handle->name = typeString<T>();
    if (!window->open())
    {
        [handle removeFromSuperview];
        return false;
    }

    handle->layer = window;
    return true;
}

// ............................................................................

}  // ~ namespace kali

// ............................................................................

#endif // ~ KALI_APP_DETAILS_INCLUDED
