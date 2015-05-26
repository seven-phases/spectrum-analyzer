
#ifndef KALI_API_ERROR_INCLUDED
#define KALI_API_ERROR_INCLUDED

#include <sys/errno.h>
#include <IOKit/IOReturn.h>

// ............................................................................

enum ApiErrors
{
    ERROR_NOT_FOUND              = kIOReturnNoDevice,
    ERROR_OPEN_FAILED            = ENOENT,
    ERROR_DISK_OPERATION_FAILED  = kIOReturnIOError,
    ERROR_NO_DATA_DETECTED       = kIOReturnNotReady,
    ERROR_ACCESS_DENIED          = kIOReturnNotPermitted,
    ERROR_BAD_ARGUMENTS          = kIOReturnBadArgument,
    ERROR_NOT_SUPPORTED          = kIOReturnUnsupported,
    ERROR_INVALID_DATA           = kIOReturnBadMedia,

    ERROR_UNSPECIFIED            = 0x76543210 // Bang!
};

// ............................................................................

#endif // ~ KALI_API_ERROR_INCLUDED
