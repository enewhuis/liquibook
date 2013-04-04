// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#ifndef order_listener_h
#define order_listener_h

#include "order.h"

namespace liquibook { namespace book {

/// @brief interface an order must implement in order to be used by OrderBook.
/// Note: structly speaking, inheriting from Order should not be required, 
///       due to the template implementation of OrderBook.
template <class OrderPtr = Order*>
class OrderListener {
public:
  /// @brief callback for an order accept
  virtual void on_accept(const OrderPtr& order) = 0;

  /// @brief callback for an order reject
  virtual void on_reject(const OrderPtr& order, const char* reason) = 0;

  /// @brief callback for an order fill
  /// @param order the inbound order
  /// @param matched_order the matched order
  /// @param fill_qty the quantity of this fill
  /// @param fill_cost the cost of this fill (qty * price)
  virtual void on_fill(const OrderPtr& order, 
                       const OrderPtr& matched_order, 
                       Quantity fill_qty, 
                       Cost fill_cost) = 0;

  /// @brief callback for an order cancellation
  virtual void on_cancel(const OrderPtr& order) = 0;

  /// @brief callback for an order cancel rejection
  virtual void on_cancel_reject(const OrderPtr& order, const char* reason) = 0;

  /// @brief callback for an order replace
  /// @param order the replaced order
  /// @param new_qty the updated order quantity (NOT open quantity)
  /// @param new_price the updated order price
  virtual void on_replace(const OrderPtr& order,
                          Quantity new_qty, 
                          Price new_price) = 0;

  /// @brief callback for an order replace rejection
  virtual void on_replace_reject(const OrderPtr& order, const char* reason) = 0;
};

} }

#endif
