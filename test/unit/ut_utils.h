// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#ifndef ut_utils_h
#define ut_utils_h

#include "book/order_book.h"
#include "impl/simple_order_book.h"
#include "impl/simple_order.h"

namespace liquibook {

using book::OrderBook;
using book::DepthLevel;

typedef impl::SimpleOrderBook<5> SimpleOrderBook;
typedef SimpleOrderBook::SimpleDepth SimpleDepth;

template <class OrderBook, class OrderPtr>
bool add_and_verify(OrderBook& order_book,
                    const OrderPtr& order,
                    const bool match_expected,
                    const bool complete_expected = false,
                    OrderConditions conditions = 0)
{
  const bool matched = order_book.add(order, conditions);
  if (matched == match_expected) {
    order_book.perform_callbacks();
    if (complete_expected) {
      // State should be complete
      return impl::os_complete == order->state();
    } else if (conditions & oc_immediate_or_cancel) {
      // State should be cancelled
      return impl::os_cancelled == order->state();
    } else {
      // State should be accepted
      return impl::os_accepted == order->state();
    }
  } else {
    return false;
  }
}


template <class OrderBook, class OrderPtr>
bool cancel_and_verify(OrderBook& order_book,
                       const OrderPtr& order,
                       impl::OrderState expected_state)
{
  order_book.cancel(order);
  order_book.perform_callbacks();
  return expected_state == order->state();
}

template <class OrderBook, class OrderPtr>
bool replace_and_verify(OrderBook& order_book,
                        const OrderPtr& order,
                        int32_t size_change,
                        Price new_price = PRICE_UNCHANGED,
                        impl::OrderState expected_state = impl::os_accepted,
                        Quantity match_qty = 0)
{
  // Calculate
  Quantity expected_order_qty = order->order_qty() + size_change;
  Quantity expected_open_qty = order->open_qty() + size_change - match_qty;
  Price expected_price = 
      (new_price == PRICE_UNCHANGED) ? order->price() : new_price;

  // Perform
  order_book.replace(order, size_change, new_price);
  order_book.perform_callbacks();

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

bool verify_depth(const DepthLevel& level,
                  const Price& price,
                  uint32_t count,
                  const Quantity& qty)
{
  bool matched = true;
  if (level.price() != price) {
    std::cout << "Price " << level.price() << std::endl;
    matched = false;
  }
  if (level.order_count() != count) {
    std::cout << "Count " << level.order_count() << std::endl;
    matched = false;
  }
  if (level.aggregate_qty() != qty) {
    std::cout << "Quantity " << level.aggregate_qty() << std::endl;
    matched = false;
  }
  return matched;
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
    if (order_->state() != impl::os_complete && !expected_open_qty_) {
      std::cout << "state " << order_->state() 
                << " expected " << impl::os_complete << std::endl;
      throw std::runtime_error("Unexpected state with no open quantity");
    // Else If the order was not filled
    } else if (expected_open_qty_) {
      bool IOC = ((conditions_ & oc_immediate_or_cancel) != 0);
      if (order_->state() != impl::os_accepted && !IOC) {
        std::cout << "state " << order_->state() 
                  << " expected " << impl::os_accepted << std::endl;
        throw std::runtime_error("Unexpected state with open quantity");
      }
      if (order_->state() != impl::os_cancelled && IOC) {
        std::cout << "state " << order_->state() 
                  << " expected " << impl::os_cancelled << std::endl;
        throw std::runtime_error("Unexpected state for IOC with open quantity");
      }
    } 
  }
};

class DepthCheck {
public:
  DepthCheck(const SimpleDepth& depth) 
  : depth_(depth)
  {
    reset();
  }

  bool verify_bid(const Price& price, int count, const Quantity& qty)
  {
    return verify_depth(*next_bid_++, price, count, qty);
  }

  bool verify_ask(const Price& price, int count, const Quantity& qty)
  {
    return verify_depth(*next_ask_++, price, count, qty);
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

#endif
