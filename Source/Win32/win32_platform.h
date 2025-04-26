/*
    Main Win32 Platform Implementation Header. Contains includes required by Win32 implementation files broadly
    as well as the Win32Platform class declaration.
*/

#ifndef WIN32_PLATFORM_H
#define WIN32_PLATFORM_H

#define WIN32_LEAN_AND_MEAN // Only include the bare minimum from Windows to not pollute namespace.
#include <Windows.h>

#include <queue>
#include <mutex>

#include "Engine/Platform.h"

class Win32PlatformDebugger : public PlatformDebugger
{
public:
    // Make sure Platform's overloads are visible in this scope for overload resolution.
    using PlatformDebugger::DisplayDebugMessage;
    virtual void DisplayDebugMessage(DebugLogMessage&& message) override;

    // Triggers a flush of all Debug Log Messages in queue.
    void Win32_FlushDebugLogQueue();

private:

    std::mutex Mutex_DebugMessageQueue;
    std::queue<DebugLogMessage> DebugMessageQueue;
};

/// @brief The Win32 Platform is meant to run on a Windows 10 and later OS-operated machine. It is centered around a Window
/// on which all rendering is done and through which all input events are registered.
/// Uses CPU rendering through the GDI library (#TODO(Marc): Obviously this platform should support hardware acceleration.)
class Win32Platform
{

    // WIN32 Internal Platform Functionnality
public:

    Win32Platform(HINSTANCE processHandle) : m_processHandle(processHandle)
    {}

    /// @brief Initializes subsystems such as debugging, line & triangle rendering...
    /// @return True if subsystems initialized appropriately.
    bool Win32_InitSubsystems();

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

    bool Win32_IsMainWindowActive() const { return m_mainWindowHandle != NULL; }

    std::shared_ptr<Win32PlatformDebugger> Win32_GetDebugger() const { return m_debugger; }

private:

    /// @brief Triggers the Window to close. It needs to be initialized again to reappear.
    void Win32_CloseWindow();

    // Handle to the main Window. NULL if inactive, any other value otherwise.
    HWND m_mainWindowHandle;

    // Handle to WinGDI Device Context associated with Main Window for drawing.
    HDC m_mainWindowDeviceContext;

    // Handle to parent process.
    HINSTANCE m_processHandle;

    std::shared_ptr<Win32PlatformDebugger> m_debugger;

};

#endif // WIN32_PLATFORM_H