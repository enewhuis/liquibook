// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include "assertiv/assertiv.h"
#include "ut_utils.h"
#include "changed_checker.h"
#include "book/order_book.h"
#include "impl/simple_order.h"

namespace liquibook {

using book::DepthLevel;
using book::OrderBook;
using book::OrderTracker;
using impl::SimpleOrder;

typedef OrderTracker<SimpleOrder*> SimpleTracker;
typedef test::ChangedChecker<5> ChangedChecker;

typedef FillCheck<SimpleOrder*> SimpleFillCheck;

TEST(TestBidsMultimapSortCorrect)
{
  SimpleOrderBook::Bids bids;
  SimpleOrder order0(true, 1250, 100);
  SimpleOrder order1(true, 1255, 100);
  SimpleOrder order2(true, 1240, 100);
  SimpleOrder order3(true,    0, 100);
  SimpleOrder order4(true, 1245, 100);

  // Insert out of price order
  bids.insert(std::make_pair(order0.price(), SimpleTracker(&order0)));
  bids.insert(std::make_pair(order1.price(), SimpleTracker(&order1)));
  bids.insert(std::make_pair(order2.price(), SimpleTracker(&order2)));
  bids.insert(std::make_pair(MARKET_ORDER_BID_SORT_PRICE, 
                             SimpleTracker(&order3)));
  bids.insert(std::make_pair(order4.price(), SimpleTracker(&order4)));
  
  // Should access in price order
  SimpleOrder* expected_order[] = {
    &order3, &order1, &order0, &order4, &order2
  };

  SimpleOrderBook::Bids::iterator bid;
  int index = 0;

  for (bid = bids.begin(); bid != bids.end(); ++bid, ++index) {
    if (expected_order[index]->price() == MARKET_ORDER_PRICE) {
      ASSERT_EQ(MARKET_ORDER_BID_SORT_PRICE, bid->first);
    } else {
      ASSERT_EQ(expected_order[index]->price(), bid->first);
    }
    ASSERT_EQ(expected_order[index], bid->second.ptr());
  }

  // Should be able to search and find
  ASSERT_TRUE((bids.upper_bound(1245))->second.ptr()->price() == 1240);
  ASSERT_TRUE((bids.lower_bound(1245))->second.ptr()->price() == 1245);
}

TEST(TestAsksMultimapSortCorrect)
{
  SimpleOrderBook::Asks asks;
  SimpleOrder order0(false, 3250, 100);
  SimpleOrder order1(false, 3235, 800);
  SimpleOrder order2(false, 3230, 200);
  SimpleOrder order3(false,    0, 200);
  SimpleOrder order4(false, 3245, 100);
  SimpleOrder order5(false, 3265, 200);

  // Insert out of price order
  asks.insert(std::make_pair(order0.price(), SimpleTracker(&order0)));
  asks.insert(std::make_pair(order1.price(), SimpleTracker(&order1)));
  asks.insert(std::make_pair(order2.price(), SimpleTracker(&order2)));
  asks.insert(std::make_pair(MARKET_ORDER_ASK_SORT_PRICE, 
                             SimpleTracker(&order3)));
  asks.insert(std::make_pair(order4.price(), SimpleTracker(&order4)));
  asks.insert(std::make_pair(order5.price(), SimpleTracker(&order5)));
  
  // Should access in price order
  SimpleOrder* expected_order[] = {
    &order3, &order2, &order1, &order4, &order0, &order5
  };

  SimpleOrderBook::Asks::iterator ask;
  int index = 0;

  for (ask = asks.begin(); ask != asks.end(); ++ask, ++index) {
    if (expected_order[index]->price() == MARKET_ORDER_PRICE) {
      ASSERT_EQ(MARKET_ORDER_ASK_SORT_PRICE, ask->first);
    } else {
      ASSERT_EQ(expected_order[index]->price(), ask->first);
    }
    ASSERT_EQ(expected_order[index], ask->second.ptr());
  }

  ASSERT_TRUE((asks.upper_bound(3235))->second.ptr()->price() == 3245);
  ASSERT_TRUE((asks.lower_bound(3235))->second.ptr()->price() == 3235);
}

TEST(TestAddCompleteBid)
{
  SimpleOrderBook order_book;
  SimpleOrder ask1(false, 1252, 100);
  SimpleOrder ask0(false, 1251, 100);
  SimpleOrder bid1(true,  1251, 100);
  SimpleOrder bid0(true,  1250, 100);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &bid0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask1, false));

  // Verify sizes
  ASSERT_EQ(1, order_book.bids().size());
  ASSERT_EQ(2, order_book.asks().size());

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));

  // Match - complete
  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&bid1, 100, 125100);
    SimpleFillCheck fc2(&ask0, 100, 125100);
    ASSERT_TRUE(add_and_verify(order_book, &bid1, true, true));
  ); }

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));

  // Verify sizes
  ASSERT_EQ(1, order_book.bids().size());
  ASSERT_EQ(1, order_book.asks().size());
}

TEST(TestAddCompleteAsk)
{
  SimpleOrderBook order_book;
  SimpleOrder ask0(false, 1251, 100);
  SimpleOrder ask1(false, 1250, 100);
  SimpleOrder bid0(true,  1250, 100);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &bid0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask0, false));

  // Verify sizes
  ASSERT_EQ(1, order_book.bids().size());
  ASSERT_EQ(1, order_book.asks().size());

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));

  // Match - complete
  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&ask1, 100, 125000);
    SimpleFillCheck fc2(&bid0, 100, 125000);
    ASSERT_TRUE(add_and_verify(order_book, &ask1, true, true));
  ); }

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(   0, 0,   0));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));

  // Verify sizes
  ASSERT_EQ(0, order_book.bids().size());
  ASSERT_EQ(1, order_book.asks().size());
}

TEST(TestAddMultiMatchBid)
{
  SimpleOrderBook order_book;
  SimpleOrder ask1(false, 1252, 100);
  SimpleOrder ask0(false, 1251, 300);
  SimpleOrder ask2(false, 1251, 200);
  SimpleOrder bid1(true,  1251, 500);
  SimpleOrder bid0(true,  1250, 100);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &bid0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask1, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask2, false));

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1251, 2, 500));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));

  // Verify sizes
  ASSERT_EQ(1, order_book.bids().size());
  ASSERT_EQ(3, order_book.asks().size());

  // Match - complete
  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&bid1, 500, 1251 * 500);
    SimpleFillCheck fc2(&ask2, 200, 1251 * 200);
    SimpleFillCheck fc3(&ask0, 300, 1251 * 300);
    ASSERT_TRUE(add_and_verify(order_book, &bid1, true, true));
  ); }

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));

  // Verify sizes
  ASSERT_EQ(1, order_book.bids().size());
  ASSERT_EQ(1, order_book.asks().size());

  // Verify remaining
  ASSERT_EQ(&ask1, order_book.asks().begin()->second.ptr());
}

TEST(TestAddMultiMatchAsk)
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
  ASSERT_TRUE(add_and_verify(order_book, &bid0, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid1, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid2, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask1, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask2, false));

  // Verify sizes
  ASSERT_EQ(3, order_book.bids().size());
  ASSERT_EQ(3, order_book.asks().size());

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(9250, 2, 600));
  ASSERT_TRUE(dc.verify_bid(9248, 1, 100));
  ASSERT_TRUE(dc.verify_ask(9251, 2, 500));
  ASSERT_TRUE(dc.verify_ask(9252, 1, 100));

  // Match - complete
  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&ask3, 600, 9250 * 600);
    SimpleFillCheck fc2(&bid0, 100, 9250 * 100);
    SimpleFillCheck fc3(&bid1, 500, 9250 * 500);
    ASSERT_TRUE(add_and_verify(order_book, &ask3, true, true));
  ); }

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(9248, 1, 100));
  ASSERT_TRUE(dc.verify_ask(9251, 2, 500));
  ASSERT_TRUE(dc.verify_ask(9252, 1, 100));

  // Verify sizes
  ASSERT_EQ(1, order_book.bids().size());
  ASSERT_EQ(3, order_book.asks().size());

  // Verify remaining
  ASSERT_EQ(&bid2, order_book.bids().begin()->second.ptr());
}

TEST(TestAddPartialMatchBid)
{
  SimpleOrderBook order_book;
  SimpleOrder ask0(false, 7253, 300);
  SimpleOrder ask1(false, 7252, 100);
  SimpleOrder ask2(false, 7251, 200);
  SimpleOrder bid1(true,  7251, 350);
  SimpleOrder bid0(true,  7250, 100);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &bid0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask1, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask2, false));

  // Verify sizes
  ASSERT_EQ(1, order_book.bids().size());
  ASSERT_EQ(3, order_book.asks().size());

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(7250, 1, 100));
  ASSERT_TRUE(dc.verify_ask(7251, 1, 200));
  ASSERT_TRUE(dc.verify_ask(7252, 1, 100));
  ASSERT_TRUE(dc.verify_ask(7253, 1, 300));

  // Match - partial
  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&bid1, 200, 7251 * 200);
    SimpleFillCheck fc2(&ask2, 200, 7251 * 200);
    ASSERT_TRUE(add_and_verify(order_book, &bid1, true, false));
  ); }

  // Verify sizes
  ASSERT_EQ(2, order_book.bids().size());
  ASSERT_EQ(2, order_book.asks().size());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(7251, 1, 150));
  ASSERT_TRUE(dc.verify_bid(7250, 1, 100));
  ASSERT_TRUE(dc.verify_ask(7252, 1, 100));
  ASSERT_TRUE(dc.verify_ask(7253, 1, 300));

  // Verify remaining
  ASSERT_EQ(&ask1, order_book.asks().begin()->second.ptr());
  ASSERT_EQ(&bid1, order_book.bids().begin()->second.ptr());
}

TEST(TestAddPartialMatchAsk)
{
  SimpleOrderBook order_book;
  SimpleOrder ask0(false, 1253, 300);
  SimpleOrder ask1(false, 1251, 400);
  SimpleOrder bid1(true,  1251, 350);
  SimpleOrder bid0(true,  1250, 100);
  SimpleOrder bid2(true,  1250, 200);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &bid0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask0, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid1, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid2, false));

  // Verify sizes
  ASSERT_EQ(3, order_book.bids().size());
  ASSERT_EQ(1, order_book.asks().size());

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1251, 1, 350));
  ASSERT_TRUE(dc.verify_bid(1250, 2, 300));
  ASSERT_TRUE(dc.verify_ask(1253, 1, 300));

  // Match - partial
  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&ask1, 350, 1251 * 350);
    SimpleFillCheck fc2(&bid1, 350, 1251 * 350);
    ASSERT_TRUE(add_and_verify(order_book, &ask1,  true, false));
  ); }


  // Verify sizes
  ASSERT_EQ(2, order_book.bids().size());
  ASSERT_EQ(2, order_book.asks().size());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1250, 2, 300));
  ASSERT_TRUE(dc.verify_ask(1251, 1,  50));
  ASSERT_TRUE(dc.verify_ask(1253, 1, 300));

  // Verify remaining
  ASSERT_EQ(&bid0, order_book.bids().begin()->second.ptr());
  ASSERT_EQ(&ask1, order_book.asks().begin()->second.ptr());
}

TEST(TestAddMultiPartialMatchBid)
{
  SimpleOrderBook order_book;
  SimpleOrder ask1(false, 1252, 100);
  SimpleOrder ask2(false, 1251, 200);
  SimpleOrder ask0(false, 1251, 300);
  SimpleOrder bid1(true,  1251, 750);
  SimpleOrder bid0(true,  1250, 100);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &bid0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask1, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask2, false));

  // Verify sizes
  ASSERT_EQ(1, order_book.bids().size());
  ASSERT_EQ(3, order_book.asks().size());

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1251, 2, 500));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));

  // Match - partial
  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&bid1, 500, 1251 * 500);
    SimpleFillCheck fc2(&ask0, 300, 1251 * 300);
    SimpleFillCheck fc3(&ask2, 200, 1251 * 200);
    ASSERT_TRUE(add_and_verify(order_book, &bid1, true, false));
  ); }

  // Verify sizes
  ASSERT_EQ(2, order_book.bids().size());
  ASSERT_EQ(1, order_book.asks().size());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1251, 1, 250));
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));

  // Verify remaining
  ASSERT_EQ(&ask1, order_book.asks().begin()->second.ptr());
  ASSERT_EQ(&bid1, order_book.bids().begin()->second.ptr());
}

TEST(TestAddMultiPartialMatchAsk)
{
  SimpleOrderBook order_book;
  SimpleOrder ask0(false, 1253, 300);
  SimpleOrder ask1(false, 1251, 700);
  SimpleOrder bid1(true,  1251, 370);
  SimpleOrder bid2(true,  1251, 200);
  SimpleOrder bid0(true,  1250, 100);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &bid0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask0, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid1, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid2, false));

  // Verify sizes
  ASSERT_EQ(3, order_book.bids().size());
  ASSERT_EQ(1, order_book.asks().size());

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1251, 2, 570));
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1253, 1, 300));

  // Match - partial
  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&ask1, 570, 1251 * 570);
    SimpleFillCheck fc2(&bid1, 370, 1251 * 370);
    SimpleFillCheck fc3(&bid2, 200, 1251 * 200);
    ASSERT_TRUE(add_and_verify(order_book, &ask1,  true, false));
  ); }

  // Verify sizes
  ASSERT_EQ(1, order_book.bids().size());
  ASSERT_EQ(2, order_book.asks().size());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 130));
  ASSERT_TRUE(dc.verify_ask(1253, 1, 300));

  // Verify remaining
  ASSERT_EQ(&bid0, order_book.bids().begin()->second.ptr());
  ASSERT_EQ(100, order_book.bids().begin()->second.open_qty());
  ASSERT_EQ(&ask1, order_book.asks().begin()->second.ptr());
  ASSERT_EQ(130, order_book.asks().begin()->second.open_qty());
}

TEST(TestRepeatMatchBid)
{
  SimpleOrderBook order_book;
  SimpleOrder ask3(false, 1251, 400);
  SimpleOrder ask2(false, 1251, 200);
  SimpleOrder ask1(false, 1251, 300);
  SimpleOrder ask0(false, 1251, 100);
  SimpleOrder bid1(true,  1251, 900);
  SimpleOrder bid0(true,  1250, 100);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &bid0, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid1, false));

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1251, 1, 900));
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));

  // Match - repeated
  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&bid1, 100, 125100);
    SimpleFillCheck fc2(&ask0, 100, 125100);
    ASSERT_TRUE(add_and_verify(order_book, &ask0, true, true));
  ); }

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1251, 1, 800));
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));

  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&bid1, 300, 1251 * 300);
    SimpleFillCheck fc2(&ask1, 300, 1251 * 300);
    ASSERT_TRUE(add_and_verify(order_book, &ask1, true, true));
  ); }

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1251, 1, 500));
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));

  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&bid1, 200, 1251 * 200);
    SimpleFillCheck fc2(&ask2, 200, 1251 * 200);
    ASSERT_TRUE(add_and_verify(order_book, &ask2, true, true));
  ); }

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1251, 1, 300));
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));

  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&bid1, 300, 1251 * 300);
    SimpleFillCheck fc2(&ask3, 300, 1251 * 300);
    ASSERT_TRUE(add_and_verify(order_book, &ask3, true));
  ); }

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));

  // Verify sizes
  ASSERT_EQ(1, order_book.bids().size());
  ASSERT_EQ(1, order_book.asks().size());
}

TEST(TestRepeatMatchAsk)
{
  SimpleOrderBook order_book;
  SimpleOrder ask0(false,  1252, 100);
  SimpleOrder ask1(false,  1251, 900);
  SimpleOrder bid0(true, 1251, 100);
  SimpleOrder bid1(true, 1251, 300);
  SimpleOrder bid2(true, 1251, 200);
  SimpleOrder bid3(true, 1251, 400);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &ask0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask1, false));

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_ask(1251, 1, 900));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));

  ASSERT_EQ(&ask1, order_book.asks().begin()->second.ptr());

  // Match - repeated
  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&ask1, 100, 125100);
    SimpleFillCheck fc2(&bid0, 100, 125100);
    ASSERT_TRUE(add_and_verify(order_book, &bid0, true, true));
  ); }

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_ask(1251, 1, 800));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));

  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&ask1, 300, 1251 * 300);
    SimpleFillCheck fc2(&bid1, 300, 1251 * 300);
    ASSERT_TRUE(add_and_verify(order_book, &bid1, true, true));
  ); }

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_ask(1251, 1, 500));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));

  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&ask1, 200, 1251 * 200);
    SimpleFillCheck fc2(&bid2, 200, 1251 * 200);
    ASSERT_TRUE(add_and_verify(order_book, &bid2, true, true));
  ); }

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_ask(1251, 1, 300));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));

  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&ask1, 300, 1251 * 300);
    SimpleFillCheck fc2(&bid3, 300, 1251 * 300);
    ASSERT_TRUE(add_and_verify(order_book, &bid3, true));
  ); }

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1251, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));

  // Verify sizes
  ASSERT_EQ(1, order_book.bids().size());
  ASSERT_EQ(1, order_book.asks().size());
}

TEST(TestAddMarketOrderBid)
{
  SimpleOrderBook order_book;
  SimpleOrder ask1(false, 1252, 100);
  SimpleOrder ask0(false, 1251, 100);
  SimpleOrder bid1(true,     0, 100);
  SimpleOrder bid0(true,  1250, 100);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &bid0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask1, false));

  // Verify sizes
  ASSERT_EQ(1, order_book.bids().size());
  ASSERT_EQ(2, order_book.asks().size());

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));

  // Match - complete
  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&bid1, 100, 125100);
    SimpleFillCheck fc2(&ask0, 100, 125100);
    ASSERT_TRUE(add_and_verify(order_book, &bid1, true, true));
  ); }

  // Verify sizes
  ASSERT_EQ(1, order_book.bids().size());
  ASSERT_EQ(1, order_book.asks().size());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));
}

TEST(TestAddMarketOrderAsk)
{
  SimpleOrderBook order_book;
  SimpleOrder ask0(false, 1252, 100);
  SimpleOrder ask1(false,    0, 100);
  SimpleOrder bid1(true,  1251, 100);
  SimpleOrder bid0(true,  1250, 100);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &bid0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask0, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid1, false));

  // Verify sizes
  ASSERT_EQ(2, order_book.bids().size());
  ASSERT_EQ(1, order_book.asks().size());

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1251, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));

  // Match - complete
  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&bid1, 100, 125100);
    SimpleFillCheck fc2(&ask1, 100, 125100);
    ASSERT_TRUE(add_and_verify(order_book, &ask1, true, true));
  ); }

  // Verify sizes
  ASSERT_EQ(1, order_book.bids().size());
  ASSERT_EQ(1, order_book.asks().size());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));
}

TEST(TestAddMarketOrderBidMultipleMatch)
{
  SimpleOrderBook order_book;
  SimpleOrder ask1(false, 12520, 300);
  SimpleOrder ask0(false, 12510, 200);
  SimpleOrder bid1(true,      0, 500);
  SimpleOrder bid0(true,  12500, 100);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &bid0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask1, false));

  // Verify sizes
  ASSERT_EQ(1, order_book.bids().size());
  ASSERT_EQ(2, order_book.asks().size());

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(12500, 1, 100));
  ASSERT_TRUE(dc.verify_ask(12510, 1, 200));
  ASSERT_TRUE(dc.verify_ask(12520, 1, 300));

  // Match - complete
  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&bid1, 500, 12510 * 200 + 12520 * 300);
    SimpleFillCheck fc2(&ask0, 200, 12510 * 200);
    SimpleFillCheck fc3(&ask1, 300, 12520 * 300);
    ASSERT_TRUE(add_and_verify(order_book, &bid1, true, true));
  ); }

  // Verify sizes
  ASSERT_EQ(1, order_book.bids().size());
  ASSERT_EQ(0, order_book.asks().size());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(12500, 1, 100));
  ASSERT_TRUE(dc.verify_ask(    0, 0,   0));
}

TEST(TestAddMarketOrderAskMultipleMatch)
{
  SimpleOrderBook order_book;
  SimpleOrder ask0(false, 12520, 100);
  SimpleOrder ask1(false,     0, 600);
  SimpleOrder bid1(true,  12510, 200);
  SimpleOrder bid0(true,  12500, 400);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &bid0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask0, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid1, false));

  // Verify sizes
  ASSERT_EQ(2, order_book.bids().size());
  ASSERT_EQ(1, order_book.asks().size());

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(12510, 1, 200));
  ASSERT_TRUE(dc.verify_bid(12500, 1, 400));
  ASSERT_TRUE(dc.verify_ask(12520, 1, 100));

  // Match - complete
  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&bid0, 400, 12500 * 400);
    SimpleFillCheck fc2(&bid1, 200, 12510 * 200);
    SimpleFillCheck fc3(&ask1, 600, 12500 * 400 + 12510 * 200);
    ASSERT_TRUE(add_and_verify(order_book, &ask1, true, true));
  ); }

  // Verify sizes
  ASSERT_EQ(0, order_book.bids().size());
  ASSERT_EQ(1, order_book.asks().size());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(    0, 0,   0));
  ASSERT_TRUE(dc.verify_ask(12520, 1, 100));
}

TEST(TestMatchMarketOrderBid)
{
  SimpleOrderBook order_book;
  SimpleOrder ask0(false, 1253, 100);
  SimpleOrder bid1(true,     0, 100);
  SimpleOrder bid0(true,  1250, 100);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &bid0, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid1, false));

  // Verify sizes
  ASSERT_EQ(2, order_book.bids().size());
  ASSERT_EQ(0, order_book.asks().size());

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));
  ASSERT_TRUE(dc.verify_bid(   0, 0,   0));
  ASSERT_TRUE(dc.verify_ask(   0, 0,   0));

  // Match - complete
  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&bid1, 100, 125300);
    SimpleFillCheck fc2(&ask0, 100, 125300);
    ASSERT_TRUE(add_and_verify(order_book, &ask0, true, true));
  ); }

  // Verify sizes
  ASSERT_EQ(1, order_book.bids().size());
  ASSERT_EQ(0, order_book.asks().size());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));
  ASSERT_TRUE(dc.verify_ask(   0, 0,   0));
}

TEST(TestMatchMarketOrderAsk)
{
  SimpleOrderBook order_book;
  SimpleOrder ask0(false, 1252, 100);
  SimpleOrder ask1(false,    0, 100);
  SimpleOrder bid0(true,  1250, 100);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &ask0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask1, false));

  // Verify sizes
  ASSERT_EQ(0, order_book.bids().size());
  ASSERT_EQ(2, order_book.asks().size());

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));
  ASSERT_TRUE(dc.verify_ask(   0, 0,   0));
  ASSERT_TRUE(dc.verify_bid(   0, 0,   0));

  // Match - complete
  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&bid0, 100, 125000);
    SimpleFillCheck fc2(&ask1, 100, 125000);
    ASSERT_TRUE(add_and_verify(order_book, &bid0, true, true));
  ); }

  // Verify sizes
  ASSERT_EQ(0, order_book.bids().size());
  ASSERT_EQ(1, order_book.asks().size());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));
  ASSERT_TRUE(dc.verify_ask(   0, 0,   0));
  ASSERT_TRUE(dc.verify_bid(   0, 0,   0));
}

TEST(TestMatchMultipleMarketOrderBid)
{
  SimpleOrderBook order_book;
  SimpleOrder ask0(false, 1253, 400);
  SimpleOrder bid1(true,     0, 100);
  SimpleOrder bid2(true,     0, 200);
  SimpleOrder bid0(true,  1250, 100);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &bid0, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid1, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid2, false));

  // Verify sizes
  ASSERT_EQ(3, order_book.bids().size());
  ASSERT_EQ(0, order_book.asks().size());

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_ask(   0, 0,   0));
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));
  ASSERT_TRUE(dc.verify_bid(   0, 0,   0));

  // Match - complete
  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&bid1, 100, 1253 * 100);
    SimpleFillCheck fc2(&bid2, 200, 1253 * 200);
    SimpleFillCheck fc3(&ask0, 300, 1253 * 300);
    ASSERT_TRUE(add_and_verify(order_book, &ask0, true, false));
  ); }

  // Verify sizes
  ASSERT_EQ(1, order_book.bids().size());
  ASSERT_EQ(1, order_book.asks().size());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_ask(1253, 1, 100));
  ASSERT_TRUE(dc.verify_ask(   0, 0,   0));
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));
  ASSERT_TRUE(dc.verify_bid(   0, 0,   0));
}


TEST(TestMatchMultipleMarketOrderAsk)
{
  SimpleOrderBook order_book;
  SimpleOrder ask0(false, 1252, 100);
  SimpleOrder ask2(false,    0, 400);
  SimpleOrder ask1(false,    0, 100);
  SimpleOrder bid0(true,  1250, 300);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &ask0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask1, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask2, false));

  // Verify sizes
  ASSERT_EQ(0, order_book.bids().size());
  ASSERT_EQ(3, order_book.asks().size());

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));
  ASSERT_TRUE(dc.verify_ask(   0, 0,   0));
  ASSERT_TRUE(dc.verify_bid(   0, 0,   0));

  // Match - partiaL
  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&bid0, 300, 1250 * 300);
    SimpleFillCheck fc2(&ask1, 100, 1250 * 100);
    SimpleFillCheck fc3(&ask2, 200, 1250 * 200);
    ASSERT_TRUE(add_and_verify(order_book, &bid0, true, true));
  ); }

  // Verify sizes
  ASSERT_EQ(0, order_book.bids().size());
  ASSERT_EQ(2, order_book.asks().size());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));
  ASSERT_TRUE(dc.verify_ask(   0, 0,   0));
  ASSERT_TRUE(dc.verify_bid(   0, 0,   0));
}

TEST(TestCancelBid)
{
  SimpleOrderBook order_book;
  SimpleOrder ask2(false, 1252, 100);
  SimpleOrder ask1(false, 1251, 100);
  SimpleOrder ask0(false, 1251, 100);
  SimpleOrder bid0(true,  1250, 100);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &bid0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask1, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask2, false));

  // Verify sizes
  ASSERT_EQ(1, order_book.bids().size());
  ASSERT_EQ(3, order_book.asks().size());

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1251, 2, 200));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));

  // Cancel bid
  ASSERT_TRUE(cancel_and_verify(order_book, &bid0, impl::os_cancelled));

  // Cancel correctness
  ASSERT_TRUE(cancel_and_verify(order_book, &ask1, impl::os_cancelled));
  ASSERT_TRUE(cancel_and_verify(order_book, &ask1, impl::os_cancelled));

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(0,    0,   0));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));

  // Verify sizes
  ASSERT_EQ(0, order_book.bids().size());
  ASSERT_EQ(2, order_book.asks().size());
}

TEST(TestCancelAskAndMatch)
{
  SimpleOrderBook order_book;
  SimpleOrder ask1(false, 1252, 100);
  SimpleOrder ask0(false, 1251, 100);
  SimpleOrder bid2(true,  1252, 100);
  SimpleOrder bid0(true,  1250, 100);
  SimpleOrder bid1(true,  1250, 100);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &bid0, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid1, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask1, false));

  // Verify sizes
  ASSERT_EQ(2, order_book.bids().size());
  ASSERT_EQ(2, order_book.asks().size());

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1250, 2, 200));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));

  // Cancel bid
  ASSERT_TRUE(cancel_and_verify(order_book, &ask0, impl::os_cancelled));

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1250, 2, 200));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));
  ASSERT_TRUE(dc.verify_ask(   0, 0,   0));

  // Match - partiaL
  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&bid2, 100, 1252 * 100);
    SimpleFillCheck fc2(&ask1, 100, 1252 * 100);
    ASSERT_TRUE(add_and_verify(order_book, &bid2, true, true));
  ); }

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1250, 2, 200));
  ASSERT_TRUE(dc.verify_ask(   0, 0,   0));

  // Cancel bid
  ASSERT_TRUE(cancel_and_verify(order_book, &bid0, impl::os_cancelled));

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));
  ASSERT_TRUE(dc.verify_ask(   0, 0,   0));

  // Verify sizes
  ASSERT_EQ(1, order_book.bids().size());
  ASSERT_EQ(0, order_book.asks().size());
}

TEST(TestCancelBidFail)
{
  SimpleOrderBook order_book;
  SimpleOrder ask0(false, 1251, 100);
  SimpleOrder ask1(false, 1250, 100);
  SimpleOrder bid0(true,  1250, 100);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &bid0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask0, false));

  // Verify sizes
  ASSERT_EQ(1, order_book.bids().size());
  ASSERT_EQ(1, order_book.asks().size());

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));
  ASSERT_TRUE(dc.verify_ask(   0, 0,   0));
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));
  ASSERT_TRUE(dc.verify_bid(   0, 0,   0));

  // Match - complete
  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&ask1, 100, 125000);
    SimpleFillCheck fc2(&bid0, 100, 125000);
    ASSERT_TRUE(add_and_verify(order_book, &ask1, true, true));
  ); }

  // Verify sizes
  ASSERT_EQ(0, order_book.bids().size());
  ASSERT_EQ(1, order_book.asks().size());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));
  ASSERT_TRUE(dc.verify_ask(   0, 0,   0));
  ASSERT_TRUE(dc.verify_bid(   0, 0,   0));

  // Cancel a filled order
  ASSERT_TRUE(cancel_and_verify(order_book, &bid0, impl::os_complete));

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));
  ASSERT_TRUE(dc.verify_ask(   0, 0,   0));
  ASSERT_TRUE(dc.verify_bid(   0, 0,   0));
}

TEST(TestCancelAskFail)
{
  SimpleOrderBook order_book;
  SimpleOrder ask1(false, 1252, 100);
  SimpleOrder ask0(false, 1251, 100);
  SimpleOrder bid1(true,  1251, 100);
  SimpleOrder bid0(true,  1250, 100);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &bid0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask1, false));

  // Verify sizes
  ASSERT_EQ(1, order_book.bids().size());
  ASSERT_EQ(2, order_book.asks().size());

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));
  ASSERT_TRUE(dc.verify_ask(   0, 0,   0));
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));
  ASSERT_TRUE(dc.verify_bid(   0, 0,   0));

  // Match - complete
  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&bid1, 100, 125100);
    SimpleFillCheck fc2(&ask0, 100, 125100);
    ASSERT_TRUE(add_and_verify(order_book, &bid1, true, true));
  ); }

  // Verify sizes
  ASSERT_EQ(1, order_book.bids().size());
  ASSERT_EQ(1, order_book.asks().size());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));
  ASSERT_TRUE(dc.verify_ask(   0, 0,   0));
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));
  ASSERT_TRUE(dc.verify_bid(   0, 0,   0));

  // Cancel a filled order
  ASSERT_TRUE(cancel_and_verify(order_book, &ask0, impl::os_complete));

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));
  ASSERT_TRUE(dc.verify_ask(   0, 0,   0));
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));
  ASSERT_TRUE(dc.verify_bid(   0, 0,   0));
}

TEST(TestCancelBidRestore)
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
  ASSERT_TRUE(add_and_verify(order_book, &ask0,  false));
  ASSERT_TRUE(add_and_verify(order_book, &ask1,  false));
  ASSERT_TRUE(add_and_verify(order_book, &ask2,  false));
  ASSERT_TRUE(add_and_verify(order_book, &ask3,  false));
  ASSERT_TRUE(add_and_verify(order_book, &ask4,  false));
  ASSERT_TRUE(add_and_verify(order_book, &ask5,  false));
  ASSERT_TRUE(add_and_verify(order_book, &ask6,  false));
  ASSERT_TRUE(add_and_verify(order_book, &ask7,  false));
  ASSERT_TRUE(add_and_verify(order_book, &ask8,  false));
  ASSERT_TRUE(add_and_verify(order_book, &ask9,  false));
  ASSERT_TRUE(add_and_verify(order_book, &ask10, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid0,  false));
  ASSERT_TRUE(add_and_verify(order_book, &bid1,  false));
  ASSERT_TRUE(add_and_verify(order_book, &bid2,  false));
  ASSERT_TRUE(add_and_verify(order_book, &bid3,  false));
  ASSERT_TRUE(add_and_verify(order_book, &bid4,  false));
  ASSERT_TRUE(add_and_verify(order_book, &bid5,  false));
  ASSERT_TRUE(add_and_verify(order_book, &bid6,  false));
  ASSERT_TRUE(add_and_verify(order_book, &bid7,  false));
  ASSERT_TRUE(add_and_verify(order_book, &bid8,  false));
  ASSERT_TRUE(add_and_verify(order_book, &bid9,  false));
  ASSERT_TRUE(add_and_verify(order_book, &bid10, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid11, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid12, false));

  // Verify sizes
  ASSERT_EQ(13, order_book.bids().size());
  ASSERT_EQ(11, order_book.asks().size());

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1249, 3,  500));
  ASSERT_TRUE(dc.verify_bid(1248, 1,  400));
  ASSERT_TRUE(dc.verify_bid(1246, 2, 1100));
  ASSERT_TRUE(dc.verify_bid(1245, 3,  500));
  ASSERT_TRUE(dc.verify_bid(1244, 2, 1000));
  ASSERT_TRUE(dc.verify_ask(1250, 1,  500));
  ASSERT_TRUE(dc.verify_ask(1251, 1,  400));
  ASSERT_TRUE(dc.verify_ask(1252, 2,  300));
  ASSERT_TRUE(dc.verify_ask(1254, 1,  300));
  ASSERT_TRUE(dc.verify_ask(1255, 2,  700));

  // Cancel a bid level (erase)
  ASSERT_TRUE(cancel_and_verify(order_book, &bid3, impl::os_cancelled));

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1249, 3,  500));
  ASSERT_TRUE(dc.verify_bid(1246, 2, 1100));
  ASSERT_TRUE(dc.verify_bid(1245, 3,  500));
  ASSERT_TRUE(dc.verify_bid(1244, 2, 1000));
  ASSERT_TRUE(dc.verify_bid(1242, 1,  300)); // Restored
  ASSERT_TRUE(dc.verify_ask(1250, 1,  500));
  ASSERT_TRUE(dc.verify_ask(1251, 1,  400));
  ASSERT_TRUE(dc.verify_ask(1252, 2,  300));
  ASSERT_TRUE(dc.verify_ask(1254, 1,  300));
  ASSERT_TRUE(dc.verify_ask(1255, 2,  700));
  
  // Cancel common bid levels (not erased)
  ASSERT_TRUE(cancel_and_verify(order_book, &bid7, impl::os_cancelled));
  ASSERT_TRUE(cancel_and_verify(order_book, &bid4, impl::os_cancelled));

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1249, 3,  500));
  ASSERT_TRUE(dc.verify_bid(1246, 1,  500)); // Cxl 600
  ASSERT_TRUE(dc.verify_bid(1245, 2,  400)); // Cxl 100
  ASSERT_TRUE(dc.verify_bid(1244, 2, 1000));
  ASSERT_TRUE(dc.verify_bid(1242, 1,  300));
  ASSERT_TRUE(dc.verify_ask(1250, 1,  500));
  ASSERT_TRUE(dc.verify_ask(1251, 1,  400));
  ASSERT_TRUE(dc.verify_ask(1252, 2,  300));
  ASSERT_TRUE(dc.verify_ask(1254, 1,  300));
  ASSERT_TRUE(dc.verify_ask(1255, 2,  700));

  // Cancel the best bid level (erased)
  ASSERT_TRUE(cancel_and_verify(order_book, &bid1, impl::os_cancelled));
  ASSERT_TRUE(cancel_and_verify(order_book, &bid0, impl::os_cancelled));
  ASSERT_TRUE(cancel_and_verify(order_book, &bid2, impl::os_cancelled));

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1246, 1,  500));
  ASSERT_TRUE(dc.verify_bid(1245, 2,  400));
  ASSERT_TRUE(dc.verify_bid(1244, 2, 1000));
  ASSERT_TRUE(dc.verify_bid(1242, 1,  300));
  ASSERT_TRUE(dc.verify_bid(1241, 1,  400));
  ASSERT_TRUE(dc.verify_ask(1250, 1,  500));
  ASSERT_TRUE(dc.verify_ask(1251, 1,  400));
  ASSERT_TRUE(dc.verify_ask(1252, 2,  300));
  ASSERT_TRUE(dc.verify_ask(1254, 1,  300));
  ASSERT_TRUE(dc.verify_ask(1255, 2,  700));
}

TEST(TestCancelAskRestore)
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
  ASSERT_TRUE(add_and_verify(order_book, &ask0,  false));
  ASSERT_TRUE(add_and_verify(order_book, &ask1,  false));
  ASSERT_TRUE(add_and_verify(order_book, &ask2,  false));
  ASSERT_TRUE(add_and_verify(order_book, &ask3,  false));
  ASSERT_TRUE(add_and_verify(order_book, &ask4,  false));
  ASSERT_TRUE(add_and_verify(order_book, &ask5,  false));
  ASSERT_TRUE(add_and_verify(order_book, &ask6,  false));
  ASSERT_TRUE(add_and_verify(order_book, &ask7,  false));
  ASSERT_TRUE(add_and_verify(order_book, &ask8,  false));
  ASSERT_TRUE(add_and_verify(order_book, &ask9,  false));
  ASSERT_TRUE(add_and_verify(order_book, &ask10, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid0,  false));
  ASSERT_TRUE(add_and_verify(order_book, &bid1,  false));
  ASSERT_TRUE(add_and_verify(order_book, &bid2,  false));
  ASSERT_TRUE(add_and_verify(order_book, &bid3,  false));
  ASSERT_TRUE(add_and_verify(order_book, &bid4,  false));
  ASSERT_TRUE(add_and_verify(order_book, &bid5,  false));
  ASSERT_TRUE(add_and_verify(order_book, &bid6,  false));
  ASSERT_TRUE(add_and_verify(order_book, &bid7,  false));
  ASSERT_TRUE(add_and_verify(order_book, &bid8,  false));
  ASSERT_TRUE(add_and_verify(order_book, &bid9,  false));
  ASSERT_TRUE(add_and_verify(order_book, &bid10, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid11, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid12, false));

  // Verify sizes
  ASSERT_EQ(13, order_book.bids().size());
  ASSERT_EQ(11, order_book.asks().size());

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1249, 3,  500));
  ASSERT_TRUE(dc.verify_bid(1248, 1,  400));
  ASSERT_TRUE(dc.verify_bid(1246, 2, 1100));
  ASSERT_TRUE(dc.verify_bid(1245, 3,  500));
  ASSERT_TRUE(dc.verify_bid(1244, 2, 1000));
  ASSERT_TRUE(dc.verify_ask(1250, 1,  500));
  ASSERT_TRUE(dc.verify_ask(1251, 1,  400));
  ASSERT_TRUE(dc.verify_ask(1252, 2,  300));
  ASSERT_TRUE(dc.verify_ask(1254, 1,  300));
  ASSERT_TRUE(dc.verify_ask(1255, 2,  700));

  // Cancel an ask level (erase)
  ASSERT_TRUE(cancel_and_verify(order_book, &ask1, impl::os_cancelled));

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1249, 3,  500));
  ASSERT_TRUE(dc.verify_bid(1248, 1,  400));
  ASSERT_TRUE(dc.verify_bid(1246, 2, 1100));
  ASSERT_TRUE(dc.verify_bid(1245, 3,  500));
  ASSERT_TRUE(dc.verify_bid(1244, 2, 1000));
  ASSERT_TRUE(dc.verify_ask(1250, 1,  500));
  ASSERT_TRUE(dc.verify_ask(1252, 2,  300));
  ASSERT_TRUE(dc.verify_ask(1254, 1,  300));
  ASSERT_TRUE(dc.verify_ask(1255, 2,  700));
  ASSERT_TRUE(dc.verify_ask(1256, 2,  200)); // Restored

  // Cancel common ask levels (not erased)
  ASSERT_TRUE(cancel_and_verify(order_book, &ask2, impl::os_cancelled));
  ASSERT_TRUE(cancel_and_verify(order_book, &ask6, impl::os_cancelled));

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1249, 3,  500));
  ASSERT_TRUE(dc.verify_bid(1248, 1,  400));
  ASSERT_TRUE(dc.verify_bid(1246, 2, 1100));
  ASSERT_TRUE(dc.verify_bid(1245, 3,  500));
  ASSERT_TRUE(dc.verify_bid(1244, 2, 1000));
  ASSERT_TRUE(dc.verify_ask(1250, 1,  500));
  ASSERT_TRUE(dc.verify_ask(1252, 1,  200)); // Cxl 100
  ASSERT_TRUE(dc.verify_ask(1254, 1,  300));
  ASSERT_TRUE(dc.verify_ask(1255, 1,  200)); // Cxl 500
  ASSERT_TRUE(dc.verify_ask(1256, 2,  200));

  // Cancel the best ask level (erased)
  ASSERT_TRUE(cancel_and_verify(order_book, &ask0, impl::os_cancelled));

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1249, 3,  500));
  ASSERT_TRUE(dc.verify_bid(1248, 1,  400));
  ASSERT_TRUE(dc.verify_bid(1246, 2, 1100));
  ASSERT_TRUE(dc.verify_bid(1245, 3,  500));
  ASSERT_TRUE(dc.verify_bid(1244, 2, 1000));
  ASSERT_TRUE(dc.verify_ask(1252, 1,  200));
  ASSERT_TRUE(dc.verify_ask(1254, 1,  300));
  ASSERT_TRUE(dc.verify_ask(1255, 1,  200));
  ASSERT_TRUE(dc.verify_ask(1256, 2,  200));
  ASSERT_TRUE(dc.verify_ask(1257, 1,  700)); // Restored
}

TEST(TestFillCompleteBidRestoreDepth)
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
  ASSERT_TRUE(add_and_verify(order_book, &ask0,  false));
  ASSERT_TRUE(add_and_verify(order_book, &ask1,  false));
  ASSERT_TRUE(add_and_verify(order_book, &ask2,  false));
  ASSERT_TRUE(add_and_verify(order_book, &ask3,  false));
  ASSERT_TRUE(add_and_verify(order_book, &ask4,  false));
  ASSERT_TRUE(add_and_verify(order_book, &ask5,  false));
  ASSERT_TRUE(add_and_verify(order_book, &ask6,  false));
  ASSERT_TRUE(add_and_verify(order_book, &ask7,  false));
  ASSERT_TRUE(add_and_verify(order_book, &ask8,  false));
  ASSERT_TRUE(add_and_verify(order_book, &ask9,  false));
  ASSERT_TRUE(add_and_verify(order_book, &ask10, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid0,  false));
  ASSERT_TRUE(add_and_verify(order_book, &bid1,  false));
  ASSERT_TRUE(add_and_verify(order_book, &bid2,  false));
  ASSERT_TRUE(add_and_verify(order_book, &bid3,  false));
  ASSERT_TRUE(add_and_verify(order_book, &bid4,  false));
  ASSERT_TRUE(add_and_verify(order_book, &bid5,  false));
  ASSERT_TRUE(add_and_verify(order_book, &bid6,  false));
  ASSERT_TRUE(add_and_verify(order_book, &bid7,  false));
  ASSERT_TRUE(add_and_verify(order_book, &bid8,  false));
  ASSERT_TRUE(add_and_verify(order_book, &bid9,  false));
  ASSERT_TRUE(add_and_verify(order_book, &bid10, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid11, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid12, false));

  // Verify sizes
  ASSERT_EQ(13, order_book.bids().size());
  ASSERT_EQ(11, order_book.asks().size());

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1249, 3,  500));
  ASSERT_TRUE(dc.verify_bid(1248, 1,  400));
  ASSERT_TRUE(dc.verify_bid(1246, 2, 1100));
  ASSERT_TRUE(dc.verify_bid(1245, 3,  500));
  ASSERT_TRUE(dc.verify_bid(1244, 2, 1000));
  ASSERT_TRUE(dc.verify_ask(1250, 1,  500));
  ASSERT_TRUE(dc.verify_ask(1251, 1,  400));
  ASSERT_TRUE(dc.verify_ask(1252, 2,  300));
  ASSERT_TRUE(dc.verify_ask(1254, 1,  300));
  ASSERT_TRUE(dc.verify_ask(1255, 2,  700));

  // Fill the top bid level (erase) and add an ask level (insert)
  SimpleOrder cross_ask(false,  1249, 800);
  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&bid0,      100, 1249 * 100);
    SimpleFillCheck fc2(&bid1,      200, 1249 * 200);
    SimpleFillCheck fc3(&bid2,      200, 1249 * 200);
    SimpleFillCheck fc4(&cross_ask, 500, 1249 * 500);
    ASSERT_TRUE(add_and_verify(order_book, &cross_ask, true, false));
  ); }

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1248, 1,  400));
  ASSERT_TRUE(dc.verify_bid(1246, 2, 1100));
  ASSERT_TRUE(dc.verify_bid(1245, 3,  500));
  ASSERT_TRUE(dc.verify_bid(1244, 2, 1000));
  ASSERT_TRUE(dc.verify_bid(1242, 1,  300)); // Restored
  ASSERT_TRUE(dc.verify_ask(1249, 1,  300)); // Inserted
  ASSERT_TRUE(dc.verify_ask(1250, 1,  500));
  ASSERT_TRUE(dc.verify_ask(1251, 1,  400));
  ASSERT_TRUE(dc.verify_ask(1252, 2,  300));
  ASSERT_TRUE(dc.verify_ask(1254, 1,  300));
  
  // Fill the top bid level (erase) but do not add an ask level (no insert)
  SimpleOrder cross_ask2(false,  1248, 400);
  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&bid3,       400, 1248 * 400);
    SimpleFillCheck fc4(&cross_ask2, 400, 1248 * 400);
    ASSERT_TRUE(add_and_verify(order_book, &cross_ask2, true, true));
  ); }

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1246, 2, 1100));
  ASSERT_TRUE(dc.verify_bid(1245, 3,  500));
  ASSERT_TRUE(dc.verify_bid(1244, 2, 1000));
  ASSERT_TRUE(dc.verify_bid(1242, 1,  300));
  ASSERT_TRUE(dc.verify_bid(1241, 1,  400)); // Restored
  ASSERT_TRUE(dc.verify_ask(1249, 1,  300));
  ASSERT_TRUE(dc.verify_ask(1250, 1,  500));
  ASSERT_TRUE(dc.verify_ask(1251, 1,  400));
  ASSERT_TRUE(dc.verify_ask(1252, 2,  300));
  ASSERT_TRUE(dc.verify_ask(1254, 1,  300));

  // Fill the top bid level (erase) and add ask level (insert),
  //    but nothing to restore
  SimpleOrder cross_ask3(false,  1246, 2400);
  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&bid4,        600, 1246 * 600);
    SimpleFillCheck fc2(&bid5,        500, 1246 * 500);
    SimpleFillCheck fc3(&cross_ask3, 1100, 1246 * 1100);
    ASSERT_TRUE(add_and_verify(order_book, &cross_ask3, true, false));
  ); }

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1245, 3,  500));
  ASSERT_TRUE(dc.verify_bid(1244, 2, 1000));
  ASSERT_TRUE(dc.verify_bid(1242, 1,  300));
  ASSERT_TRUE(dc.verify_bid(1241, 1,  400));
  ASSERT_TRUE(dc.verify_bid(   0, 0,    0)); // Nothing to restore
  ASSERT_TRUE(dc.verify_ask(1246, 1, 1300));
  ASSERT_TRUE(dc.verify_ask(1249, 1,  300));
  ASSERT_TRUE(dc.verify_ask(1250, 1,  500));
  ASSERT_TRUE(dc.verify_ask(1251, 1,  400));
  ASSERT_TRUE(dc.verify_ask(1252, 2,  300));

  // Partial fill the top bid level (reduce) 
  SimpleOrder cross_ask4(false,  1245, 250);
  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&bid6,        200, 1245 * 200);
    SimpleFillCheck fc2(&bid7,         50, 1245 *  50);
    SimpleFillCheck fc3(&cross_ask4,  250, 1245 * 250);
    ASSERT_TRUE(add_and_verify(order_book, &cross_ask4, true, true));
  ); }

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1245, 2,  250)); // 1 filled, 1 reduced
  ASSERT_TRUE(dc.verify_bid(1244, 2, 1000));
  ASSERT_TRUE(dc.verify_bid(1242, 1,  300));
  ASSERT_TRUE(dc.verify_bid(1241, 1,  400));
  ASSERT_TRUE(dc.verify_bid(   0, 0,    0));
  ASSERT_TRUE(dc.verify_ask(1246, 1, 1300));
  ASSERT_TRUE(dc.verify_ask(1249, 1,  300));
  ASSERT_TRUE(dc.verify_ask(1250, 1,  500));
  ASSERT_TRUE(dc.verify_ask(1251, 1,  400));
  ASSERT_TRUE(dc.verify_ask(1252, 2,  300));
}

TEST(TestFillCompleteAskRestoreDepth)
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
  ASSERT_TRUE(add_and_verify(order_book, &ask0,  false));
  ASSERT_TRUE(add_and_verify(order_book, &ask1,  false));
  ASSERT_TRUE(add_and_verify(order_book, &ask2,  false));
  ASSERT_TRUE(add_and_verify(order_book, &ask3,  false));
  ASSERT_TRUE(add_and_verify(order_book, &ask4,  false));
  ASSERT_TRUE(add_and_verify(order_book, &ask5,  false));
  ASSERT_TRUE(add_and_verify(order_book, &ask6,  false));
  ASSERT_TRUE(add_and_verify(order_book, &ask7,  false));
  ASSERT_TRUE(add_and_verify(order_book, &ask8,  false));
  ASSERT_TRUE(add_and_verify(order_book, &ask9,  false));
  ASSERT_TRUE(add_and_verify(order_book, &ask10, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid0,  false));
  ASSERT_TRUE(add_and_verify(order_book, &bid1,  false));
  ASSERT_TRUE(add_and_verify(order_book, &bid2,  false));
  ASSERT_TRUE(add_and_verify(order_book, &bid3,  false));
  ASSERT_TRUE(add_and_verify(order_book, &bid4,  false));
  ASSERT_TRUE(add_and_verify(order_book, &bid5,  false));
  ASSERT_TRUE(add_and_verify(order_book, &bid6,  false));
  ASSERT_TRUE(add_and_verify(order_book, &bid7,  false));
  ASSERT_TRUE(add_and_verify(order_book, &bid8,  false));
  ASSERT_TRUE(add_and_verify(order_book, &bid9,  false));
  ASSERT_TRUE(add_and_verify(order_book, &bid10, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid11, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid12, false));

  // Verify sizes
  ASSERT_EQ(13, order_book.bids().size());
  ASSERT_EQ(11, order_book.asks().size());

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1249, 3,  500));
  ASSERT_TRUE(dc.verify_bid(1248, 1,  400));
  ASSERT_TRUE(dc.verify_bid(1246, 2, 1100));
  ASSERT_TRUE(dc.verify_bid(1245, 3,  500));
  ASSERT_TRUE(dc.verify_bid(1244, 2, 1000));
  ASSERT_TRUE(dc.verify_ask(1250, 1,  500));
  ASSERT_TRUE(dc.verify_ask(1251, 1,  400));
  ASSERT_TRUE(dc.verify_ask(1252, 2,  300));
  ASSERT_TRUE(dc.verify_ask(1254, 1,  300));
  ASSERT_TRUE(dc.verify_ask(1255, 2,  700));

  // Fill the top ask level (erase) and add a bid level (insert)
  SimpleOrder cross_bid(true,  1250, 800);
  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&ask0,      500, 1250 * 500);
    SimpleFillCheck fc4(&cross_bid, 500, 1250 * 500);
    ASSERT_TRUE(add_and_verify(order_book, &cross_bid, true, false));
  ); }

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1250, 1,  300));
  ASSERT_TRUE(dc.verify_bid(1249, 3,  500));
  ASSERT_TRUE(dc.verify_bid(1248, 1,  400));
  ASSERT_TRUE(dc.verify_bid(1246, 2, 1100));
  ASSERT_TRUE(dc.verify_bid(1245, 3,  500));
  ASSERT_TRUE(dc.verify_ask(1251, 1,  400));
  ASSERT_TRUE(dc.verify_ask(1252, 2,  300));
  ASSERT_TRUE(dc.verify_ask(1254, 1,  300));
  ASSERT_TRUE(dc.verify_ask(1255, 2,  700));
  ASSERT_TRUE(dc.verify_ask(1256, 2,  200)); // Restored

  // Fill the top ask level (erase) but do not add an bid level (no insert)
  SimpleOrder cross_bid2(true,  1251, 400);
  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&ask1,       400, 1251 * 400);
    SimpleFillCheck fc4(&cross_bid2, 400, 1251 * 400);
    ASSERT_TRUE(add_and_verify(order_book, &cross_bid2, true, true));
  ); }

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1250, 1,  300));
  ASSERT_TRUE(dc.verify_bid(1249, 3,  500));
  ASSERT_TRUE(dc.verify_bid(1248, 1,  400));
  ASSERT_TRUE(dc.verify_bid(1246, 2, 1100));
  ASSERT_TRUE(dc.verify_bid(1245, 3,  500));
  ASSERT_TRUE(dc.verify_ask(1252, 2,  300));
  ASSERT_TRUE(dc.verify_ask(1254, 1,  300));
  ASSERT_TRUE(dc.verify_ask(1255, 2,  700));
  ASSERT_TRUE(dc.verify_ask(1256, 2,  200));
  ASSERT_TRUE(dc.verify_ask(1257, 1,  700)); // Restored

  // Fill the top ask level (erase) and add bid level (insert),
  //    but nothing to restore
  SimpleOrder cross_bid3(true,  1252, 2400);
  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&ask2,        100, 1252 * 100);
    SimpleFillCheck fc2(&ask3,        200, 1252 * 200);
    SimpleFillCheck fc3(&cross_bid3,  300, 1252 * 300);
    ASSERT_TRUE(add_and_verify(order_book, &cross_bid3, true, false));
  ); }

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1252, 1, 2100)); // Insert
  ASSERT_TRUE(dc.verify_bid(1250, 1,  300));
  ASSERT_TRUE(dc.verify_bid(1249, 3,  500));
  ASSERT_TRUE(dc.verify_bid(1248, 1,  400));
  ASSERT_TRUE(dc.verify_bid(1246, 2, 1100));
  ASSERT_TRUE(dc.verify_ask(1254, 1,  300));
  ASSERT_TRUE(dc.verify_ask(1255, 2,  700));
  ASSERT_TRUE(dc.verify_ask(1256, 2,  200));
  ASSERT_TRUE(dc.verify_ask(1257, 1,  700));
  ASSERT_TRUE(dc.verify_ask(1258, 1,  600)); // Restored

  // Fill the top ask level (erase) but nothing to restore
  SimpleOrder cross_bid4(true,  1254, 300);
  { ASSERT_NO_THROW(
    SimpleFillCheck fc2(&ask4,        300, 1254 * 300);
    SimpleFillCheck fc3(&cross_bid4,  300, 1254 * 300);
    ASSERT_TRUE(add_and_verify(order_book, &cross_bid4, true, true));
  ); }

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1252, 1, 2100));
  ASSERT_TRUE(dc.verify_bid(1250, 1,  300));
  ASSERT_TRUE(dc.verify_bid(1249, 3,  500));
  ASSERT_TRUE(dc.verify_bid(1248, 1,  400));
  ASSERT_TRUE(dc.verify_bid(1246, 2, 1100));
  ASSERT_TRUE(dc.verify_ask(1255, 2,  700));
  ASSERT_TRUE(dc.verify_ask(1256, 2,  200));
  ASSERT_TRUE(dc.verify_ask(1257, 1,  700));
  ASSERT_TRUE(dc.verify_ask(1258, 1,  600));
  ASSERT_TRUE(dc.verify_ask(   0, 0,    0)); // Nothing to restore

  // Partial fill the top ask level (reduce) 
  SimpleOrder cross_bid5(true,  1255, 550);
  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&ask5,        200, 1255 * 200);
    SimpleFillCheck fc2(&ask6,        350, 1255 * 350);
    SimpleFillCheck fc3(&cross_bid5,  550, 1255 * 550);
    ASSERT_TRUE(add_and_verify(order_book, &cross_bid5, true, true));
  ); }

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1252, 1, 2100));
  ASSERT_TRUE(dc.verify_bid(1250, 1,  300));
  ASSERT_TRUE(dc.verify_bid(1249, 3,  500));
  ASSERT_TRUE(dc.verify_bid(1248, 1,  400));
  ASSERT_TRUE(dc.verify_bid(1246, 2, 1100));
  ASSERT_TRUE(dc.verify_ask(1255, 1,  150)); // 1 filled, 1 reduced
  ASSERT_TRUE(dc.verify_ask(1256, 2,  200));
  ASSERT_TRUE(dc.verify_ask(1257, 1,  700));
  ASSERT_TRUE(dc.verify_ask(1258, 1,  600));
  ASSERT_TRUE(dc.verify_ask(   0, 0,    0));
}

TEST(TestReplaceSizeIncrease)
{
  SimpleOrderBook order_book;
  ChangedChecker cc(order_book.depth());
  SimpleOrder ask0(false, 1252, 300);
  SimpleOrder ask1(false, 1251, 200);
  SimpleOrder bid0(true,  1250, 100);
  SimpleOrder bid1(true,  1249, 100);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &bid0, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid1, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask1, false));

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1249, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 200));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 300));

  // Verify changed stamps
  ASSERT_TRUE(cc.verify_bid_changed(1, 1, 0, 0, 0));
  ASSERT_TRUE(cc.verify_ask_changed(1, 1, 0, 0, 0));
  cc.reset();

  // Replace size
  ASSERT_TRUE(replace_and_verify(order_book, &bid0, 25));
  ASSERT_TRUE(replace_and_verify(order_book, &ask0, 50));

  // Verify changed stamps
  ASSERT_TRUE(cc.verify_bid_changed(1, 0, 0, 0, 0));
  ASSERT_TRUE(cc.verify_ask_changed(0, 1, 0, 0, 0));
  cc.reset();

  // Verify orders
  ASSERT_EQ(125, bid0.order_qty());
  ASSERT_EQ(350, ask0.order_qty());
  
  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1250, 1, 125));
  ASSERT_TRUE(dc.verify_bid(1249, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 200));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 350));
}

TEST(TestReplaceSizeDecrease)
{
  SimpleOrderBook order_book;
  ChangedChecker cc(order_book.depth());
  SimpleOrder ask1(false, 1252, 200);
  SimpleOrder ask0(false, 1252, 300);
  SimpleOrder bid1(true,  1251, 100);
  SimpleOrder bid0(true,  1250, 100);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &bid0, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid1, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask1, false));

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1251, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 2, 500));

  // Verify changed stamps
  ASSERT_TRUE(cc.verify_bid_changed(1, 1, 0, 0, 0));
  ASSERT_TRUE(cc.verify_ask_changed(1, 0, 0, 0, 0));
  cc.reset();

  // Replace size
  ASSERT_TRUE(replace_and_verify(order_book, &bid0, -60));
  ASSERT_TRUE(replace_and_verify(order_book, &ask0, -150));

  // Verify orders
  ASSERT_EQ(40, bid0.order_qty());
  ASSERT_EQ(150, ask0.order_qty());
  
  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1251, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1250, 1,  40));
  ASSERT_TRUE(dc.verify_ask(1252, 2, 350));

  // Verify changed stamps
  ASSERT_TRUE(cc.verify_bid_changed(0, 1, 0, 0, 0));
  ASSERT_TRUE(cc.verify_ask_changed(1, 0, 0, 0, 0));
  cc.reset();
}

TEST(TestReplaceSizeDecreaseCancel)
{
  SimpleOrderBook order_book;
  ChangedChecker cc(order_book.depth());
  SimpleOrder ask1(false, 1252, 200);
  SimpleOrder ask0(false, 1252, 300);
  SimpleOrder bid1(true,  1251, 400);
  SimpleOrder bid0(true,  1250, 100);
  SimpleOrder bid2(true,  1249, 700);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &bid0, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid1, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid2, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask1, false));

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_ask(1252, 2, 500));
  ASSERT_TRUE(dc.verify_bid(1251, 1, 400));
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1249, 1, 700));

  // Verify changed stamps
  ASSERT_TRUE(cc.verify_bid_changed(1, 1, 1, 0, 0));
  ASSERT_TRUE(cc.verify_ask_changed(1, 0, 0, 0, 0));
  cc.reset();

  // Partial Fill existing book
  SimpleOrder cross_bid(true,  1252, 125);
  SimpleOrder cross_ask(false, 1251, 100);
  
  // Bid matching best ask
  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&cross_bid, 125, 1252 * 125);
    SimpleFillCheck fc2(&ask0,      125, 1252 * 125);
    ASSERT_TRUE(add_and_verify(order_book, &cross_bid, true, true));
  ); }

  // TODO: don't insert when when inbound will be filled
  ASSERT_TRUE(cc.verify_bid_changed(0, 0, 0, 0, 0));
  ASSERT_TRUE(cc.verify_ask_changed(1, 0, 0, 0, 0));
  cc.reset();

  // Ask matching best bid
  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&cross_ask, 100, 1251 * 100);
    SimpleFillCheck fc2(&bid1,      100, 1251 * 100);
    ASSERT_TRUE(add_and_verify(order_book, &cross_ask, true, true));
  ); }

  ASSERT_TRUE(cc.verify_bid_changed(1, 0, 0, 0, 0));
  ASSERT_TRUE(cc.verify_ask_changed(0, 0, 0, 0, 0));

  // Verify quantity
  ASSERT_EQ(175, ask0.open_qty());
  ASSERT_EQ(300, bid1.open_qty());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1251, 1, 300));
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1249, 1, 700));
  ASSERT_TRUE(dc.verify_ask(1252, 2, 375));

  // Replace size - cancel
  ASSERT_TRUE(replace_and_verify(
      order_book, &ask0, -175, PRICE_UNCHANGED, impl::os_cancelled)); 

  // Verify orders
  ASSERT_EQ(125, ask0.order_qty());
  ASSERT_EQ(0, ask0.open_qty());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1251, 1, 300));
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1249, 1, 700));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 200));

  // Replace size - reduce level
  ASSERT_TRUE(replace_and_verify(
      order_book, &bid1, -100, PRICE_UNCHANGED, impl::os_accepted)); 

  // Verify orders
  ASSERT_EQ(300, bid1.order_qty());
  ASSERT_EQ(200, bid1.open_qty());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1251, 1, 200));
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1249, 1, 700));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 200));

  // Replace size - cancel and erase level
  ASSERT_TRUE(replace_and_verify(
      order_book, &bid1, -200, PRICE_UNCHANGED, impl::os_cancelled)); 

  // Verify orders
  ASSERT_EQ(100, bid1.order_qty());
  ASSERT_EQ(0, bid1.open_qty());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1249, 1, 700));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 200));
}

TEST(TestReplaceSizeDecreaseTooMuch)
{
  SimpleOrderBook order_book;
  SimpleOrder ask1(false, 1252, 200);
  SimpleOrder ask0(false, 1252, 300);
  SimpleOrder bid1(true,  1251, 100);
  SimpleOrder bid0(true,  1250, 100);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &bid0, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid1, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask1, false));

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1251, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 2, 500));

  SimpleOrder cross_bid(true,  1252, 200);
  // Partial fill existing order
  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&cross_bid, 200, 1252 * 200);
    SimpleFillCheck fc2(&ask0,      200, 1252 * 200);
    ASSERT_TRUE(add_and_verify(order_book, &cross_bid, true, true));
  ); }

  // Verify open quantity
  ASSERT_EQ(100, ask0.open_qty());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1251, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 2, 300));

  // Replace size - not enough left
  order_book.replace(&ask0, -150, PRICE_UNCHANGED);
  order_book.perform_callbacks();

  // Verify orders
  ASSERT_EQ(100, ask0.open_qty());
  ASSERT_EQ(300, ask0.order_qty());

  // Verify open quantity unchanged
  ASSERT_EQ(impl::os_accepted, ask0.state());
  ASSERT_EQ(100, ask0.open_qty());

  // Verify depth unchanged
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1251, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 2, 300));
}

TEST(TestReplaceSizeIncreaseDecrease)
{
  SimpleOrderBook order_book;
  SimpleOrder ask1(false, 1252, 200);
  SimpleOrder ask0(false, 1251, 300);
  SimpleOrder bid1(true,  1251, 100);
  SimpleOrder bid0(true,  1250, 100);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &bid0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask1, false));

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 300));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 200));

  // Replace size
  ASSERT_TRUE(replace_and_verify(order_book, &ask0, 50));
  ASSERT_TRUE(replace_and_verify(order_book, &bid0, 25));

  ASSERT_TRUE(replace_and_verify(order_book, &ask0, -100));
  ASSERT_TRUE(replace_and_verify(order_book, &bid0, 25));

  ASSERT_TRUE(replace_and_verify(order_book, &ask0, 300));
  ASSERT_TRUE(replace_and_verify(order_book, &bid0, -75));

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1250, 1, 75));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 550));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 200));
}

TEST(TestReplaceBidPriceChange)
{
  SimpleOrderBook order_book;
  ChangedChecker cc(order_book.depth());
  SimpleOrder ask0(false, 1253, 300);
  SimpleOrder ask1(false, 1252, 200);
  SimpleOrder bid1(true,  1251, 140);
  SimpleOrder bid0(true,  1250, 120);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &bid0, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid1, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask1, false));

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1251, 1, 140));
  ASSERT_TRUE(dc.verify_bid(1250, 1, 120));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 200));
  ASSERT_TRUE(dc.verify_ask(1253, 1, 300));

  // Verify changed stamps
  ASSERT_TRUE(cc.verify_bid_changed(1, 1, 0, 0, 0));
  ASSERT_TRUE(cc.verify_ask_changed(1, 1, 0, 0, 0));
  cc.reset();

  // Replace price increase 1250 -> 1251
  ASSERT_TRUE(replace_and_verify(order_book, &bid0, SIZE_UNCHANGED, 1251));

  // Verify price change in book
  SimpleOrderBook::Bids::const_iterator bid = order_book.bids().begin();
  ASSERT_EQ(1251, bid->first);
  ASSERT_EQ(&bid1, bid->second.ptr());
  ASSERT_EQ(1251, (++bid)->first);
  ASSERT_EQ(&bid0, bid->second.ptr());
  ASSERT_TRUE(order_book.bids().end() == ++bid);

  // Verify order
  ASSERT_EQ(1251, bid0.price());
  ASSERT_EQ(120, bid0.order_qty());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1251, 2, 260));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 200));
  ASSERT_TRUE(dc.verify_ask(1253, 1, 300));

  // Verify changed stamps
  ASSERT_TRUE(cc.verify_bid_changed(1, 1, 0, 0, 0));
  ASSERT_TRUE(cc.verify_ask_changed(0, 0, 0, 0, 0));
  cc.reset();

  // Replace price decrease 1251 -> 1250
  ASSERT_TRUE(replace_and_verify(order_book, &bid1, SIZE_UNCHANGED, 1250));

  // Verify price change in book
  bid = order_book.bids().begin();
  ASSERT_EQ(1251, bid->first);
  ASSERT_EQ(&bid0, bid->second.ptr());
  ASSERT_EQ(1250, (++bid)->first);
  ASSERT_EQ(&bid1, bid->second.ptr());
  ASSERT_TRUE(order_book.bids().end() == ++bid);

  // Verify order
  ASSERT_EQ(1250, bid1.price());
  ASSERT_EQ(140, bid1.order_qty());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1251, 1, 120));
  ASSERT_TRUE(dc.verify_bid(1250, 1, 140));
  ASSERT_TRUE(dc.verify_bid(   0, 0,   0));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 200));
  ASSERT_TRUE(dc.verify_ask(1253, 1, 300));

  // Verify changed stamps
  ASSERT_TRUE(cc.verify_bid_changed(1, 1, 0, 0, 0));
  ASSERT_TRUE(cc.verify_ask_changed(0, 0, 0, 0, 0));
}

TEST(TestReplaceAskPriceChange)
{
  SimpleOrderBook order_book;
  ChangedChecker cc(order_book.depth());

  SimpleOrder ask0(false, 1253, 300);
  SimpleOrder ask1(false, 1252, 200);
  SimpleOrder bid1(true,  1251, 140);
  SimpleOrder bid0(true,  1250, 120);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &bid0, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid1, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask1, false));

  // Verify changed stamps
  ASSERT_TRUE(cc.verify_bid_changed(1, 1, 0, 0, 0));
  ASSERT_TRUE(cc.verify_ask_changed(1, 1, 0, 0, 0));
  cc.reset();

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1251, 1, 140));
  ASSERT_TRUE(dc.verify_bid(1250, 1, 120));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 200));
  ASSERT_TRUE(dc.verify_ask(1253, 1, 300));

  // Replace price increase 1252 -> 1253
  ASSERT_TRUE(replace_and_verify(order_book, &ask1, SIZE_UNCHANGED, 1253));

  // Verify changed stamps
  ASSERT_TRUE(cc.verify_bid_changed(0, 0, 0, 0, 0));
  ASSERT_TRUE(cc.verify_ask_changed(1, 1, 0, 0, 0));
  cc.reset();

  // Verify price change in book
  SimpleOrderBook::Asks::const_iterator ask = order_book.asks().begin();
  ASSERT_EQ(1253, ask->first);
  ASSERT_EQ(&ask0, ask->second.ptr());
  ASSERT_EQ(1253, (++ask)->first);
  ASSERT_EQ(&ask1, ask->second.ptr());
  ASSERT_TRUE(order_book.asks().end() == ++ask);

  // Verify order
  ASSERT_EQ(1253, ask1.price());
  ASSERT_EQ(200, ask1.order_qty());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1251, 1, 140));
  ASSERT_TRUE(dc.verify_bid(1250, 1, 120));
  ASSERT_TRUE(dc.verify_ask(1253, 2, 500));

  // Replace price decrease 1253 -> 1252
  ASSERT_TRUE(replace_and_verify(order_book, &ask0, SIZE_UNCHANGED, 1252));

  // Verify price change in book
  ask = order_book.asks().begin();
  ASSERT_EQ(1252, ask->first);
  ASSERT_EQ(&ask0, ask->second.ptr());
  ASSERT_EQ(1253, (++ask)->first);
  ASSERT_EQ(&ask1, ask->second.ptr());
  ASSERT_TRUE(order_book.asks().end() == ++ask);

  // Verify order
  ASSERT_EQ(1252, ask0.price());
  ASSERT_EQ(300, ask0.order_qty());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1251, 1, 140));
  ASSERT_TRUE(dc.verify_bid(1250, 1, 120));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 300));
  ASSERT_TRUE(dc.verify_ask(1253, 1, 200));
  ASSERT_TRUE(dc.verify_ask(   0, 0,   0));
}

TEST(TestReplaceBidPriceChangeErase)
{
  SimpleOrderBook order_book;
  SimpleOrder ask0(false, 1253, 300);
  SimpleOrder ask1(false, 1252, 200);
  SimpleOrder bid1(true,  1251, 140);
  SimpleOrder bid0(true,  1250, 120);
  SimpleOrder bid2(true,  1249, 100);
  SimpleOrder bid3(true,  1248, 200);
  SimpleOrder bid4(true,  1247, 400);
  SimpleOrder bid5(true,  1246, 800);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &bid0, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid1, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid2, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid3, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid4, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid5, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask1, false));

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1251, 1, 140));
  ASSERT_TRUE(dc.verify_bid(1250, 1, 120));
  ASSERT_TRUE(dc.verify_bid(1249, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1248, 1, 200));
  ASSERT_TRUE(dc.verify_bid(1247, 1, 400));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 200));
  ASSERT_TRUE(dc.verify_ask(1253, 1, 300));

  // Replace price increase 1250 -> 1251
  ASSERT_TRUE(replace_and_verify(order_book, &bid0, SIZE_UNCHANGED, 1251));

  // Verify price change in book
  SimpleOrderBook::Bids::const_iterator bid = order_book.bids().begin();
  ASSERT_EQ(1251, bid->first);
  ASSERT_EQ(&bid1, bid->second.ptr());
  ASSERT_EQ(1251, (++bid)->first);
  ASSERT_EQ(&bid0, bid->second.ptr());
  ASSERT_EQ(1249, (++bid)->first);
  ASSERT_EQ(&bid2, bid->second.ptr());

  // Verify order
  ASSERT_EQ(1251, bid0.price());
  ASSERT_EQ(120, bid0.order_qty());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1251, 2, 260));
  ASSERT_TRUE(dc.verify_bid(1249, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1248, 1, 200));
  ASSERT_TRUE(dc.verify_bid(1247, 1, 400));
  ASSERT_TRUE(dc.verify_bid(1246, 1, 800));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 200));
  ASSERT_TRUE(dc.verify_ask(1253, 1, 300));

  // Replace price decrease 1251 -> 1250
  ASSERT_TRUE(replace_and_verify(order_book, &bid1, SIZE_UNCHANGED, 1250));

  // Verify price change in book
  bid = order_book.bids().begin();
  ASSERT_EQ(1251, bid->first);
  ASSERT_EQ(&bid0, bid->second.ptr());
  ASSERT_EQ(1250, (++bid)->first);
  ASSERT_EQ(&bid1, bid->second.ptr());
  ASSERT_EQ(1249, (++bid)->first);
  ASSERT_EQ(&bid2, bid->second.ptr());

  // Verify order
  ASSERT_EQ(1250, bid1.price());
  ASSERT_EQ(140, bid1.order_qty());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1251, 1, 120));
  ASSERT_TRUE(dc.verify_bid(1250, 1, 140));
  ASSERT_TRUE(dc.verify_bid(1249, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1248, 1, 200));
  ASSERT_TRUE(dc.verify_bid(1247, 1, 400));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 200));
  ASSERT_TRUE(dc.verify_ask(1253, 1, 300));
}

TEST(TestReplaceAskPriceChangeErase)
{
  SimpleOrderBook order_book;
  SimpleOrder ask5(false, 1258, 304);
  SimpleOrder ask4(false, 1256, 330);
  SimpleOrder ask3(false, 1255, 302);
  SimpleOrder ask2(false, 1254, 310);
  SimpleOrder ask0(false, 1253, 300);
  SimpleOrder ask1(false, 1252, 200);
  SimpleOrder bid1(true,  1251, 140);
  SimpleOrder bid0(true,  1250, 120);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &bid0, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid1, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask1, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask2, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask3, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask4, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask5, false));

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1251, 1, 140));
  ASSERT_TRUE(dc.verify_bid(1250, 1, 120));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 200));
  ASSERT_TRUE(dc.verify_ask(1253, 1, 300));
  ASSERT_TRUE(dc.verify_ask(1254, 1, 310));
  ASSERT_TRUE(dc.verify_ask(1255, 1, 302));
  ASSERT_TRUE(dc.verify_ask(1256, 1, 330));

  // Replace price increase 1252 -> 1253
  ASSERT_TRUE(replace_and_verify(order_book, &ask1, SIZE_UNCHANGED, 1253));

  // Verify price change in book
  SimpleOrderBook::Asks::const_iterator ask = order_book.asks().begin();
  ASSERT_EQ(1253, ask->first);
  ASSERT_EQ(&ask0, ask->second.ptr());
  ASSERT_EQ(1253, (++ask)->first);
  ASSERT_EQ(&ask1, ask->second.ptr());
  ASSERT_EQ(1254, (++ask)->first);
  ASSERT_EQ(&ask2, ask->second.ptr());

  // Verify order
  ASSERT_EQ(1253, ask1.price());
  ASSERT_EQ(200, ask1.order_qty());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1251, 1, 140));
  ASSERT_TRUE(dc.verify_bid(1250, 1, 120));
  ASSERT_TRUE(dc.verify_ask(1253, 2, 500));
  ASSERT_TRUE(dc.verify_ask(1254, 1, 310));
  ASSERT_TRUE(dc.verify_ask(1255, 1, 302));
  ASSERT_TRUE(dc.verify_ask(1256, 1, 330));
  ASSERT_TRUE(dc.verify_ask(1258, 1, 304));

  // Replace price decrease 1253 -> 1252
  ASSERT_TRUE(replace_and_verify(order_book, &ask0, SIZE_UNCHANGED, 1252));

  // Verify price change in book
  ask = order_book.asks().begin();
  ASSERT_EQ(1252, ask->first);
  ASSERT_EQ(&ask0, ask->second.ptr());
  ASSERT_EQ(1253, (++ask)->first);
  ASSERT_EQ(&ask1, ask->second.ptr());
  ASSERT_EQ(1254, (++ask)->first);
  ASSERT_EQ(&ask2, ask->second.ptr());

  // Verify order
  ASSERT_EQ(1252, ask0.price());
  ASSERT_EQ(300, ask0.order_qty());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1251, 1, 140));
  ASSERT_TRUE(dc.verify_bid(1250, 1, 120));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 300));
  ASSERT_TRUE(dc.verify_ask(1253, 1, 200));
  ASSERT_TRUE(dc.verify_ask(1254, 1, 310));
  ASSERT_TRUE(dc.verify_ask(1255, 1, 302));
  ASSERT_TRUE(dc.verify_ask(1256, 1, 330));
}

// A potential problem
// When restroing a level into the depth, the orders (and thus the restored
// level already reflect the post-fill quantity, but the fill callback has 
// yet to be processed.  As such, a multilevel fill can have fills at the 
// restoration price double-counted
// but the 
TEST(TestBidMultiLevelFillRestore)
{
  SimpleOrderBook order_book;
  SimpleOrder ask1(false, 0, 1300);
  SimpleOrder ask0(false, 1252, 100);
  SimpleOrder bid0(true,  1251, 200);
  SimpleOrder bid1(true,  1250, 200);
  SimpleOrder bid2(true,  1250, 200);
  SimpleOrder bid3(true,  1248, 200);
  SimpleOrder bid4(true,  1247, 200);
  SimpleOrder bid5(true,  1246, 200);
  SimpleOrder bid6(true,  1245, 200); // Partial match
  SimpleOrder bid7(true,  1244, 200);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &bid0, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid1, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid2, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid3, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid4, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid5, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid6, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid7, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask0, false));

  // Verify sizes
  ASSERT_EQ(8, order_book.bids().size());
  ASSERT_EQ(1, order_book.asks().size());

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1251, 1, 200));
  ASSERT_TRUE(dc.verify_bid(1250, 2, 400));
  ASSERT_TRUE(dc.verify_bid(1248, 1, 200));
  ASSERT_TRUE(dc.verify_bid(1247, 1, 200));
  ASSERT_TRUE(dc.verify_bid(1246, 1, 200));

  // Match - complete
  { ASSERT_NO_THROW(
    SimpleFillCheck fc0(&bid0,  200,  250200);
    SimpleFillCheck fc1(&bid1,  200,  250000);
    SimpleFillCheck fc2(&bid2,  200,  250000);
    SimpleFillCheck fc3(&bid3,  200,  249600);
    SimpleFillCheck fc4(&bid4,  200,  249400);
    SimpleFillCheck fc5(&bid5,  200,  249200);
    SimpleFillCheck fc6(&bid6,  100,  124500);
    SimpleFillCheck fc7(&ask1, 1300, 1622900);
    ASSERT_TRUE(add_and_verify(order_book, &ask1, true, true));
  ); }

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1245, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1244, 1, 200));
}

TEST(TestAskMultiLevelFillRestore)
{
  SimpleOrderBook order_book;
  SimpleOrder ask0(false,  1251, 200); // Partial match
  SimpleOrder ask1(false,  1250, 200);
  SimpleOrder ask2(false,  1250, 300);
  SimpleOrder ask3(false,  1248, 200);
  SimpleOrder ask4(false,  1247, 200);
  SimpleOrder ask5(false,  1245, 200);
  SimpleOrder ask6(false,  1245, 200);
  SimpleOrder ask7(false,  1244, 200);
  SimpleOrder bid1(true, 0, 1550);
  SimpleOrder bid0(true, 1242, 100);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &ask0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask1, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask2, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask3, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask4, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask5, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask6, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask7, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid0, false));

  // Verify sizes
  ASSERT_EQ(8, order_book.asks().size());
  ASSERT_EQ(1, order_book.bids().size());

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_ask(1244, 1, 200));
  ASSERT_TRUE(dc.verify_ask(1245, 2, 400));
  ASSERT_TRUE(dc.verify_ask(1247, 1, 200));
  ASSERT_TRUE(dc.verify_ask(1248, 1, 200));
  ASSERT_TRUE(dc.verify_ask(1250, 2, 500));
  ASSERT_TRUE(dc.verify_bid(1242, 1, 100));

  // Match - complete
  { ASSERT_NO_THROW(
    SimpleFillCheck fc7(&ask7,  200,  248800);
    SimpleFillCheck fc6(&ask6,  200,  249000);
    SimpleFillCheck fc5(&ask5,  200,  249000);
    SimpleFillCheck fc4(&ask4,  200,  249400);
    SimpleFillCheck fc3(&ask3,  200,  249600);
    SimpleFillCheck fc2(&ask2,  300,  375000);
    SimpleFillCheck fc1(&ask1,  200,  250000);
    SimpleFillCheck fc0(&ask0,   50,   62550);
    SimpleFillCheck fc8(&bid1, 1550, 1933350);
    ASSERT_TRUE(add_and_verify(order_book, &bid1, true, true));
  ); }

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_ask(1251, 1, 150));
  ASSERT_TRUE(dc.verify_bid(1242, 1, 100));
}

TEST(TestReplaceBidMatch)
{
  SimpleOrderBook order_book;
  ChangedChecker cc(order_book.depth());
  SimpleOrder ask1(false, 1254, 200);
  SimpleOrder ask0(false, 1253, 300);
  SimpleOrder bid1(true,  1251, 140);
  SimpleOrder bid0(true,  1250, 120);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &bid0, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid1, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask1, false));

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1251, 1, 140));
  ASSERT_TRUE(dc.verify_bid(1250, 1, 120));
  ASSERT_TRUE(dc.verify_ask(1253, 1, 300));
  ASSERT_TRUE(dc.verify_ask(1254, 1, 200));

  // Verify changed stamps
  ASSERT_TRUE(cc.verify_bid_changed(1, 1, 0, 0, 0));
  ASSERT_TRUE(cc.verify_ask_changed(1, 1, 0, 0, 0));
  cc.reset();

  // Replace price increase new best 1250 -> 1252
  ASSERT_TRUE(replace_and_verify(order_book, &bid0, SIZE_UNCHANGED, 1252));

  // Verify price change in book
  SimpleOrderBook::Bids::const_iterator bid = order_book.bids().begin();
  ASSERT_EQ(1252, bid->first);
  ASSERT_EQ(&bid0, bid->second.ptr());
  ASSERT_EQ(1251, (++bid)->first);
  ASSERT_EQ(&bid1, bid->second.ptr());
  ASSERT_TRUE(order_book.bids().end() == ++bid);

  // Verify order
  ASSERT_EQ(1252, bid0.price());
  ASSERT_EQ(120, bid0.order_qty());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1252, 1, 120));
  ASSERT_TRUE(dc.verify_bid(1251, 1, 140));
  ASSERT_TRUE(dc.verify_ask(1253, 1, 300));
  ASSERT_TRUE(dc.verify_ask(1254, 1, 200));

  // Verify changed stamps
  ASSERT_TRUE(cc.verify_bid_changed(1, 1, 1, 0, 0));
  ASSERT_TRUE(cc.verify_ask_changed(0, 0, 0, 0, 0));
  cc.reset();

  // Match - complete
  { ASSERT_NO_THROW(
    SimpleFillCheck fc0(&ask0,  140, 140 * 1253);
    SimpleFillCheck fc1(&bid1,  140, 140 * 1253);
    // Replace price increase match 1251 -> 1253
    ASSERT_TRUE(replace_and_verify(order_book, &bid1, SIZE_UNCHANGED, 1253,
                  impl::os_complete, 140));
  ); }

  // Verify price change in book
  bid = order_book.bids().begin();
  ASSERT_EQ(1252, bid->first);
  ASSERT_EQ(&bid0, bid->second.ptr());
  ASSERT_TRUE(order_book.bids().end() == ++bid);

  // Verify order
  ASSERT_EQ(1253, bid1.price());
  ASSERT_EQ(140, bid1.order_qty());
  ASSERT_EQ(0, bid1.open_qty());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1252, 1, 120));
  ASSERT_TRUE(dc.verify_bid(   0, 0,   0));
  ASSERT_TRUE(dc.verify_ask(1253, 1, 160));
  ASSERT_TRUE(dc.verify_ask(1254, 1, 200));

  // Verify changed stamps
  ASSERT_TRUE(cc.verify_bid_changed(1, 1, 1, 0, 0));
  ASSERT_TRUE(cc.verify_ask_changed(1, 0, 0, 0, 0));
}

TEST(TestReplaceAskMatch)
{
  SimpleOrderBook order_book;
  ChangedChecker cc(order_book.depth());
  SimpleOrder ask1(false, 1254, 200);
  SimpleOrder ask0(false, 1253, 300);
  SimpleOrder bid1(true,  1251, 140);
  SimpleOrder bid0(true,  1250, 120);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &bid0, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid1, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask1, false));

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1251, 1, 140));
  ASSERT_TRUE(dc.verify_bid(1250, 1, 120));
  ASSERT_TRUE(dc.verify_ask(1253, 1, 300));
  ASSERT_TRUE(dc.verify_ask(1254, 1, 200));

  // Verify changed stamps
  ASSERT_TRUE(cc.verify_bid_changed(1, 1, 0, 0, 0));
  ASSERT_TRUE(cc.verify_ask_changed(1, 1, 0, 0, 0));
  cc.reset();

  // Replace price decrease new best 1254 -> 1252
  ASSERT_TRUE(replace_and_verify(order_book, &ask1, SIZE_UNCHANGED, 1252));

  // Verify price change in book
  SimpleOrderBook::Asks::const_iterator ask = order_book.asks().begin();
  ASSERT_EQ(1252, ask->first);
  ASSERT_EQ(&ask1, ask->second.ptr());
  ASSERT_EQ(1253, (++ask)->first);
  ASSERT_EQ(&ask0, ask->second.ptr());
  ASSERT_TRUE(order_book.asks().end() == ++ask);

  // Verify order
  ASSERT_EQ(1252, ask1.price());
  ASSERT_EQ(200, ask1.order_qty());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1251, 1, 140));
  ASSERT_TRUE(dc.verify_bid(1250, 1, 120));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 200));
  ASSERT_TRUE(dc.verify_ask(1253, 1, 300));

  // Verify changed stamps
  ASSERT_TRUE(cc.verify_bid_changed(0, 0, 0, 0, 0));
  ASSERT_TRUE(cc.verify_ask_changed(1, 1, 1, 0, 0));
  cc.reset();

  // Match - complete
  { ASSERT_NO_THROW(
    SimpleFillCheck fc0(&ask0,  140, 140 * 1251);
    SimpleFillCheck fc1(&bid1,  140, 140 * 1251);
    // Replace price decrease match 1253 -> 1251
    ASSERT_TRUE(replace_and_verify(order_book, &ask0, SIZE_UNCHANGED, 1251,
                  impl::os_accepted, 140));
  ); }

  // Verify price change in book
  ask = order_book.asks().begin();
  ASSERT_EQ(1251, ask->first);
  ASSERT_EQ(&ask0, ask->second.ptr());
  ASSERT_EQ(1252, (++ask)->first);
  ASSERT_EQ(&ask1, ask->second.ptr());
  ASSERT_TRUE(order_book.asks().end() == ++ask);

  // Verify order
  ASSERT_EQ(1251, ask0.price());
  ASSERT_EQ(300, ask0.order_qty());
  ASSERT_EQ(160, ask0.open_qty());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1250, 1, 120));
  ASSERT_TRUE(dc.verify_bid(   0, 0,   0));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 160));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 200));

  // Verify changed stamps
  ASSERT_TRUE(cc.verify_bid_changed(1, 1, 0, 0, 0));
  ASSERT_TRUE(cc.verify_ask_changed(1, 1, 1, 0, 0));
}

} // namespace
