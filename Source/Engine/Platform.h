/*
    Main header file for the abstract Platform class the Engine makes use of directly, to be implemented by Platform implementation code.
    Contains a main Platform class as well as various abstract Platform Resource classes and data structures.
*/

#ifndef PLATFORM_H
#define PLATFORM_H

#include <memory>
#include <atomic>
#include "Engine.h"

/// Abstract platform implementation classes, to be implemented in Platform code and passed to the Engine on initialization.
/// Their role is to give the Engine access to platform resources in a manner it can understand.

/// @brief Platform Debugger is used by the Engine to have the Platform output debug information through some platform-specific channel that can "outlive"
/// the engine itself in case of fatal failure, usually a console.
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

/// @brief Common RGBA bitmap format we expect the platform to be able to use.
union Pixel_RGBA
{
    struct
    {
        uint8_t r, g, b, a;
    };

    uint32_t pixel;
};

/// @brief Platform Renderer allows the Engine to draw to the Platform's display by giving access to memory-mapped pixel data.
/// Note that we assume the platform only handles a single display for simplicity.
class PlatformRenderer
{
public:

    /// @brief Thread safe memory-mapped pixel data allowing the Engine to draw pixels directly to Platform display.
    /// When allocated by the platform, some unique pixel buffer should be allocated with it and locked for Engine use. Once the Engine releases the drawer,
    /// The platform should display it.
    class MemoryMapDrawer
    {
    public:

        MemoryMapDrawer() = delete;
        MemoryMapDrawer(uint16_t w, uint16_t h, uint16_t offsetX, uint16_t offsetY, Pixel_RGBA* buff)
            : m_width(w), m_height(h), m_offsetX(offsetX), m_offsetY(offsetY), m_pixelBuffer(buff)
            {
                m_bReadyToDraw = false;
                m_bShouldBeDiscarded = false;
            }

        inline uint16_t GetWidth() const { return m_width; }
        inline uint16_t GetHeight() const { return m_height; }
        inline uint16_t GetOffsetX() const { return m_offsetX; }
        inline uint16_t GetOffsetY() const { return m_offsetY; }
        inline Pixel_RGBA* GetPixelBufferPtr() const { return m_pixelBuffer; }
        
        /// @brief Marks this Drawer as ready to be drawn by the Platform.
        inline void SetReadyToDraw() { m_bReadyToDraw = true; } // #TODO(Marc): Add an assertion system so we can assert that flag wasn't already set.
        inline bool IsReadyToDraw() { return m_bReadyToDraw; }
        
        /// @brief Sets the Drawer's state as "Drawn", giving back control to the Engine in case it needs to make changes to the pixels buffer.
        inline void SetDrawn() { m_bReadyToDraw = false; }

        /// @brief Marks this Drawer as "to be discarded" by the Platform, usually after the next Render call.
        inline void Discard() { m_bShouldBeDiscarded = true; }
        inline bool ShouldDiscard() { return m_bShouldBeDiscarded; }

    private:
        // Pixel Width & Height of drawer. Total pixel count should be Width * Height.
        uint16_t m_width, m_height;
        // Pixel offset of the drawer from, conventionally, the top-left corner of the display.
        uint16_t m_offsetX, m_offsetY;

        // Specifies whether this Drawer is ready to be drawn. Effectively acts as a lock (and may be implemented with locks under the hood depending on platform).
        // When false, the Engine is still modifying it and the platform should leave it alone.
        // When true, the Engine is done modifying it and the platform should draw it on next Render call.
        std::atomic<bool> m_bReadyToDraw;

        /// Specifies whether this Drawer should be discarded by the Platform after the next Render call, whether it is ready to be drawn or not (in the former case,
        /// it WILL be drawn before being discarded).
        std::atomic<bool> m_bShouldBeDiscarded;

        // Internal pointer to allocated pixel buffer memory.
        Pixel_RGBA* m_pixelBuffer;
    };

    /// @brief Allocates and returns a new Memory Map Drawer for drawing over the entirety of the available display space.
    /// @return Newly allocated Memory Map Drawer. Since it is supposed to cover the entire display space, its width and height are set by the platform.
    /// The drawer should be fully ready for modification by the Engine code, and once "released" (bReadyToDraw is set) should be drawn on the next call to the
    /// Platform Render function.
    /// #TODO(Marc): Support non-full displays so specific screen elements may be drawn separately, moved around...
    virtual std::shared_ptr<MemoryMapDrawer> AllocateFullDisplayDrawer() = 0;

    /// @brief Triggers a rendering update on the platform, whererin it will go through every drawing resource / request that are ready to draw and do so,
    /// discarding them as appropriate.
    /// @note  Depending on the platform, it might actually execute the work synchronously or just signal some other thread to do it.
    /// In the latter case, it will probably lock all further rendering resource allocation and the command buffer until done, but in the meantime the Engine should
    /// have plenty of non-rendering work to do in the meantime.
    virtual void RenderUpdate() = 0;
};

#endif // PLATFORM_H