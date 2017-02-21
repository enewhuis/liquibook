// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include "simple_order.h"
#include <book/depth_order_book.h>
#include <iostream>

namespace liquibook { namespace simple {

// @brief binding of DepthOrderBook template with SimpleOrder* order pointer.
template <int SIZE = 5>
class SimpleOrderBook : public book::DepthOrderBook<SimpleOrder*, SIZE> {
public:
  typedef book::Callback<SimpleOrder*> SimpleCallback;
  typedef uint32_t FillId;

  SimpleOrderBook();

  // Override callback handling to update SimpleOrder state
  virtual void perform_callback(SimpleCallback& cb);
private:
  FillId fill_id_;
};

template <int SIZE>
SimpleOrderBook<SIZE>::SimpleOrderBook() 
: fill_id_(0)
{
}

template <int SIZE>
inline void
SimpleOrderBook<SIZE>::perform_callback(SimpleCallback& cb)
{
  book::DepthOrderBook<SimpleOrder*, SIZE>::perform_callback(cb);
  switch(cb.type) {
    case SimpleCallback::cb_order_accept:
      cb.order->accept();
      break;
    case SimpleCallback::cb_order_fill: {
      // Increment fill ID once
      ++fill_id_;
      // Update the orders
      book::Cost fill_cost = cb.quantity * cb.price;
      cb.matched_order->fill(cb.quantity, fill_cost, fill_id_);
      cb.order->fill(cb.quantity, fill_cost, fill_id_);
      break;
    }
    case SimpleCallback::cb_order_cancel:
      cb.order->cancel();
      break;
    case SimpleCallback::cb_order_replace:
      // Modify the order itself
      cb.order->replace(cb.delta, cb.price);
      break;
    default:
      // Nothing
      break;
  }
}
} }
