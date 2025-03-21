/*
    Main Win32 Platform Entry Point & Implementation file.
*/

#include "win32_platform.h"
#include "Engine/Engine.h"
#include <iostream>

std::shared_ptr<Win32Platform> Win32_Platform;
std::shared_ptr<Engine> Win32_Engine;

std::thread Win32_PlatformThread;
std::thread Win32_EngineThread;

// Main thread function for the Engine thread.
void Win32_EngineThreadFunc()
{
    // Initialize Engine.
    Win32_Engine->Initialize(std::dynamic_pointer_cast<Platform>(Win32_Platform));

    // Update the Engine so long as it hasn't been flagged for shutdown.
    while (!Win32_Engine->ShouldShutdown())
    {
        // #TODO: Handle measuring time so the viewer may contain real-time elements. 
        Win32_Engine->Tick(0.01);
    }

    // Shutdown Engine gracefully.
    Win32_Engine->OnShutdown();

    // END OF ENGINE THREAD FUNC
}

// Main thread function for the Platform thread.
void Win32_PlatformThreadFunc()
{
    // #TODO: Poll for input and handle resource allocation requests.
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

    std::cout << "Creating & Initializing Engine !\n";

    Win32_Engine = std::make_shared<Engine>();
    Win32_Platform = std::make_shared<Win32Platform>();

    // Start Platform Thread.
    Win32_PlatformThread = std::thread(Win32_PlatformThreadFunc);

    // Start Engine Thread.
    Win32_EngineThread = std::thread(Win32_EngineThreadFunc);

    // Join threads.
    Win32_EngineThread.join();
    Win32_PlatformThread.join();

    // End of program: output a message depending on shutdown reason for Engine & Platform.
    switch(Win32_Engine->GetShutdownReason())
    {
        case(Engine::ShutdownReason::REQUESTED):
            std::cout << "Engine Shutdown on request.\n";
        case(Engine::ShutdownReason::BAD_INIT):
            std::cout << "Engine Shutdown due to initialization failure ! Check initialization parameters.\n";
            break;
        case(Engine::ShutdownReason::RUNTIME_ERROR):
            std::cout << "Engine Shutdown due to runtime error ! Check previous messages for a fatal error.\n";
            break;
        case(Engine::ShutdownReason::PLATFORM_ERROR):
            std::cout << "Engine Shutdown due to a platform-layer error. Check next messages for more details.\n";
        default:
            std::cout << "Engine Shutdown reason unknown ! Something has gone very wrong.\n";
            break;
    }

    // #TODO: Platform-level shutdown reason.

    // Fake getchar to pause the console at the end of the program.
    std::cout << "Program has ended. Press ENTER to continue.\n";
    std::getchar();

    // Free whatever console may still be running.
    FreeConsole();

    return 0;
}