/*
    Main header file for the abstract Platform class the Engine makes use of directly, to be implemented by Platform implementation code.
    Contains a main Platform class as well as various abstract Platform Resource classes and data structures.
*/

#ifndef PLATFORM_H
#define PLATFORM_H

#include <memory>
#include <thread>
#include <functional>
#include <mutex>

#include "Engine.h"

// #TODO: Declare all relevant types of platform resources here.

/// @brief Abstract platform implementation class, to be implemented in Platform code and passed to the Engine on initialization.
/// Gives the Engine access to platform resources in a manner it can understand.
class Platform
{
public:

    /// @brief Asks the platform to display the passed Debug message string & category on a platform-specific debug logging system, 
    /// usually a console.
    /// @param msgStr Character string to be displayed in the debug message.
    /// @param cat Message category / severity.
    void DisplayDebugMessage(std::string&& msgStr, DebugLogMessage::Category cat = DebugLogMessage::Category::LOG);

    /// @brief Asks the platform to display the passed Debug Message structure on a platform-specific debug logging system, 
    /// usually a console.
    /// @param msg DebugLogMessage structure to display.
    virtual void DisplayDebugMessage(DebugLogMessage&& msg) = 0;

};

#endif // PLATFORM_H