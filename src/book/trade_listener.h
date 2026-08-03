// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

namespace liquibook { namespace book {

/// @brief listener of trade events.   Implement to build a trade feed.
template <class OrderBook >
class TradeListener {
public:
  /// @brief callback for a trade
  /// @param book the order book of the fill (not defined whether this is before
  ///      or after fill)
  /// @param qty the quantity of this fill
  /// @param price the price of this fill
  virtual void on_trade(const OrderBook* book,
                        Quantity qty,
                        Price price) = 0;
};

} }
