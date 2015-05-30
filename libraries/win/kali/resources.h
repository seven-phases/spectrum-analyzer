
#ifndef KALI_RESOURCES_INCLUDED
#define KALI_RESOURCES_INCLUDED

#include <windows.h>
#include "kali/dbgutils.h"

// ............................................................................

namespace kali     {
namespace resource {

// ............................................................................

template <typename T, bool Editable = true>
struct Raw
{
    T*  data() const {return data_;}
    int size() const {return size_;}

    Raw(const char* type, const char* id,
        HMODULE module = app->module())
            : data_(0), size_(0), handle(0)
    {
        if (HRSRC rc = ::FindResource(module, id, type))
        {
            handle = ::LoadResource(module, rc);
            size_  = ::SizeofResource(module, rc);
            data_  = (T*) (Editable ? memcpy(malloc(size_),
                ::LockResource(handle), size_) :
                ::LockResource(handle));
        }
        else
            trace("%s: FindResource(%s) failed [%i]\n",
                FUNCTION_, id, ::GetLastError());
    }

    ~Raw()
    {
        #pragma warning(push)
        #pragma warning(disable: 4127)
        if (Editable)
            free(data_);
        if (handle)
            ::FreeResource(handle);
        #pragma warning(pop)
    }

private:
    T*      data_;
    int     size_;
    HGLOBAL handle;

private:
    Raw(const Raw& f);
    Raw& operator = (const Raw&);
};

// ............................................................................

struct Stream : private Raw <void, false>
{
    operator IStream* () const {return stream;}

    Stream(const char* type, const char* id,
        HMODULE module = app->module())
            : Raw(type, id, module), stream(0)
    {
        handle =  ::GlobalAlloc(GMEM_MOVEABLE, size());
        memcpy(::GlobalLock(handle), data(), size());
        ::GlobalUnlock(handle);
        int ret = ::CreateStreamOnHGlobal(handle, TRUE, &stream);
        if (ret)
            trace("%s: CreateStreamOnHGlobal() failed [%i]\n",
                FUNCTION_, ret);
    }

    ~Stream()
    {
        if (stream)
            stream->Release();
        if (handle)
            ::GlobalFree(handle);
    }

private:
    IStream* stream;
    HGLOBAL  handle;

private:
    Stream(const Stream& f);
    Stream& operator = (const Stream&);
};

// ............................................................................

#ifdef GDIPVER

struct ImageBits
{
    enum ImageBitsFlags
    {
        Default = 0,
        Flip    = 1 << 0,
    };

    Size size() const {return size_;}

    const void* data(unsigned flags = Default) {return data(flags, size_);}
    const void* data(unsigned flags, Size dstSize)
    {
        if (bits.Scan0)
            clear();
        load(flags, dstSize);
        return bits.Scan0;
    }

    ImageBits(const char* name) :
        bits(),
        image(resource::Stream("images", name)),
        size_(image.GetWidth(), image.GetHeight())
    {
        if (int ret = image.GetLastStatus())
            trace("%s: \"%s\" load failed [%i]\n", FUNCTION_, name, ret);
    }

    ~ImageBits() {clear();}

private:

    void load(unsigned flags, Size dstSize)
    {
        bits.Width       = size_.w;
        bits.Height      = size_.h;
        bits.Stride      = dstSize.w * 4;
        bits.PixelFormat = PixelFormat32bppARGB;
        bits.Scan0       = malloc(dstSize.w * dstSize.h * 4);

        #if DBG
            memset(bits.Scan0, 0xB1, dstSize.w * dstSize.h * 4);
        #endif

        if (flags & Flip)
        {
            bits.Stride = -bits.Stride;
            bits.Scan0  = ((char*) bits.Scan0)
                - bits.Stride * (bits.Height - 1);
        }

        Gdiplus::Rect rect(0, 0, size_.w, size_.h);
        int ret = image.LockBits(&rect, Gdiplus::ImageLockModeRead
            | Gdiplus::ImageLockModeUserInputBuf, bits.PixelFormat, &bits);
        if (ret)
            trace("%s: LockBits failed [%i]\n", FUNCTION_, ret);

        if (flags & Flip)
            bits.Scan0  = ((char*) bits.Scan0)
                + bits.Stride * (bits.Height - 1);
    }

    void clear()
    {
        image.UnlockBits(&bits);
        free(bits.Scan0);
        bits = Gdiplus::BitmapData();
    }

private:
    struct Initializer {Initializer() {app->initGraphics();}};
    Initializer         initializer_; // need to init gdi+ first
    Gdiplus::BitmapData bits;
    Gdiplus::Bitmap     image;
    Size                size_;

private:
    ImageBits(const ImageBits& f);
    ImageBits& operator = (const ImageBits&);
};

#endif

// ............................................................................

namespace details {

// ............................................................................

inline int screenDPI()
{
    HDC dc  = ::GetDC(0);
    int ret = ::GetDeviceCaps(dc, LOGPIXELSY);
    ::ReleaseDC(0, dc);
    return ret;
}

inline void patchDialogFont(char* data, int size)
{
    char* p = data;
    static const wchar_t f1[] = L"MS Shell Dlg 2";
    static const wchar_t f2[] = L"Segoe UI";
    const int f1size = sizeof(f1) - sizeof(L"");
    const int f2size = sizeof(f2) - sizeof(L"");

    const int dpi = screenDPI();
    NONCLIENTMETRICSW ncm = {sizeof(ncm)};
    ::SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);
    int height = (-ncm.lfMessageFont.lfHeight * 72 + (dpi / 2)) / dpi;

    while (--size > 0)
    {
        if (!wcscmp((wchar_t*) p, f1))
        {
            *((short*) (p - 6)) = short(height);
            if (!wcscmp(ncm.lfMessageFont.lfFaceName, f2))
            {
                memcpy(p, f2, sizeof(f2) - 2);
                memmove(p + f2size, p + f1size, size + 1 - f1size);
            }

            return;
        }
        ++p;
    }
}

// ............................................................................

inline BOOL CALLBACK firstIconAux_(HMODULE, LPCTSTR, LPTSTR name, LONG_PTR dst)
{
    if (!IS_INTRESOURCE(name))
        *((string*) dst) = (const char*) name;
    return 0;
}

inline string firstIcon()
{
    string value;
    ::EnumResourceNames(app->module(),
        RT_GROUP_ICON, firstIconAux_, (LONG_PTR) &value);
    return value;
}

// ............................................................................

} // ~ namespace details
} // ~ namespace resource
} // ~ namespace kali

// ............................................................................

#endif // ~ KALI_RESOURCES_INCLUDED
