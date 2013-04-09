// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#ifndef depth_listener_h
#define depth_listener_h

#include "order_book.h"

namespace liquibook { namespace book {

/// @brief listener of depth events
template <class OrderBook>
class DepthListener {
public:
  /// @brief callback for change in tracked aggregated depth
  virtual void on_depth_change(
      const OrderBook* book,
      const typename OrderBook::DepthTracker* depth) = 0;
};

} }

#endif
