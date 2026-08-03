// Copyright (c) 2012 -- 2017 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include "depth_check.h"
#include <book/order_book.h>
#include <simple/simple_order_book.h>
#include <simple/simple_order.h>


using namespace liquibook::book;

namespace liquibook {

typedef simple::SimpleOrderBook<5> SimpleOrderBook;
typedef simple::SimpleOrderBook<5>::DepthTracker SimpleDepth;

template <class OrderBook, class OrderPtr>
bool add_and_verify(OrderBook& order_book,
                    const OrderPtr& order,
                    const bool match_expected,
                    const bool complete_expected = false,
                    OrderConditions conditions = 0)
{
  const bool matched = order_book.add(order, conditions);
  if (matched == match_expected) {
    if (complete_expected) {
      // State should be complete
      return simple::os_complete == order->state();
    } else if (conditions & oc_immediate_or_cancel) {
      // State should be cancelled
      return simple::os_cancelled == order->state();
    } else {
      // State should be accepted
      return simple::os_accepted == order->state();
    }
  } else {
    return false;
  }
}


template <class OrderBook, class OrderPtr>
bool cancel_and_verify(OrderBook& order_book,
                       const OrderPtr& order,
                       simple::OrderState expected_state)
{
  order_book.cancel(order);
  return expected_state == order->state();
}

template <class OrderBook, class OrderPtr>
bool replace_and_verify(OrderBook& order_book,
                        const OrderPtr& order,
                        int32_t size_change,
                        Price new_price = PRICE_UNCHANGED,
                        simple::OrderState expected_state = simple::os_accepted,
                        Quantity match_qty = 0)
{
  // Calculate
  Quantity expected_order_qty = order->order_qty() + size_change;
  Quantity expected_open_qty = order->open_qty() + size_change - match_qty;
  Price expected_price = 
      (new_price == PRICE_UNCHANGED) ? order->price() : new_price;

  // Perform
  order_book.replace(order, size_change, new_price);

  // Verify
  bool correct = true;
  if (expected_state != order->state()) {
    correct = false;
    std::cout << "State " << order->state() << std::endl;
  }
  if (expected_order_qty != order->order_qty()) {
    correct = false;
    std::cout << "Order Qty " << order->order_qty() << std::endl;
  }
  if (expected_open_qty != order->open_qty()) {
    correct = false;
    std::cout << "Open Qty " << order->open_qty() << std::endl;
  }
  if (expected_price != order->price()) {
    correct = false;
    std::cout << "Price " << order->price() << std::endl;
  }
  return correct;
}

template <class OrderPtr>
class FillCheck {
public:
  FillCheck(OrderPtr order, 
            Quantity filled_qty,
            Cost filled_cost,
            OrderConditions conditions = 0)
  : order_(order),
    expected_filled_qty_(order->filled_qty() + filled_qty),
    expected_open_qty_(order->order_qty() - expected_filled_qty_),
    expected_filled_cost_(order->filled_cost() + (filled_cost)),
    conditions_(conditions)
  {
  }

  ~FillCheck() {
    verify_filled();
  }

  private:
  OrderPtr order_;
  Quantity expected_filled_qty_;
  Quantity expected_open_qty_;
  Cost expected_filled_cost_;
  OrderConditions conditions_;

  void verify_filled() {
    if (expected_filled_qty_ !=  order_->filled_qty()) {
      std::cout << "filled_qty " << order_->filled_qty() 
                << " expected " << expected_filled_qty_ << std::endl;
      throw std::runtime_error("Unexpected filled quantity");
    }
    if (expected_open_qty_ !=  order_->open_qty()) {
      std::cout << "open_qty " << order_->open_qty() 
                << " expected " << expected_open_qty_ << std::endl;
      throw std::runtime_error("Unexpected open quantity");
    }
    if (expected_filled_cost_ !=  order_->filled_cost()) {
      std::cout << "filled_cost " << order_->filled_cost() 
                << " expected " << expected_filled_cost_ << std::endl;
      throw std::runtime_error("Unexpected filled cost");
    }
    // If the order was filled, and is not complete
    if (order_->state() != simple::os_complete && !expected_open_qty_) {
      std::cout << "state " << order_->state() 
                << " expected " << simple::os_complete << std::endl;
      throw std::runtime_error("Unexpected state with no open quantity");
    // Else If the order was not filled
    } else if (expected_open_qty_) {
      bool IOC = ((conditions_ & oc_immediate_or_cancel) != 0);
      if (order_->state() != simple::os_accepted && !IOC) {
        std::cout << "state " << order_->state() 
                  << " expected " << simple::os_accepted << std::endl;
        throw std::runtime_error("Unexpected state with open quantity");
      }
      if (order_->state() != simple::os_cancelled && IOC) {
        std::cout << "state " << order_->state() 
                  << " expected " << simple::os_cancelled << std::endl;
        throw std::runtime_error("Unexpected state for IOC with open quantity");
      }
    } 
  }
};


} // namespace
