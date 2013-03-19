// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include "simple_order.h"

#include <iostream>

namespace liquibook { namespace impl {

uint32_t SimpleOrder::last_order_id_(0);

SimpleOrder::SimpleOrder(bool is_buy,
                         Price price,
                         Quantity qty)
: state_(os_new),
  is_buy_(is_buy),
  price_(price),
  order_qty_(qty),
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

Price
SimpleOrder::price() const
{
  return price_;
}

Quantity
SimpleOrder::order_qty() const
{
  return order_qty_;
}

Quantity
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

const Quantity&
SimpleOrder::filled_qty() const
{
  return filled_qty_;
}

const Cost&
SimpleOrder::filled_cost() const
{
  return filled_cost_;
}

void
SimpleOrder::fill(Quantity fill_qty,
                  Cost fill_cost,
                  FillId /*fill_id*/)
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
SimpleOrder::replace(Quantity new_order_qty, Price new_price)
{
  if (os_accepted == state_) {
    order_qty_ = new_order_qty;
    price_ = new_price;
  }
}

} }

