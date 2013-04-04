// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include "assertiv/assertiv.h"
#include "ut_utils.h"
#include "changed_checker.h"
#include "book/order_book.h"
#include "impl/simple_order.h"

namespace liquibook {

using book::OrderBook;
using impl::SimpleOrder;

typedef SimpleOrder* OrderPtr;

class OrderCbListener : public OrderListener<OrderPtr>
{
public:
  virtual void on_accept(const OrderPtr& order)
  {
    accepts_.push_back(order);
  }
  virtual void on_reject(const OrderPtr& order, const char* reason)
  {
    rejects_.push_back(order);
  }
  virtual void on_fill(const OrderPtr& order, 
                       const OrderPtr& matched_order, 
                       Quantity fill_qty, 
                       Cost fill_cost)
  {
    fills_.push_back(order);
  }
  virtual void on_cancel(const OrderPtr& order)
  {
    cancels_.push_back(order);
  }
  virtual void on_cancel_reject(const OrderPtr& order, const char* reason)
  {
    cancel_rejects_.push_back(order);
  }
  virtual void on_replace(const OrderPtr& order,
                          Quantity new_qty, 
                          Price new_price)
  {
    replaces_.push_back(order);
  }
  virtual void on_replace_reject(const OrderPtr& order, const char* reason)
  {
    replace_rejects_.push_back(order);
  }

  void reset()
  {
    accepts_.erase(accepts_.begin(), accepts_.end());
    rejects_.erase(rejects_.begin(), rejects_.end());
    fills_.erase(fills_.begin(), fills_.end());
    cancels_.erase(cancels_.begin(), cancels_.end());
    cancel_rejects_.erase(cancel_rejects_.begin(), cancel_rejects_.end());
    replaces_.erase(replaces_.begin(), replaces_.end());
    replace_rejects_.erase(replace_rejects_.begin(), replace_rejects_.end());
  }
public:
  typedef std::vector<const SimpleOrder*> OrderVector;
  OrderVector accepts_;
  OrderVector rejects_;
  OrderVector fills_;
  OrderVector cancels_;
  OrderVector cancel_rejects_;
  OrderVector replaces_;
  OrderVector replace_rejects_;
};

TEST(TestOrderCallbacks)
{
  SimpleOrder order0(false, 3250, 100);
  SimpleOrder order1(true,  3250, 800);
  SimpleOrder order2(false, 3230, 0);
  SimpleOrder order3(false, 3240, 200);
  SimpleOrder order4(true,  3250, 600);

  OrderCbListener listener;
  OrderBook<OrderPtr> order_book;
  order_book.set_order_listener(&listener);
  // Add order, should be accepted
  order_book.add(&order0);
  order_book.perform_callbacks();
  ASSERT_EQ(1, listener.accepts_.size());
  listener.reset();
  // Add matching order, should be accepted, followed by a fill
  order_book.add(&order1);
  order_book.perform_callbacks();
  ASSERT_EQ(1, listener.accepts_.size());
  ASSERT_EQ(1, listener.fills_.size());
  listener.reset();
  // Add invalid order, should be rejected
  order_book.add(&order2);
  order_book.perform_callbacks();
  ASSERT_EQ(1, listener.rejects_.size());
  listener.reset();
  // Cancel only valid order, should be cancelled
  order_book.cancel(&order1);
  order_book.perform_callbacks();
  ASSERT_EQ(1, listener.cancels_.size());
  listener.reset();
  // Cancel filled order, should be rejected
  order_book.cancel(&order0);
  order_book.perform_callbacks();
  ASSERT_EQ(1, listener.cancel_rejects_.size());
  listener.reset();
  // Add a new order and replace it, should be replaced
  order_book.add(&order3);
  order_book.replace(&order3, 0, 3250);
  order_book.perform_callbacks();
  ASSERT_EQ(1, listener.accepts_.size());
  ASSERT_EQ(1, listener.replaces_.size());
  listener.reset();
  // Add matching order, should be accepted, followed by a fill
  order_book.add(&order4);
  order_book.perform_callbacks();
  ASSERT_EQ(1, listener.accepts_.size());
  ASSERT_EQ(1, listener.fills_.size());
  listener.reset();
  // Replace matched order, with too large of a size decrease, replace
  // should be rejected
  order_book.replace(&order3, -500);
  order_book.perform_callbacks();
  ASSERT_EQ(0, listener.replaces_.size());
  ASSERT_EQ(1, listener.replace_rejects_.size());
}

} // namespace liquibook
