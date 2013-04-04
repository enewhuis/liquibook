// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#ifndef order_book_listener_h
#define order_book_listener_h

#include "order_book.h"

namespace liquibook { namespace book {

/// @brief generic listener of order book events
template <class OrderPtr = Order*>
class OrderBookListener {
public:
  typedef OrderBook<OrderPtr> TypedOrderBook;

  /// @brief callback for change anywhere in order book
  virtual void on_order_book_change(const TypedOrderBook& book) = 0;
};

} }

#endif
