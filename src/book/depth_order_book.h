// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#ifndef depth_order_book_h
#define depth_order_book_h

#include "order_book.h"
#include "depth.h"
#include "bbo_listener.h"
#include "depth_listener.h"

#include <iostream>

namespace liquibook { namespace book {

/// @brief Implementation of order book child class, that incorporates
///        aggregate depth tracking.  Overrides perform_callback() method to 
//         track depth aggregated by price.
template <class OrderPtr = Order*, int SIZE = 5>
class DepthOrderBook : public OrderBook<OrderPtr> {
public:
  typedef Depth<SIZE> DepthTracker;
  typedef BboListener<DepthOrderBook >TypedBboListener;
  typedef DepthListener<DepthOrderBook >TypedDepthListener;
  typedef Callback<OrderPtr> DobCallback;

  /// @brief construct
  DepthOrderBook();

  /// @brief set the BBO listener
  void set_bbo_listener(TypedBboListener* bbo_listener);

  /// @brief set the depth listener
  void set_depth_listener(TypedDepthListener* depth_listener);

  /// @brief handle a single callback
  virtual void perform_callback(DobCallback& cb);

  // @brief access the depth tracker
  DepthTracker& depth();

  // @brief access the depth tracker
  const DepthTracker& depth() const;

private:
  DepthTracker depth_;
  TypedBboListener* bbo_listener_;
  TypedDepthListener* depth_listener_;
};

template <class OrderPtr, int SIZE>
DepthOrderBook<OrderPtr, SIZE>::DepthOrderBook()
: bbo_listener_(NULL),
  depth_listener_(NULL)
{
}

template <class OrderPtr, int SIZE>
void
DepthOrderBook<OrderPtr, SIZE>::set_bbo_listener(TypedBboListener* listener)
{
  bbo_listener_ = listener;
}

template <class OrderPtr, int SIZE>
void
DepthOrderBook<OrderPtr, SIZE>::set_depth_listener(TypedDepthListener* listener)
{
  depth_listener_ = listener;
}

template <class OrderPtr, int SIZE>
inline void
DepthOrderBook<OrderPtr, SIZE>::perform_callback(DobCallback& cb)
{
  OrderBook<OrderPtr>::perform_callback(cb);
  switch(cb.type) {
    case DobCallback::cb_order_accept:
      // If the order is a limit order
      if (cb.order->is_limit()) {
        // If the order is completely filled on acceptance, do not modify 
        // depth unnecessarily
        if (cb.accept_match_qty == cb.order->order_qty()) {
          // Don't tell depth about this order - it's going away immediately.
          // Instead tell Depth about future fills to ignore
          depth_.ignore_fill_qty(cb.accept_match_qty, cb.order->is_buy());
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
        bool matched_order_filled = 
                 (cb.fill_flags & DobCallback::ff_matched_filled) != 0;
        // Inform the depth
        depth_.fill_order(cb.matched_order->price(), 
                          cb.fill_qty,
                          matched_order_filled,
                          cb.matched_order->is_buy());
      }
      // If the inbound order is a limit order
      if (cb.order->is_limit()) {
        bool inbound_order_filled = 
                 cb.fill_flags & DobCallback::ff_inbound_filled;
        // Inform the depth
        depth_.fill_order(cb.order->price(), 
                          cb.fill_qty,
                          inbound_order_filled,
                          cb.order->is_buy());
      }
      break;
    }
    case DobCallback::cb_order_cancel:
      // If the order is a limit order
      if (cb.order->is_limit()) {
        // If the close erases a level
        depth_.close_order(cb.order->price(), 
                           cb.cxl_open_qty, 
                           cb.order->is_buy());
      }
      break;

    case DobCallback::cb_order_replace:
    {
      // Remember current values
      Quantity current_qty = cb.repl_curr_open_qty;
      Quantity new_qty = current_qty + cb.repl_size_delta;

      // Notify the depth
      depth_.replace_order(cb.order->price(), cb.repl_new_price, 
                           current_qty, new_qty, cb.order->is_buy());
      break;
    }
    case DobCallback::cb_book_update:
      // Book was updated, see if the depth we track was effected
      if (depth_.changed()) {
        if (depth_listener_) {
          depth_listener_->on_depth_change(this, &depth_);
        }
        if (bbo_listener_) {
          ChangeId last_change = depth_.last_published_change();
          // May have been the first level which changed
          if ((depth_.bids()->changed_since(last_change)) ||
              (depth_.asks()->changed_since(last_change))) {
            bbo_listener_->on_bbo_change(this, &depth_);
          }
        }
        // Start tracking changes again...
        depth_.published();
      }
      break;

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
