#include "Engine.h"
#include "Platform.h"

// Standard Platform functions

void PlatformDebugger::DisplayDebugMessage(std::string&& msgStr, DebugLogMessage::Category cat)
{
    DisplayDebugMessage(DebugLogMessage{msgStr, cat});
}

// Engine implementation

void Engine::Initialize(std::shared_ptr<PlatformDebugger> platformDebugger)
{
    // Initialize internal shared pointer to platform by copying the one passed.
    m_platformDebugger = platformDebugger;
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
