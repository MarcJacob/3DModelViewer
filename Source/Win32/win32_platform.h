/*
    Main Win32 Platform Implementation Header. Contains includes required by Win32 implementation files broadly
    as well as the Win32Platform class declaration.
*/

#ifndef WIN32_PLATFORM_H
#define WIN32_PLATFORM_H

#define WIN32_LEAN_AND_MEAN // Only include the bare minimum from Windows to not pollute namespace.
#include <Windows.h>

#include "Engine/Platform.h"

/// @brief The Win32 Platform is meant to run on a Windows 10 and later OS-operated machine. It is centered around a Window
/// on which all rendering is done and through which all input events are registered.
/// Uses CPU rendering through the GDI library (#TODO(Marc): Obviously this platform should support hardware acceleration.)
class Win32Platform : public Platform
{
public:

    Win32Platform(HINSTANCE processHandle) : Platform(), Win32_ProcessHandle(processHandle)
    {}

    /// @brief Creates and displays the Win32 Main Window, setting it up for rendering capabilities and handling input events.
    /// @param hInstance HINSTANCE of process.
    /// @return True if initialization was successful, false otherwise.
    bool Win32_InitWindow();

    /// @brief Ran constantly as long as the platform is active, allowing it to poll for new input and trigger rendering updates.
    /// NOTE(Marc): This means that Input polling and CPU Rendering will live on the same thread. 
    void Win32_Update();

    /// @brief Processes a message received from the Platform's Main Window. Returns whether the message was handled. If not,
    /// the default handler for this message type will be called.
    /// @param messageType Type index of the message.
    /// @param wParam W Param of the message (see doc).
    /// @param lParam L Param of the message (see doc).
    /// @return True if the message was handled, false otherwise.
    bool Win32_ProcessWindowMessage(int messageType, WPARAM wParam, LPARAM lParam);

    bool Win32_IsMainWindowActive() const { return Win32_MainWindowHandle != NULL; }

private:

    /// @brief Triggers the Window to close. It needs to be initialized again to reappear.
    void Win32_CloseWindow();

    // Handle to the main Window. NULL if inactive, any other value otherwise.
    HWND Win32_MainWindowHandle;

    // Handle to parent process.
    HINSTANCE Win32_ProcessHandle;
};

#endif // WIN32_PLATFORM_H