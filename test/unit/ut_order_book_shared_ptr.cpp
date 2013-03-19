// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include "assertiv/assertiv.h"
#include "ut_utils.h"
#include "changed_checker.h"
#include "book/order_book.h"
#include "impl/simple_order.h"
#include "impl/simple_order_book.h"
#include <boost/shared_ptr.hpp>

namespace liquibook {

using book::DepthLevel;
using book::OrderBook;
using book::OrderTracker;
using impl::SimpleOrder;


typedef boost::shared_ptr<SimpleOrder> SimpleOrderPtr;
class SharedPtrOrderBook : public OrderBook<SimpleOrderPtr>
{
  virtual void perform_callback(OrderBook<SimpleOrderPtr>::TypedCallback& cb)
  {
    switch(cb.type) {
      case TypedCallback::cb_order_accept:
        cb.order->accept();
        break;
      case TypedCallback::cb_order_fill: {
        Cost fill_cost = cb.fill_price * cb.fill_qty;
        cb.order->fill(cb.fill_qty, fill_cost, 0);
        cb.matched_order->fill(cb.fill_qty, fill_cost, 0);
        break;
      }
      case TypedCallback::cb_order_cancel:
        cb.order->cancel();
        break;
      case TypedCallback::cb_order_replace:
        cb.order->replace(cb.new_order_qty, cb.new_price);
        break;
      default:
        // Nothing
        break;
    }
  }
};

typedef FillCheck<SimpleOrderPtr> SharedFillCheck;

TEST(TestSharedPointerBuild)
{
  SharedPtrOrderBook order_book;
  SimpleOrderPtr ask1(new SimpleOrder(false, 1252, 100));
  SimpleOrderPtr ask0(new SimpleOrder(false, 1251, 100));
  SimpleOrderPtr bid1(new SimpleOrder(true,  1251, 100));
  SimpleOrderPtr bid0(new SimpleOrder(true,  1250, 100));

  // No match
  ASSERT_TRUE(add_and_verify(order_book, bid0, false));
  ASSERT_TRUE(add_and_verify(order_book, ask0, false));
  ASSERT_TRUE(add_and_verify(order_book, ask1, false));

  // Verify sizes
  ASSERT_EQ(1, order_book.bids().size());
  ASSERT_EQ(2, order_book.asks().size());

  // Match - complete
  { ASSERT_NO_THROW(
    SharedFillCheck fc1(bid1, 100, 125100);
    SharedFillCheck fc2(ask0, 100, 125100);
    ASSERT_TRUE(add_and_verify(order_book, bid1, true, true));
  ); }

  // Verify sizes
  ASSERT_EQ(1, order_book.bids().size());
  ASSERT_EQ(1, order_book.asks().size());
}

TEST(TestSharedCancelBid)
{
  SharedPtrOrderBook order_book;
  SimpleOrderPtr ask1(new SimpleOrder(false, 1252, 100));
  SimpleOrderPtr ask0(new SimpleOrder(false, 1251, 100));
  SimpleOrderPtr bid0(new SimpleOrder(true,  1250, 100));

  // No match
  ASSERT_TRUE(add_and_verify(order_book, bid0, false));
  ASSERT_TRUE(add_and_verify(order_book, ask0, false));
  ASSERT_TRUE(add_and_verify(order_book, ask1, false));

  // Verify sizes
  ASSERT_EQ(1, order_book.bids().size());
  ASSERT_EQ(2, order_book.asks().size());

  // Cancel bid
  ASSERT_TRUE(cancel_and_verify(order_book, bid0, impl::os_cancelled));

  // Verify sizes
  ASSERT_EQ(0, order_book.bids().size());
  ASSERT_EQ(2, order_book.asks().size());
}

} // namespace
