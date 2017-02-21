// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include "types.h"

namespace liquibook { namespace book {

template <class OrderPtr>
class OrderBook;

// Callback events
//   New order accept
//     - order accept
//     - fill (2) and/or quote (if not complete)
//     - depth/bbo ?
//   New order reject
//     - order reject
//   Order fill
//     - fill (2)
//     - trade
//     - quote (2)
//     - depth/bbo ?
//   Order cancel
//     - order cancel
//     - quote
//     - depth/bbo ?
//   Order cancel reject
//     - order cancel reject
//   Order replace
//     - order replace
//     - fill (2) and/or quote (if not complete)
//     - depth/bbo ?
//   Order replace reject
//     - order replace reject

/// @brief notification from OrderBook of an event
template <class OrderPtr = Order*>
class Callback {
public:
  typedef OrderBook<OrderPtr > TypedOrderBook;

  enum CbType {
    cb_unknown,
    cb_order_accept,
    cb_order_reject,
    cb_order_fill,
    cb_order_cancel,
    cb_order_cancel_reject,
    cb_order_replace,
    cb_order_replace_reject,
    cb_book_update
  };

  enum FillFlags {
    ff_neither_filled = 0,
    ff_inbound_filled = 1,
    ff_matched_filled = 2,
    ff_both_filled    = 4
  };

  Callback();

  /// @brief create a new accept callback
  static Callback<OrderPtr> accept(const OrderPtr& order,
                                   const TransId& trans_id);
  /// @brief create a new reject callback
  static Callback<OrderPtr> reject(const OrderPtr& order,
                                   const char* reason,
                                   const TransId& trans_id);
  /// @brief create a new fill callback
  static Callback<OrderPtr> fill(const OrderPtr& inbound_order,
                                 const OrderPtr& matched_order,
                                 const Quantity& fill_qty,
                                 const Price& fill_price,
                                 FillFlags fill_flags,
                                 const TransId& trans_id);
  /// @brief create a new cancel callback
  static Callback<OrderPtr> cancel(const OrderPtr& order,
                                   const Quantity& open_qty,
                                   const TransId& trans_id);
  /// @brief create a new cancel reject callback
  static Callback<OrderPtr> cancel_reject(const OrderPtr& order,
                                          const char* reason,
                                          const TransId& trans_id);
  /// @brief create a new replace callback
  static Callback<OrderPtr> replace(const OrderPtr& order,
                                    const Quantity& curr_open_qty,
                                    const int32_t& size_delta,
                                    const Price& new_price,
                                    const TransId& trans_id);
  /// @brief create a new replace reject callback
  static Callback<OrderPtr> replace_reject(const OrderPtr& order,
                                           const char* reason,
                                           const TransId& trans_id);

  static Callback<OrderPtr> book_update(const TypedOrderBook* book,
                                        const TransId& trans_id);
  CbType type;
  OrderPtr order;
  const TypedOrderBook* order_book;  // In case the cb is handled externally
  OrderPtr matched_order; // fill
  TransId trans_id;
  union {
    struct { // Accept
      Quantity accept_match_qty;
    };
    struct { // Fill
      Quantity fill_qty;
      Price fill_price;
      uint8_t fill_flags;
    };
    struct { // Cancel
      Quantity cxl_open_qty;
    };
    struct { // Replace
      Quantity repl_curr_open_qty; // Open qty of order before replace
      int32_t repl_size_delta;
      Price repl_new_price;
    };
    const char* reject_reason;
  };
};

template <class OrderPtr>
Callback<OrderPtr>::Callback()
: type(cb_unknown),
  order_book(0),
  trans_id(0),
  fill_qty(0),
  fill_price(0)
{
}

template <class OrderPtr>
Callback<OrderPtr> Callback<OrderPtr>::accept(
  const OrderPtr& order,
  const TransId& trans_id)
{
  Callback<OrderPtr> result;
  result.type = cb_order_accept;
  result.order = order;
  result.trans_id = trans_id;
  return result;
}

template <class OrderPtr>
Callback<OrderPtr> Callback<OrderPtr>::reject(
  const OrderPtr& order,
  const char* reason,
  const TransId& trans_id)
{
  Callback<OrderPtr> result;
  result.type = cb_order_reject;
  result.order = order;
  result.reject_reason = reason;
  result.trans_id = trans_id;
  return result;
}

template <class OrderPtr>
Callback<OrderPtr> Callback<OrderPtr>::fill(
  const OrderPtr& inbound_order,
  const OrderPtr& matched_order,
  const Quantity& fill_qty,
  const Price& fill_price,
  FillFlags fill_flags,
  const TransId& trans_id)
{
  Callback<OrderPtr> result;
  result.type = cb_order_fill;
  result.order = inbound_order;
  result.matched_order = matched_order;
  result.fill_qty = fill_qty;
  result.fill_price = fill_price;
  result.fill_flags = fill_flags;
  result.trans_id = trans_id;
  return result;
}

template <class OrderPtr>
Callback<OrderPtr> Callback<OrderPtr>::cancel(
  const OrderPtr& order,
  const Quantity& open_qty,
  const TransId& trans_id)
{
  // TODO save the open qty
  Callback<OrderPtr> result;
  result.type = cb_order_cancel;
  result.order = order;
  result.cxl_open_qty = open_qty;
  result.trans_id = trans_id;
  return result;
}

template <class OrderPtr>
Callback<OrderPtr> Callback<OrderPtr>::cancel_reject(
  const OrderPtr& order,
  const char* reason,
  const TransId& trans_id)
{
  Callback<OrderPtr> result;
  result.type = cb_order_cancel_reject;
  result.order = order;
  result.reject_reason = reason;
  result.trans_id = trans_id;
  return result;
}

template <class OrderPtr>
Callback<OrderPtr> Callback<OrderPtr>::replace(
  const OrderPtr& order,
  const Quantity& curr_open_qty,
  const int32_t& size_delta,
  const Price& new_price,
  const TransId& trans_id)
{
  // TODO save the order open qty
  Callback<OrderPtr> result;
  result.type = cb_order_replace;
  result.order = order;
  result.repl_curr_open_qty = curr_open_qty;
  result.repl_size_delta = size_delta;
  result.repl_new_price = new_price;
  result.trans_id = trans_id;
  return result;
}

template <class OrderPtr>
Callback<OrderPtr> Callback<OrderPtr>::replace_reject(
  const OrderPtr& order,
  const char* reason,
  const TransId& trans_id)
{
  Callback<OrderPtr> result;
  result.type = cb_order_replace_reject;
  result.order = order;
  result.reject_reason = reason;
  result.trans_id = trans_id;
  return result;
}

template <class OrderPtr>
Callback<OrderPtr>
Callback<OrderPtr>::book_update(const OrderBook<OrderPtr>* book,
                                const TransId& trans_id)
{
  Callback<OrderPtr> result;
  result.type = cb_book_update;
  result.order_book = book;
  result.trans_id = trans_id;
  return result;
}

} }
