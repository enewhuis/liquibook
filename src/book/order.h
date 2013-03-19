// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#ifndef order_h
#define order_h

#include "types.h"

namespace liquibook { namespace book {

/// @brief generic listener of order events.  Used by common version of 
///   OrderBook::process_callback().
class Order {
public:
  /// @brief is this a limit order?
  bool is_limit() const;

  /// @brief is this order a buy?
  virtual bool is_buy() const = 0;

  /// @brief get the price of this order, or 0 if a market order
  virtual Price price() const = 0;

  /// @brief get the quantity of this order
  virtual Quantity order_qty() const = 0;
};

} }

#endif
