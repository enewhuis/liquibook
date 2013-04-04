// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#ifndef bbo_listener_h
#define bbo_listener_h

namespace liquibook { namespace book {

/// @brief generic listener of top-of-book events
template <class OrderBook, class DepthTracker >
class BboListener {
public:
  /// @brief callback for top of book change
  virtual void on_bbo_change(const OrderBook& book, 
                             const DepthTracker& depth) = 0;
};

} }

#endif
