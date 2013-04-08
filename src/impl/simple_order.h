// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#ifndef simpleorder_h
#define simpleorder_h

#include "book/order.h"
#include "book/types.h"

namespace liquibook { namespace impl {

enum OrderState {
  os_new,
  os_accepted,
  os_complete,
  os_cancelled,
  os_rejected
};

/// @brief impelementation of the Order interface for testing purposes.
class SimpleOrder : public book::Order {
public:
  SimpleOrder(bool is_buy,
              book::Price price,
              book::Quantity qty);

  /// @brief get the order's state
  const OrderState& state() const;

  /// @brief is this order a buy?
  virtual bool is_buy() const;

  /// @brief get the limit price of this order
  virtual book::Price price() const;

  /// @brief get the quantity of this order
  virtual book::Quantity order_qty() const;

  /// @brief get the open quantity of this order
  virtual book::Quantity open_qty() const;

  /// @brief get the filled quantity of this order
  virtual const book::Quantity& filled_qty() const;

  /// @brief get the total filled cost of this order
  const book::Cost& filled_cost() const;

  /// @brief notify of a fill of this order
  /// @param fill_qty the number of shares in this fill
  /// @param fill_cost the total amount of this fill
  /// @fill_id the unique identifier of this fill
  virtual void fill(book::Quantity fill_qty, 
                    book::Cost fill_cost,
                    book::FillId fill_id);

  /// @brief exchange accepted this order
  void accept();
  /// @brief exchange cancelled this order
  void cancel();

  /// @brief exchange replaced this order
  /// @param size_delta change to the order quantity
  /// @param new_price the new price
  void replace(book::Quantity size_delta, book::Price new_price);

private:
  OrderState state_;
  bool is_buy_;
  book::Price    price_;
  book::Quantity order_qty_;
  book::Quantity filled_qty_;
  book::Cost filled_cost_;
  static uint32_t last_order_id_;

public:
  const uint32_t order_id_;
};

} }

#endif
