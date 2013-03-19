// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include "assertiv/assertiv.h"
#include "ut_utils.h"

namespace liquibook {

using impl::SimpleOrder;
typedef FillCheck<SimpleOrder*> SimpleFillCheck;

OrderConditions IOC(oc_immediate_or_cancel);
OrderConditions FOK(oc_all_or_none | oc_immediate_or_cancel);

TEST(TestIocBidNoMatch)
{
  SimpleOrderBook order_book;
  SimpleOrder ask2(false, 1252, 100);
  SimpleOrder ask1(false, 1251, 100);
  SimpleOrder ask0(false, 1250, 100);
  SimpleOrder bid0(true,  1250, 100);
  SimpleOrder bid1(true,  1249, 100);
  SimpleOrder bid2(true,  1248, 100);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &ask1, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask2, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid1, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid2, false));

  // Verify sizes
  ASSERT_EQ(2, order_book.bids().size());
  ASSERT_EQ(2, order_book.asks().size());

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1249, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1248, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));

  // No Match - will cancel order
  { ASSERT_NO_THROW(
    SimpleFillCheck fc0(&bid0, 0, 0, IOC);
    SimpleFillCheck fc1(&bid1, 0, 0);
    SimpleFillCheck fc2(&bid2, 0, 0);
    //SimpleFillCheck fc3(&ask0, 0, 0);
    SimpleFillCheck fc4(&ask1, 0, 0);
    SimpleFillCheck fc5(&ask2, 0, 0);
    ASSERT_TRUE(add_and_verify(order_book, &bid0, false, false, IOC));
  ); }

  // Verify sizes
  ASSERT_EQ(2, order_book.bids().size());
  ASSERT_EQ(2, order_book.asks().size());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1249, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1248, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));
}

TEST(TestIocBidPartialMatch)
{
  SimpleOrderBook order_book;
  SimpleOrder ask2(false, 1252, 100);
  SimpleOrder ask1(false, 1251, 100);
  SimpleOrder ask0(false, 1250, 100);
  SimpleOrder bid0(true,  1250, 300);
  SimpleOrder bid1(true,  1249, 100);
  SimpleOrder bid2(true,  1248, 100);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &ask0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask1, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask2, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid1, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid2, false));

  // Verify sizes
  ASSERT_EQ(2, order_book.bids().size());
  ASSERT_EQ(3, order_book.asks().size());

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1249, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1248, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1250, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));

  // Partial Match - will cancel order
  { ASSERT_NO_THROW(
    SimpleFillCheck fc0(&bid0, 100, 125000, IOC);
    SimpleFillCheck fc1(&bid1, 0, 0);
    SimpleFillCheck fc2(&bid2, 0, 0);
    SimpleFillCheck fc3(&ask0, 100, 125000);
    SimpleFillCheck fc4(&ask1, 0, 0);
    SimpleFillCheck fc5(&ask2, 0, 0);
    ASSERT_TRUE(add_and_verify(order_book, &bid0, true, false, IOC));
  ); }

  // Verify sizes
  ASSERT_EQ(2, order_book.bids().size());
  ASSERT_EQ(2, order_book.asks().size());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1249, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1248, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));
}

TEST(TestIocBidFullMatch)
{
  SimpleOrderBook order_book;
  SimpleOrder ask2(false, 1252, 100);
  SimpleOrder ask1(false, 1251, 100);
  SimpleOrder ask0(false, 1250, 400);
  SimpleOrder bid0(true,  1250, 300);
  SimpleOrder bid1(true,  1249, 100);
  SimpleOrder bid2(true,  1248, 100);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &ask0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask1, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask2, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid1, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid2, false));

  // Verify sizes
  ASSERT_EQ(2, order_book.bids().size());
  ASSERT_EQ(3, order_book.asks().size());

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1249, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1248, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1250, 1, 400));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));

  // Full Match - will complete order
  { ASSERT_NO_THROW(
    SimpleFillCheck fc0(&bid0, 300, 1250 * 300, IOC);
    SimpleFillCheck fc1(&bid1, 0, 0);
    SimpleFillCheck fc2(&bid2, 0, 0);
    SimpleFillCheck fc3(&ask0, 300, 1250 * 300);
    SimpleFillCheck fc4(&ask1, 0, 0);
    SimpleFillCheck fc5(&ask2, 0, 0);
    ASSERT_TRUE(add_and_verify(order_book, &bid0, true, true, IOC));
  ); }

  // Verify sizes
  ASSERT_EQ(2, order_book.bids().size());
  ASSERT_EQ(3, order_book.asks().size());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1249, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1248, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1250, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));
}

TEST(TestIocBidMultiMatch)
{
  SimpleOrderBook order_book;
  SimpleOrder ask2(false, 1252, 100);
  SimpleOrder ask1(false, 1251, 100);
  SimpleOrder ask0(false, 1250, 400);
  SimpleOrder bid0(true,  1251, 500);
  SimpleOrder bid1(true,  1249, 100);
  SimpleOrder bid2(true,  1248, 100);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &ask0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask1, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask2, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid1, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid2, false));

  // Verify sizes
  ASSERT_EQ(2, order_book.bids().size());
  ASSERT_EQ(3, order_book.asks().size());

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1249, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1248, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1250, 1, 400));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));

  // Full Match - will complete order
  { ASSERT_NO_THROW(
    SimpleFillCheck fc0(&bid0, 500, 625100, IOC);
    SimpleFillCheck fc1(&bid1, 0, 0);
    SimpleFillCheck fc2(&bid2, 0, 0);
    SimpleFillCheck fc3(&ask0, 400, 1250 * 400);
    SimpleFillCheck fc4(&ask1, 100, 1251 * 100);
    SimpleFillCheck fc5(&ask2, 0, 0);
    ASSERT_TRUE(add_and_verify(order_book, &bid0, true, true, IOC));
  ); }

  // Verify sizes
  ASSERT_EQ(2, order_book.bids().size());
  ASSERT_EQ(1, order_book.asks().size());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1249, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1248, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));
}

TEST(TestFokBidNoMatch)
{
  SimpleOrderBook order_book;
  SimpleOrder ask2(false, 1252, 100);
  SimpleOrder ask1(false, 1251, 100);
  SimpleOrder ask0(false, 1250, 100);
  SimpleOrder bid0(true,  1250, 100);
  SimpleOrder bid1(true,  1249, 100);
  SimpleOrder bid2(true,  1248, 100);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &ask1, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask2, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid1, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid2, false));

  // Verify sizes
  ASSERT_EQ(2, order_book.bids().size());
  ASSERT_EQ(2, order_book.asks().size());

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1249, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1248, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));

  // No Match - will cancel order
  { ASSERT_NO_THROW(
    SimpleFillCheck fc0(&bid0, 0, 0, FOK);
    SimpleFillCheck fc1(&bid1, 0, 0);
    SimpleFillCheck fc2(&bid2, 0, 0);
    //SimpleFillCheck fc3(&ask0, 0, 0);
    SimpleFillCheck fc4(&ask1, 0, 0);
    SimpleFillCheck fc5(&ask2, 0, 0);
    ASSERT_TRUE(add_and_verify(order_book, &bid0, false, false, FOK));
  ); }

  // Verify sizes
  ASSERT_EQ(2, order_book.bids().size());
  ASSERT_EQ(2, order_book.asks().size());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1249, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1248, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));
}

TEST(TestFokBidPartialMatch)
{
  SimpleOrderBook order_book;
  SimpleOrder ask2(false, 1252, 100);
  SimpleOrder ask1(false, 1251, 100);
  SimpleOrder ask0(false, 1250, 100);
  SimpleOrder bid0(true,  1250, 300);
  SimpleOrder bid1(true,  1249, 100);
  SimpleOrder bid2(true,  1248, 100);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &ask0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask1, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask2, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid1, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid2, false));

  // Verify sizes
  ASSERT_EQ(2, order_book.bids().size());
  ASSERT_EQ(3, order_book.asks().size());

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1249, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1248, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1250, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));

  // Partial Match - will not fill and will cancel order
  { ASSERT_NO_THROW(
    SimpleFillCheck fc0(&bid0, 0, 0, FOK);
    SimpleFillCheck fc1(&bid1, 0, 0);
    SimpleFillCheck fc2(&bid2, 0, 0);
    SimpleFillCheck fc3(&ask0, 0, 0);
    SimpleFillCheck fc4(&ask1, 0, 0);
    SimpleFillCheck fc5(&ask2, 0, 0);
    ASSERT_TRUE(add_and_verify(order_book, &bid0, false, false, FOK));
  ); }

  // Verify sizes
  ASSERT_EQ(2, order_book.bids().size());
  ASSERT_EQ(3, order_book.asks().size());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1249, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1248, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1250, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));
}

TEST(TestFokBidFullMatch)
{
  SimpleOrderBook order_book;
  SimpleOrder ask2(false, 1252, 100);
  SimpleOrder ask1(false, 1251, 100);
  SimpleOrder ask0(false, 1250, 400);
  SimpleOrder bid0(true,  1250, 300);
  SimpleOrder bid1(true,  1249, 100);
  SimpleOrder bid2(true,  1248, 100);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &ask0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask1, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask2, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid1, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid2, false));

  // Verify sizes
  ASSERT_EQ(2, order_book.bids().size());
  ASSERT_EQ(3, order_book.asks().size());

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1249, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1248, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1250, 1, 400));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));

  // Full Match - will complete order
  { ASSERT_NO_THROW(
    SimpleFillCheck fc0(&bid0, 300, 1250 * 300, FOK);
    SimpleFillCheck fc1(&bid1, 0, 0);
    SimpleFillCheck fc2(&bid2, 0, 0);
    SimpleFillCheck fc3(&ask0, 300, 1250 * 300);
    SimpleFillCheck fc4(&ask1, 0, 0);
    SimpleFillCheck fc5(&ask2, 0, 0);
    ASSERT_TRUE(add_and_verify(order_book, &bid0, true, true, FOK));
  ); }

  // Verify sizes
  ASSERT_EQ(2, order_book.bids().size());
  ASSERT_EQ(3, order_book.asks().size());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1249, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1248, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1250, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));
}

TEST(TestFokBidMultiMatch)
{
  SimpleOrderBook order_book;
  SimpleOrder ask2(false, 1252, 100);
  SimpleOrder ask1(false, 1251, 100);
  SimpleOrder ask0(false, 1250, 400);
  SimpleOrder bid0(true,  1251, 500);
  SimpleOrder bid1(true,  1249, 100);
  SimpleOrder bid2(true,  1248, 100);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &ask0, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask1, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask2, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid1, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid2, false));

  // Verify sizes
  ASSERT_EQ(2, order_book.bids().size());
  ASSERT_EQ(3, order_book.asks().size());

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1249, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1248, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1250, 1, 400));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));

  // Full Match - will complete order
  { ASSERT_NO_THROW(
    SimpleFillCheck fc0(&bid0, 500, 625100, FOK);
    SimpleFillCheck fc1(&bid1, 0, 0);
    SimpleFillCheck fc2(&bid2, 0, 0);
    SimpleFillCheck fc3(&ask0, 400, 1250 * 400);
    SimpleFillCheck fc4(&ask1, 100, 1251 * 100);
    SimpleFillCheck fc5(&ask2, 0, 0);
    ASSERT_TRUE(add_and_verify(order_book, &bid0, true, true, FOK));
  ); }

  // Verify sizes
  ASSERT_EQ(2, order_book.bids().size());
  ASSERT_EQ(1, order_book.asks().size());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1249, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1248, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));
}

TEST(TestIocAskNoMatch)
{
  SimpleOrderBook order_book;
  SimpleOrder ask2(false, 1252, 100);
  SimpleOrder ask1(false, 1251, 100);
  SimpleOrder ask0(false, 1250, 100);
  SimpleOrder bid0(true,  1250, 100);
  SimpleOrder bid1(true,  1249, 100);
  SimpleOrder bid2(true,  1248, 100);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &ask1, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask2, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid1, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid2, false));

  // Verify sizes
  ASSERT_EQ(2, order_book.bids().size());
  ASSERT_EQ(2, order_book.asks().size());

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1249, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1248, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));

  // No Match - will cancel order
  { ASSERT_NO_THROW(
    // SimpleFillCheck fc0(&bid0, 0, 0);
    SimpleFillCheck fc1(&bid1, 0, 0);
    SimpleFillCheck fc2(&bid2, 0, 0);
    SimpleFillCheck fc3(&ask0, 0, 0, IOC);
    SimpleFillCheck fc4(&ask1, 0, 0);
    SimpleFillCheck fc5(&ask2, 0, 0);
    ASSERT_TRUE(add_and_verify(order_book, &ask0, false, false, IOC));
  ); }

  // Verify sizes
  ASSERT_EQ(2, order_book.bids().size());
  ASSERT_EQ(2, order_book.asks().size());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1249, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1248, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));
}

TEST(TestIocAskPartialMatch)
{
  SimpleOrderBook order_book;
  SimpleOrder ask2(false, 1252, 100);
  SimpleOrder ask1(false, 1251, 100);
  SimpleOrder ask0(false, 1250, 300);
  SimpleOrder bid0(true,  1250, 100);
  SimpleOrder bid1(true,  1249, 100);
  SimpleOrder bid2(true,  1248, 100);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &ask1, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask2, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid0, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid1, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid2, false));

  // Verify sizes
  ASSERT_EQ(3, order_book.bids().size());
  ASSERT_EQ(2, order_book.asks().size());

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1249, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1248, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));

  // Partial Match - will cancel order
  { ASSERT_NO_THROW(
    SimpleFillCheck fc0(&bid0, 100, 125000);
    SimpleFillCheck fc1(&bid1, 0, 0);
    SimpleFillCheck fc2(&bid2, 0, 0);
    SimpleFillCheck fc3(&ask0, 100, 125000, IOC);
    SimpleFillCheck fc4(&ask1, 0, 0);
    SimpleFillCheck fc5(&ask2, 0, 0);
    ASSERT_TRUE(add_and_verify(order_book, &ask0, true, false, IOC));
  ); }

  // Verify sizes
  ASSERT_EQ(2, order_book.bids().size());
  ASSERT_EQ(2, order_book.asks().size());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1249, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1248, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));
}

TEST(TestIocAskFullMatch)
{
  SimpleOrderBook order_book;
  SimpleOrder ask2(false, 1252, 100);
  SimpleOrder ask1(false, 1251, 100);
  SimpleOrder ask0(false, 1250, 300);
  SimpleOrder bid0(true,  1250, 300);
  SimpleOrder bid1(true,  1249, 100);
  SimpleOrder bid2(true,  1248, 100);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &ask1, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask2, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid0, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid1, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid2, false));

  // Verify sizes
  ASSERT_EQ(3, order_book.bids().size());
  ASSERT_EQ(2, order_book.asks().size());

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1250, 1, 300));
  ASSERT_TRUE(dc.verify_bid(1249, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1248, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));

  // Full match
  { ASSERT_NO_THROW(
    SimpleFillCheck fc0(&bid0, 300, 1250 * 300);
    SimpleFillCheck fc1(&bid1, 0, 0);
    SimpleFillCheck fc2(&bid2, 0, 0);
    SimpleFillCheck fc3(&ask0, 300, 1250 * 300, IOC);
    SimpleFillCheck fc4(&ask1, 0, 0);
    SimpleFillCheck fc5(&ask2, 0, 0);
    ASSERT_TRUE(add_and_verify(order_book, &ask0, true, true, IOC));
  ); }

  // Verify sizes
  ASSERT_EQ(2, order_book.bids().size());
  ASSERT_EQ(2, order_book.asks().size());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1249, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1248, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));
}

TEST(TestIocAskMultiMatch)
{
  SimpleOrderBook order_book;
  SimpleOrder ask2(false, 1252, 100);
  SimpleOrder ask1(false, 1251, 100);
  SimpleOrder ask0(false, 1249, 400);
  SimpleOrder bid0(true,  1250, 300);
  SimpleOrder bid1(true,  1249, 100);
  SimpleOrder bid2(true,  1248, 100);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &ask1, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask2, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid0, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid1, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid2, false));

  // Verify sizes
  ASSERT_EQ(3, order_book.bids().size());
  ASSERT_EQ(2, order_book.asks().size());

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1250, 1, 300));
  ASSERT_TRUE(dc.verify_bid(1249, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1248, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));

  // Full match
  { ASSERT_NO_THROW(
    SimpleFillCheck fc0(&bid0, 300, 1250 * 300);
    SimpleFillCheck fc1(&bid1, 100, 1249 * 100);
    SimpleFillCheck fc2(&bid2, 0, 0);
    SimpleFillCheck fc3(&ask0, 400, 499900, IOC);
    SimpleFillCheck fc4(&ask1, 0, 0);
    SimpleFillCheck fc5(&ask2, 0, 0);
    ASSERT_TRUE(add_and_verify(order_book, &ask0, true, true, IOC));
  ); }

  // Verify sizes
  ASSERT_EQ(1, order_book.bids().size());
  ASSERT_EQ(2, order_book.asks().size());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1248, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));
}

TEST(TestFokAskNoMatch)
{
  SimpleOrderBook order_book;
  SimpleOrder ask2(false, 1252, 100);
  SimpleOrder ask1(false, 1251, 100);
  SimpleOrder ask0(false, 1250, 100);
  SimpleOrder bid0(true,  1250, 100);
  SimpleOrder bid1(true,  1249, 100);
  SimpleOrder bid2(true,  1248, 100);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &ask1, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask2, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid1, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid2, false));

  // Verify sizes
  ASSERT_EQ(2, order_book.bids().size());
  ASSERT_EQ(2, order_book.asks().size());

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1249, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1248, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));

  // No Match - will cancel order
  { ASSERT_NO_THROW(
    // SimpleFillCheck fc0(&bid0, 0, 0);
    SimpleFillCheck fc1(&bid1, 0, 0);
    SimpleFillCheck fc2(&bid2, 0, 0);
    SimpleFillCheck fc3(&ask0, 0, 0, FOK);
    SimpleFillCheck fc4(&ask1, 0, 0);
    SimpleFillCheck fc5(&ask2, 0, 0);
    ASSERT_TRUE(add_and_verify(order_book, &ask0, false, false, FOK));
  ); }

  // Verify sizes
  ASSERT_EQ(2, order_book.bids().size());
  ASSERT_EQ(2, order_book.asks().size());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1249, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1248, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));
}

TEST(TestFokAskPartialMatch)
{
  SimpleOrderBook order_book;
  SimpleOrder ask2(false, 1252, 100);
  SimpleOrder ask1(false, 1251, 100);
  SimpleOrder ask0(false, 1250, 300);
  SimpleOrder bid0(true,  1250, 100);
  SimpleOrder bid1(true,  1249, 100);
  SimpleOrder bid2(true,  1248, 100);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &ask1, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask2, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid0, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid1, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid2, false));

  // Verify sizes
  ASSERT_EQ(3, order_book.bids().size());
  ASSERT_EQ(2, order_book.asks().size());

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1249, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1248, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));

  // Partial Match - will not fill and will cancel order
  { ASSERT_NO_THROW(
    SimpleFillCheck fc0(&bid0, 0, 0);
    SimpleFillCheck fc1(&bid1, 0, 0);
    SimpleFillCheck fc2(&bid2, 0, 0);
    SimpleFillCheck fc3(&ask0, 0, 0, FOK);
    SimpleFillCheck fc4(&ask1, 0, 0);
    SimpleFillCheck fc5(&ask2, 0, 0);
    ASSERT_TRUE(add_and_verify(order_book, &ask0, false, false, FOK));
  ); }

  // Verify sizes
  ASSERT_EQ(3, order_book.bids().size());
  ASSERT_EQ(2, order_book.asks().size());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1249, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1248, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));
}

TEST(TestFokAskFullMatch)
{
  SimpleOrderBook order_book;
  SimpleOrder ask2(false, 1252, 100);
  SimpleOrder ask1(false, 1251, 100);
  SimpleOrder ask0(false, 1250, 300);
  SimpleOrder bid0(true,  1250, 300);
  SimpleOrder bid1(true,  1249, 100);
  SimpleOrder bid2(true,  1248, 100);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &ask1, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask2, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid0, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid1, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid2, false));

  // Verify sizes
  ASSERT_EQ(3, order_book.bids().size());
  ASSERT_EQ(2, order_book.asks().size());

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1250, 1, 300));
  ASSERT_TRUE(dc.verify_bid(1249, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1248, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));

  // Full Match
  { ASSERT_NO_THROW(
    SimpleFillCheck fc0(&bid0, 300, 1250 * 300);
    SimpleFillCheck fc1(&bid1, 0, 0);
    SimpleFillCheck fc2(&bid2, 0, 0);
    SimpleFillCheck fc3(&ask0, 300, 1250 * 300, FOK);
    SimpleFillCheck fc4(&ask1, 0, 0);
    SimpleFillCheck fc5(&ask2, 0, 0);
    ASSERT_TRUE(add_and_verify(order_book, &ask0, true, true, FOK));
  ); }

  // Verify sizes
  ASSERT_EQ(2, order_book.bids().size());
  ASSERT_EQ(2, order_book.asks().size());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1249, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1248, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));
}

TEST(TestFokAskMultiMatch)
{
  SimpleOrderBook order_book;
  SimpleOrder ask2(false, 1252, 100);
  SimpleOrder ask1(false, 1251, 100);
  SimpleOrder ask0(false, 1249, 400);
  SimpleOrder bid0(true,  1250, 300);
  SimpleOrder bid1(true,  1249, 100);
  SimpleOrder bid2(true,  1248, 100);

  // No match
  ASSERT_TRUE(add_and_verify(order_book, &ask1, false));
  ASSERT_TRUE(add_and_verify(order_book, &ask2, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid0, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid1, false));
  ASSERT_TRUE(add_and_verify(order_book, &bid2, false));

  // Verify sizes
  ASSERT_EQ(3, order_book.bids().size());
  ASSERT_EQ(2, order_book.asks().size());

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1250, 1, 300));
  ASSERT_TRUE(dc.verify_bid(1249, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1248, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));

  // Full match
  { ASSERT_NO_THROW(
    SimpleFillCheck fc0(&bid0, 300, 1250 * 300);
    SimpleFillCheck fc1(&bid1, 100, 1249 * 100);
    SimpleFillCheck fc2(&bid2, 0, 0);
    SimpleFillCheck fc3(&ask0, 400, 499900, FOK);
    SimpleFillCheck fc4(&ask1, 0, 0);
    SimpleFillCheck fc5(&ask2, 0, 0);
    ASSERT_TRUE(add_and_verify(order_book, &ask0, true, true, FOK));
  ); }

  // Verify sizes
  ASSERT_EQ(1, order_book.bids().size());
  ASSERT_EQ(2, order_book.asks().size());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1248, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));
}

} // Namespace
