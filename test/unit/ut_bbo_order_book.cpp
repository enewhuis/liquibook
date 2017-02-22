// Copyright (c) 2012 - 2017 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.

#define BOOST_TEST_NO_MAIN LiquibookTest
#include <boost/test/unit_test.hpp>

#include <book/depth_constants.h>
#include <book/order_book.h>
#include <simple/simple_order.h>
#include <simple/simple_order_book.h>
#include "changed_checker.h"
#include "depth_check.h"
#include "ut_utils.h"

using namespace liquibook::book;

namespace liquibook {

using book::DepthLevel;
using book::OrderBook;
using book::OrderTracker;
using simple::SimpleOrder;

typedef OrderTracker<SimpleOrder*> SimpleTracker;
typedef test::ChangedChecker<5> ChangedChecker;
typedef SimpleOrderBook::DepthTracker SimpleDepth;

typedef FillCheck<SimpleOrder*> SimpleFillCheck;

BOOST_AUTO_TEST_CASE(TestBboBidsMultimapSortCorrect)
{
  SimpleOrderBook::Bids bids;
  SimpleOrder order0(true, 1250, 100);
  SimpleOrder order1(true, 1255, 100);
  SimpleOrder order2(true, 1240, 100);
  SimpleOrder order3(true,    0, 100);
  SimpleOrder order4(true, 1245, 100);

  // Insert out of price order
  bids.insert(std::make_pair(ComparablePrice(true, order0.price()), SimpleTracker(&order0)));
  bids.insert(std::make_pair(ComparablePrice(true, order1.price()), SimpleTracker(&order1)));
  bids.insert(std::make_pair(ComparablePrice(true, order2.price()), SimpleTracker(&order2)));
  bids.insert(std::make_pair(ComparablePrice(true, 0), 
                             SimpleTracker(&order3)));
  bids.insert(std::make_pair(ComparablePrice(true, order4.price()), SimpleTracker(&order4)));
  
  // Should access in price order
  SimpleOrder* expected_order[] = {
    &order3, &order1, &order0, &order4, &order2
  };

  SimpleOrderBook::Bids::iterator bid;
  int index = 0;

  for (bid = bids.begin(); bid != bids.end(); ++bid, ++index) {
    if (expected_order[index]->price() == MARKET_ORDER_PRICE) {
      BOOST_CHECK_EQUAL(MARKET_ORDER_PRICE, bid->first);
    } else {
      BOOST_CHECK_EQUAL(expected_order[index]->price(), bid->first);
    }
    BOOST_CHECK_EQUAL(expected_order[index], bid->second.ptr());
  }

  // Should be able to search and find
  BOOST_CHECK((bids.upper_bound(book::ComparablePrice(true, 1245)))->second.ptr()->price() == 1240);
  BOOST_CHECK((bids.lower_bound(book::ComparablePrice(true, 1245)))->second.ptr()->price() == 1245);
}

BOOST_AUTO_TEST_CASE(TestBboAsksMultimapSortCorrect)
{
  SimpleOrderBook::Asks asks;
  SimpleOrder order0(false, 3250, 100);
  SimpleOrder order1(false, 3235, 800);
  SimpleOrder order2(false, 3230, 200);
  SimpleOrder order3(false,    0, 200);
  SimpleOrder order4(false, 3245, 100);
  SimpleOrder order5(false, 3265, 200);

  // Insert out of price order
  asks.insert(std::make_pair(book::ComparablePrice(false, order0.price()), SimpleTracker(&order0)));
  asks.insert(std::make_pair(book::ComparablePrice(false, order1.price()), SimpleTracker(&order1)));
  asks.insert(std::make_pair(book::ComparablePrice(false, order2.price()), SimpleTracker(&order2)));
  asks.insert(std::make_pair(book::ComparablePrice(false, MARKET_ORDER_PRICE), 
                             SimpleTracker(&order3)));
  asks.insert(std::make_pair(book::ComparablePrice(false, order4.price()), SimpleTracker(&order4)));
  asks.insert(std::make_pair(book::ComparablePrice(false, order5.price()), SimpleTracker(&order5)));
  
  // Should access in price order
  SimpleOrder* expected_order[] = {
    &order3, &order2, &order1, &order4, &order0, &order5
  };

  SimpleOrderBook::Asks::iterator ask;
  int index = 0;

  for (ask = asks.begin(); ask != asks.end(); ++ask, ++index) {
    if (expected_order[index]->price() == MARKET_ORDER_PRICE) {
      BOOST_CHECK_EQUAL(MARKET_ORDER_ASK_SORT_PRICE, ask->first);
    } else {
      BOOST_CHECK_EQUAL(expected_order[index]->price(), ask->first);
    }
    BOOST_CHECK_EQUAL(expected_order[index], ask->second.ptr());
  }

  BOOST_CHECK((asks.upper_bound(book::ComparablePrice(false, 3235)))->second.ptr()->price() == 3245);
  BOOST_CHECK((asks.lower_bound(book::ComparablePrice(false, 3235)))->second.ptr()->price() == 3235);
}

BOOST_AUTO_TEST_CASE(TestBboAddCompleteBid)
{
  SimpleOrderBook order_book;
  SimpleOrder ask1(false, 1252, 100);
  SimpleOrder ask0(false, 1251, 100);
  SimpleOrder bid1(true,  1251, 100);
  SimpleOrder bid0(true,  1250, 100);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &bid0, false));
  BOOST_CHECK(add_and_verify(order_book, &ask0, false));
  BOOST_CHECK(add_and_verify(order_book, &ask1, false));

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(1250, 1, 100));
  BOOST_CHECK(dc.verify_ask(1251, 1, 100));

  // Match - complete
  {
    SimpleFillCheck fc1(&bid1, 100, 125100);
    SimpleFillCheck fc2(&ask0, 100, 125100);
    BOOST_CHECK(add_and_verify(order_book, &bid1, true, true));
  }

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1250, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(1, order_book.asks().size());
}

BOOST_AUTO_TEST_CASE(TestBboAddCompleteAsk)
{
  SimpleOrderBook order_book;
  SimpleOrder ask0(false, 1251, 100);
  SimpleOrder ask1(false, 1250, 100);
  SimpleOrder bid0(true,  1250, 100);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &bid0, false));
  BOOST_CHECK(add_and_verify(order_book, &ask0, false));

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(1, order_book.asks().size());

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(1250, 1, 100));
  BOOST_CHECK(dc.verify_ask(1251, 1, 100));

  // Match - complete
  {
    SimpleFillCheck fc1(&ask1, 100, 125000);
    SimpleFillCheck fc2(&bid0, 100, 125000);
    BOOST_CHECK(add_and_verify(order_book, &ask1, true, true));
  }

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(   0, 0,   0));
  BOOST_CHECK(dc.verify_ask(1251, 1, 100));

  // Verify sizes
  BOOST_CHECK_EQUAL(0, order_book.bids().size());
  BOOST_CHECK_EQUAL(1, order_book.asks().size());
}

BOOST_AUTO_TEST_CASE(TestBboAddMultiMatchBid)
{
  SimpleOrderBook order_book;
  SimpleOrder ask1(false, 1252, 100);
  SimpleOrder ask0(false, 1251, 300);
  SimpleOrder ask2(false, 1251, 200);
  SimpleOrder bid1(true,  1251, 500);
  SimpleOrder bid0(true,  1250, 100);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &bid0, false));
  BOOST_CHECK(add_and_verify(order_book, &ask0, false));
  BOOST_CHECK(add_and_verify(order_book, &ask1, false));
  BOOST_CHECK(add_and_verify(order_book, &ask2, false));

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(1250, 1, 100));
  BOOST_CHECK(dc.verify_ask(1251, 2, 500));

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(3, order_book.asks().size());

  // Match - complete
  {
    SimpleFillCheck fc1(&bid1, 500, 1251 * 500);
    SimpleFillCheck fc2(&ask2, 200, 1251 * 200);
    SimpleFillCheck fc3(&ask0, 300, 1251 * 300);
    BOOST_CHECK(add_and_verify(order_book, &bid1, true, true));
  }

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1250, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(1, order_book.asks().size());

  // Verify remaining
  BOOST_CHECK_EQUAL(&ask1, order_book.asks().begin()->second.ptr());
}

BOOST_AUTO_TEST_CASE(TestBboAddMultiMatchAsk)
{
  SimpleOrderBook order_book;
  SimpleOrder ask1(false, 9252, 100);
  SimpleOrder ask0(false, 9251, 300);
  SimpleOrder ask2(false, 9251, 200);
  SimpleOrder ask3(false, 9250, 600);
  SimpleOrder bid0(true,  9250, 100);
  SimpleOrder bid1(true,  9250, 500);
  SimpleOrder bid2(true,  9248, 100);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &bid0, false));
  BOOST_CHECK(add_and_verify(order_book, &bid1, false));
  BOOST_CHECK(add_and_verify(order_book, &bid2, false));
  BOOST_CHECK(add_and_verify(order_book, &ask0, false));
  BOOST_CHECK(add_and_verify(order_book, &ask1, false));
  BOOST_CHECK(add_and_verify(order_book, &ask2, false));

  // Verify sizes
  BOOST_CHECK_EQUAL(3, order_book.bids().size());
  BOOST_CHECK_EQUAL(3, order_book.asks().size());

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(9250, 2, 600));
  BOOST_CHECK(dc.verify_ask(9251, 2, 500));

  // Match - complete
  {
    SimpleFillCheck fc1(&ask3, 600, 9250 * 600);
    SimpleFillCheck fc2(&bid0, 100, 9250 * 100);
    SimpleFillCheck fc3(&bid1, 500, 9250 * 500);
    BOOST_CHECK(add_and_verify(order_book, &ask3, true, true));
  }

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(9248, 1, 100));
  BOOST_CHECK(dc.verify_ask(9251, 2, 500));

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(3, order_book.asks().size());

  // Verify remaining
  BOOST_CHECK_EQUAL(&bid2, order_book.bids().begin()->second.ptr());
}

BOOST_AUTO_TEST_CASE(TestBboAddPartialMatchBid)
{
  SimpleOrderBook order_book;
  SimpleOrder ask0(false, 7253, 300);
  SimpleOrder ask1(false, 7252, 100);
  SimpleOrder ask2(false, 7251, 200);
  SimpleOrder bid1(true,  7251, 350);
  SimpleOrder bid0(true,  7250, 100);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &bid0, false));
  BOOST_CHECK(add_and_verify(order_book, &ask0, false));
  BOOST_CHECK(add_and_verify(order_book, &ask1, false));
  BOOST_CHECK(add_and_verify(order_book, &ask2, false));

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(3, order_book.asks().size());

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(7250, 1, 100));
  BOOST_CHECK(dc.verify_ask(7251, 1, 200));

  // Match - partial
  {
    SimpleFillCheck fc1(&bid1, 200, 7251 * 200);
    SimpleFillCheck fc2(&ask2, 200, 7251 * 200);
    BOOST_CHECK(add_and_verify(order_book, &bid1, true, false));
  }

  // Verify sizes
  BOOST_CHECK_EQUAL(2, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(7251, 1, 150));
  BOOST_CHECK(dc.verify_ask(7252, 1, 100));

  // Verify remaining
  BOOST_CHECK_EQUAL(&ask1, order_book.asks().begin()->second.ptr());
  BOOST_CHECK_EQUAL(&bid1, order_book.bids().begin()->second.ptr());
}

BOOST_AUTO_TEST_CASE(TestBboAddPartialMatchAsk)
{
  SimpleOrderBook order_book;
  SimpleOrder ask0(false, 1253, 300);
  SimpleOrder ask1(false, 1251, 400);
  SimpleOrder bid1(true,  1251, 350);
  SimpleOrder bid0(true,  1250, 100);
  SimpleOrder bid2(true,  1250, 200);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &bid0, false));
  BOOST_CHECK(add_and_verify(order_book, &ask0, false));
  BOOST_CHECK(add_and_verify(order_book, &bid1, false));
  BOOST_CHECK(add_and_verify(order_book, &bid2, false));

  // Verify sizes
  BOOST_CHECK_EQUAL(3, order_book.bids().size());
  BOOST_CHECK_EQUAL(1, order_book.asks().size());

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(1251, 1, 350));
  BOOST_CHECK(dc.verify_ask(1253, 1, 300));

  // Match - partial
  {
    SimpleFillCheck fc1(&ask1, 350, 1251 * 350);
    SimpleFillCheck fc2(&bid1, 350, 1251 * 350);
    BOOST_CHECK(add_and_verify(order_book, &ask1,  true, false));
  }


  // Verify sizes
  BOOST_CHECK_EQUAL(2, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1250, 2, 300));
  BOOST_CHECK(dc.verify_ask(1251, 1,  50));

  // Verify remaining
  BOOST_CHECK_EQUAL(&bid0, order_book.bids().begin()->second.ptr());
  BOOST_CHECK_EQUAL(&ask1, order_book.asks().begin()->second.ptr());
}

BOOST_AUTO_TEST_CASE(TestBboAddMultiPartialMatchBid)
{
  SimpleOrderBook order_book;
  SimpleOrder ask1(false, 1252, 100);
  SimpleOrder ask2(false, 1251, 200);
  SimpleOrder ask0(false, 1251, 300);
  SimpleOrder bid1(true,  1251, 750);
  SimpleOrder bid0(true,  1250, 100);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &bid0, false));
  BOOST_CHECK(add_and_verify(order_book, &ask0, false));
  BOOST_CHECK(add_and_verify(order_book, &ask1, false));
  BOOST_CHECK(add_and_verify(order_book, &ask2, false));

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(3, order_book.asks().size());

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(1250, 1, 100));
  BOOST_CHECK(dc.verify_ask(1251, 2, 500));

  // Match - partial
  {
    SimpleFillCheck fc1(&bid1, 500, 1251 * 500);
    SimpleFillCheck fc2(&ask0, 300, 1251 * 300);
    SimpleFillCheck fc3(&ask2, 200, 1251 * 200);
    BOOST_CHECK(add_and_verify(order_book, &bid1, true, false));
  }

  // Verify sizes
  BOOST_CHECK_EQUAL(2, order_book.bids().size());
  BOOST_CHECK_EQUAL(1, order_book.asks().size());

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1251, 1, 250));
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));

  // Verify remaining
  BOOST_CHECK_EQUAL(&ask1, order_book.asks().begin()->second.ptr());
  BOOST_CHECK_EQUAL(&bid1, order_book.bids().begin()->second.ptr());
}

BOOST_AUTO_TEST_CASE(TestBboAddMultiPartialMatchAsk)
{
  SimpleOrderBook order_book;
  SimpleOrder ask0(false, 1253, 300);
  SimpleOrder ask1(false, 1251, 700);
  SimpleOrder bid1(true,  1251, 370);
  SimpleOrder bid2(true,  1251, 200);
  SimpleOrder bid0(true,  1250, 100);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &bid0, false));
  BOOST_CHECK(add_and_verify(order_book, &ask0, false));
  BOOST_CHECK(add_and_verify(order_book, &bid1, false));
  BOOST_CHECK(add_and_verify(order_book, &bid2, false));

  // Verify sizes
  BOOST_CHECK_EQUAL(3, order_book.bids().size());
  BOOST_CHECK_EQUAL(1, order_book.asks().size());

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(1251, 2, 570));
  BOOST_CHECK(dc.verify_ask(1253, 1, 300));

  // Match - partial
  {
    SimpleFillCheck fc1(&ask1, 570, 1251 * 570);
    SimpleFillCheck fc2(&bid1, 370, 1251 * 370);
    SimpleFillCheck fc3(&bid2, 200, 1251 * 200);
    BOOST_CHECK(add_and_verify(order_book, &ask1,  true, false));
  }

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1250, 1, 100));
  BOOST_CHECK(dc.verify_ask(1251, 1, 130));

  // Verify remaining
  BOOST_CHECK_EQUAL(&bid0, order_book.bids().begin()->second.ptr());
  BOOST_CHECK_EQUAL(100, order_book.bids().begin()->second.open_qty());
  BOOST_CHECK_EQUAL(&ask1, order_book.asks().begin()->second.ptr());
  BOOST_CHECK_EQUAL(130, order_book.asks().begin()->second.open_qty());
}

BOOST_AUTO_TEST_CASE(TestBboRepeatMatchBid)
{
  SimpleOrderBook order_book;
  SimpleOrder ask3(false, 1251, 400);
  SimpleOrder ask2(false, 1251, 200);
  SimpleOrder ask1(false, 1251, 300);
  SimpleOrder ask0(false, 1251, 100);
  SimpleOrder bid1(true,  1251, 900);
  SimpleOrder bid0(true,  1250, 100);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &bid0, false));
  BOOST_CHECK(add_and_verify(order_book, &bid1, false));

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(1251, 1, 900));

  // Match - repeated
  {
    SimpleFillCheck fc1(&bid1, 100, 125100);
    SimpleFillCheck fc2(&ask0, 100, 125100);
    BOOST_CHECK(add_and_verify(order_book, &ask0, true, true));
  }

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1251, 1, 800));

  {
    SimpleFillCheck fc1(&bid1, 300, 1251 * 300);
    SimpleFillCheck fc2(&ask1, 300, 1251 * 300);
    BOOST_CHECK(add_and_verify(order_book, &ask1, true, true));
  }

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1251, 1, 500));

  {
    SimpleFillCheck fc1(&bid1, 200, 1251 * 200);
    SimpleFillCheck fc2(&ask2, 200, 1251 * 200);
    BOOST_CHECK(add_and_verify(order_book, &ask2, true, true));
  }

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1251, 1, 300));

  {
    SimpleFillCheck fc1(&bid1, 300, 1251 * 300);
    SimpleFillCheck fc2(&ask3, 300, 1251 * 300);
    BOOST_CHECK(add_and_verify(order_book, &ask3, true));
  }

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1250, 1, 100));
  BOOST_CHECK(dc.verify_ask(1251, 1, 100));

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(1, order_book.asks().size());
}

BOOST_AUTO_TEST_CASE(TestBboRepeatMatchAsk)
{
  SimpleOrderBook order_book;
  SimpleOrder ask0(false,  1252, 100);
  SimpleOrder ask1(false,  1251, 900);
  SimpleOrder bid0(true, 1251, 100);
  SimpleOrder bid1(true, 1251, 300);
  SimpleOrder bid2(true, 1251, 200);
  SimpleOrder bid3(true, 1251, 400);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &ask0, false));
  BOOST_CHECK(add_and_verify(order_book, &ask1, false));

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_ask(1251, 1, 900));

  BOOST_CHECK_EQUAL(&ask1, order_book.asks().begin()->second.ptr());

  // Match - repeated
  {
    SimpleFillCheck fc1(&ask1, 100, 125100);
    SimpleFillCheck fc2(&bid0, 100, 125100);
    BOOST_CHECK(add_and_verify(order_book, &bid0, true, true));
  }

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_ask(1251, 1, 800));

  {
    SimpleFillCheck fc1(&ask1, 300, 1251 * 300);
    SimpleFillCheck fc2(&bid1, 300, 1251 * 300);
    BOOST_CHECK(add_and_verify(order_book, &bid1, true, true));
  }

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_ask(1251, 1, 500));

  {
    SimpleFillCheck fc1(&ask1, 200, 1251 * 200);
    SimpleFillCheck fc2(&bid2, 200, 1251 * 200);
    BOOST_CHECK(add_and_verify(order_book, &bid2, true, true));
  }

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_ask(1251, 1, 300));

  {
    SimpleFillCheck fc1(&ask1, 300, 1251 * 300);
    SimpleFillCheck fc2(&bid3, 300, 1251 * 300);
    BOOST_CHECK(add_and_verify(order_book, &bid3, true));
  }

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1251, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(1, order_book.asks().size());
}

BOOST_AUTO_TEST_CASE(TestBboAddMarketOrderBid)
{
  SimpleOrderBook order_book;
  SimpleOrder ask1(false, 1252, 100);
  SimpleOrder ask0(false, 1251, 100);
  SimpleOrder bid1(true,     0, 100);
  SimpleOrder bid0(true,  1250, 100);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &bid0, false));
  BOOST_CHECK(add_and_verify(order_book, &ask0, false));
  BOOST_CHECK(add_and_verify(order_book, &ask1, false));

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(1250, 1, 100));
  BOOST_CHECK(dc.verify_ask(1251, 1, 100));

  // Match - complete
  {
    SimpleFillCheck fc1(&bid1, 100, 125100);
    SimpleFillCheck fc2(&ask0, 100, 125100);
    BOOST_CHECK(add_and_verify(order_book, &bid1, true, true));
  }

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(1, order_book.asks().size());

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1250, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));
}

BOOST_AUTO_TEST_CASE(TestBboAddMarketOrderAsk)
{
  SimpleOrderBook order_book;
  SimpleOrder ask0(false, 1252, 100);
  SimpleOrder ask1(false,    0, 100);
  SimpleOrder bid1(true,  1251, 100);
  SimpleOrder bid0(true,  1250, 100);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &bid0, false));
  BOOST_CHECK(add_and_verify(order_book, &ask0, false));
  BOOST_CHECK(add_and_verify(order_book, &bid1, false));

  // Verify sizes
  BOOST_CHECK_EQUAL(2, order_book.bids().size());
  BOOST_CHECK_EQUAL(1, order_book.asks().size());

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(1251, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));

  // Match - complete
  {
    SimpleFillCheck fc1(&bid1, 100, 125100);
    SimpleFillCheck fc2(&ask1, 100, 125100);
    BOOST_CHECK(add_and_verify(order_book, &ask1, true, true));
  }

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(1, order_book.asks().size());

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1250, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));
}

BOOST_AUTO_TEST_CASE(TestBboAddMarketOrderBidMultipleMatch)
{
  SimpleOrderBook order_book;
  SimpleOrder ask1(false, 12520, 300);
  SimpleOrder ask0(false, 12510, 200);
  SimpleOrder bid1(true,      0, 500);
  SimpleOrder bid0(true,  12500, 100);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &bid0, false));
  BOOST_CHECK(add_and_verify(order_book, &ask0, false));
  BOOST_CHECK(add_and_verify(order_book, &ask1, false));

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(12500, 1, 100));
  BOOST_CHECK(dc.verify_ask(12510, 1, 200));

  // Match - complete
  {
    SimpleFillCheck fc1(&bid1, 500, 12510 * 200 + 12520 * 300);
    SimpleFillCheck fc2(&ask0, 200, 12510 * 200);
    SimpleFillCheck fc3(&ask1, 300, 12520 * 300);
    BOOST_CHECK(add_and_verify(order_book, &bid1, true, true));
  }

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(0, order_book.asks().size());

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(12500, 1, 100));
  BOOST_CHECK(dc.verify_ask(    0, 0,   0));
}

BOOST_AUTO_TEST_CASE(TestBboAddMarketOrderAskMultipleMatch)
{
  SimpleOrderBook order_book;
  SimpleOrder ask0(false, 12520, 100);
  SimpleOrder ask1(false,     0, 600);
  SimpleOrder bid1(true,  12510, 200);
  SimpleOrder bid0(true,  12500, 400);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &bid0, false));
  BOOST_CHECK(add_and_verify(order_book, &ask0, false));
  BOOST_CHECK(add_and_verify(order_book, &bid1, false));

  // Verify sizes
  BOOST_CHECK_EQUAL(2, order_book.bids().size());
  BOOST_CHECK_EQUAL(1, order_book.asks().size());

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(12510, 1, 200));
  BOOST_CHECK(dc.verify_ask(12520, 1, 100));

  // Match - complete
  {
    SimpleFillCheck fc1(&bid0, 400, 12500 * 400);
    SimpleFillCheck fc2(&bid1, 200, 12510 * 200);
    SimpleFillCheck fc3(&ask1, 600, 12500 * 400 + 12510 * 200);
    BOOST_CHECK(add_and_verify(order_book, &ask1, true, true));
  }

  // Verify sizes
  BOOST_CHECK_EQUAL(0, order_book.bids().size());
  BOOST_CHECK_EQUAL(1, order_book.asks().size());

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(    0, 0,   0));
  BOOST_CHECK(dc.verify_ask(12520, 1, 100));
}

BOOST_AUTO_TEST_CASE(TestBboMatchMarketOrderBid)
{
  SimpleOrderBook order_book;
  SimpleOrder ask0(false, 1253, 100);
  SimpleOrder bid1(true,     0, 100);
  SimpleOrder bid0(true,  1250, 100);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &bid0, false));
  BOOST_CHECK(add_and_verify(order_book, &bid1, false));

  // Verify sizes
  BOOST_CHECK_EQUAL(2, order_book.bids().size());
  BOOST_CHECK_EQUAL(0, order_book.asks().size());

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(1250, 1, 100));
  BOOST_CHECK(dc.verify_ask(   0, 0,   0));

  // Match - complete
  {
    SimpleFillCheck fc1(&bid1, 100, 125300);
    SimpleFillCheck fc2(&ask0, 100, 125300);
    BOOST_CHECK(add_and_verify(order_book, &ask0, true, true));
  }

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(0, order_book.asks().size());

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1250, 1, 100));
  BOOST_CHECK(dc.verify_ask(   0, 0,   0));
}

BOOST_AUTO_TEST_CASE(TestBboMatchMarketOrderAsk)
{
  SimpleOrderBook order_book;
  SimpleOrder ask0(false, 1252, 100);
  SimpleOrder ask1(false,    0, 100);
  SimpleOrder bid0(true,  1250, 100);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &ask0, false));
  BOOST_CHECK(add_and_verify(order_book, &ask1, false));

  // Verify sizes
  BOOST_CHECK_EQUAL(0, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));
  BOOST_CHECK(dc.verify_bid(   0, 0,   0));

  // Match - complete
  {
    SimpleFillCheck fc1(&bid0, 100, 125000);
    SimpleFillCheck fc2(&ask1, 100, 125000);
    BOOST_CHECK(add_and_verify(order_book, &bid0, true, true));
  }

  // Verify sizes
  BOOST_CHECK_EQUAL(0, order_book.bids().size());
  BOOST_CHECK_EQUAL(1, order_book.asks().size());

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));
}

BOOST_AUTO_TEST_CASE(TestBboMatchMultipleMarketOrderBid)
{
  SimpleOrderBook order_book;
  SimpleOrder ask0(false, 1253, 400);
  SimpleOrder bid1(true,     0, 100);
  SimpleOrder bid2(true,     0, 200);
  SimpleOrder bid0(true,  1250, 100);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &bid0, false));
  BOOST_CHECK(add_and_verify(order_book, &bid1, false));
  BOOST_CHECK(add_and_verify(order_book, &bid2, false));

  // Verify sizes
  BOOST_CHECK_EQUAL(3, order_book.bids().size());
  BOOST_CHECK_EQUAL(0, order_book.asks().size());

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_ask(   0, 0,   0));
  BOOST_CHECK(dc.verify_bid(1250, 1, 100));

  // Match - complete
  {
    SimpleFillCheck fc1(&bid1, 100, 1253 * 100);
    SimpleFillCheck fc2(&bid2, 200, 1253 * 200);
    SimpleFillCheck fc3(&ask0, 300, 1253 * 300);
    BOOST_CHECK(add_and_verify(order_book, &ask0, true, false));
  }

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(1, order_book.asks().size());

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_ask(1253, 1, 100));
  BOOST_CHECK(dc.verify_bid(1250, 1, 100));
}


BOOST_AUTO_TEST_CASE(TestBboMatchMultipleMarketOrderAsk)
{
  SimpleOrderBook order_book;
  SimpleOrder ask0(false, 1252, 100);
  SimpleOrder ask2(false,    0, 400);
  SimpleOrder ask1(false,    0, 100);
  SimpleOrder bid0(true,  1250, 300);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &ask0, false));
  BOOST_CHECK(add_and_verify(order_book, &ask1, false));
  BOOST_CHECK(add_and_verify(order_book, &ask2, false));

  // Verify sizes
  BOOST_CHECK_EQUAL(0, order_book.bids().size());
  BOOST_CHECK_EQUAL(3, order_book.asks().size());

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));
  BOOST_CHECK(dc.verify_bid(   0, 0,   0));

  // Match - partiaL
  {
    SimpleFillCheck fc1(&bid0, 300, 1250 * 300);
    SimpleFillCheck fc2(&ask1, 100, 1250 * 100);
    SimpleFillCheck fc3(&ask2, 200, 1250 * 200);
    BOOST_CHECK(add_and_verify(order_book, &bid0, true, true));
  }

  // Verify sizes
  BOOST_CHECK_EQUAL(0, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));
  BOOST_CHECK(dc.verify_bid(   0, 0,   0));
}

BOOST_AUTO_TEST_CASE(TestBboCancelBid)
{
  SimpleOrderBook order_book;
  SimpleOrder ask1(false, 1252, 100);
  SimpleOrder ask0(false, 1251, 100);
  SimpleOrder bid0(true,  1250, 100);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &bid0, false));
  BOOST_CHECK(add_and_verify(order_book, &ask0, false));
  BOOST_CHECK(add_and_verify(order_book, &ask1, false));

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(1250, 1, 100));
  BOOST_CHECK(dc.verify_ask(1251, 1, 100));

  // Cancel bid
  BOOST_CHECK(cancel_and_verify(order_book, &bid0, simple::os_cancelled));

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(0,    0,   0));
  BOOST_CHECK(dc.verify_ask(1251, 1, 100));

  // Verify sizes
  BOOST_CHECK_EQUAL(0, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());
}

BOOST_AUTO_TEST_CASE(TestBboCancelAskAndMatch)
{
  SimpleOrderBook order_book;
  SimpleOrder ask1(false, 1252, 100);
  SimpleOrder ask0(false, 1251, 100);
  SimpleOrder bid2(true,  1252, 100);
  SimpleOrder bid0(true,  1250, 100);
  SimpleOrder bid1(true,  1250, 100);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &bid0, false));
  BOOST_CHECK(add_and_verify(order_book, &bid1, false));
  BOOST_CHECK(add_and_verify(order_book, &ask0, false));
  BOOST_CHECK(add_and_verify(order_book, &ask1, false));

  // Verify sizes
  BOOST_CHECK_EQUAL(2, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(1250, 2, 200));
  BOOST_CHECK(dc.verify_ask(1251, 1, 100));

  // Cancel bid
  BOOST_CHECK(cancel_and_verify(order_book, &ask0, simple::os_cancelled));

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1250, 2, 200));
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));

  // Match - partiaL
  {
    SimpleFillCheck fc1(&bid2, 100, 1252 * 100);
    SimpleFillCheck fc2(&ask1, 100, 1252 * 100);
    BOOST_CHECK(add_and_verify(order_book, &bid2, true, true));
  }

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1250, 2, 200));
  BOOST_CHECK(dc.verify_ask(   0, 0,   0));

  // Cancel bid
  BOOST_CHECK(cancel_and_verify(order_book, &bid0, simple::os_cancelled));

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1250, 1, 100));
  BOOST_CHECK(dc.verify_ask(   0, 0,   0));

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(0, order_book.asks().size());
}

BOOST_AUTO_TEST_CASE(TestBboCancelBidFail)
{
  SimpleOrderBook order_book;
  SimpleOrder ask0(false, 1251, 100);
  SimpleOrder ask1(false, 1250, 100);
  SimpleOrder bid0(true,  1250, 100);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &bid0, false));
  BOOST_CHECK(add_and_verify(order_book, &ask0, false));

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(1, order_book.asks().size());

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_ask(1251, 1, 100));
  BOOST_CHECK(dc.verify_bid(1250, 1, 100));

  // Match - complete
  {
    SimpleFillCheck fc1(&ask1, 100, 125000);
    SimpleFillCheck fc2(&bid0, 100, 125000);
    BOOST_CHECK(add_and_verify(order_book, &ask1, true, true));
  }

  // Verify sizes
  BOOST_CHECK_EQUAL(0, order_book.bids().size());
  BOOST_CHECK_EQUAL(1, order_book.asks().size());

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_ask(1251, 1, 100));
  BOOST_CHECK(dc.verify_bid(   0, 0,   0));

  // Cancel a filled order
  BOOST_CHECK(cancel_and_verify(order_book, &bid0, simple::os_complete));

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_ask(1251, 1, 100));
  BOOST_CHECK(dc.verify_bid(   0, 0,   0));
}

BOOST_AUTO_TEST_CASE(TestBboCancelAskFail)
{
  SimpleOrderBook order_book;
  SimpleOrder ask1(false, 1252, 100);
  SimpleOrder ask0(false, 1251, 100);
  SimpleOrder bid1(true,  1251, 100);
  SimpleOrder bid0(true,  1250, 100);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &bid0, false));
  BOOST_CHECK(add_and_verify(order_book, &ask0, false));
  BOOST_CHECK(add_and_verify(order_book, &ask1, false));

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_ask(1251, 1, 100));
  BOOST_CHECK(dc.verify_bid(1250, 1, 100));

  // Match - complete
  {
    SimpleFillCheck fc1(&bid1, 100, 125100);
    SimpleFillCheck fc2(&ask0, 100, 125100);
    BOOST_CHECK(add_and_verify(order_book, &bid1, true, true));
  }

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(1, order_book.asks().size());

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));
  BOOST_CHECK(dc.verify_bid(1250, 1, 100));

  // Cancel a filled order
  BOOST_CHECK(cancel_and_verify(order_book, &ask0, simple::os_complete));

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_ask(1252, 1, 100));
  BOOST_CHECK(dc.verify_bid(1250, 1, 100));
}

BOOST_AUTO_TEST_CASE(TestBboCancelBidRestore)
{
  SimpleOrderBook order_book;
  SimpleOrder ask10(false, 1258, 600);
  SimpleOrder ask9(false,  1257, 700);
  SimpleOrder ask8(false,  1256, 100);
  SimpleOrder ask7(false,  1256, 100);
  SimpleOrder ask6(false,  1255, 500);
  SimpleOrder ask5(false,  1255, 200);
  SimpleOrder ask4(false,  1254, 300);
  SimpleOrder ask3(false,  1252, 200);
  SimpleOrder ask2(false,  1252, 100);
  SimpleOrder ask1(false,  1251, 400);
  SimpleOrder ask0(false,  1250, 500);

  SimpleOrder bid0(true,   1249, 100);
  SimpleOrder bid1(true,   1249, 200);
  SimpleOrder bid2(true,   1249, 200);
  SimpleOrder bid3(true,   1248, 400);
  SimpleOrder bid4(true,   1246, 600);
  SimpleOrder bid5(true,   1246, 500);
  SimpleOrder bid6(true,   1245, 200);
  SimpleOrder bid7(true,   1245, 100);
  SimpleOrder bid8(true,   1245, 200);
  SimpleOrder bid9(true,   1244, 700);
  SimpleOrder bid10(true,  1244, 300);
  SimpleOrder bid11(true,  1242, 300);
  SimpleOrder bid12(true,  1241, 400);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &ask0,  false));
  BOOST_CHECK(add_and_verify(order_book, &ask1,  false));
  BOOST_CHECK(add_and_verify(order_book, &ask2,  false));
  BOOST_CHECK(add_and_verify(order_book, &ask3,  false));
  BOOST_CHECK(add_and_verify(order_book, &ask4,  false));
  BOOST_CHECK(add_and_verify(order_book, &ask5,  false));
  BOOST_CHECK(add_and_verify(order_book, &ask6,  false));
  BOOST_CHECK(add_and_verify(order_book, &ask7,  false));
  BOOST_CHECK(add_and_verify(order_book, &ask8,  false));
  BOOST_CHECK(add_and_verify(order_book, &ask9,  false));
  BOOST_CHECK(add_and_verify(order_book, &ask10, false));
  BOOST_CHECK(add_and_verify(order_book, &bid0,  false));
  BOOST_CHECK(add_and_verify(order_book, &bid1,  false));
  BOOST_CHECK(add_and_verify(order_book, &bid2,  false));
  BOOST_CHECK(add_and_verify(order_book, &bid3,  false));
  BOOST_CHECK(add_and_verify(order_book, &bid4,  false));
  BOOST_CHECK(add_and_verify(order_book, &bid5,  false));
  BOOST_CHECK(add_and_verify(order_book, &bid6,  false));
  BOOST_CHECK(add_and_verify(order_book, &bid7,  false));
  BOOST_CHECK(add_and_verify(order_book, &bid8,  false));
  BOOST_CHECK(add_and_verify(order_book, &bid9,  false));
  BOOST_CHECK(add_and_verify(order_book, &bid10, false));
  BOOST_CHECK(add_and_verify(order_book, &bid11, false));
  BOOST_CHECK(add_and_verify(order_book, &bid12, false));

  // Verify sizes
  BOOST_CHECK_EQUAL(13, order_book.bids().size());
  BOOST_CHECK_EQUAL(11, order_book.asks().size());

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(1249, 3,  500));
  BOOST_CHECK(dc.verify_ask(1250, 1,  500));

  // Cancel a bid level (erase)
  BOOST_CHECK(cancel_and_verify(order_book, &bid3, simple::os_cancelled));

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1249, 3,  500));
  BOOST_CHECK(dc.verify_ask(1250, 1,  500));
  
  // Cancel common bid levels (not erased)
  BOOST_CHECK(cancel_and_verify(order_book, &bid7, simple::os_cancelled));
  BOOST_CHECK(cancel_and_verify(order_book, &bid4, simple::os_cancelled));

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1249, 3,  500));
  BOOST_CHECK(dc.verify_ask(1250, 1,  500));

  // Cancel the best bid level (erased)
  BOOST_CHECK(cancel_and_verify(order_book, &bid1, simple::os_cancelled));
  BOOST_CHECK(cancel_and_verify(order_book, &bid0, simple::os_cancelled));
  BOOST_CHECK(cancel_and_verify(order_book, &bid2, simple::os_cancelled));

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1246, 1,  500));
  BOOST_CHECK(dc.verify_ask(1250, 1,  500));
}

BOOST_AUTO_TEST_CASE(TestBboCancelAskRestore)
{
  SimpleOrderBook order_book;
  SimpleOrder ask10(false, 1258, 600);
  SimpleOrder ask9(false,  1257, 700);
  SimpleOrder ask8(false,  1256, 100);
  SimpleOrder ask7(false,  1256, 100);
  SimpleOrder ask6(false,  1255, 500);
  SimpleOrder ask5(false,  1255, 200);
  SimpleOrder ask4(false,  1254, 300);
  SimpleOrder ask3(false,  1252, 200);
  SimpleOrder ask2(false,  1252, 100);
  SimpleOrder ask1(false,  1251, 400);
  SimpleOrder ask0(false,  1250, 500);

  SimpleOrder bid0(true,   1249, 100);
  SimpleOrder bid1(true,   1249, 200);
  SimpleOrder bid2(true,   1249, 200);
  SimpleOrder bid3(true,   1248, 400);
  SimpleOrder bid4(true,   1246, 600);
  SimpleOrder bid5(true,   1246, 500);
  SimpleOrder bid6(true,   1245, 200);
  SimpleOrder bid7(true,   1245, 100);
  SimpleOrder bid8(true,   1245, 200);
  SimpleOrder bid9(true,   1244, 700);
  SimpleOrder bid10(true,  1244, 300);
  SimpleOrder bid11(true,  1242, 300);
  SimpleOrder bid12(true,  1241, 400);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &ask0,  false));
  BOOST_CHECK(add_and_verify(order_book, &ask1,  false));
  BOOST_CHECK(add_and_verify(order_book, &ask2,  false));
  BOOST_CHECK(add_and_verify(order_book, &ask3,  false));
  BOOST_CHECK(add_and_verify(order_book, &ask4,  false));
  BOOST_CHECK(add_and_verify(order_book, &ask5,  false));
  BOOST_CHECK(add_and_verify(order_book, &ask6,  false));
  BOOST_CHECK(add_and_verify(order_book, &ask7,  false));
  BOOST_CHECK(add_and_verify(order_book, &ask8,  false));
  BOOST_CHECK(add_and_verify(order_book, &ask9,  false));
  BOOST_CHECK(add_and_verify(order_book, &ask10, false));
  BOOST_CHECK(add_and_verify(order_book, &bid0,  false));
  BOOST_CHECK(add_and_verify(order_book, &bid1,  false));
  BOOST_CHECK(add_and_verify(order_book, &bid2,  false));
  BOOST_CHECK(add_and_verify(order_book, &bid3,  false));
  BOOST_CHECK(add_and_verify(order_book, &bid4,  false));
  BOOST_CHECK(add_and_verify(order_book, &bid5,  false));
  BOOST_CHECK(add_and_verify(order_book, &bid6,  false));
  BOOST_CHECK(add_and_verify(order_book, &bid7,  false));
  BOOST_CHECK(add_and_verify(order_book, &bid8,  false));
  BOOST_CHECK(add_and_verify(order_book, &bid9,  false));
  BOOST_CHECK(add_and_verify(order_book, &bid10, false));
  BOOST_CHECK(add_and_verify(order_book, &bid11, false));
  BOOST_CHECK(add_and_verify(order_book, &bid12, false));

  // Verify sizes
  BOOST_CHECK_EQUAL(13, order_book.bids().size());
  BOOST_CHECK_EQUAL(11, order_book.asks().size());

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(1249, 3,  500));
  BOOST_CHECK(dc.verify_ask(1250, 1,  500));

  // Cancel an ask level (erase)
  BOOST_CHECK(cancel_and_verify(order_book, &ask1, simple::os_cancelled));

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1249, 3,  500));
  BOOST_CHECK(dc.verify_ask(1250, 1,  500));

  // Cancel common ask levels (not erased)
  BOOST_CHECK(cancel_and_verify(order_book, &ask2, simple::os_cancelled));
  BOOST_CHECK(cancel_and_verify(order_book, &ask6, simple::os_cancelled));

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1249, 3,  500));
  BOOST_CHECK(dc.verify_ask(1250, 1,  500));

  // Cancel the best ask level (erased)
  BOOST_CHECK(cancel_and_verify(order_book, &ask0, simple::os_cancelled));

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1249, 3,  500));
  BOOST_CHECK(dc.verify_ask(1252, 1,  200));
}

BOOST_AUTO_TEST_CASE(TestBboFillCompleteBidRestoreDepth)
{
  SimpleOrderBook order_book;
  SimpleOrder ask10(false, 1258, 600);
  SimpleOrder ask9(false,  1257, 700);
  SimpleOrder ask8(false,  1256, 100);
  SimpleOrder ask7(false,  1256, 100);
  SimpleOrder ask6(false,  1255, 500);
  SimpleOrder ask5(false,  1255, 200);
  SimpleOrder ask4(false,  1254, 300);
  SimpleOrder ask3(false,  1252, 200);
  SimpleOrder ask2(false,  1252, 100);
  SimpleOrder ask1(false,  1251, 400);
  SimpleOrder ask0(false,  1250, 500);

  SimpleOrder bid0(true,   1249, 100);
  SimpleOrder bid1(true,   1249, 200);
  SimpleOrder bid2(true,   1249, 200);
  SimpleOrder bid3(true,   1248, 400);
  SimpleOrder bid4(true,   1246, 600);
  SimpleOrder bid5(true,   1246, 500);
  SimpleOrder bid6(true,   1245, 200);
  SimpleOrder bid7(true,   1245, 100);
  SimpleOrder bid8(true,   1245, 200);
  SimpleOrder bid9(true,   1244, 700);
  SimpleOrder bid10(true,  1244, 300);
  SimpleOrder bid11(true,  1242, 300);
  SimpleOrder bid12(true,  1241, 400);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &ask0,  false));
  BOOST_CHECK(add_and_verify(order_book, &ask1,  false));
  BOOST_CHECK(add_and_verify(order_book, &ask2,  false));
  BOOST_CHECK(add_and_verify(order_book, &ask3,  false));
  BOOST_CHECK(add_and_verify(order_book, &ask4,  false));
  BOOST_CHECK(add_and_verify(order_book, &ask5,  false));
  BOOST_CHECK(add_and_verify(order_book, &ask6,  false));
  BOOST_CHECK(add_and_verify(order_book, &ask7,  false));
  BOOST_CHECK(add_and_verify(order_book, &ask8,  false));
  BOOST_CHECK(add_and_verify(order_book, &ask9,  false));
  BOOST_CHECK(add_and_verify(order_book, &ask10, false));
  BOOST_CHECK(add_and_verify(order_book, &bid0,  false));
  BOOST_CHECK(add_and_verify(order_book, &bid1,  false));
  BOOST_CHECK(add_and_verify(order_book, &bid2,  false));
  BOOST_CHECK(add_and_verify(order_book, &bid3,  false));
  BOOST_CHECK(add_and_verify(order_book, &bid4,  false));
  BOOST_CHECK(add_and_verify(order_book, &bid5,  false));
  BOOST_CHECK(add_and_verify(order_book, &bid6,  false));
  BOOST_CHECK(add_and_verify(order_book, &bid7,  false));
  BOOST_CHECK(add_and_verify(order_book, &bid8,  false));
  BOOST_CHECK(add_and_verify(order_book, &bid9,  false));
  BOOST_CHECK(add_and_verify(order_book, &bid10, false));
  BOOST_CHECK(add_and_verify(order_book, &bid11, false));
  BOOST_CHECK(add_and_verify(order_book, &bid12, false));

  // Verify sizes
  BOOST_CHECK_EQUAL(13, order_book.bids().size());
  BOOST_CHECK_EQUAL(11, order_book.asks().size());

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(1249, 3,  500));
  BOOST_CHECK(dc.verify_ask(1250, 1,  500));

  // Fill the top bid level (erase) and add an ask level (insert)
  SimpleOrder cross_ask(false,  1249, 800);
  {
    SimpleFillCheck fc1(&bid0,      100, 1249 * 100);
    SimpleFillCheck fc2(&bid1,      200, 1249 * 200);
    SimpleFillCheck fc3(&bid2,      200, 1249 * 200);
    SimpleFillCheck fc4(&cross_ask, 500, 1249 * 500);
    BOOST_CHECK(add_and_verify(order_book, &cross_ask, true, false));
  }

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1248, 1,  400));
  BOOST_CHECK(dc.verify_ask(1249, 1,  300)); // Inserted
  
  // Fill the top bid level (erase) but do not add an ask level (no insert)
  SimpleOrder cross_ask2(false,  1248, 400);
  {
    SimpleFillCheck fc1(&bid3,       400, 1248 * 400);
    SimpleFillCheck fc4(&cross_ask2, 400, 1248 * 400);
    BOOST_CHECK(add_and_verify(order_book, &cross_ask2, true, true));
  }

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1246, 2, 1100));
  BOOST_CHECK(dc.verify_ask(1249, 1,  300));

  // Fill the top bid level (erase) and add ask level (insert),
  //    but nothing to restore
  SimpleOrder cross_ask3(false,  1246, 2400);
  {
    SimpleFillCheck fc1(&bid4,        600, 1246 * 600);
    SimpleFillCheck fc2(&bid5,        500, 1246 * 500);
    SimpleFillCheck fc3(&cross_ask3, 1100, 1246 * 1100);
    BOOST_CHECK(add_and_verify(order_book, &cross_ask3, true, false));
  }

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1245, 3,  500));
  BOOST_CHECK(dc.verify_ask(1246, 1, 1300));

  // Partial fill the top bid level (reduce) 
  SimpleOrder cross_ask4(false,  1245, 250);
  {
    SimpleFillCheck fc1(&bid6,        200, 1245 * 200);
    SimpleFillCheck fc2(&bid7,         50, 1245 *  50);
    SimpleFillCheck fc3(&cross_ask4,  250, 1245 * 250);
    BOOST_CHECK(add_and_verify(order_book, &cross_ask4, true, true));
  }

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1245, 2,  250)); // 1 filled, 1 reduced
  BOOST_CHECK(dc.verify_ask(1246, 1, 1300));
}

BOOST_AUTO_TEST_CASE(TestBboFillCompleteAskRestoreDepth)
{
  SimpleOrderBook order_book;
  SimpleOrder ask10(false, 1258, 600);
  SimpleOrder ask9(false,  1257, 700);
  SimpleOrder ask8(false,  1256, 100);
  SimpleOrder ask7(false,  1256, 100);
  SimpleOrder ask6(false,  1255, 500);
  SimpleOrder ask5(false,  1255, 200);
  SimpleOrder ask4(false,  1254, 300);
  SimpleOrder ask3(false,  1252, 200);
  SimpleOrder ask2(false,  1252, 100);
  SimpleOrder ask1(false,  1251, 400);
  SimpleOrder ask0(false,  1250, 500);

  SimpleOrder bid0(true,   1249, 100);
  SimpleOrder bid1(true,   1249, 200);
  SimpleOrder bid2(true,   1249, 200);
  SimpleOrder bid3(true,   1248, 400);
  SimpleOrder bid4(true,   1246, 600);
  SimpleOrder bid5(true,   1246, 500);
  SimpleOrder bid6(true,   1245, 200);
  SimpleOrder bid7(true,   1245, 100);
  SimpleOrder bid8(true,   1245, 200);
  SimpleOrder bid9(true,   1244, 700);
  SimpleOrder bid10(true,  1244, 300);
  SimpleOrder bid11(true,  1242, 300);
  SimpleOrder bid12(true,  1241, 400);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &ask0,  false));
  BOOST_CHECK(add_and_verify(order_book, &ask1,  false));
  BOOST_CHECK(add_and_verify(order_book, &ask2,  false));
  BOOST_CHECK(add_and_verify(order_book, &ask3,  false));
  BOOST_CHECK(add_and_verify(order_book, &ask4,  false));
  BOOST_CHECK(add_and_verify(order_book, &ask5,  false));
  BOOST_CHECK(add_and_verify(order_book, &ask6,  false));
  BOOST_CHECK(add_and_verify(order_book, &ask7,  false));
  BOOST_CHECK(add_and_verify(order_book, &ask8,  false));
  BOOST_CHECK(add_and_verify(order_book, &ask9,  false));
  BOOST_CHECK(add_and_verify(order_book, &ask10, false));
  BOOST_CHECK(add_and_verify(order_book, &bid0,  false));
  BOOST_CHECK(add_and_verify(order_book, &bid1,  false));
  BOOST_CHECK(add_and_verify(order_book, &bid2,  false));
  BOOST_CHECK(add_and_verify(order_book, &bid3,  false));
  BOOST_CHECK(add_and_verify(order_book, &bid4,  false));
  BOOST_CHECK(add_and_verify(order_book, &bid5,  false));
  BOOST_CHECK(add_and_verify(order_book, &bid6,  false));
  BOOST_CHECK(add_and_verify(order_book, &bid7,  false));
  BOOST_CHECK(add_and_verify(order_book, &bid8,  false));
  BOOST_CHECK(add_and_verify(order_book, &bid9,  false));
  BOOST_CHECK(add_and_verify(order_book, &bid10, false));
  BOOST_CHECK(add_and_verify(order_book, &bid11, false));
  BOOST_CHECK(add_and_verify(order_book, &bid12, false));

  // Verify sizes
  BOOST_CHECK_EQUAL(13, order_book.bids().size());
  BOOST_CHECK_EQUAL(11, order_book.asks().size());

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(1249, 3,  500));
  BOOST_CHECK(dc.verify_ask(1250, 1,  500));

  // Fill the top ask level (erase) and add a bid level (insert)
  SimpleOrder cross_bid(true,  1250, 800);
  {
    SimpleFillCheck fc1(&ask0,      500, 1250 * 500);
    SimpleFillCheck fc4(&cross_bid, 500, 1250 * 500);
    BOOST_CHECK(add_and_verify(order_book, &cross_bid, true, false));
  }

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1250, 1,  300));
  BOOST_CHECK(dc.verify_ask(1251, 1,  400));

  // Fill the top ask level (erase) but do not add an bid level (no insert)
  SimpleOrder cross_bid2(true,  1251, 400);
  {
    SimpleFillCheck fc1(&ask1,       400, 1251 * 400);
    SimpleFillCheck fc4(&cross_bid2, 400, 1251 * 400);
    BOOST_CHECK(add_and_verify(order_book, &cross_bid2, true, true));
  }

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1250, 1,  300));
  BOOST_CHECK(dc.verify_ask(1252, 2,  300));

  // Fill the top ask level (erase) and add bid level (insert),
  //    but nothing to restore
  SimpleOrder cross_bid3(true,  1252, 2400);
  {
    SimpleFillCheck fc1(&ask2,        100, 1252 * 100);
    SimpleFillCheck fc2(&ask3,        200, 1252 * 200);
    SimpleFillCheck fc3(&cross_bid3,  300, 1252 * 300);
    BOOST_CHECK(add_and_verify(order_book, &cross_bid3, true, false));
  }

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1252, 1, 2100)); // Insert
  BOOST_CHECK(dc.verify_ask(1254, 1,  300));

  // Fill the top ask level (erase) but nothing to restore
  SimpleOrder cross_bid4(true,  1254, 300);
  {
    SimpleFillCheck fc2(&ask4,        300, 1254 * 300);
    SimpleFillCheck fc3(&cross_bid4,  300, 1254 * 300);
    BOOST_CHECK(add_and_verify(order_book, &cross_bid4, true, true));
  }

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1252, 1, 2100));
  BOOST_CHECK(dc.verify_ask(1255, 2,  700));

  // Partial fill the top ask level (reduce) 
  SimpleOrder cross_bid5(true,  1255, 550);
  {
    SimpleFillCheck fc1(&ask5,        200, 1255 * 200);
    SimpleFillCheck fc2(&ask6,        350, 1255 * 350);
    SimpleFillCheck fc3(&cross_bid5,  550, 1255 * 550);
    BOOST_CHECK(add_and_verify(order_book, &cross_bid5, true, true));
  }

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1252, 1, 2100));
  BOOST_CHECK(dc.verify_ask(1255, 1,  150)); // 1 filled, 1 reduced
}

BOOST_AUTO_TEST_CASE(TestBboReplaceSizeDecrease)
{
  SimpleOrderBook order_book;
  ChangedChecker cc(order_book.depth());
  SimpleOrder ask1(false, 1252, 200);
  SimpleOrder ask0(false, 1252, 300);
  SimpleOrder bid1(true,  1251, 100);
  SimpleOrder bid0(true,  1250, 100);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &bid0, false));
  BOOST_CHECK(add_and_verify(order_book, &bid1, false));
  BOOST_CHECK(add_and_verify(order_book, &ask0, false));
  BOOST_CHECK(add_and_verify(order_book, &ask1, false));

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(1251, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 2, 500));

  // Verify changed stamps
  BOOST_CHECK(cc.verify_bbo_changed(true, true));
  cc.reset();

  // Replace size
  BOOST_CHECK(replace_and_verify(order_book, &bid0, -60));
  BOOST_CHECK(replace_and_verify(order_book, &ask0, -150));

  // Verify orders
  BOOST_CHECK_EQUAL(40, bid0.order_qty());
  BOOST_CHECK_EQUAL(150, ask0.order_qty());
  
  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1251, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 2, 350));

  // Verify changed stamps
  BOOST_CHECK(cc.verify_bbo_changed(false, true));
}

BOOST_AUTO_TEST_CASE(TestBboReplaceSizeDecreaseCancel)
{
  SimpleOrderBook order_book;
  ChangedChecker cc(order_book.depth());
  SimpleOrder ask1(false, 1252, 200);
  SimpleOrder ask0(false, 1252, 300);
  SimpleOrder bid1(true,  1251, 400);
  SimpleOrder bid0(true,  1250, 100);
  SimpleOrder bid2(true,  1249, 700);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &bid0, false));
  BOOST_CHECK(add_and_verify(order_book, &bid1, false));
  BOOST_CHECK(add_and_verify(order_book, &bid2, false));
  BOOST_CHECK(add_and_verify(order_book, &ask0, false));
  BOOST_CHECK(add_and_verify(order_book, &ask1, false));

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(1251, 1, 400));
  BOOST_CHECK(dc.verify_ask(1252, 2, 500));

  // Partial Fill existing book
  SimpleOrder cross_bid(true,  1252, 125);
  SimpleOrder cross_ask(false, 1251, 100);
  
  {
    SimpleFillCheck fc1(&cross_bid, 125, 1252 * 125);
    SimpleFillCheck fc2(&ask0,      125, 1252 * 125);
    BOOST_CHECK(add_and_verify(order_book, &cross_bid, true, true));
  }
  {
    SimpleFillCheck fc1(&cross_ask, 100, 1251 * 100);
    SimpleFillCheck fc2(&bid1,      100, 1251 * 100);
    BOOST_CHECK(add_and_verify(order_book, &cross_ask, true, true));
  }

  // Verify quantity
  BOOST_CHECK_EQUAL(175, ask0.open_qty());
  BOOST_CHECK_EQUAL(300, bid1.open_qty());

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1251, 1, 300));
  BOOST_CHECK(dc.verify_ask(1252, 2, 375));

  // Replace size - cancel
  BOOST_CHECK(replace_and_verify(
      order_book, &ask0, -175, PRICE_UNCHANGED, simple::os_cancelled)); 

  // Verify orders
  BOOST_CHECK_EQUAL(125, ask0.order_qty());
  BOOST_CHECK_EQUAL(0, ask0.open_qty());

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1251, 1, 300));
  BOOST_CHECK(dc.verify_ask(1252, 1, 200));

  // Replace size - reduce level
  BOOST_CHECK(replace_and_verify(
      order_book, &bid1, -100, PRICE_UNCHANGED, simple::os_accepted)); 

  // Verify orders
  BOOST_CHECK_EQUAL(300, bid1.order_qty());
  BOOST_CHECK_EQUAL(200, bid1.open_qty());

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1251, 1, 200));
  BOOST_CHECK(dc.verify_ask(1252, 1, 200));

  // Replace size - cancel and erase level
  BOOST_CHECK(replace_and_verify(
      order_book, &bid1, -200, PRICE_UNCHANGED, simple::os_cancelled)); 

  // Verify orders
  BOOST_CHECK_EQUAL(100, bid1.order_qty());
  BOOST_CHECK_EQUAL(0, bid1.open_qty());

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1250, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 1, 200));
}

BOOST_AUTO_TEST_CASE(TestBboReplaceSizeDecreaseTooMuch)
{
  SimpleOrderBook order_book;
  SimpleOrder ask1(false, 1252, 200);
  SimpleOrder ask0(false, 1252, 300);
  SimpleOrder bid1(true,  1251, 100);
  SimpleOrder bid0(true,  1250, 100);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &bid0, false));
  BOOST_CHECK(add_and_verify(order_book, &bid1, false));
  BOOST_CHECK(add_and_verify(order_book, &ask0, false));
  BOOST_CHECK(add_and_verify(order_book, &ask1, false));

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(1251, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 2, 500));

  SimpleOrder cross_bid(true,  1252, 200);
  // Partial fill existing order
  {
    SimpleFillCheck fc1(&cross_bid, 200, 1252 * 200);
    SimpleFillCheck fc2(&ask0,      200, 1252 * 200);
    BOOST_CHECK(add_and_verify(order_book, &cross_bid, true, true));
  }

  // Verify open quantity
  BOOST_CHECK_EQUAL(100, ask0.open_qty());

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1251, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 2, 300));

  // Replace size - not enough left
  order_book.replace(&ask0, -150, PRICE_UNCHANGED);

  // Verify ask0 state
  BOOST_CHECK_EQUAL(0, ask0.open_qty());
  BOOST_CHECK_EQUAL(200, ask0.order_qty());
  BOOST_CHECK_EQUAL(simple::os_cancelled, ask0.state());

  // Verify depth unchanged
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1251, 1, 100));
  BOOST_CHECK(dc.verify_ask(1252, 1, 200));
}

BOOST_AUTO_TEST_CASE(TestBboReplaceSizeIncreaseDecrease)
{
  SimpleOrderBook order_book;
  SimpleOrder ask1(false, 1252, 200);
  SimpleOrder ask0(false, 1251, 300);
  SimpleOrder bid1(true,  1251, 100);
  SimpleOrder bid0(true,  1250, 100);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &bid0, false));
  BOOST_CHECK(add_and_verify(order_book, &ask0, false));
  BOOST_CHECK(add_and_verify(order_book, &ask1, false));

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(1250, 1, 100));
  BOOST_CHECK(dc.verify_ask(1251, 1, 300));

  // Replace size
  BOOST_CHECK(replace_and_verify(order_book, &ask0, 50));
  BOOST_CHECK(replace_and_verify(order_book, &bid0, 25));

  BOOST_CHECK(replace_and_verify(order_book, &ask0, -100));
  BOOST_CHECK(replace_and_verify(order_book, &bid0, 25));

  BOOST_CHECK(replace_and_verify(order_book, &ask0, 300));
  BOOST_CHECK(replace_and_verify(order_book, &bid0, -75));

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1250, 1, 75));
  BOOST_CHECK(dc.verify_ask(1251, 1, 550));
}

BOOST_AUTO_TEST_CASE(TestBboReplaceBidPriceChange)
{
  SimpleOrderBook order_book;
  SimpleOrder ask0(false, 1253, 300);
  SimpleOrder ask1(false, 1252, 200);
  SimpleOrder bid1(true,  1251, 140);
  SimpleOrder bid0(true,  1250, 120);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &bid0, false));
  BOOST_CHECK(add_and_verify(order_book, &bid1, false));
  BOOST_CHECK(add_and_verify(order_book, &ask0, false));
  BOOST_CHECK(add_and_verify(order_book, &ask1, false));

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(1251, 1, 140));
  BOOST_CHECK(dc.verify_ask(1252, 1, 200));

  // Replace price increase
  BOOST_CHECK(replace_and_verify(order_book, &bid0, SIZE_UNCHANGED, 1251));

  // Verify price change in book
  SimpleOrderBook::Bids::const_iterator bid = order_book.bids().begin();
  BOOST_CHECK_EQUAL(1251, bid->first);
  BOOST_CHECK_EQUAL(&bid1, bid->second.ptr());
  BOOST_CHECK_EQUAL(1251, (++bid)->first);
  BOOST_CHECK_EQUAL(&bid0, bid->second.ptr());
  BOOST_CHECK(order_book.bids().end() == ++bid);

  // Verify order
  BOOST_CHECK_EQUAL(1251, bid0.price());
  BOOST_CHECK_EQUAL(120, bid0.order_qty());

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1251, 2, 260));
  BOOST_CHECK(dc.verify_ask(1252, 1, 200));

  // Replace price decrease
  BOOST_CHECK(replace_and_verify(order_book, &bid1, SIZE_UNCHANGED, 1250));

  // Verify price change in book
  bid = order_book.bids().begin();
  BOOST_CHECK_EQUAL(1251, bid->first);
  BOOST_CHECK_EQUAL(&bid0, bid->second.ptr());
  BOOST_CHECK_EQUAL(1250, (++bid)->first);
  BOOST_CHECK_EQUAL(&bid1, bid->second.ptr());
  BOOST_CHECK(order_book.bids().end() == ++bid);

  // Verify order
  BOOST_CHECK_EQUAL(1250, bid1.price());
  BOOST_CHECK_EQUAL(140, bid1.order_qty());

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1251, 1, 120));
  BOOST_CHECK(dc.verify_ask(1252, 1, 200));
}

BOOST_AUTO_TEST_CASE(TestBboReplaceAskPriceChange)
{
  SimpleOrderBook order_book;
  ChangedChecker cc(order_book.depth());

  SimpleOrder ask0(false, 1253, 300);
  SimpleOrder ask1(false, 1252, 200);
  SimpleOrder bid1(true,  1251, 140);
  SimpleOrder bid0(true,  1250, 120);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &bid0, false));
  BOOST_CHECK(add_and_verify(order_book, &bid1, false));
  BOOST_CHECK(add_and_verify(order_book, &ask0, false));
  BOOST_CHECK(add_and_verify(order_book, &ask1, false));

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(1251, 1, 140));
  BOOST_CHECK(dc.verify_ask(1252, 1, 200));

  // Replace price increase 1252 -> 1253
  BOOST_CHECK(replace_and_verify(order_book, &ask1, SIZE_UNCHANGED, 1253));

  // Verify price change in book
  SimpleOrderBook::Asks::const_iterator ask = order_book.asks().begin();
  BOOST_CHECK_EQUAL(1253, ask->first);
  BOOST_CHECK_EQUAL(&ask0, ask->second.ptr());
  BOOST_CHECK_EQUAL(1253, (++ask)->first);
  BOOST_CHECK_EQUAL(&ask1, ask->second.ptr());
  BOOST_CHECK(order_book.asks().end() == ++ask);

  // Verify order
  BOOST_CHECK_EQUAL(1253, ask1.price());
  BOOST_CHECK_EQUAL(200, ask1.order_qty());

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1251, 1, 140));
  BOOST_CHECK(dc.verify_ask(1253, 2, 500));

  // Replace price decrease 1253 -> 1252
  BOOST_CHECK(replace_and_verify(order_book, &ask0, SIZE_UNCHANGED, 1252));

  // Verify price change in book
  ask = order_book.asks().begin();
  BOOST_CHECK_EQUAL(1252, ask->first);
  BOOST_CHECK_EQUAL(&ask0, ask->second.ptr());
  BOOST_CHECK_EQUAL(1253, (++ask)->first);
  BOOST_CHECK_EQUAL(&ask1, ask->second.ptr());
  BOOST_CHECK(order_book.asks().end() == ++ask);

  // Verify order
  BOOST_CHECK_EQUAL(1252, ask0.price());
  BOOST_CHECK_EQUAL(300, ask0.order_qty());

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(1251, 1, 140));
  BOOST_CHECK(dc.verify_ask(1252, 1, 300));
}
} // namespace
