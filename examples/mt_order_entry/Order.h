#pragma once

#include "OrderFwd.h"
// todo fix include path
#include "../../src/book/types.h"

#include <string>

namespace orderentry
{

class Order
{
public:
    enum State{
        Submitted,
        Rejected, // Terminal state
        Accepted,
        ModifyRequested,
        ModifyRejected,
        Modified,
        PartialFilled,
        Filled, // Terminal State
        CancelRequested,
        CancelRejected,
        Cancelled, // Terminal state
        Unknown
    };

public:
    Order(const std::string & id,
        bool buy_side,
        liquibook::book::Quantity quantity,
        std::string symbol,
        liquibook::book::Price price,
        liquibook::book::Price stopPrice,
        bool aon,
        bool ioc);

    ///////////;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    // Implement the liquibook::book::order concept.

    /// @brief is this a limit order?
    bool is_limit() const;

    /// @brief is this order a buy?
    bool is_buy() const;

    /// @brief is this all-or-none
    bool is_aon() const;

    /// @brief is this immediate-or-cancel
    bool is_ioc() const;

    bool is_stop() const;

    std::string symbol() const;

    std::string order_id() const;

    /// @brief get the price of this order, or 0 if a market order
    liquibook::book::Price price() const;

    /// @brief get the quantity of this order
    liquibook::book::Quantity order_qty() const;

    /// @brief get the stop price of this order, or 0 if it is not a stop order
    liquibook::book::Price stop_price() const;

private:
    std::string id_;
    bool buy_side_;
    liquibook::book::Quantity quantity_;
    std::string symbol_;
    liquibook::book::Price price_;
    liquibook::book::Price stopPrice_;
    bool aon_;
    bool ioc_;
};

std::ostream & operator << (std::ostream & out, Order order);

}
