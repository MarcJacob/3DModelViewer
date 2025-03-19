/*
    Main include file for Engine configuration, initialization and updating.
    While the functions are implemented in Engine code, they are made to be called from the Platform code.
*/

#ifndef ENGINE_H
#define ENGINE_H

#include <memory>

// Abstract platform implementation forward declaration.
class Platform;

/// @brief Main Engine class, to be linked to an abstract Platform Implementation object. 
/// Uses Platform resources to display data from 3D asset file with a supported format.
class Engine
{
    public:

    Engine() : m_platform(nullptr)
    {}

    /// @brief Checks whether the Engine has been appropriately initialized.
    /// @return Returns whether the Engine has been initialized correctly and is ready to display a model and tick.
    bool IsInitialized() const;

    /// @brief Initializes the Engine to run on a Platform.
    /// @param platform Shared pointer to the underlying platform implementation.
    void Initialize(std::shared_ptr<Platform> platform);

    /// @brief Integrates the advancement of time for all time-related elements of the view,
    /// including animation and taking movement input into account.
    /// @param timeSeconds Time in seconds to advance by.
    /// @note A Tick is a single unit of compacted advancement of time, where causes in the current view state
    // causes consequences in the view state to be, over the requested amount of time. Often the effects will be linear,
    // wherin time will merely be a multiplier over those consequences. For some critical systems there may be a more
    // complex integration.
    void Tick(double timeSeconds);

    /// @brief Shuts down the Engine, making it cleanly release any and all resources it might be using, and gracefully
    /// exit any sort of editing process.
    void Shutdown();

    private:
    
    // Shared pointer to underlying Platform implementation.
    // Should only ever be accessed directly by the Engine class which acts as an interface to the platform
    // for the rest of the Engine Code.
    std::shared_ptr<Platform> m_platform;
};

#endif // ENGINE_H

