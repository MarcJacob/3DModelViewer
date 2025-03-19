#include "Engine.h"

bool Engine::IsInitialized() const
{
    // Initialization is done when a platform is assigned.
    return m_platform != nullptr;
}

void Engine::Initialize(std::shared_ptr<Platform> platform)
{
    // Initialize internal shared pointer to platform by copying the one passed.
    m_platform = platform;

    
}

void Engine::Tick(double timeSeconds)
{
}

void Engine::Shutdown()
{
}
