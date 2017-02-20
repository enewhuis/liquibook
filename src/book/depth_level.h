// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include "depth_constants.h"

namespace liquibook { namespace book {

/// @brief a single level of the limit order book aggregated by price
class DepthLevel {
public:
  /// @brief construct
  DepthLevel();

  /// @brief assign
  DepthLevel& operator=(const DepthLevel& rhs);

  /// @brief get price
  const Price& price() const;
  /// @brief get count
  uint32_t order_count() const;
  /// @brief get aggregate quantity
  Quantity aggregate_qty() const;
  /// @brief is this level part of the excess
  bool is_excess() const { return is_excess_; }

  void init(Price price, bool is_excess);

  /// @brief add an order to the level
  /// @param qty open quantity of the order
  void add_order(Quantity qty);

  /// @brief increase the quantity of existing orders
  /// @param qty amount to increase the quantity by
  void increase_qty(Quantity qty);

  /// @brief decrease the quantity of existing orders
  /// @param qty amount to decrease the quantity by
  void decrease_qty(Quantity qty);

  /// @brief overwrite all values of the level
  /// @param price the level price
  /// @param qty the aggegate quantity
  /// @param order_count the number of orders
  /// @param last_change the last change ID (optional)
  void set(Price price, 
           Quantity qty,
           uint32_t order_count,
           ChangeId last_change = 0);

  /// @brief cancel or fill an order, decrease count and quantity
  /// @param qty the closed quantity
  /// @return true if the level is now empty
  bool close_order(Quantity qty);

  /// @brief set last changed stamp on this level
  void last_change(ChangeId last_change) { last_change_ = last_change; }

  /// @brief get last change stamp for this level
  ChangeId last_change() const { return last_change_; }

  /// @brief has the level changed since the given stamp?
  /// @param last_published_change the stamp to compare to
  bool changed_since(ChangeId last_published_change) const;

private:
  Price price_;
  uint32_t order_count_;
  Quantity aggregate_qty_;
  bool is_excess_;
public:
  ChangeId last_change_;
};

inline bool
DepthLevel::changed_since(ChangeId last_published_change) const
{
  return last_change_ > last_published_change;
}

inline
DepthLevel::DepthLevel()
  : price_(INVALID_LEVEL_PRICE),
  order_count_(0),
  aggregate_qty_(0)
{
}

inline
DepthLevel&
DepthLevel::operator=(const DepthLevel& rhs)
{
  price_ = rhs.price_;
  order_count_ = rhs.order_count_;
  aggregate_qty_ = rhs.aggregate_qty_;
  if (rhs.price_ != INVALID_LEVEL_PRICE) {
    last_change_ = rhs.last_change_;
  }

  // Do not copy is_excess_

  return *this;
}

inline
const Price&
DepthLevel::price() const
{
  return price_;
}

inline
void
DepthLevel::init(Price price, bool is_excess)
{
  price_ = price;
  order_count_ = 0;
  aggregate_qty_ = 0;
  is_excess_ = is_excess;
}

inline
uint32_t
DepthLevel::order_count() const
{
  return order_count_;
}

inline
Quantity
DepthLevel::aggregate_qty() const
{
  return aggregate_qty_;
}

inline
void
DepthLevel::add_order(Quantity qty)
{
  // Increment/increase
  ++order_count_;
  aggregate_qty_ += qty;
}

inline
bool
DepthLevel::close_order(Quantity qty)
{
  bool empty = false;
  // If this is the last order, reset the level
  if (order_count_ == 0) {
    throw std::runtime_error("DepthLevel::close_order "
      "order count too low");
  } else if (order_count_ == 1) {
    order_count_ = 0;
    aggregate_qty_ = 0;
    empty = true;
    // Else, decrement/decrease
  } else {
    --order_count_;
    if (aggregate_qty_ >= qty) {
      aggregate_qty_ -= qty;
    } else {
      throw std::runtime_error("DepthLevel::close_order "
        "level quantity too low");
    }
  }
  return empty;
}

inline
void
DepthLevel::set(Price price, 
  Quantity qty,
  uint32_t order_count,
  ChangeId last_change)
{
  price_ = price;
  aggregate_qty_ = qty;
  order_count_ = order_count;
  last_change_ = last_change;
}

inline
void
DepthLevel::increase_qty(Quantity qty)
{
  aggregate_qty_ += qty;
}

inline
void
DepthLevel::decrease_qty(Quantity qty)
{
  aggregate_qty_ -= qty;
}

} }

