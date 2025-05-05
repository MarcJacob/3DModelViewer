/*
    Main Win32 Platform Entry Point & Implementation file.
*/

#include "win32_platform.h"
#include "Engine/Engine.h"
#include <iostream>
#include <thread>

// Forward decs & Defines

#define WIN32_WINDOW_CLASS_NAME L"Main Window Class"

LRESULT CALLBACK Win32_MsgProc(HWND window, UINT msgType, WPARAM wParam, LPARAM lParam);

// Static Memory pointers to Platform & Engine.

std::shared_ptr<Win32Platform> Win32_Platform;
std::shared_ptr<Engine> Win32_Engine;

// WIN32 PLATFORM IMPLEMENTATION

bool Win32Platform::Win32_InitSubsystems()
{
    m_debugger = std::make_shared<Win32PlatformDebugger>();
    m_renderer = std::make_shared<Win32PlatformRenderer>();
    return true;
}

bool Win32Platform::Win32_InitWindow()
{
    // Create window class if necessary and instantiate main window.
    WNDCLASS windowClass = {};
    windowClass.hInstance = m_processHandle;
    windowClass.lpfnWndProc = &Win32_MsgProc;
    windowClass.lpszClassName = WIN32_WINDOW_CLASS_NAME;
    windowClass.style = CS_OWNDC;
    RegisterClass(&windowClass);

    m_mainWindowHandle = CreateWindow(WIN32_WINDOW_CLASS_NAME, L"Model Viewer", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, m_processHandle, NULL);

    if (m_mainWindowHandle == NULL)
    {
        // Error when creating Main Window. Shut everything down.
        DWORD errCode = GetLastError();
        std::cerr << "Error when creating Win32 Main Window ! Error Code = " << errCode << "\n";
        return false;
    }

    ShowWindow(m_mainWindowHandle, 1);

    return true;
}

void Win32Platform::Win32_PollNextMessage()
{
    MSG message;
    if (PeekMessage(&message, m_mainWindowHandle, 0, 0, PM_REMOVE) > 0)
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
}

void Win32Platform::Win32_RendererUpdate()
{
    m_renderer->Win32_TryRunRenderUpdate();
}

void Win32Platform::Win32_DebuggerUpdate()
{
    m_debugger->Win32_FlushDebugLogQueue();
}

bool Win32Platform::Win32_ProcessWindowMessage(int messageType, WPARAM wParam, LPARAM lParam)
{
    switch(messageType)
    {
        case(WM_SIZE):
            m_renderer->Win32_ResizeRendererDisplay(m_mainWindowHandle, LOWORD(lParam), HIWORD(lParam));
            return false;
        case(WM_QUIT):
        case(WM_CLOSE):
            // Close Main window, triggering the whole app to shut down.
            m_debugger->DisplayDebugMessage("Win32 Platform Main Window received Close or Quit message ! Closing window and shutting down Engine...",
                DebugLogMessage::Category::WARNING);
            Win32_CloseWindow();
            return true; // Make sure nothing further happens. We need to handle the actual closing of the window ourselves.
        default:
            return false;
    }
}

void Win32Platform::Win32_CloseWindow()
{
    CloseWindow(m_mainWindowHandle);
    m_mainWindowHandle = NULL;
}

void Win32PlatformDebugger::DisplayDebugMessage(DebugLogMessage&& message)
{
    std::lock_guard<std::mutex> queueLock(m_mutex_DebugMessageQueue);
    m_debugMessageQueue.push(message);
}

void Win32PlatformDebugger::Win32_FlushDebugLogQueue()
{
    // #NOTE(Marc): Using a single mutex and buffer both for feeding in messages and flushing to screen means that if the Engine or any other thread tries to request a message to
    // be displayed, it will have to wait for however long it takes to flush every message to screen. I don't mind it for now, because the platform thread in charge of this should
    // be quite short-looped which, outside cases where the Engine prints many messages sequentially, means the queue will never get too long anyway.
    // If this were to change, I can think of multiple solutions: a single sort of "circular" buffer with a "consumption" and "production" cursor that can work in parallel, or
    // some way to interrupt the flushing process if something is waiting for the mutex to be unlocked.
    std::lock_guard<std::mutex> messageQueueLock(m_mutex_DebugMessageQueue);
    while(!m_debugMessageQueue.empty())
    {
        DebugLogMessage& message = m_debugMessageQueue.front();

        switch(message.LogCategory)
        {
            case(DebugLogMessage::Category::SUCCESS):
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            break;
            default:
            case(DebugLogMessage::Category::LOG):
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
            break;
            case(DebugLogMessage::Category::WARNING):
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_RED);
            break;
            case(DebugLogMessage::Category::ERROR_NONFATAL):
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
            break;
            case(DebugLogMessage::Category::ERROR_FATAL):
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);
            break;
        }
        // Use standard out to output all messages. Always add a line break (triggering a flush) to each message.
        // #TODO(Marc): Add facility for multi-line and parameterized messages to be built on the Engine side.
        std::cout << message.LogMessage << "\n";
        m_debugMessageQueue.pop();
    }

    // Reset text attribute to default.
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}

void Win32PlatformRenderer::Win32_ResizeRendererDisplay(HWND windowHandle, uint16_t width, uint16_t height)
{
    m_displayWidth = width;
    m_displayHeight = height;

    if (m_windowHandle != windowHandle)
    {
        ReleaseDC(m_windowHandle, m_windowDeviceContext);
        m_windowHandle = windowHandle;
        
        m_windowDeviceContext = GetDC(windowHandle);
        if (m_windowDeviceContext == NULL)
        {
            char buff[256];
            sprintf(buff, "Arrrghh here's the error: %d", GetLastError());
            Win32_Platform->Win32_GetDebugger()->DisplayDebugMessage(buff);
            return;
        }
    }
}

std::shared_ptr<PlatformRenderer::MemoryMapDrawer> Win32PlatformRenderer::AllocateFullDisplayDrawer()
{
    std::lock_guard<std::mutex> lock(m_mutex_RenderResources);

    MemoryMapDrawerGDI newDrawerGDI = {};
    BITMAPINFO bmpInfo = {};
    {
        bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFO);
        bmpInfo.bmiHeader.biWidth = m_displayWidth;
        bmpInfo.bmiHeader.biHeight = -m_displayHeight;
        bmpInfo.bmiHeader.biPlanes = 1;
        bmpInfo.bmiHeader.biBitCount = 32;
        bmpInfo.bmiHeader.biCompression = BI_RGB;
    }

    Pixel_RGBA* pixelBuffer;
    newDrawerGDI.bmpInfo = bmpInfo;
    newDrawerGDI.bmpHandle = CreateDIBSection(m_windowDeviceContext, &bmpInfo, DIB_RGB_COLORS, reinterpret_cast<void**>(&pixelBuffer), NULL, NULL);
    
    if (pixelBuffer == nullptr)
    {
        // DIB Section creation has failed ! Fail creation of Memory Map Drawer & return immediately.
        // #TODO(Marc): Assert system.

        return nullptr;
    }

    newDrawerGDI.DIBContext = CreateCompatibleDC(m_windowDeviceContext);
    SelectObject(newDrawerGDI.DIBContext, newDrawerGDI.bmpHandle);

    std::shared_ptr<PlatformRenderer::MemoryMapDrawer> newDrawer = 
        std::make_shared<PlatformRenderer::MemoryMapDrawer>(m_displayWidth, m_displayHeight, 0, 0, pixelBuffer);

    newDrawerGDI.drawer = newDrawer;

    m_memoryMapDrawers.emplace_back(newDrawerGDI);
    return newDrawer;
}

void Win32PlatformRenderer::PerformRenderUpdate()
{
    // Draw loop for Bitmap drawers
    for(auto drawerIt = m_memoryMapDrawers.begin(); drawerIt != m_memoryMapDrawers.end(); drawerIt++)
    {
        std::shared_ptr<PlatformRenderer::MemoryMapDrawer>& drawer = drawerIt->drawer;
        if (drawer->IsReadyToDraw())
        {
            BitBlt(m_windowDeviceContext, drawer->GetOffsetX(), drawer->GetOffsetY(), drawer->GetWidth(), drawer->GetHeight(),
                drawerIt->DIBContext, 0, 0, SRCCOPY);
        }
    }

    // Erase loop for discarded Bitmap drawers
    for(auto drawerIt = m_memoryMapDrawers.begin(); drawerIt != m_memoryMapDrawers.end();)
    {
        // Discard drawer if marked for discarding.
        if (drawerIt->drawer->ShouldDiscard())
        {
            // Free DIB DC
            SelectObject(drawerIt->DIBContext, NULL);
            DeleteDC(drawerIt->DIBContext);

            // Free Bitmap
            DeleteObject(drawerIt->bmpHandle);

            drawerIt = m_memoryMapDrawers.erase(drawerIt);
        }
        else
        {
            drawerIt++;
        }
    }
}

// WIN32 MAIN ENTRY POINT & THREADS

// Threads & synchronization events.
std::thread Win32_PlatformMainThread;
std::atomic<bool> Win32_PlatformShutdownFlag;

std::thread Win32_PlatformRenderThread;
std::thread Win32_PlatformDebuggingThread;

std::thread Win32_EngineMainThread;

HANDLE Win32_PlatformInitCompleteEventHandle; // Set when Platform is done initializing, whether it succeeded or failed.
HANDLE Win32_EngineInitCompleteEventHandle; // Set when Engine is done initializing, whether it succeeded or failed.
HANDLE Win32_EngineShutdownCompleteEventHandle; // Set when Engine is done shutting down.

// Main thread function for the Win32 Engine thread.
void Win32_EngineThreadMainFunc()
{
    // Initialize Engine.
    Win32_Engine->Initialize(   std::dynamic_pointer_cast<PlatformDebugger>(Win32_Platform->Win32_GetDebugger()),
                                std::dynamic_pointer_cast<PlatformRenderer>(Win32_Platform->Win32_GetRenderer()));

    // Set Engine Init Complete event.
    SetEvent(Win32_EngineInitCompleteEventHandle);

    // Update the Engine so long as it hasn't been flagged for shutdown.
    while (!Win32_Engine->ShouldShutdown())
    {
        // #TODO(Marc): Measure passage of time on the platform and give the Engine some idea of relationship between ticks
        // and real time so real-time features may exist.
        Win32_Engine->Update();
    }

    // Run Engine Shutdown Routine.
    Win32_Engine->OnShutdown();

    // Set Engine Shutdown Complete event.
    SetEvent(Win32_EngineShutdownCompleteEventHandle);

    // END OF ENGINE THREAD
}

// Win32 Window Message callback function. Have the platform handle it. If it can't, call the default
// handler function.
LRESULT CALLBACK Win32_MsgProc(HWND window, UINT msgType, WPARAM wParam, LPARAM lParam)
{
    if (!Win32_Platform->Win32_ProcessWindowMessage(msgType, wParam, lParam))
    {
        return DefWindowProc(window, msgType, wParam, lParam);
    }

    return 0;
}

void Win32_PlatformThreadRenderFunc()
{
    Win32_Platform->Win32_GetDebugger()->DisplayDebugMessage("Win32 Render Thread has started.");

    // Constantly attempt to run render updates on the platform.
    // #NOTE(Marc): While I don't usually like active waiting like this (by "constantly retrying"), I think it's fine in this case because
    // render updates should be ran at a very high rate compared to the time they take anyway.

    while (!Win32_PlatformShutdownFlag)
    {
        Win32_Platform->Win32_RendererUpdate();
    }

    Win32_Platform->Win32_GetDebugger()->DisplayDebugMessage("Win32 Render Thread has ended.");
}

void Win32_PlatformThreadDebuggingFunc()
{
    Win32_Platform->Win32_GetDebugger()->DisplayDebugMessage("Win32 Debugger Thread has started.");

    // #TOTHINK(Marc): It may be worth adding an extra Event object so that this thread does not constantly lock / unlock the debug message buffer mutex.
    while (!Win32_PlatformShutdownFlag)
    {
        Win32_Platform->Win32_DebuggerUpdate();
    }

    Win32_Platform->Win32_GetDebugger()->DisplayDebugMessage("Win32 Debugger Thread has ended.");
}

// Main thread function for the Platform thread.
void Win32_PlatformThreadMainFunc()
{
    // Initialize platform.
    Win32_Platform->Win32_InitWindow();

    // Make sure Shutdown flag is set to false.
    Win32_PlatformShutdownFlag = false;

    // Spawn platform sub-threads
    Win32_PlatformRenderThread = std::thread(Win32_PlatformThreadRenderFunc);
    Win32_PlatformDebuggingThread = std::thread(Win32_PlatformThreadDebuggingFunc);

    // Whether Initialization succeeded or not, set the init event.
    SetEvent(Win32_PlatformInitCompleteEventHandle);

    // Wait for anything to trigger a shutdown.
    // #TOTHINK(Marc): This could probably be done on the actual Main thread. But I'll make cross that bridge when I get to it, namely when
    // I get around moving a lot of this Platform code to the Win32 Platform class (including the thread management itself).
    while(Win32_Platform->Win32_IsMainWindowActive()
            && Win32_Engine->GetState() != Engine::State::SHUTDOWN_COMPLETE)
    {
        // Perform message polling on this thread as this is the thread that owns the window.
        Win32_Platform->Win32_PollNextMessage();
    }

    // Platform has been shut down. If for some reason the Engine is not shutting down yet, make it do so immediately.
    if (Win32_Engine->GetState() < Engine::State::SHUTTING_DOWN)
    {
        Win32_Engine->TriggerShutdown(Engine::ShutdownReason::PLATFORM);
    }

    // Platform shut-down. Before doing anything on the platform, wait for the Engine to shut down.
    WaitForSingleObject(Win32_EngineShutdownCompleteEventHandle, INFINITE);

    // Set the Platform Shutdown Flag so our child threads will stop, and wait for them to do so.
    Win32_PlatformShutdownFlag = true;
    Win32_PlatformRenderThread.join();
    Win32_PlatformDebuggingThread.join();

    // END OF PLATFORM THREAD
}

int APIENTRY WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, int nCmdShow)
{
    // If no console is available, allocate one.
    // #TOTHINK(Marc): Should this be done regardless ? Probably could be a compilation OR launch parameter.
    if (AllocConsole())
    {
        freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);
        freopen_s(reinterpret_cast<FILE**>(stderr), "CONERR$", "w", stderr);
        freopen_s(reinterpret_cast<FILE**>(stdin), "CONIN$", "r", stdin);
    }
    
    // Create Platform & Engine control objects.
    Win32_Platform = std::make_shared<Win32Platform>(instance);
    Win32_Engine = std::make_shared<Engine>();

    Win32_Platform->Win32_InitSubsystems();
    Win32_Platform->Win32_GetDebugger()->DisplayDebugMessage("Initializing Platform...");

    // Create synchronization events.
    {
        // Create Platform init event on which Engine thread will wait before proceeding with Engine startup.
        Win32_PlatformInitCompleteEventHandle = CreateEvent(NULL, false, false, L"Platform Init Complete Event");

        // Create Engine Init Complete event on which Main thread will wait before confirming to the user that everything is fine
        // and waiting for normal or runtime error shutdown.
        Win32_EngineInitCompleteEventHandle = CreateEvent(NULL, false, false, L"Engine Init Complete Event");

        // Create Engine Shutdown Complete event on which Platform thread will wait before proceeding with Platform shutdown.
        Win32_EngineShutdownCompleteEventHandle = CreateEvent(NULL, false, false, L"Engine Shutdown Complete Event");
    }

    // PLATFORM STARTUP
    {
        // Start Platform Thread
        Win32_PlatformMainThread = std::thread(Win32_PlatformThreadMainFunc);
        
        // Wait for Platform Thread to finish initialization.
        WaitForSingleObject(Win32_PlatformInitCompleteEventHandle, INFINITE);

        // Check if initialization was successful. If it was, there should be an active window on the platform.
        // If not, then shut everything down immediately by jumping to PROGRAM_END.
        if (!Win32_Platform->Win32_IsMainWindowActive())
        {
            Win32_Platform->Win32_GetDebugger()->DisplayDebugMessage("Win32 Platform has failed to initialize !", DebugLogMessage::Category::ERROR_FATAL);
            goto PROGRAM_END;
        }
    }

    Win32_Platform->Win32_GetDebugger()->DisplayDebugMessage("Platform Initialized !", DebugLogMessage::Category::SUCCESS);

    Win32_Platform->Win32_GetDebugger()->DisplayDebugMessage("Initializing & Starting Engine...");

    // ENGINE STARTUP
    {
        // Start Engine Thread.
        Win32_EngineMainThread = std::thread(Win32_EngineThreadMainFunc);

        // Wait for Engine initialization to complete.
        WaitForSingleObject(Win32_EngineInitCompleteEventHandle, INFINITE);
    }

    // If Engine has initialized appropriately, display a message. If not, the Engine thread will take care of
    // indicating the failure as part of its standard shutdown routine.
    if (!Win32_Engine->ShouldShutdown())
    {
        Win32_Platform->Win32_GetDebugger()->DisplayDebugMessage("Engine initialized and running !", DebugLogMessage::Category::SUCCESS);
    }

    Sleep(2000);
    PostMessage(Win32_Platform->m_mainWindowHandle, 7, 0, 0);

    // Join threads. At this point the process main thread will just be waiting for shutdown.
    // #TOTHINK(Marc): Is that smart ? Maybe running a separate thread for Platform isn't very useful.
    // Then again, threads are a very cheap and plentiful ressource on modern machines so it probably doesn't matter.
    Win32_EngineMainThread.join();
    Win32_PlatformMainThread.join();
PROGRAM_END:

    // Final flush of the Win32 Platform's Debug Logging Queue so any messages left (sent as part of shutdowns) will be displayed. 
    Win32_Platform->Win32_GetDebugger()->Win32_FlushDebugLogQueue();

    // Fake getchar to pause the console at the end of the program.
    std::cout << "Program has ended. Press ENTER to continue.\n";
    std::getchar();

    // Free whatever console may still be running.
    FreeConsole();

    return 0;
}