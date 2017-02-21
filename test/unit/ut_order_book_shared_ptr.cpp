// Copyright (c) 2012 - 2017 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.

#define BOOST_TEST_NO_MAIN LiquibookTest
#include <boost/test/unit_test.hpp>

#include "ut_utils.h"
#include "changed_checker.h"
#include <book/order_book.h>
#include <simple/simple_order.h>
#include <simple/simple_order_book.h>
#include <memory>

namespace liquibook {

using book::DepthLevel;
using book::OrderBook;
using book::OrderTracker;
using simple::SimpleOrder;


typedef std::shared_ptr<SimpleOrder> SimpleOrderPtr;
class SharedPtrOrderBook : public OrderBook<SimpleOrderPtr>
{
  virtual void perform_callback(OrderBook<SimpleOrderPtr>::TypedCallback& cb)
  {
    switch(cb.type) {
      case TypedCallback::cb_order_accept:
        cb.order->accept();
        break;
      case TypedCallback::cb_order_fill: {
        Cost fill_cost = cb.price * cb.quantity;
        cb.order->fill(cb.quantity, fill_cost, 0);
        cb.matched_order->fill(cb.quantity, fill_cost, 0);
        break;
      }
      case TypedCallback::cb_order_cancel:
        cb.order->cancel();
        break;
      case TypedCallback::cb_order_replace:
        cb.order->replace(cb.delta, cb.price);
        break;
      default:
        // Nothing
        break;
    }
  }
};

typedef FillCheck<SimpleOrderPtr> SharedFillCheck;

BOOST_AUTO_TEST_CASE(TestSharedPointerBuild)
{
  SharedPtrOrderBook order_book;
  SimpleOrderPtr ask1(new SimpleOrder(false, 1252, 100));
  SimpleOrderPtr ask0(new SimpleOrder(false, 1251, 100));
  SimpleOrderPtr bid1(new SimpleOrder(true,  1251, 100));
  SimpleOrderPtr bid0(new SimpleOrder(true,  1250, 100));

  // No match
  BOOST_CHECK(add_and_verify(order_book, bid0, false));
  BOOST_CHECK(add_and_verify(order_book, ask0, false));
  BOOST_CHECK(add_and_verify(order_book, ask1, false));

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());

  // Match - complete
  {
    SharedFillCheck fc1(bid1, 100, 125100);
    SharedFillCheck fc2(ask0, 100, 125100);
    BOOST_CHECK(add_and_verify(order_book, bid1, true, true));
  }

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(1, order_book.asks().size());
}

BOOST_AUTO_TEST_CASE(TestSharedCancelBid)
{
  SharedPtrOrderBook order_book;
  SimpleOrderPtr ask1(new SimpleOrder(false, 1252, 100));
  SimpleOrderPtr ask0(new SimpleOrder(false, 1251, 100));
  SimpleOrderPtr bid0(new SimpleOrder(true,  1250, 100));

  // No match
  BOOST_CHECK(add_and_verify(order_book, bid0, false));
  BOOST_CHECK(add_and_verify(order_book, ask0, false));
  BOOST_CHECK(add_and_verify(order_book, ask1, false));

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());

  // Cancel bid
  BOOST_CHECK(cancel_and_verify(order_book, bid0, simple::os_cancelled));

  // Verify sizes
  BOOST_CHECK_EQUAL(0, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());
}

} // namespace
