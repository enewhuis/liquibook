// Copyright (c) 2012=2017 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <book/order_book.h>
#include <simple/simple_order_book.h>
#include <simple/simple_order.h>

using namespace liquibook::book;

namespace liquibook {

template<typename SimpleOrderBook>
class DepthCheck {
  typedef typename SimpleOrderBook::DepthTracker SimpleDepth;

public:
  DepthCheck(const SimpleDepth& depth) 
  : depth_(depth)
  {
    reset();
  }

  static bool verify_depth(const DepthLevel& level,
    const Price& price,
    uint32_t count,
    const Quantity& qty)
  {
    bool matched = true;
    if (level.price() != price) {
      std::cout << "Price " << level.price() << " expecting " << price << std::endl;
      matched = false;
    }
    if (level.order_count() != count) {
      std::cout << "Level: " << level.price() << " Count " << level.order_count() << " expecting " << count << std::endl;
      matched = false;
    }
    if (level.aggregate_qty() != qty) {
      std::cout << "Level: " << level.price() << " Quantity " << level.aggregate_qty() << " expecting " << qty << std::endl;
      matched = false;
    }
    if (level.is_excess()) {
      std::cout << "Marked as excess" << std::endl;
      matched = false;
    }
    return matched;
  }

  bool verify_bid(const Price& price, int count, const Quantity& qty)
  {
    return verify_depth(*next_bid_++, price, count, qty);
  }

  bool verify_ask(const Price& price, int count, const Quantity& qty)
  {
    return verify_depth(*next_ask_++, price, count, qty);
  }

  bool verify_bids_done()
  {
    while(next_bid_ != depth_.last_bid_level() + 1)
    {
      auto level = *next_bid_;
      if(level.order_count() != 0)
      {
        return false;
        }
      ++next_bid_;
    }
    return true;
  }

  bool verify_adds_done()
  {
    while(next_ask_ != depth_.last_ask_level() + 1)
    {
      auto level = *next_ask_;
      if(level.order_count() != 0)
      {
        return false;
      }
      ++next_bid_;
    }
    return true;
  }

  void reset()
  {
    next_bid_ = depth_.bids();
    next_ask_ = depth_.asks();
  }

private:
  const SimpleDepth& depth_;
  const DepthLevel* next_bid_;
  const DepthLevel* next_ask_;
};

} // namespace
