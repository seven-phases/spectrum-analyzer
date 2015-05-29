
#ifndef SA_WIDGETS_INCLUDED
#define SA_WIDGETS_INCLUDED

#include "sa.resizer.h"

// ............................................................................

namespace sa {
    
// ............................................................................

using namespace kali;

template <typename Base>
struct TrackMousePosition : Base
{
    bool mouseMove(int x, int y)
    {
        if (!mousePos.x && !mousePos.y)
            enableMouseLeave();
        mousePos = Point(x, y);
        return true;
    }

private:

    void enableMouseLeave() const
    {
        TRACKMOUSEEVENT tme;
        tme.cbSize      = sizeof(tme);
        tme.dwFlags     = TME_LEAVE;
        tme.hwndTrack   = this->handle;
        tme.dwHoverTime = HOVER_DEFAULT;
        ::TrackMouseEvent(&tme);
    }

protected:
    Point mousePos;
};

// ............................................................................

} // ~ namespace sa

// ............................................................................

#endif // ~ SA_WIDGETS_INCLUDED
