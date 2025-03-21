#include "Engine.h"

void Engine::Initialize(std::shared_ptr<Platform> platform)
{
    // Initialize internal shared pointer to platform by copying the one passed.
    m_platform = platform;

    // #Test: let's pretend initialization fails for some reason.
    TriggerShutdown(ShutdownReason::BAD_INIT);
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
