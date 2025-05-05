#include "Engine.h"
#include "Platform.h"

// Standard Platform functions

void PlatformDebugger::DisplayDebugMessage(std::string&& msgStr, DebugLogMessage::Category cat)
{
    DisplayDebugMessage(DebugLogMessage{msgStr, cat});
}

// Engine implementation

void Engine::Initialize(std::shared_ptr<PlatformDebugger> platformDebugger, std::shared_ptr<PlatformRenderer> platformRenderer)
{
    m_platformDebugger = platformDebugger;
    m_platformRenderer = platformRenderer;
}

void Engine::Update()
{
    // Run full Engine update: read input events, tick time-based elements, and update rendering.

    //#TODO(Marc): Input handling.

    Tick(0.01); // #TODO(Marc): Let's measure time so we can make Tick be real-time-based.

    // #TEST: Draw a red line on screen.
    std::shared_ptr<PlatformRenderer::MemoryMapDrawer> drawer = m_platformRenderer->AllocateFullDisplayDrawer();
    if (drawer != nullptr)
    {
        uint16_t width, height;
        width = drawer->GetWidth();
        height = drawer->GetHeight();

        Pixel_RGBA* pixelsBuffer = drawer->GetPixelBufferPtr();

        // Let's draw a line at arbitrary coordinates.
        for(int y = 200; y < 210; y++)
        {
            for(int x = 100; x < 500; x++)
            {
                pixelsBuffer[y * width + x].pixel = 0xFFFF0000;
            }
        }

        // Once done, flag the drawer for drawing and discarding. #NOTE(Marc): Obviously we'd normally want to keep the same drawer around so long as
        // screen resolution doesn't change, but such signals will probably be part of the input system which doesn't exist yet.
        drawer->SetReadyToDraw();
        drawer->Discard();
    }

    // Perform platform rendering update.
    m_platformRenderer->RenderUpdate();
}

void Engine::Tick(double timeSeconds)
{

}

void Engine::OnShutdown()
{
    // Display a debug message on the platform informing the user why Engine has shut down.
    switch(GetShutdownReason())
    {
        case(Engine::ShutdownReason::REQUESTED):
            m_platformDebugger->DisplayDebugMessage("Engine Shutdown on user request.", DebugLogMessage::Category::LOG);
            break;
        case(Engine::ShutdownReason::BAD_INIT):
            m_platformDebugger->DisplayDebugMessage("Engine Shutdown due to initialization failure ! Check initialization parameters.", DebugLogMessage::Category::ERROR_FATAL);
            break;
        case(Engine::ShutdownReason::RUNTIME_ERROR):
            m_platformDebugger->DisplayDebugMessage("Engine Shutdown due to runtime error ! Check previous messages for a fatal error.", DebugLogMessage::Category::ERROR_NONFATAL);
            break;
        case(Engine::ShutdownReason::PLATFORM):
            m_platformDebugger->DisplayDebugMessage("Engine Shutdown by request of Platform.", DebugLogMessage::Category::WARNING);
            break;
        default:
            m_platformDebugger->DisplayDebugMessage("Engine Shutdown reason unknown ! Something has gone very wrong.", DebugLogMessage::Category::ERROR_FATAL);
            break;
    }
}
