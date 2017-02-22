// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include "order_book.h"
#include "depth.h"
#include "bbo_listener.h"
#include "depth_listener.h"

namespace liquibook { namespace book {

/// @brief Implementation of order book child class, that incorporates
///        aggregate depth tracking.  
template <typename OrderPtr, int SIZE = 5>
class DepthOrderBook : public OrderBook<OrderPtr> {
public:
  typedef Depth<SIZE> DepthTracker;
  typedef BboListener<DepthOrderBook >TypedBboListener;
  typedef DepthListener<DepthOrderBook >TypedDepthListener;

  /// @brief construct
  DepthOrderBook(const std::string & symbol = "unknown");

  /// @brief set the BBO listener
  void set_bbo_listener(TypedBboListener* bbo_listener);

  /// @brief set the depth listener
  void set_depth_listener(TypedDepthListener* depth_listener);

  // @brief access the depth tracker
  DepthTracker& depth();

  // @brief access the depth tracker
  const DepthTracker& depth() const;

  protected:
  //////////////////////////////////
  // Implement virtual callback methods
  // needed to maintain depth book.
  virtual void on_accept(const OrderPtr& order, Quantity quantity);

  virtual void on_fill(const OrderPtr& order, 
    const OrderPtr& matched_order, 
    Quantity fill_qty, 
    Cost fill_cost,
    bool inbound_order_filled,
    bool matched_order_filled);

  virtual void on_cancel(const OrderPtr& order, Quantity quantity);

  virtual void on_replace(const OrderPtr& order,
    Quantity current_qty, 
    Quantity new_qty,
    Price new_price);

  virtual void on_order_book_change();

private:
  DepthTracker depth_;
  TypedBboListener* bbo_listener_;
  TypedDepthListener* depth_listener_;
};

template <class OrderPtr, int SIZE>
DepthOrderBook<OrderPtr, SIZE>::DepthOrderBook(const std::string & symbol)
: OrderBook<OrderPtr>(symbol),
  bbo_listener_(nullptr),
  depth_listener_(nullptr)
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
void 
DepthOrderBook<OrderPtr, SIZE>::on_accept(const OrderPtr& order, Quantity quantity)
{
  // If the order is a limit order
  if (order->is_limit()) 
  {
    // If the order is completely filled on acceptance, do not modify 
    // depth unnecessarily
    if (quantity == order->order_qty()) 
    {
      // Don't tell depth about this order - it's going away immediately.
      // Instead tell Depth about future fills to ignore
      depth_.ignore_fill_qty(quantity, order->is_buy());
    } 
    else 
    {
      // Add to bid or ask depth
      depth_.add_order(order->price(), 
        order->order_qty(), 
        order->is_buy());
    }
  }
}

template <class OrderPtr, int SIZE> 
void 
DepthOrderBook<OrderPtr, SIZE>::on_fill(const OrderPtr& order, 
  const OrderPtr& matched_order, 
  Quantity quantity, 
  Cost fill_cost,
  bool inbound_order_filled,
  bool matched_order_filled)
{
  // If the matched order is a limit order
  if (matched_order->is_limit()) {
    // Inform the depth
    depth_.fill_order(matched_order->price(), 
      quantity,
      matched_order_filled,
      matched_order->is_buy());
  }
  // If the inbound order is a limit order
  if (order->is_limit()) {
    // Inform the depth
    depth_.fill_order(order->price(), 
      quantity,
      inbound_order_filled,
      order->is_buy());
  }
}

template <class OrderPtr, int SIZE> 
void 
DepthOrderBook<OrderPtr, SIZE>::on_cancel(const OrderPtr& order, Quantity quantity)
{
  // If the order is a limit order
  if (order->is_limit()) {
    // If the close erases a level
    depth_.close_order(order->price(), 
      quantity, 
      order->is_buy());
  }
}

template <class OrderPtr, int SIZE> 
void 
DepthOrderBook<OrderPtr, SIZE>::on_replace(const OrderPtr& order,
  Quantity current_qty, 
  Quantity new_qty,
  Price new_price)
{
  // Notify the depth
  depth_.replace_order(order->price(), new_price, 
    current_qty, new_qty, order->is_buy());
}

template <class OrderPtr, int SIZE> 
void 
DepthOrderBook<OrderPtr, SIZE>::on_order_book_change()
{
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
