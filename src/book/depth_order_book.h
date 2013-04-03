// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#ifndef depth_order_book_h
#define depth_order_book_h

#include "book/order_book.h"
#include "book/depth.h"
#include <iostream>

namespace liquibook { namespace book {

/// @brief Implementation of order book child class, that incorporates
///        aggregate depth tracking.  Overrides perform_callback() method to 
//         track depth aggregated by price.
template <class OrderPtr = Order*, int SIZE = 5>
class DepthOrderBook : public book::OrderBook<OrderPtr> {
public:
  typedef typename book::Depth<SIZE> DepthTracker;
  typedef book::Callback<OrderPtr> DobCallback;

  DepthOrderBook();

  virtual void perform_callback(DobCallback& cb);
  DepthTracker& depth();
  const DepthTracker& depth() const;

private:
  book::FillId fill_id_;
  DepthTracker depth_;
};

template <class OrderPtr, int SIZE>
DepthOrderBook<OrderPtr, SIZE>::DepthOrderBook()
: fill_id_(0)
{
}

template <class OrderPtr, int SIZE>
inline void
DepthOrderBook<OrderPtr, SIZE>::perform_callback(DobCallback& cb)
{
  switch(cb.type) {
    case DobCallback::cb_order_accept:
      cb.order->accept();
      // If the order is a limit order
      if (cb.order->is_limit()) {
        // If the order is completely filled on acceptance, do not modify 
        // depth unnecessarily
        if (cb.match_qty == cb.order->order_qty()) {
          // Don't tell depth about this order - it's going away immediately.
          // Instead tell Depth about future fills to ignore
          depth_.ignore_fill_qty(cb.match_qty, cb.order->is_buy());
        } else {
          // Add to bid or ask depth
          depth_.add_order(cb.order->price(), 
                           cb.order->order_qty(), 
                           cb.order->is_buy());
        }
      }
      break;

    case DobCallback::cb_order_fill: {
      // If the matched order is a limit order
      if (cb.matched_order->is_limit()) {
        // Inform the depth
        depth_.fill_order(cb.matched_order->price(), 
                          cb.matched_order->open_qty(),
                          cb.fill_qty,
                          cb.matched_order->is_buy());
      }
      // If the inbound order is a limit order
      if (cb.order->is_limit()) {
        // Inform the depth
        depth_.fill_order(cb.order->price(), 
                          cb.order->open_qty(),
                          cb.fill_qty,
                          cb.order->is_buy());
      }
      // Increment fill ID once
      ++fill_id_;
      // Update the orders
      book::Cost fill_cost = cb.fill_qty * cb.fill_price;
      cb.matched_order->fill(cb.fill_qty, fill_cost, fill_id_);
      cb.order->fill(cb.fill_qty, fill_cost, fill_id_);
      break;
    }
    case DobCallback::cb_order_cancel:
      // If the order is a limit order
      if (cb.order->is_limit()) {
        // If the close erases a level
        depth_.close_order(cb.order->price(), 
                           cb.order->open_qty(), 
                           cb.order->is_buy());
      }
      cb.order->cancel();
      break;

    case DobCallback::cb_order_replace:
    {
      // Remember current values
      book::Price current_price = cb.order->price();
      book::Quantity current_qty = cb.order->open_qty();

      // Modify the order itself
      cb.order->replace(cb.new_order_qty, cb.new_price);

      // Notify the depth
      depth_.replace_order(current_price, cb.new_price, 
                           current_qty, cb.order->open_qty(),
                           cb.order->is_buy());
      break;
    }
    default:
      // Nothing
      break;
  }
}

template <class OrderPtr, int SIZE>
inline typename DepthOrderBook<OrderPtr, SIZE>::DepthTracker&
DepthOrderBook<OrderPtr, SIZE>::depth()
{
  return depth_;
}

template <class OrderPtr, int SIZE>
inline const typename DepthOrderBook<OrderPtr, SIZE>::DepthTracker&
DepthOrderBook<OrderPtr, SIZE>::depth() const
{
  return depth_;
}

} }

#endif
