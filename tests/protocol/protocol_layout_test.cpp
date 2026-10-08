#include <cstddef>

#include "exsim/protocol/itch/itch.hpp"
#include "exsim/protocol/moldudp64/moldudp64.hpp"
#include "exsim/protocol/ouch/ouch.hpp"
#include "exsim/protocol/soupbintcp/soupbintcp.hpp"

namespace {

namespace itch = exsim::protocol::itch;
namespace mold = exsim::protocol::moldudp64;
namespace soup = exsim::protocol::soupbintcp;
namespace ouch = exsim::protocol::ouch;

// ---- ITCH 5.0 ----

// Common header shared by every ITCH message
template <typename Msg>
consteval bool itch_header_ok() {
    return offsetof(Msg, message_type) == 0 && offsetof(Msg, stock_locate) == 1 &&
           offsetof(Msg, tracking_number) == 3 && offsetof(Msg, timestamp) == 5;
}

static_assert(itch_header_ok<itch::SystemEvent>());
static_assert(itch_header_ok<itch::StockDirectory>());
static_assert(itch_header_ok<itch::StockTradingAction>());
static_assert(itch_header_ok<itch::AddOrderNoMPID>());
static_assert(itch_header_ok<itch::OrderExecuted>());
static_assert(itch_header_ok<itch::OrderCancel>());
static_assert(itch_header_ok<itch::OrderDelete>());
static_assert(itch_header_ok<itch::OrderReplace>());

static_assert(offsetof(itch::SystemEvent, event_code) == 11);

static_assert(offsetof(itch::StockDirectory, stock) == 11);
static_assert(offsetof(itch::StockDirectory, market_category) == 19);
static_assert(offsetof(itch::StockDirectory, financial_status_indicator) == 20);
static_assert(offsetof(itch::StockDirectory, round_lot_size) == 21);
static_assert(offsetof(itch::StockDirectory, round_lots_only) == 25);
static_assert(offsetof(itch::StockDirectory, issue_classification) == 26);
static_assert(offsetof(itch::StockDirectory, issue_subtype) == 27);
static_assert(offsetof(itch::StockDirectory, authenticity) == 29);
static_assert(offsetof(itch::StockDirectory, short_sale_threshold_indicator) == 30);
static_assert(offsetof(itch::StockDirectory, ipo_flag) == 31);
static_assert(offsetof(itch::StockDirectory, luld_reference_price_tier) == 32);
static_assert(offsetof(itch::StockDirectory, etp_flag) == 33);
static_assert(offsetof(itch::StockDirectory, etp_leverage_factor) == 34);
static_assert(offsetof(itch::StockDirectory, inverse_indicator) == 38);

static_assert(offsetof(itch::StockTradingAction, stock) == 11);
static_assert(offsetof(itch::StockTradingAction, trading_state) == 19);
static_assert(offsetof(itch::StockTradingAction, reserved) == 20);
static_assert(offsetof(itch::StockTradingAction, reason) == 21);

static_assert(offsetof(itch::AddOrderNoMPID, order_reference_number) == 11);
static_assert(offsetof(itch::AddOrderNoMPID, buy_sell_indicator) == 19);
static_assert(offsetof(itch::AddOrderNoMPID, shares) == 20);
static_assert(offsetof(itch::AddOrderNoMPID, stock) == 24);
static_assert(offsetof(itch::AddOrderNoMPID, price) == 32);

static_assert(offsetof(itch::OrderExecuted, order_reference_number) == 11);
static_assert(offsetof(itch::OrderExecuted, executed_shares) == 19);
static_assert(offsetof(itch::OrderExecuted, match_number) == 23);

static_assert(offsetof(itch::OrderCancel, order_reference_number) == 11);
static_assert(offsetof(itch::OrderCancel, cancelled_shares) == 19);

static_assert(offsetof(itch::OrderDelete, order_reference_number) == 11);

static_assert(offsetof(itch::OrderReplace, original_order_reference_number) == 11);
static_assert(offsetof(itch::OrderReplace, new_order_reference_number) == 19);
static_assert(offsetof(itch::OrderReplace, shares) == 27);
static_assert(offsetof(itch::OrderReplace, price) == 31);

// ---- MoldUDP64 ----

static_assert(offsetof(mold::DownstreamHeader, session) == 0);
static_assert(offsetof(mold::DownstreamHeader, sequence_number) == 10);
static_assert(offsetof(mold::DownstreamHeader, message_count) == 18);

static_assert(offsetof(mold::RequestPacket, session) == 0);
static_assert(offsetof(mold::RequestPacket, sequence_number) == 10);
static_assert(offsetof(mold::RequestPacket, requested_message_count) == 18);

static_assert(sizeof(mold::MessageBlockHeader) == 2);

// ---- SoupBinTCP 4.0 ----

static_assert(offsetof(soup::PacketHeader, packet_length) == 0);
static_assert(offsetof(soup::PacketHeader, packet_type) == 2);

static_assert(offsetof(soup::LoginRequest, username) == 3);
static_assert(offsetof(soup::LoginRequest, password) == 9);
static_assert(offsetof(soup::LoginRequest, requested_session) == 19);
static_assert(offsetof(soup::LoginRequest, requested_sequence_number) == 29);

static_assert(offsetof(soup::LoginAccepted, session) == 3);
static_assert(offsetof(soup::LoginAccepted, sequence_number) == 13);

static_assert(offsetof(soup::LoginRejected, reject_reason_code) == 3);

// ---- OUCH 5.0 inbound ----

static_assert(offsetof(ouch::EnterOrder, user_ref_num) == 1);
static_assert(offsetof(ouch::EnterOrder, side) == 5);
static_assert(offsetof(ouch::EnterOrder, quantity) == 6);
static_assert(offsetof(ouch::EnterOrder, symbol) == 10);
static_assert(offsetof(ouch::EnterOrder, price) == 18);
static_assert(offsetof(ouch::EnterOrder, time_in_force) == 26);
static_assert(offsetof(ouch::EnterOrder, display) == 27);
static_assert(offsetof(ouch::EnterOrder, capacity) == 28);
static_assert(offsetof(ouch::EnterOrder, intermarket_sweep_eligibility) == 29);
static_assert(offsetof(ouch::EnterOrder, cross_type) == 30);
static_assert(offsetof(ouch::EnterOrder, cl_ord_id) == 31);
static_assert(offsetof(ouch::EnterOrder, appendage_length) == 45);

static_assert(offsetof(ouch::ReplaceOrderRequest, orig_user_ref_num) == 1);
static_assert(offsetof(ouch::ReplaceOrderRequest, user_ref_num) == 5);
static_assert(offsetof(ouch::ReplaceOrderRequest, quantity) == 9);
static_assert(offsetof(ouch::ReplaceOrderRequest, price) == 13);
static_assert(offsetof(ouch::ReplaceOrderRequest, time_in_force) == 21);
static_assert(offsetof(ouch::ReplaceOrderRequest, display) == 22);
static_assert(offsetof(ouch::ReplaceOrderRequest, intermarket_sweep_eligibility) == 23);
static_assert(offsetof(ouch::ReplaceOrderRequest, cl_ord_id) == 24);
static_assert(offsetof(ouch::ReplaceOrderRequest, appendage_length) == 38);

static_assert(offsetof(ouch::CancelOrderRequest, user_ref_num) == 1);
static_assert(offsetof(ouch::CancelOrderRequest, quantity) == 5);

// ---- OUCH 5.0 outbound ----

static_assert(offsetof(ouch::SystemEvent, timestamp) == 1);
static_assert(offsetof(ouch::SystemEvent, event_code) == 9);

static_assert(offsetof(ouch::OrderAccepted, timestamp) == 1);
static_assert(offsetof(ouch::OrderAccepted, user_ref_num) == 9);
static_assert(offsetof(ouch::OrderAccepted, side) == 13);
static_assert(offsetof(ouch::OrderAccepted, quantity) == 14);
static_assert(offsetof(ouch::OrderAccepted, symbol) == 18);
static_assert(offsetof(ouch::OrderAccepted, price) == 26);
static_assert(offsetof(ouch::OrderAccepted, time_in_force) == 34);
static_assert(offsetof(ouch::OrderAccepted, display) == 35);
static_assert(offsetof(ouch::OrderAccepted, order_reference_number) == 36);
static_assert(offsetof(ouch::OrderAccepted, capacity) == 44);
static_assert(offsetof(ouch::OrderAccepted, intermarket_sweep_eligibility) == 45);
static_assert(offsetof(ouch::OrderAccepted, cross_type) == 46);
static_assert(offsetof(ouch::OrderAccepted, order_state) == 47);
static_assert(offsetof(ouch::OrderAccepted, cl_ord_id) == 48);
static_assert(offsetof(ouch::OrderAccepted, appendage_length) == 62);

static_assert(offsetof(ouch::OrderReplaced, timestamp) == 1);
static_assert(offsetof(ouch::OrderReplaced, orig_user_ref_num) == 9);
static_assert(offsetof(ouch::OrderReplaced, user_ref_num) == 13);
static_assert(offsetof(ouch::OrderReplaced, side) == 17);
static_assert(offsetof(ouch::OrderReplaced, quantity) == 18);
static_assert(offsetof(ouch::OrderReplaced, symbol) == 22);
static_assert(offsetof(ouch::OrderReplaced, price) == 30);
static_assert(offsetof(ouch::OrderReplaced, time_in_force) == 38);
static_assert(offsetof(ouch::OrderReplaced, display) == 39);
static_assert(offsetof(ouch::OrderReplaced, order_reference_number) == 40);
static_assert(offsetof(ouch::OrderReplaced, capacity) == 48);
static_assert(offsetof(ouch::OrderReplaced, intermarket_sweep_eligibility) == 49);
static_assert(offsetof(ouch::OrderReplaced, cross_type) == 50);
static_assert(offsetof(ouch::OrderReplaced, order_state) == 51);
static_assert(offsetof(ouch::OrderReplaced, cl_ord_id) == 52);
static_assert(offsetof(ouch::OrderReplaced, appendage_length) == 66);

static_assert(offsetof(ouch::OrderCanceled, timestamp) == 1);
static_assert(offsetof(ouch::OrderCanceled, user_ref_num) == 9);
static_assert(offsetof(ouch::OrderCanceled, quantity) == 13);
static_assert(offsetof(ouch::OrderCanceled, reason) == 17);

static_assert(offsetof(ouch::OrderExecuted, timestamp) == 1);
static_assert(offsetof(ouch::OrderExecuted, user_ref_num) == 9);
static_assert(offsetof(ouch::OrderExecuted, quantity) == 13);
static_assert(offsetof(ouch::OrderExecuted, price) == 17);
static_assert(offsetof(ouch::OrderExecuted, liquidity_flag) == 25);
static_assert(offsetof(ouch::OrderExecuted, match_number) == 26);
static_assert(offsetof(ouch::OrderExecuted, appendage_length) == 34);

static_assert(offsetof(ouch::OrderRejected, timestamp) == 1);
static_assert(offsetof(ouch::OrderRejected, user_ref_num) == 9);
static_assert(offsetof(ouch::OrderRejected, reason) == 13);
static_assert(offsetof(ouch::OrderRejected, cl_ord_id) == 15);

}   // namespace
