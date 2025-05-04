/*
    Main include file for Engine configuration, initialization and updating.
    While the functions are implemented in Engine code, they are made to be called from the Platform code.
*/

#ifndef ENGINE_H
#define ENGINE_H

#include <memory>

#include <string>

// Abstract platform services forward declaration.
class PlatformDebugger;
class PlatformRenderer;

struct DebugLogMessage
{
    enum class Category
    {
        SUCCESS, // Message indicating something went well !
        LOG, // Standard message indicating a fact that is in itself neither good or bad.
        WARNING, // Standard message indicating something irregular / incorrect happened, but not in a way that will necessarily cause a problem.
        ERROR_NONFATAL, // Message indicating something went wrong, but not to the point the program will require an Engine restart.
        ERROR_FATAL // Message indicating something went *very* wrong to the point it will require a Engine restart. Logging in this category will trigger
        // an Engine shutdown.
    };

    std::string LogMessage;
    Category LogCategory;
};

/// @brief Main Engine class, to be linked to an abstract Platform Implementation object. 
/// Uses Platform resources to display data from 3D asset file with a supported format.
class Engine
{
    public:

    // Lifecycle states of the Engine.
    enum class State
    {
        CONSTRUCTED, // Default state: Engine object has been constructed but isn't ready to run yet.
        INITIALIZED, // Engine is initialized and can Tick.
        RUNNING, // Engine has Ticked at least once successfully and can keep ticking.
        SHUTTING_DOWN, // Engine is in the process of shutting down. Don't ask it for anything !
        SHUTDOWN_COMPLETE // Engine has shut down and is no longer functionnal. Check shutdown reason for more. It may be re-initialized and started again.
    };

    // Possible broad reasons for shutting down.
    enum class ShutdownReason
    {
        UNKNOWN, // No reason was passed before shutting down, meaning the shutting down process may have been avoided entirely. This is bad !
        REQUESTED, // Normal shutdown, with everything going as expected. Usually triggered by user.
        BAD_INIT, // Engine shut down before it even started because Initialization went wrong. Check initialization parameters.
        RUNTIME_ERROR, // Engine shut down because a fatal (but non-program-crashing) error has happened.
        PLATFORM, // Engine shut down due to error or signal on the Platform layer. Only used by shutdowns triggered from Platform code.
    };

    Engine() : m_platformDebugger(nullptr), m_state(State::CONSTRUCTED), 
    m_shouldShutdown(false), m_shutdownReason(ShutdownReason::UNKNOWN)
    {}

    // GETTERS

    State GetState() const { return m_state; }

    bool ShouldShutdown() const { return m_shouldShutdown; }
    ShutdownReason GetShutdownReason() const { return m_shutdownReason; }

    // FUNCTIONNALITY

    /// @brief Initializes the Engine to run a set of platform service implementations.
    /// @param platform Shared pointer to the underlying platform debugger implementation.
    /// @Note(Marc): Is it wise to make each "service" a separate parameter here ? Perhaps a structure combining them together would work better. I don't know the total amount
    // of Service classes there will be yet so doing it might be premature.
    void Initialize(std::shared_ptr<PlatformDebugger> platformDebugger,
                    std::shared_ptr<PlatformRenderer> platformRenderer);

    /// @brief Performs a full update of the Engine, taking into account incoming events, the passage of time, and
    /// consequently updating render elements and the general state of the program as needed.
    void Update();

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
    void OnShutdown();

    /// @brief Triggers the engine to shutdown. Takes effect after the end of current Tick.
    /// @param Reason Reason for shutdown. By default, "Requested" which means the shutdown was natural and nothing unexpected has happened.
    /// #TODO(Marc): Since this will be the general-use function for shutting things down in error scenarios, we could plug this into the logging system
    // so that an Error shutdown is triggered automatically when logging an Error message.
    void TriggerShutdown(ShutdownReason Reason = ShutdownReason::REQUESTED) { m_shutdownReason = Reason; m_shouldShutdown = true; }

    std::shared_ptr<PlatformDebugger> GetDebugger() const { return m_platformDebugger; }
    std::shared_ptr<PlatformRenderer> GetRenderer() const { return m_platformRenderer; }

private:

    // Whether the engine has been flagged for shutting down. This will trigger the shutting down of the Engine and then the whole program
    // after current frame ends.
    bool m_shouldShutdown;
    // The reason for shutting down. Unless currently shutting down, this is set to "UNKNOWN" meaning if it is still that after actual shut down, then
    // something *very* wrong must have happened.
    ShutdownReason m_shutdownReason;
    
    // Current state of the engine's lifecycle.
    State m_state;

    // Shared pointer to underlying Platform Debugger implementation.
    std::shared_ptr<PlatformDebugger> m_platformDebugger;

    // Shared pointer to underlying Platform Renderer implementation.
    std::shared_ptr<PlatformRenderer> m_platformRenderer;
};

#endif // ENGINE_H

