// Copyright (c) 2012 - 2017 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.

#define BOOST_TEST_NO_MAIN LiquibookTest
#include <boost/test/unit_test.hpp>

#include "ut_utils.h"
#include "changed_checker.h"
#include <book/order_book.h>
#include <simple/simple_order.h>

namespace liquibook {

using book::DepthLevel;
using book::OrderBook;
using book::OrderTracker;
using simple::SimpleOrder;

namespace
{
  const bool sideBuy = true;
  const bool sideSell = false;

  const Price prcMkt = 0;
  const Price prc53 = 53;
  const Price prc54 = 54;
  const Price prc55 = 55;
  const Price prc56 = 56;
  const Price prc57 = 57;

  const Quantity q100 = 100;
  const Quantity q1000 = 1000;

  const bool expectMatch = true;
  const bool expectNoMatch = false;

  const bool expectComplete = true;
  const bool expectNoComplete = false;
}

typedef OrderTracker<SimpleOrder*> SimpleTracker;
typedef test::ChangedChecker<5> ChangedChecker;

typedef FillCheck<SimpleOrder*> SimpleFillCheck;

BOOST_AUTO_TEST_CASE(TestStopOrdersOffMarketNoTrade)
{
  SimpleOrderBook book;
  SimpleOrder order0(sideBuy, prc55, q100);
  SimpleOrder order1(sideSell, prcMkt, q100);

  // Enter order to generate a trade establishing market price
  BOOST_CHECK(add_and_verify(book, &order0, expectNoMatch));
  BOOST_CHECK(add_and_verify(book, &order1, expectMatch, expectComplete));

  BOOST_CHECK_EQUAL(prc55, book.market_price());

  SimpleOrder order2(sideBuy, prcMkt, q100, prc56);
  SimpleOrder order3(sideSell, prcMkt, q100, prc54);
  BOOST_CHECK(add_and_verify(book, &order2, expectNoMatch));
  BOOST_CHECK(add_and_verify(book, &order3, expectNoMatch));
  
  // Orders were accepted, but not traded
  BOOST_CHECK_EQUAL(simple::os_accepted, order2.state());
  BOOST_CHECK_EQUAL(simple::os_accepted, order3.state());
}

BOOST_AUTO_TEST_CASE(TestStopMarketOrdersOnMarketTradeImmediately)
{
  SimpleOrderBook book;
  SimpleOrder order0(sideBuy, prc55, q100);
  SimpleOrder order1(sideSell, prcMkt, q100);

  // Enter order to generate a trade establishing market price
  BOOST_CHECK(add_and_verify(book, &order0, expectNoMatch));
  BOOST_CHECK(add_and_verify(book, &order1, expectMatch, expectComplete));

  BOOST_CHECK_EQUAL(prc55, book.market_price());

  SimpleOrder order2(sideBuy, prcMkt, q100, prc55);
  SimpleOrder order3(sideSell, prcMkt, q100, prc55);
  BOOST_CHECK(add_and_verify(book, &order2, expectNoMatch));
  BOOST_CHECK(add_and_verify(book, &order3, expectMatch, expectComplete));
}

BOOST_AUTO_TEST_CASE(TestStopMarketOrdersTradeWhenStopPriceReached)
{
  SimpleOrderBook book;
  SimpleOrder order0(sideBuy, prc53, q100);
  SimpleOrder order1(sideSell, prc57, q100);
  book.set_market_price(prc55);

  // Enter seed orders and be sure they don't trade with each other.
  BOOST_CHECK(add_and_verify(book, &order0, expectNoMatch));
  BOOST_CHECK(add_and_verify(book, &order1, expectNoMatch));

  // enter stop orders.  Be sure they don't trade yet
  SimpleOrder order2(sideBuy, prcMkt, q100, prc56);
  SimpleOrder order3(sideSell, prcMkt, q100, prc54);
  BOOST_CHECK(add_and_verify(book, &order2, expectNoMatch));
  BOOST_CHECK(add_and_verify(book, &order3, expectNoMatch));

  SimpleOrder order4(sideBuy, prc56, q1000, 0, book::oc_all_or_none);
  SimpleOrder order5(sideSell, prc56, q1000, 0, book::oc_all_or_none);

  // Scope for fill checks
  {
    SimpleFillCheck fc0(&order0, 0, 0);
    SimpleFillCheck fc1(&order1, q100, q100 * prc57);
    SimpleFillCheck fc2(&order2, q100, q100 * prc57);
    // Trade at 56 which should trigger order2 which should trade with order 1 at order 1's price
    BOOST_CHECK(add_and_verify(book, &order4, expectNoMatch, expectNoComplete, book::oc_all_or_none));
    BOOST_CHECK(add_and_verify(book, &order5, expectMatch, expectComplete, book::oc_all_or_none));
  }
  BOOST_CHECK_EQUAL(prc57, book.market_price());

  SimpleOrder order6(sideBuy, prc54, q1000, 0, book::oc_all_or_none);
  SimpleOrder order7(sideSell, prc54, q1000, 0, book::oc_all_or_none);

  // Scope for fill checks
  {
    SimpleFillCheck fc0(&order0, q100, q100 * prc53);
    SimpleFillCheck fc3(&order3, q100, q100 * prc53);
    // Trade at 54 which should trigger order3 which should trade with order 0 at order 0's price
    BOOST_CHECK(add_and_verify(book, &order6, expectNoMatch, expectNoComplete, book::oc_all_or_none));
    BOOST_CHECK(add_and_verify(book, &order7, expectMatch, expectComplete, book::oc_all_or_none));
  }
  BOOST_CHECK_EQUAL(prc53, book.market_price());
}

BOOST_AUTO_TEST_CASE(TestStopOrdersCancelBeforeTrigger)
{
  SimpleOrderBook book;
  book.set_market_price(prc55);

  SimpleOrder bid(sideBuy, prcMkt, q100, prc56);
  SimpleOrder ask(sideSell, prcMkt, q100, prc54);
  BOOST_CHECK(add_and_verify(book, &bid, expectNoMatch));
  BOOST_CHECK(add_and_verify(book, &ask, expectNoMatch));
  
  // Orders were accepted, but not traded
  BOOST_CHECK_EQUAL(simple::os_accepted, bid.state());
  BOOST_CHECK_EQUAL(simple::os_accepted, ask.state());
  // Cancel bid and ask
  BOOST_CHECK(cancel_and_verify(book, &bid, simple::os_cancelled));
  BOOST_CHECK(cancel_and_verify(book, &ask, simple::os_cancelled));
}

} // namespace
