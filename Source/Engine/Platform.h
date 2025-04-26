/*
    Main header file for the abstract Platform class the Engine makes use of directly, to be implemented by Platform implementation code.
    Contains a main Platform class as well as various abstract Platform Resource classes and data structures.
*/

#ifndef PLATFORM_H
#define PLATFORM_H

#include <memory>
#include "Engine.h"

/// Abstract platform implementation classes, to be implemented in Platform code and passed to the Engine on initialization.
/// Their role is to give the Engine access to platform resources in a manner it can understand.

class PlatformDebugger
{
public:

    /// @brief Asks the platform to display the passed Debug message string & category on a platform-specific debug logging system, 
    /// usually a console.
    /// @param msgStr Character string to be displayed in the debug message.
    /// @param cat Message category / severity.
    void DisplayDebugMessage(std::string&& msgStr, DebugLogMessage::Category cat = DebugLogMessage::Category::LOG);

    /// @brief Asks the platform to display the passed Debug Message structure on a platform-specific debug logging system, 
    /// usually a console. Note: this may block the calling thread, but the platform should keep the potential blocking time very low.
    /// @param msg DebugLogMessage structure to display.
    virtual void DisplayDebugMessage(DebugLogMessage&& msg) = 0;
};

#endif // PLATFORM_H