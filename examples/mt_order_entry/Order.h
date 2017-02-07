// Copyright (c) 2017 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include "OrderFwd.h"
#include <book/types.h>

#include <string>
#include <vector>

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

    struct StateChange
    {
        State state_;
        std::string description_;
        StateChange()
          : state_(Unknown)
          {}

        StateChange(State state, const std::string & description = "")
            : state_(state)
            , description_(description)
        {}
    };    
    typedef std::vector<StateChange> History;
public:
    Order(const std::string & id,
        bool buy_side,
        liquibook::book::Quantity quantity,
        std::string symbol,
        liquibook::book::Price price,
        liquibook::book::Price stopPrice,
        bool aon,
        bool ioc);

    //////////////////////////
    // Implement the 
    // liquibook::book::order
    // concept.

    /// @brief is this a limit order?
    bool is_limit() const;

    /// @brief is this order a buy?
    bool is_buy() const;

    /// @brief get the price of this order, or 0 if a market order
    liquibook::book::Price price() const;

    /// @brief get the stop price (if any) for this order.
    /// @returns the stop price or zero if not a stop order
    liquibook::book::Price stop_price() const;

    /// @brief get the quantity of this order
    liquibook::book::Quantity order_qty() const;

    /// @brief if no trades should happen until the order
    /// can be filled completely.
    /// Note: one or more trades may be used to fill the order.
    virtual bool all_or_none() const;

    /// @brief After generating as many trades as possible against
    /// orders already on the market, cancel any remaining quantity.
    virtual bool immediate_or_cancel() const;

    std::string symbol() const;

    std::string order_id() const;

    uint32_t quantityFilled() const;

    uint32_t quantityOnMarket() const;

    uint32_t fillCost() const;

    Order & verbose(bool verbose = true);
    bool isVerbose()const;
    const History & history() const;
    const StateChange & currentState() const;

    ///////////////////////////
    // Order life cycle events
    void onSubmitted();
    void onAccepted();
    void onRejected(const char * reason);

    void onFilled(
        liquibook::book::Quantity fill_qty, 
        liquibook::book::Cost fill_cost);

    void onCancelRequested();
    void onCancelled();
    void onCancelRejected(const char * reason);

    void onReplaceRequested(
        const int32_t& size_delta, 
        liquibook::book::Price new_price);

    void onReplaced(const int32_t& size_delta, 
        liquibook::book::Price new_price);

    void onReplaceRejected(const char * reaseon);

private:
    std::string id_;
    bool buy_side_;
    std::string symbol_;
    liquibook::book::Quantity quantity_;
    liquibook::book::Price price_;
    liquibook::book::Price stopPrice_;

    bool aon_;
    bool ioc_;

    liquibook::book::Quantity quantityFilled_;
    int32_t quantityOnMarket_;
    uint32_t fillCost_;
    
    std::vector<StateChange> history_;
    bool verbose_;
};

std::ostream & operator << (std::ostream & out, const Order & order);
std::ostream & operator << (std::ostream & out, const Order::StateChange & event);

}
