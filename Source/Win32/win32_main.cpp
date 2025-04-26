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

// WIN32 PLATFORM IMPLEMENTATION

bool Win32Platform::Win32_InitSubsystems()
{
    m_debugger = std::make_shared<Win32PlatformDebugger>();
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

    m_mainWindowHandle = CreateWindow(WIN32_WINDOW_CLASS_NAME, L"Model Viewer", WS_VISIBLE | WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, m_processHandle, NULL);

    if (m_mainWindowHandle == NULL)
    {
        // Error when creating Main Window. Shut everything down.
        DWORD errCode = GetLastError();
        std::cerr << "Error when creating Win32 Main Window ! Error Code = " << errCode << "\n";
        return false;
    }

    m_mainWindowDeviceContext = GetDC(m_mainWindowHandle);

    return true;
}

void Win32Platform::Win32_Update()
{
    // Poll messages
    MSG message;
    while(PeekMessage(&message, m_mainWindowHandle, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    m_debugger->Win32_FlushDebugLogQueue();
    
}

bool Win32Platform::Win32_ProcessWindowMessage(int messageType, WPARAM wParam, LPARAM lParam)
{
    switch(messageType)
    {
        case(WM_QUIT):
        case(WM_CLOSE):
            // Close Main window, triggering the whole app to shut down.
            m_debugger->DisplayDebugMessage("Win32 Platform Main Window received Close or Quit message ! Closing window and shutting down Engine...",
                DebugLogMessage::Category::WARNING);
            Win32_CloseWindow();
            return true;
        case(WM_PAINT):
            // Fill window with black
            PAINTSTRUCT paint;
            BeginPaint(m_mainWindowHandle, &paint);
            FillRect(m_mainWindowDeviceContext, &paint.rcPaint, CreateSolidBrush(RGB(0, 0, 0)));
            EndPaint(m_mainWindowHandle, &paint);
            return true;
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
    std::lock_guard<std::mutex> queueLock(Mutex_DebugMessageQueue);
    DebugMessageQueue.push(message);
}

void Win32PlatformDebugger::Win32_FlushDebugLogQueue()
{
    // #NOTE(Marc): Using a single mutex and buffer both for feeding in messages and flushing to screen means that if the Engine or any other thread tries to request a message to
    // be displayed, it will have to wait for however long it takes to flush every message to screen. I don't mind it for now, because the platform thread in charge of this should
    // be quite short-looped which, outside cases where the Engine prints many messages sequentially, means the queue will never get too long anyway.
    // If this were to change, I can think of multiple solutions: a single sort of "circular" buffer with a "consumption" and "production" cursor that can work in parallel, or
    // some way to interrupt the flushing process if something is waiting for the mutex to be unlocked.
    std::lock_guard<std::mutex> messageQueueLock(Mutex_DebugMessageQueue);
    while(!DebugMessageQueue.empty())
    {
        DebugLogMessage& message = DebugMessageQueue.front();

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
        DebugMessageQueue.pop();
    }

    // Reset text attribute to default.
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}

// WIN32 MAIN ENTRY POINT & STATIC DATA

std::shared_ptr<Win32Platform> Win32_Platform;
std::shared_ptr<Engine> Win32_Engine;

// Threads & synchronization events.
std::thread Win32_PlatformThread;
std::thread Win32_EngineThread;

HANDLE Win32_PlatformInitCompleteEventHandle; // Set when Platform is done initializing, whether it succeeded or failed.
HANDLE Win32_EngineInitCompleteEventHandle; // Set when Engine is done initializing, whether it succeeded or failed.
HANDLE Win32_EngineShutdownCompleteEventHandle; // Set when Engine is done shutting down.

// Main thread function for the Engine thread.
void Win32_EngineThreadFunc()
{
    // Initialize Engine.
    Win32_Engine->Initialize(std::dynamic_pointer_cast<PlatformDebugger>(Win32_Platform->Win32_GetDebugger()));

    // Set Engine Init Complete event.
    SetEvent(Win32_EngineInitCompleteEventHandle);

    // Update the Engine so long as it hasn't been flagged for shutdown.
    while (!Win32_Engine->ShouldShutdown())
    {
        // #TODO(Marc): Measure passage of time on the platform and give the Engine some idea of relationship between ticks
        // and real time so real-time features may exist.
        Win32_Engine->Tick(0.01);
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

// Main thread function for the Platform thread.
void Win32_PlatformThreadFunc()
{
    // Initialize platform.
    Win32_Platform->Win32_InitWindow();

    // Whether Initialization succeeded or not, set the init event.
    SetEvent(Win32_PlatformInitCompleteEventHandle);

    // Run platform update until the Engine shuts down or until specific events happen.
    while(1)
    {
        // Check stop conditions (Main Window closed for some reason or engine was shut down).
        if (!Win32_Platform->Win32_IsMainWindowActive()
        || Win32_Engine->GetState() == Engine::State::SHUTDOWN_COMPLETE)
        {
            break;
        }

        Win32_Platform->Win32_Update();
    }

    // Platform has been shut down. If for some reason the Engine is not shutting down yet, make it do so immediately.
    if (Win32_Engine->GetState() < Engine::State::SHUTTING_DOWN)
    {
        Win32_Engine->TriggerShutdown(Engine::ShutdownReason::PLATFORM);
    }

    // Platform shut-down. Before doing anything on the platform, wait for the Engine to shut down.
    WaitForSingleObject(Win32_EngineShutdownCompleteEventHandle, INFINITE);

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
        Win32_PlatformThread = std::thread(Win32_PlatformThreadFunc);
        
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
        Win32_EngineThread = std::thread(Win32_EngineThreadFunc);

        // Wait for Engine initialization to complete.
        WaitForSingleObject(Win32_EngineInitCompleteEventHandle, INFINITE);
    }

    // If Engine has initialized appropriately, display a message. If not, the Engine thread will take care of
    // indicating the failure as part of its standard shutdown routine.
    if (!Win32_Engine->ShouldShutdown())
    {
        Win32_Platform->Win32_GetDebugger()->DisplayDebugMessage("Engine initialized and running !", DebugLogMessage::Category::SUCCESS);
    }

    // Join threads. At this point the process main thread will just be waiting for shutdown.
    // #TOTHINK(Marc): Is that smart ? Maybe running a separate thread for Platform isn't very useful.
    // Then again, threads are a very cheap and plentiful ressource on modern machines so it probably doesn't matter.
    Win32_EngineThread.join();
    Win32_PlatformThread.join();

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