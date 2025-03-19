/*
    Main header file for the abstract Platform class the Engine makes use of directly, to be implemented by Platform implementation code.
    Contains a main Platform class as well as various abstract Platform Resource classes and data structures.
*/

#ifndef PLATFORM_H
#define PLATFORM_H

// #TODO: Declare all relevant types of platform resources here.

/// @brief Abstract platform implementation class, to be implemented in Platform code and passed to the Engine on initialization.
/// Gives the Engine access to platform resources in a manner it can understand.
/// This class is an interface. It does not give access to any stored data.
class Platform
{
public:

    Platform(): m_shouldShutdown(false)
    {}

    bool ShouldShutdown() const { return m_shouldShutdown; }

    /// @brief Triggers the platform, and in turn the Engine, to shutdown. Takes effect after the end of current Tick.
    void TriggerShutdown() { m_shouldShutdown = true; }

private:

    bool m_shouldShutdown;
};

#endif // PLATFORM_H