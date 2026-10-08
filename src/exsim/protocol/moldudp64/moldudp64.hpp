#pragma once

#include <cstdint>

namespace exsim {
namespace protocol {
namespace moldudp64 {

struct __attribute__((packed)) DownstreamHeader {
    char session[10];
    uint64_t sequence_number;
    uint16_t message_count;
};

struct __attribute__((packed)) RequestPacket {
    char session[10];
    uint64_t sequence_number;
    uint16_t requested_message_count;
};

struct __attribute__((packed)) MessageBlockHeader {
    uint16_t message_length;
};

inline constexpr uint16_t kHeartbeatCount = 0;
inline constexpr uint16_t kEndOfSessionCount = 0xFFFF;

static_assert(sizeof(DownstreamHeader) == 20);
static_assert(sizeof(RequestPacket) == 20);


}   // namesapce moldudp64
}   // namespace protocol
}   // namespace exsim