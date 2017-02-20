// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include "types.h"

namespace liquibook { namespace book {

/// @brief interface an order must implement in order to be used by OrderBook.
/// Note: structly speaking, inheriting from Order should not be required, 
///       due to the template implementation of OrderBook.
class Order {
public:
  /// @brief is this a limit order?
  bool is_limit() const;

  /// @brief is this order a buy?
  virtual bool is_buy() const = 0;

  /// @brief get the price of this order, or 0 if a market order
  virtual Price price() const = 0;

  /// @brief get the stop price (if any) for this order.
  /// @returns the stop price or zero if not a stop order
  virtual Price stop_price() const;

  /// @brief get the quantity of this order
  virtual Quantity order_qty() const = 0;

  /// @brief if no trades should happen until the order
  /// can be filled completely.
  /// Note: one or more trades may be used to fill the order.
  virtual bool all_or_none() const;

  /// @brief After generating as many trades as possible against
  /// orders already on the market, cancel any remaining quantity.
  virtual bool immediate_or_cancel() const;
};

inline
bool
Order::is_limit() const 
{
  return (price() > 0);
}

inline
Price
Order::stop_price() const
{
  // default to not a stop order
  return 0;
}

inline
bool
Order::all_or_none() const
{
  // default to normal
  return false;
}

inline
bool
Order::immediate_or_cancel() const
{
  // default to normal
  return false;
}

} }
