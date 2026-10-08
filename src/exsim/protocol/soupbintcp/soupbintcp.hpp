#pragma once

#include <cstdint>

namespace exsim {
namespace protocol {
namespace soupbintcp {

// Client -> Server
inline constexpr char kLoginRequest = 'L';
inline constexpr char kUnsequencedData = 'U';
inline constexpr char kClientHeartbeat = 'R';
inline constexpr char kLogoutRequest = 'O';

// Server -> Client
inline constexpr char kLoginAccepted = 'A';
inline constexpr char kLoginRejected = 'J';
inline constexpr char kSequencedData = 'S';
inline constexpr char kServerHeartbeat = 'H';
inline constexpr char kEndOfSession = 'Z';

// Both directions
inline constexpr char kDebug = '+';

// Login Rejected reject reason codes
inline constexpr char kRejectNotAuthorized = 'A';
inline constexpr char kRejectSessionNotAvailable = 'S';

struct __attribute__((packed)) PacketHeader {
    uint16_t packet_length;
    char packet_type;
};

struct __attribute__((packed)) LoginRequest {
    uint16_t packet_length;
    char packet_type;
    char username[6];
    char password[10];
    char requested_session[10];
    char requested_sequence_number[20];
};

struct __attribute__((packed)) LoginAccepted {
    uint16_t packet_length;
    char packet_type;
    char session[10];
    char sequence_number[20];
};

struct __attribute__((packed)) LoginRejected {
    uint16_t packet_length;
    char packet_type;
    char reject_reason_code;
};

static_assert(sizeof(PacketHeader) == 3);
static_assert(sizeof(LoginRequest) == 49);
static_assert(sizeof(LoginAccepted) == 33);
static_assert(sizeof(LoginRejected) == 4);

}   // namespace soupbintcp
}   // namespace protocol
}   // namespace exsim
