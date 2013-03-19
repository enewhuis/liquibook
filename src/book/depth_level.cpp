// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include "depth_level.h"
#include "types.h"
#include <stdexcept>

namespace liquibook { namespace book {

DepthLevel::DepthLevel()
: price_(INVALID_LEVEL_PRICE),
  order_count_(0),
  aggregate_qty_(0)
{
}

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

const Price&
DepthLevel::price() const
{
  return price_;
}

void
DepthLevel::init(Price price, bool is_excess)
{
  price_ = price;
  order_count_ = 0;
  aggregate_qty_ = 0;
  is_excess_ = is_excess;
}

uint32_t
DepthLevel::order_count() const
{
  return order_count_;
}

Quantity
DepthLevel::aggregate_qty() const
{
  return aggregate_qty_;
}

void
DepthLevel::add_order(Quantity qty)
{
  // Increment/increase
  ++order_count_;
  aggregate_qty_ += qty;
}

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

void
DepthLevel::increase_qty(Quantity qty)
{
  aggregate_qty_ += qty;
}

void
DepthLevel::decrease_qty(Quantity qty)
{
  aggregate_qty_ -= qty;
}

} }

