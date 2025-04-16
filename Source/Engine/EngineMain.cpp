#include "Engine.h"
#include "Platform.h"

// Standard Platform functions

void Platform::DisplayDebugMessage(std::string&& msgStr, DebugLogMessage::Category&& cat)
{
    DisplayDebugMessage(DebugLogMessage{msgStr, cat});
}

// Engine implementation

void Engine::Initialize(std::shared_ptr<Platform> platform)
{
    // Initialize internal shared pointer to platform by copying the one passed.
    m_platform = platform;
}

void Engine::Tick(double timeSeconds)
{

}

void Engine::OnShutdown()
{
    if (m_shutdownReason == ShutdownReason::BAD_INIT)
    {
        // Nothing to do when shutting down due to bad initialization. Return immediately.
        return;
    }
}
