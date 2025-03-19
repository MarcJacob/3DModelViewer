/*
    Main Win32 Platform Entry Point & Implementation file.
*/

#include "win32_platform.h"
#include "Engine/Engine.h"
#include <iostream>

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

    std::shared_ptr<Engine> engine = std::make_shared<Engine>();
    std::shared_ptr<Win32Platform> platform = std::make_shared<Win32Platform>();
    
    // Initialize Engine.
    {
        engine->Initialize(std::dynamic_pointer_cast<Platform>(platform));

        if (!engine->IsInitialized())
        {
            // Engine failed to initialize for some reason.
            std::cerr << "Error ! Engine failed to Initialize.\n";
            return 1;
        }
    }

    while (!platform->ShouldShutdown())
    {
        // #TODO: Handle input & time passage !
        engine->Tick(0.01);
    }

    // Shutdown Engine gracefully.
    engine->Shutdown();

    // Fake getchar to pause the console at the end of the program.
    std::cout << "Program has ended. Press ENTER to continue.\n";
    std::getchar();

    // Free whatever console may still be running.
    FreeConsole();

    return 0;
}