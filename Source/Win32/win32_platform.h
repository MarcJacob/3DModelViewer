/*
    Main Win32 Platform Implementation Header. Contains includes required by Win32 implementation files broadly
    as well as the Win32Platform class declaration.
*/

#ifndef WIN32_PLATFORM_H
#define WIN32_PLATFORM_H

#define WIN32_LEAN_AND_MEAN // Only include the bare minimum from Windows to not pollute namespace.
#include <Windows.h>

#include "Engine/Platform.h"

class Win32Platform : public Platform
{
public:

private:

};

#endif // WIN32_PLATFORM_H