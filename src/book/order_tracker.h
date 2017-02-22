// Copyright (c) 2012 - 2017 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include "types.h"

namespace liquibook { namespace book {

/// @brief Tracker of an order's state, to keep inside the OrderBook.  
///   Kept separate from the order itself.
template <typename OrderPtr>
class OrderTracker {
public:
  /// @brief construct
  OrderTracker(const OrderPtr& order, OrderConditions conditions = 0);

  /// @brief modify the order quantity
  void change_qty(int32_t delta);

  /// @brief fill an order
  /// @param qty the number of shares filled in this fill
  void fill(Quantity qty); 

  /// @brief is there no remaining open quantity in this order?
  bool filled() const;

  /// @brief get the total filled quantity of this order
  Quantity filled_qty() const;

  /// @brief get the open quantity of this order
  Quantity open_qty() const;

  /// @brief get the order pointer
  const OrderPtr& ptr() const;

  /// @brief get the order pointer
  OrderPtr& ptr();

  /// @ brief is this order marked all or none?
  bool all_or_none() const;

  /// @ brief is this order marked immediate or cancel?
  bool immediate_or_cancel() const;

  Quantity reserve(int32_t reserved);

private:
  OrderPtr order_;
  Quantity open_qty_;
  int32_t reserved_;
  OrderConditions conditions_;
};

template <class OrderPtr>
OrderTracker<OrderPtr>::OrderTracker(
  const OrderPtr& order,
  OrderConditions conditions)
: order_(order),
  open_qty_(order->order_qty()),
  reserved_(0),
  conditions_(conditions)
{
#if defined(LIQUIBOOK_ORDER_KNOWS_CONDITIONS)
  if(order->all_or_none())
  {
    conditions |= oc_all_or_none;
  }
  if(order->immediate_or_cancel())
  {
    conditions |= oc_immediate_or_cancel;
  }
#endif
}

template <class OrderPtr>
Quantity
OrderTracker<OrderPtr>::reserve(int32_t reserved)
{
  reserved_ += reserved;
  return open_qty_  - reserved_;
}

template <class OrderPtr>
void
OrderTracker<OrderPtr>::change_qty(int32_t delta)
{
  if ((delta < 0 && 
      (int)open_qty_ < std::abs(delta))) {
    throw 
        std::runtime_error("Replace size reduction larger than open quantity");
  }
  open_qty_ += delta;
}

template <class OrderPtr>
void
OrderTracker<OrderPtr>::fill(Quantity qty) 
{
  if (qty > open_qty_) {
    throw std::runtime_error("Fill size larger than open quantity");
  }
  open_qty_ -= qty;
}

template <class OrderPtr>
bool
OrderTracker<OrderPtr>::filled() const
{
  return open_qty_ == 0;
}

template <class OrderPtr>
Quantity
OrderTracker<OrderPtr>::filled_qty() const
{
  return order_->order_qty() - open_qty();
}

// TODO: Rename this to be available and change the rest of the
// system to use that, then provide a method to get to the open
// quantity without considering reserved
template <class OrderPtr>
Quantity
OrderTracker<OrderPtr>::open_qty() const
{
  return open_qty_ - reserved_;
}

template <class OrderPtr>
const OrderPtr&
OrderTracker<OrderPtr>::ptr() const
{
  return order_;
}

template <class OrderPtr>
OrderPtr&
OrderTracker<OrderPtr>::ptr()
{
  return order_;
}

template <class OrderPtr>
bool
OrderTracker<OrderPtr>::all_or_none() const
{
  return bool(conditions_ & oc_all_or_none);
}

template <class OrderPtr>
bool
OrderTracker<OrderPtr>::immediate_or_cancel() const
{
    return bool((conditions_ & oc_immediate_or_cancel) != 0);
}

} }
