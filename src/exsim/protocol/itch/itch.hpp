#pragma once

#include <cstdint>

namespace exsim {
namespace protocol {
namespace itch {

// Message types
inline constexpr char kSystemEvent = 'S';
inline constexpr char kStockDirectory = 'R';
inline constexpr char kStockTradingAction = 'H';
inline constexpr char kAddOrderNoMPID = 'A';
inline constexpr char kOrderExecuted = 'E';
inline constexpr char kOrderCancel = 'X';
inline constexpr char kOrderDelete = 'D';
inline constexpr char kOrderReplace = 'U';

// System Event codes
inline constexpr char kStartOfMessages = 'O';
inline constexpr char kStartOfSystemHours = 'S';
inline constexpr char kStartOfMarketHours = 'Q';
inline constexpr char kEndOfMarketHours = 'M';
inline constexpr char kEndOfSystemHours = 'E';
inline constexpr char kEndOfMessages = 'C';

// Buy/Sell indicator
inline constexpr char kBuy = 'B';
inline constexpr char kSell = 'S';

// Trading states
inline constexpr char kHalted = 'H';
inline constexpr char kPaused = 'P';
inline constexpr char kQuotationOnly = 'Q';
inline constexpr char kTrading = 'T';

struct __attribute__((packed)) SystemEvent {
    char message_type;
    uint16_t stock_locate;
    uint16_t tracking_number;
    uint8_t timestamp[6];
    char event_code;
};

struct __attribute__((packed)) StockDirectory {
    char message_type;
    uint16_t stock_locate;
    uint16_t tracking_number;
    uint8_t timestamp[6];
    char stock[8];
    char market_category;
    char financial_status_indicator;
    uint32_t round_lot_size;
    char round_lots_only;
    char issue_classification;
    char issue_subtype[2];
    char authenticity;
    char short_sale_threshold_indicator;
    char ipo_flag;
    char luld_reference_price_tier;
    char etp_flag;
    uint32_t etp_leverage_factor;
    char inverse_indicator;
};

struct __attribute__((packed)) StockTradingAction {
    char message_type;
    uint16_t stock_locate;
    uint16_t tracking_number;
    uint8_t timestamp[6];
    char stock[8];
    char trading_state;
    char reserved;
    char reason[4];
};

struct __attribute__((packed)) AddOrderNoMPID {
    char message_type;
    uint16_t stock_locate;
    uint16_t tracking_number;
    uint8_t timestamp[6];
    uint64_t order_reference_number;
    char buy_sell_indicator;
    uint32_t shares;
    char stock[8];
    uint32_t price;
};

struct __attribute__((packed)) OrderExecuted {
    char message_type;
    uint16_t stock_locate;
    uint16_t tracking_number;
    uint8_t timestamp[6];
    uint64_t order_reference_number;
    uint32_t executed_shares;
    uint64_t match_number;
};

struct __attribute__((packed)) OrderCancel {
    char message_type;
    uint16_t stock_locate;
    uint16_t tracking_number;
    uint8_t timestamp[6];
    uint64_t order_reference_number;
    uint32_t cancelled_shares;
};

struct __attribute__((packed)) OrderDelete {
    char message_type;
    uint16_t stock_locate;
    uint16_t tracking_number;
    uint8_t timestamp[6];
    uint64_t order_reference_number;
};

struct __attribute__((packed)) OrderReplace {
    char message_type;
    uint16_t stock_locate;
    uint16_t tracking_number;
    uint8_t timestamp[6];
    uint64_t original_order_reference_number;
    uint64_t new_order_reference_number;
    uint32_t shares;
    uint32_t price;
};

static_assert(sizeof(SystemEvent) == 12);
static_assert(sizeof(StockDirectory) == 39);
static_assert(sizeof(StockTradingAction) == 25);
static_assert(sizeof(AddOrderNoMPID) == 36);
static_assert(sizeof(OrderExecuted) == 31);
static_assert(sizeof(OrderCancel) == 23);
static_assert(sizeof(OrderDelete) == 19);
static_assert(sizeof(OrderReplace) == 35);

}   // namespace itch
}   // namespace protocol
}   // namespace exsim