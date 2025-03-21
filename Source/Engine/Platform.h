/*
    Main header file for the abstract Platform class the Engine makes use of directly, to be implemented by Platform implementation code.
    Contains a main Platform class as well as various abstract Platform Resource classes and data structures.
*/

#ifndef PLATFORM_H
#define PLATFORM_H

#include <memory>
#include <thread>
#include <functional>

// #TODO: Declare all relevant types of platform resources here.

/// @brief Abstract platform implementation class, to be implemented in Platform code and passed to the Engine on initialization.
/// Gives the Engine access to platform resources in a manner it can understand.
/// This class is an interface. It does not give access to any stored data.
class Platform
{
public:

};

#endif // PLATFORM_H