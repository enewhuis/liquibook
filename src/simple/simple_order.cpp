// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include "simple_order.h"

#include <iostream>

namespace liquibook { namespace simple {

uint32_t SimpleOrder::last_order_id_(0);

SimpleOrder::SimpleOrder(
  bool is_buy,
  book::Price price,
  book::Quantity qty,
  book::Price stop_price,
  book::OrderConditions conditions)
: state_(os_new),
  is_buy_(is_buy),
  order_qty_(qty),
  price_(price),
  stop_price_(stop_price),
  conditions_(conditions),
  filled_qty_(0),
  filled_cost_(0),
  order_id_(++last_order_id_)
{
}

const OrderState&
SimpleOrder::state() const
{
  return state_;
}

bool 
SimpleOrder::is_buy() const
{
  return is_buy_;
}

book::Price
SimpleOrder::price() const
{
  return price_;
}

book::Price
SimpleOrder::stop_price() const
{
  return stop_price_;
}

book::OrderConditions
SimpleOrder::conditions() const
{
  return conditions_;
}

bool
SimpleOrder::all_or_none() const
{
  return (conditions_ & book::OrderCondition::oc_all_or_none) != 0;
}

bool
SimpleOrder::immediate_or_cancel() const
{
  return (conditions_ & book::OrderCondition::oc_immediate_or_cancel) != 0;
}
book::Quantity
SimpleOrder::order_qty() const
{
  return order_qty_;
}

book::Quantity
SimpleOrder::open_qty() const
{
  // If not completely filled, calculate
  if (filled_qty_ < order_qty_) {
    return order_qty_ - filled_qty_;
  // Else prevent accidental overflow
  } else {
    return 0;
  }
}

const book::Quantity&
SimpleOrder::filled_qty() const
{
  return filled_qty_;
}

const book::Cost&
SimpleOrder::filled_cost() const
{
  return filled_cost_;
}

void
SimpleOrder::fill(book::Quantity fill_qty,
                  book::Cost fill_cost,
                  book::FillId /*fill_id*/)
{
  filled_qty_ += fill_qty;
  filled_cost_ += fill_cost;
  if (!open_qty()) {
    state_ = os_complete;
  }
}

void
SimpleOrder::accept()
{
  if (os_new == state_) {
    state_ = os_accepted;
  }
}

void
SimpleOrder::cancel()
{
  if (os_complete != state_) {
    state_ = os_cancelled;
  }
}

void
SimpleOrder::replace(book::Quantity size_delta, book::Price new_price)
{
  if (os_accepted == state_) {
    order_qty_ += size_delta;
    price_ = new_price;
  }
}

} }

