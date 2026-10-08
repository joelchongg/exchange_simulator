#pragma once

#include <cstdint>

namespace exsim {
namespace protocol {
namespace ouch {

// Inbound message types (Client -> Exchange)
inline constexpr char kEnterOrder = 'O';
inline constexpr char kReplaceOrderRequest = 'U';
inline constexpr char kCancelOrderRequest = 'X';

// Outbound message types (Exchange -> Client)
inline constexpr char kSystemEvent = 'S';
inline constexpr char kOrderAccepted = 'A';
inline constexpr char kOrderReplaced = 'U';
inline constexpr char kOrderCanceled = 'C';
inline constexpr char kOrderExecuted = 'E';
inline constexpr char kOrderRejected = 'J';

// System Event codes
inline constexpr char kStartOfDay = 'S';
inline constexpr char kEndOfDay = 'E';

// Side
inline constexpr char kBuy = 'B';
inline constexpr char kSell = 'S';
inline constexpr char kSellShort = 'T';
inline constexpr char kSellShortExempt = 'E';

// Time In Force
inline constexpr char kTifDay = '0';
inline constexpr char kTifIoc = '3';
inline constexpr char kTifGtx = '5';
inline constexpr char kTifGtt = '6';
inline constexpr char kTifAfterHours = 'E';

// Display
inline constexpr char kDisplayVisible = 'Y';
inline constexpr char kDisplayHidden = 'N';
inline constexpr char kDisplayAttributable = 'A';
inline constexpr char kDisplayConformant = 'Z';     // outbound only

// Capacity
inline constexpr char kCapacityAgency = 'A';
inline constexpr char kCapacityPrincipal = 'P';
inline constexpr char kCapacityRiskless = 'R';
inline constexpr char kCapacityOther = 'O';

// InterMarket Sweep Eligibility
inline constexpr char kIsoEligible = 'Y';
inline constexpr char kIsoNotEligible = 'N';

// Cross Type
inline constexpr char kCrossContinuousMarket = 'N';

// Order State
inline constexpr char kOrderLive = 'L';
inline constexpr char kOrderDead = 'D';

// Order Cancel reasons
inline constexpr char kCancelClosed = 'E';
inline constexpr char kCancelHalted = 'H';
inline constexpr char kCancelImmediateOrCancel = 'I';
inline constexpr char kCancelSelfMatchPrevention = 'Q';
inline constexpr char kCancelSupervisory = 'S';
inline constexpr char kCancelTimeout = 'T';
inline constexpr char kCancelUserRequested = 'U';
inline constexpr char kCancelSystem = 'Z';

// Order Reject reasons
inline constexpr uint16_t kRejectDestinationClosed = 0x0002;
inline constexpr uint16_t kRejectHalted = 0x0007;
inline constexpr uint16_t kRejectInvalidSide = 0x0009;
inline constexpr uint16_t kRejectProcessingError = 0x000A;
inline constexpr uint16_t kRejectOther = 0x000F;
inline constexpr uint16_t kRejectInvalidQuantity = 0x0013;
inline constexpr uint16_t kRejectReplaceNotAllowed = 0x0015;
inline constexpr uint16_t kRejectInvalidSymbol = 0x0017;
inline constexpr uint16_t kRejectInvalidPrice = 0x001D;
inline constexpr uint16_t kRejectPortMessageRate = 0x0029;

// Liquidity Flags
inline constexpr char kLiquidityAdded = 'A';
inline constexpr char kLiquidityRemoved = 'R';

// ---- Inbound ----

struct __attribute__((packed)) EnterOrder {
    char message_type;
    uint32_t user_ref_num;
    char side;
    uint32_t quantity;
    char symbol[8];
    uint64_t price;
    char time_in_force;
    char display;
    char capacity;
    char intermarket_sweep_eligibility;
    char cross_type;
    char cl_ord_id[14];
    uint16_t appendage_length;
};

struct __attribute__((packed)) ReplaceOrderRequest {
    char message_type;
    uint32_t orig_user_ref_num;
    uint32_t user_ref_num;
    uint32_t quantity;
    uint64_t price;
    char time_in_force;
    char display;
    char intermarket_sweep_eligibility;
    char cl_ord_id[14];
    uint16_t appendage_length;
};

// Appendage length is optional on this message
struct __attribute__((packed)) CancelOrderRequest {
    char message_type;
    uint32_t user_ref_num;
    uint32_t quantity;
};

// ---- Outbound ----

struct __attribute__((packed)) SystemEvent {
    char message_type;
    uint64_t timestamp;
    char event_code;
};

struct __attribute__((packed)) OrderAccepted {
    char message_type;
    uint64_t timestamp;
    uint32_t user_ref_num;
    char side;
    uint32_t quantity;
    char symbol[8];
    uint64_t price;
    char time_in_force;
    char display;
    uint64_t order_reference_number;
    char capacity;
    char intermarket_sweep_eligibility;
    char cross_type;
    char order_state;
    char cl_ord_id[14];
    uint16_t appendage_length;
};

struct __attribute__((packed)) OrderReplaced {
    char message_type;
    uint64_t timestamp;
    uint32_t orig_user_ref_num;
    uint32_t user_ref_num;
    char side;
    uint32_t quantity;
    char symbol[8];
    uint64_t price;
    char time_in_force;
    char display;
    uint64_t order_reference_number;
    char capacity;
    char intermarket_sweep_eligibility;
    char cross_type;
    char order_state;
    char cl_ord_id[14];
    uint16_t appendage_length;
};

// Appendage length is optional on this message
struct __attribute__((packed)) OrderCanceled {
    char message_type;
    uint64_t timestamp;
    uint32_t user_ref_num;
    uint32_t quantity;
    char reason;
};

struct __attribute__((packed)) OrderExecuted {
    char message_type;
    uint64_t timestamp;
    uint32_t user_ref_num;
    uint32_t quantity;
    uint64_t price;
    char liquidity_flag;
    uint64_t match_number;
    uint16_t appendage_length;
};

// Appendage length is optional on this message
struct __attribute__((packed)) OrderRejected {
    char message_type;
    uint64_t timestamp;
    uint32_t user_ref_num;
    uint16_t reason;
    char cl_ord_id[14];
};

// Wire sizes from the OUCH 5.0 spec (without appendage)
static_assert(sizeof(EnterOrder) == 47);
static_assert(sizeof(ReplaceOrderRequest) == 40);
static_assert(sizeof(CancelOrderRequest) == 9);
static_assert(sizeof(SystemEvent) == 10);
static_assert(sizeof(OrderAccepted) == 64);
static_assert(sizeof(OrderReplaced) == 68);
static_assert(sizeof(OrderCanceled) == 18);
static_assert(sizeof(OrderExecuted) == 36);
static_assert(sizeof(OrderRejected) == 29);

}   // namespace ouch
}   // namespace protocol
}   // namespace exsim
