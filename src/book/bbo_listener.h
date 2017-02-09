// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

namespace liquibook { namespace book {

/// @brief generic listener of top-of-book events
template <class OrderBook>
class BboListener {
public:
  /// @brief callback for top of book change
  virtual void on_bbo_change(
      const OrderBook* book, 
      const typename OrderBook::DepthTracker* depth) = 0;
};

} }
