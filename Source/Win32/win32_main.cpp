#include <iostream>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>


int APIENTRY WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, int nCmdShow)
{
    // If no console is available, allocate one.
    // #TOTHINK(Marc): Should this be done regardless ? Probably could be a compilation OR launch parameter.
    if (AllocConsole())
    {
        freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);
        freopen_s(reinterpret_cast<FILE**>(stdin), "CONIN$", "r", stdin);    
    }

    std::cout << "Hello World !\n";

    // Fake getchar to pause the console at the end of the program.
    std::cout << "Program has ended. Press ENTER to continue.\n";
    std::getchar();

    // Free whatever console may still be running.
    FreeConsole();

    return 0;
}