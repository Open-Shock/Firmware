#pragma once

#include <WString.h>

#include <cstdint>

namespace ShockLink::CaptivePortal
{
    bool Start();
    void Stop();
    bool IsRunning();
    void Update();

    bool BroadcastMessageTXT(const char *data, std::size_t len);
    bool BroadcastMessageBIN(const std::uint8_t *data, std::size_t len);
    inline bool BroadcastMessageTXT(const String &message)
    {
        return BroadcastMessageTXT(message.c_str(), message.length());
    }
};