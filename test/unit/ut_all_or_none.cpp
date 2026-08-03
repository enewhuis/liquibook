// Copyright (c) 2012 - 2017 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.

#define BOOST_TEST_NO_MAIN LiquibookTest
#include <boost/test/unit_test.hpp>

#include "ut_utils.h"

namespace liquibook {

using simple::SimpleOrder;
typedef FillCheck<SimpleOrder*> SimpleFillCheck;

namespace {
const OrderConditions AON(oc_all_or_none);
const OrderConditions noConditions(0);

const Quantity qty1 = 100;
const Quantity qty2 = qty1 + qty1;
const Quantity qty3 = qty2 + qty1;
const Quantity qty4 = qty2 + qty2;
const Quantity qty6 = qty2 + qty4;
const Quantity qty7 = 700; // derive this?
const Quantity qtyNone = 0;

const Price prc0 = 1250;
const Price prc1 = 1251;
const Price prc2 = 1252;
const Price prc3 = 1253;
const Price prcNone = 0;
const Price MARKET_ORDER_PRICE = MARKET_ORDER_PRICE;

const bool buySide = true;
const bool sellSide = false;
const bool expectMatch = true;
const bool expectNoMatch = false;
const bool expectComplete = true;
const bool expectNoComplete = false;

}


BOOST_AUTO_TEST_CASE(TestRegBidMatchAon)
{
  SimpleOrderBook order_book;
  SimpleOrder ask2(sellSide, prc2, qty1);
  SimpleOrder ask1(sellSide, prc1, qty1); // AON
  SimpleOrder ask0(sellSide, prc1, qty2); // AON, but skipped
  SimpleOrder bid1(buySide, prc1, qty1);
  SimpleOrder bid0(buySide, prc0, qty1);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &bid0, expectNoMatch));
  BOOST_CHECK(add_and_verify(order_book, &ask0, expectNoMatch, expectNoComplete, AON));
  BOOST_CHECK(add_and_verify(order_book, &ask1, expectNoMatch, expectNoComplete, AON));
  BOOST_CHECK(add_and_verify(order_book, &ask2, expectNoMatch));

  // Verify sizes
  BOOST_CHECK_EQUAL(1UL, order_book.bids().size());
  BOOST_CHECK_EQUAL(3UL, order_book.asks().size());

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(prc0, 1, qty1));
  BOOST_CHECK(dc.verify_ask(prc1, 2, qty1 + qty2));
  BOOST_CHECK(dc.verify_ask(prc2, 1, qty1));

  // Match - complete
  {
    SimpleFillCheck fc1(&bid1, qty1, prc1 * qty1);
    SimpleFillCheck fc2(&ask1, qty1, prc1 * qty1);
    BOOST_CHECK(add_and_verify(order_book, &bid1, expectMatch, expectComplete));
  }

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(prc0, 1, qty1));
  BOOST_CHECK(dc.verify_ask(prc1, 1, qty2));
  BOOST_CHECK(dc.verify_ask(prc2, 1, qty1));

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());
}

BOOST_AUTO_TEST_CASE(TestRegBidMatchMulti)
{
  SimpleOrderBook order_book;
  SimpleOrder ask2(sellSide, prc1, qty7);
  SimpleOrder ask1(sellSide, prc1, qty1); // AON
  SimpleOrder ask0(sellSide, prc1, qty1); // AON
  SimpleOrder bid1(buySide, prc1, qty4);
  SimpleOrder bid0(buySide, prc0, qty1);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &bid0, expectNoMatch));
  BOOST_CHECK(add_and_verify(order_book, &ask0, expectNoMatch, expectNoComplete, AON));
  BOOST_CHECK(add_and_verify(order_book, &ask1, expectNoMatch, expectNoComplete, AON));
  BOOST_CHECK(add_and_verify(order_book, &ask2, expectNoMatch, expectNoComplete));

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(3, order_book.asks().size());

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(prc0, 1, qty1));
  BOOST_CHECK(dc.verify_ask(prc1, 3, qty7 + qty1 + qty1));

  // Match - complete
  {
    SimpleFillCheck fc0(&bid1, qty4, prc1 * qty4);
    SimpleFillCheck fc1(&ask0, qty1, prc1 * qty1);
    SimpleFillCheck fc2(&ask1, qty1, prc1 * qty1);
    SimpleFillCheck fc3(&ask2, qty2, prc1 * qty2);
    BOOST_CHECK(add_and_verify(order_book, &bid1, expectMatch, expectComplete));
  }

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(prc0, 1, qty1));
  BOOST_CHECK(dc.verify_ask(prc1, 1, qty4 + qty1));

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(1, order_book.asks().size());
}

BOOST_AUTO_TEST_CASE(TestAonBidNoMatch)
{
  SimpleOrderBook order_book;
  SimpleOrder ask1(sellSide, prc2, qty1); // no match, price
  SimpleOrder ask0(sellSide, prc1, qty1); 
  SimpleOrder bid1(buySide, prc1, qty3); // no match, AON
  SimpleOrder bid0(buySide, prc0, qty1); // no match, price

  // No match
  BOOST_CHECK(add_and_verify(order_book, &bid0, expectNoMatch));
  BOOST_CHECK(add_and_verify(order_book, &ask0, expectNoMatch));
  BOOST_CHECK(add_and_verify(order_book, &ask1, expectNoMatch));

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(prc0, 1, qty1));
  BOOST_CHECK(dc.verify_ask(prc1, 1, qty1));
  BOOST_CHECK(dc.verify_ask(prc2, 1, qty1));

  // Match - complete
  {
    SimpleFillCheck fc1(&bid1, qtyNone, prcNone);
    SimpleFillCheck fc2(&ask0, qtyNone, prcNone);
    BOOST_CHECK(add_and_verify(order_book, &bid1, expectNoMatch, expectNoComplete, AON));
  }

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(prc1, 1, qty3));
  BOOST_CHECK(dc.verify_bid(prc0, 1, qty1));
  BOOST_CHECK(dc.verify_ask(prc1, 1, qty1));
  BOOST_CHECK(dc.verify_ask(prc2, 1, qty1));

  // Verify sizes
  BOOST_CHECK_EQUAL(2, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());
}

BOOST_AUTO_TEST_CASE(TestAonBidMatchReg)
{
  SimpleOrderBook order_book;
  SimpleOrder ask1(sellSide, prc2, qty1);
  SimpleOrder ask0(sellSide, prc1, qty4);
  SimpleOrder bid1(buySide, prc1, qty3); // AON
  SimpleOrder bid0(buySide, prc0, qty1);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &bid0, expectNoMatch));
  BOOST_CHECK(add_and_verify(order_book, &ask0, expectNoMatch));
  BOOST_CHECK(add_and_verify(order_book, &ask1, expectNoMatch));

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(prc0, 1, qty1));
  BOOST_CHECK(dc.verify_ask(prc1, 1, qty4));
  BOOST_CHECK(dc.verify_ask(prc2, 1, qty1));

  // Match - complete
  {
    SimpleFillCheck fc1(&bid1, qty3, prc1 * qty3);
    SimpleFillCheck fc2(&ask0, qty3, prc1 * qty3);
    BOOST_CHECK(add_and_verify(order_book, &bid1, expectMatch, expectComplete, AON));
  }

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(prc0, 1, qty1));
  BOOST_CHECK(dc.verify_ask(prc1, 1, qty1));
  BOOST_CHECK(dc.verify_ask(prc2, 1, qty1));

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());
}

BOOST_AUTO_TEST_CASE(TestAonBidMatchMulti)
{
  SimpleOrderBook order_book;
  SimpleOrder ask3(sellSide, prc2, qty1);
  SimpleOrder ask2(sellSide, prc2, qty1);
  SimpleOrder ask1(sellSide, prc1, qty4); // AON no match
  SimpleOrder ask0(sellSide, prc1, qty4);
  SimpleOrder bid1(buySide, MARKET_ORDER_PRICE, qty6); // AON
  SimpleOrder bid0(buySide, prc0, qty1);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &bid0, expectNoMatch));
  BOOST_CHECK(add_and_verify(order_book, &ask0, expectNoMatch));
  BOOST_CHECK(add_and_verify(order_book, &ask1, expectNoMatch, expectNoComplete, AON));
  BOOST_CHECK(add_and_verify(order_book, &ask2, expectNoMatch));
  BOOST_CHECK(add_and_verify(order_book, &ask3, expectNoMatch));

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(4, order_book.asks().size());

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(prc0, 1, qty1));
  BOOST_CHECK(dc.verify_ask(prc1, 2, qty4 + qty4));
  BOOST_CHECK(dc.verify_ask(prc2, 2, qty1 + qty1));

  // Match - complete
  { 
  //ASSERT_NO_THROW(
    SimpleFillCheck fc1(&bid1, qty6, prc1 * qty2 + prc1 * qty4);
    SimpleFillCheck fc2(&ask0, qty2, prc1 * qty2);
    SimpleFillCheck fc3(&ask1, qty4, prc1 * qty4);
    SimpleFillCheck fc4(&ask2, 0, prc2 * 0);
    SimpleFillCheck fc5(&ask3, 0, prc2 * 0);
    BOOST_CHECK(add_and_verify(order_book, &bid1, expectMatch, expectComplete, AON));
  //); 
  }

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(prc0, 1, qty1));
  BOOST_CHECK(dc.verify_ask(prc1, 1, qty2));
  BOOST_CHECK(dc.verify_ask(prc2, 2, qty1 + qty1));

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(3, order_book.asks().size());
}

BOOST_AUTO_TEST_CASE(TestAonBidNoMatchMulti)
{
  SimpleOrderBook order_book;
  SimpleOrder ask2(sellSide, prc2, qty4); // AON no match
  SimpleOrder ask1(sellSide, prc2, qty1);
  SimpleOrder ask0(sellSide, prc1, qty4);
  SimpleOrder bid1(buySide, MARKET_ORDER_PRICE, qty6); // AON
  SimpleOrder bid0(buySide, prc0, qty1);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &bid0, expectNoMatch));
  BOOST_CHECK(add_and_verify(order_book, &ask0, expectNoMatch));
  BOOST_CHECK(add_and_verify(order_book, &ask1, expectNoMatch));
  BOOST_CHECK(add_and_verify(order_book, &ask2, expectNoMatch, expectNoComplete, AON));

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(3, order_book.asks().size());

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(prc0, 1, qty1));
  BOOST_CHECK(dc.verify_ask(prc1, 1, qty4));
  BOOST_CHECK(dc.verify_ask(prc2, 2, qty4 + qty1));

  // Match - complete
  { 
  //ASSERT_NO_THROW(
    SimpleFillCheck fc0(&bid0, qtyNone, prcNone);
    SimpleFillCheck fc1(&bid1, qty6, qty2 * prc1 + qty4 * prc2); // filled 600 @ 751000
    SimpleFillCheck fc2(&ask0, qty2, qty2 * prc1); // filled 200 @ 250200
    SimpleFillCheck fc3(&ask1, qtyNone, prcNone); // 0
    SimpleFillCheck fc4(&ask2, qty4, qty4 * prc2); // filled 400 @ 500800
    BOOST_CHECK(add_and_verify(order_book, &bid1, expectMatch, expectComplete, AON));
  //); 
  }

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(prc0, 1, qty1));
  BOOST_CHECK(dc.verify_ask(prc1, 1, qty2));
}

BOOST_AUTO_TEST_CASE(TestAonBidMatchAon)
{
  SimpleOrderBook order_book;
  SimpleOrder ask1(sellSide, prc2, qty1);
  SimpleOrder ask0(sellSide, prc1, qty3); // AON
  SimpleOrder bid1(buySide, prc1, qty3); // AON
  SimpleOrder bid0(buySide, prc0, qty1);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &bid0, expectNoMatch));
  BOOST_CHECK(add_and_verify(order_book, &ask0, expectNoMatch, expectNoComplete, AON));
  BOOST_CHECK(add_and_verify(order_book, &ask1, expectNoMatch));

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(prc0, 1, qty1));
  BOOST_CHECK(dc.verify_ask(prc1, 1, qty3));
  BOOST_CHECK(dc.verify_ask(prc2, 1, qty1));

  // Match - complete
  {
    SimpleFillCheck fc1(&bid1, qty3, prc1 * qty3);
    SimpleFillCheck fc2(&ask0, qty3, prc1 * qty3);
    BOOST_CHECK(add_and_verify(order_book, &bid1, expectMatch, expectComplete, AON));
  }

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(prc0, 1, qty1));
  BOOST_CHECK(dc.verify_ask(prc2, 1, qty1));

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(1, order_book.asks().size());
}

BOOST_AUTO_TEST_CASE(TestRegAskMatchAon)
{
  SimpleOrderBook order_book;
  SimpleOrder ask0(sellSide, prc2, qty1);
  SimpleOrder ask1(sellSide, prc1, qty1);
  SimpleOrder bid1(buySide, prc1, qty2); // AON, but skipped
  SimpleOrder bid2(buySide, prc1, qty1); // AON
  SimpleOrder bid0(buySide, prc0, qty1);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &bid0, expectNoMatch));
  BOOST_CHECK(add_and_verify(order_book, &bid1, expectNoMatch, expectNoComplete, AON));
  BOOST_CHECK(add_and_verify(order_book, &bid2, expectNoMatch, expectNoComplete, AON));
  BOOST_CHECK(add_and_verify(order_book, &ask0, expectNoMatch));

  // Verify sizes
  BOOST_CHECK_EQUAL(3, order_book.bids().size());
  BOOST_CHECK_EQUAL(1, order_book.asks().size());

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(prc1, 2, qty3));
  BOOST_CHECK(dc.verify_bid(prc0, 1, qty1));
  BOOST_CHECK(dc.verify_ask(prc2, 1, qty1));

  // Match - complete
  {
    SimpleFillCheck fc1(&bid2, qty1, prc1 * qty1);
    SimpleFillCheck fc2(&ask1, qty1, prc1 * qty1);
    BOOST_CHECK(add_and_verify(order_book, &ask1, expectMatch, expectComplete));
  }

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(prc1, 1, qty2));
  BOOST_CHECK(dc.verify_bid(prc0, 1, qty1));
  BOOST_CHECK(dc.verify_ask(prc2, 1, qty1));

  // Verify sizes
  BOOST_CHECK_EQUAL(2, order_book.bids().size());
  BOOST_CHECK_EQUAL(1, order_book.asks().size());
}

BOOST_AUTO_TEST_CASE(TestRegAskMatchMulti)
{
  SimpleOrderBook order_book;
  SimpleOrder ask0(sellSide, prc2, qty1);
  SimpleOrder ask1(sellSide, prc0, qty4);
  SimpleOrder bid1(buySide, prc1, qty1); // AON
  SimpleOrder bid2(buySide, prc1, qty1); // AON
  SimpleOrder bid0(buySide, prc0, qty7);

  // Calculate some expected results
  // ask1(400) matches bid 1(100), bid2(100), and part(200) of bid0 
  // leaving 500 shares of bid 0)
  Quantity bid0FillQty = qty4 - qty1 - qty1;
  Quantity bid0RemainQty = qty7 - bid0FillQty;
  uint32_t bid0FillAmount = bid0FillQty * prc0;
  uint32_t bid1FillAmount = prc1 * qty1;
  uint32_t bid2FillAmount = prc1 * qty1;
  uint32_t ask1FillAmount = bid1FillAmount + bid2FillAmount + bid0FillAmount;
  
  // No match
  BOOST_CHECK(add_and_verify(order_book, &ask0, expectNoMatch));
  BOOST_CHECK(add_and_verify(order_book, &bid0, expectNoMatch));
  BOOST_CHECK(add_and_verify(order_book, &bid1, expectNoMatch, expectNoComplete, AON));
  BOOST_CHECK(add_and_verify(order_book, &bid2, expectNoMatch, expectNoComplete, AON));

  // Verify sizes
  BOOST_CHECK_EQUAL(3, order_book.bids().size());
  BOOST_CHECK_EQUAL(1, order_book.asks().size());

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(prc1, 2, qty2));
  BOOST_CHECK(dc.verify_bid(prc0, 1, qty7));
  BOOST_CHECK(dc.verify_ask(prc2, 1, qty1));

  // Match - complete
  {
    SimpleFillCheck fc0(&bid1, qty1, prc1 * qty1);
    SimpleFillCheck fc1(&bid2, qty1, prc1 * qty1);
    SimpleFillCheck fc2(&bid0, qty2, prc0 * qty2);
    SimpleFillCheck fc3(&ask1, qty4, ask1FillAmount);
    BOOST_CHECK(add_and_verify(order_book, &ask1, expectMatch, expectComplete));
  }

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(prc0, 1, bid0RemainQty));
  BOOST_CHECK(dc.verify_ask(prc2, 1, qty1));

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(1, order_book.asks().size());
}

BOOST_AUTO_TEST_CASE(TestAonAskNoMatch)
{
  SimpleOrderBook order_book;
  SimpleOrder ask0(sellSide, prc2, qty1);
  SimpleOrder ask1(sellSide, prc1, qty4); // AON
  SimpleOrder bid1(buySide, prc1, qty1);
  SimpleOrder bid2(buySide, prc1, qty1);
  SimpleOrder bid0(buySide, prc0, qty7);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &ask0, expectNoMatch));
  BOOST_CHECK(add_and_verify(order_book, &bid0, expectNoMatch));
  BOOST_CHECK(add_and_verify(order_book, &bid1, expectNoMatch));
  BOOST_CHECK(add_and_verify(order_book, &bid2, expectNoMatch));

  // Verify sizes
  BOOST_CHECK_EQUAL(3, order_book.bids().size());
  BOOST_CHECK_EQUAL(1, order_book.asks().size());

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(prc1, 2, qty2));
  BOOST_CHECK(dc.verify_bid(prc0, 1, qty7));
  BOOST_CHECK(dc.verify_ask(prc2, 1, qty1));

  // Match - complete
  {
    SimpleFillCheck fc0(&bid1, qtyNone, prcNone);
    SimpleFillCheck fc1(&bid2, qtyNone, prcNone);
    SimpleFillCheck fc2(&bid0, qtyNone, prcNone);
    SimpleFillCheck fc3(&ask1, qtyNone, prcNone);
    BOOST_CHECK(add_and_verify(order_book, &ask1, expectNoMatch, expectNoComplete, AON));
  }

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(prc1, 2, qty2));
  BOOST_CHECK(dc.verify_bid(prc0, 1, qty7));
  BOOST_CHECK(dc.verify_ask(prc1, 1, qty4));
  BOOST_CHECK(dc.verify_ask(prc2, 1, qty1));

  // Verify sizes
  BOOST_CHECK_EQUAL(3, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());
}

BOOST_AUTO_TEST_CASE(TestAonAskMatchReg)
{
  SimpleOrderBook order_book;
  SimpleOrder ask0(sellSide, prc2, qty1);
  SimpleOrder ask1(sellSide, prc1, qty1); // AON
  SimpleOrder bid1(buySide, prc1, qty1);
  SimpleOrder bid0(buySide, prc0, qty7);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &ask0, expectNoMatch));
  BOOST_CHECK(add_and_verify(order_book, &bid0, expectNoMatch));
  BOOST_CHECK(add_and_verify(order_book, &bid1, expectNoMatch));

  // Verify sizes
  BOOST_CHECK_EQUAL(2, order_book.bids().size());
  BOOST_CHECK_EQUAL(1, order_book.asks().size());

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(prc1, 1, qty1));
  BOOST_CHECK(dc.verify_bid(prc0, 1, qty7));
  BOOST_CHECK(dc.verify_ask(prc2, 1, qty1));

  // Match - complete
  {
    SimpleFillCheck fc0(&bid1, qty1, prc1 * qty1);
    SimpleFillCheck fc3(&ask1, qty1, prc1 * qty1);
    BOOST_CHECK(add_and_verify(order_book, &ask1, expectMatch, expectComplete, AON));
  }

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(prc0, 1, qty7));
  BOOST_CHECK(dc.verify_ask(prc2, 1, qty1));

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(1, order_book.asks().size());
}

BOOST_AUTO_TEST_CASE(TestAonAskMatchMulti)
{
  SimpleOrderBook order_book;
  SimpleOrder ask0(sellSide, prc2, qty1); // no match due to price
  SimpleOrder ask1(sellSide, prc0, qty6); // AON
  SimpleOrder bid1(buySide, prc1, qty1); // AON
  SimpleOrder bid2(buySide, prc1, qty1);
  SimpleOrder bid3(buySide, prc1, qty1);
  SimpleOrder bid0(buySide, prc0, qty7);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &ask0, expectNoMatch));
  BOOST_CHECK(add_and_verify(order_book, &bid0, expectNoMatch));
  BOOST_CHECK(add_and_verify(order_book, &bid1, expectNoMatch, expectNoComplete, AON));
  BOOST_CHECK(add_and_verify(order_book, &bid2, expectNoMatch));
  BOOST_CHECK(add_and_verify(order_book, &bid3, expectNoMatch));

  // Verify sizes
  BOOST_CHECK_EQUAL(4, order_book.bids().size());
  BOOST_CHECK_EQUAL(1, order_book.asks().size());

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(prc1, 3, qty3));
  BOOST_CHECK(dc.verify_bid(prc0, 1, qty7));
  BOOST_CHECK(dc.verify_ask(prc2, 1, qty1));

  // Match - complete
  { 
  // ASSERT_NO_THROW(
    
    uint32_t b1Cost = prc1 * qty1;
    SimpleFillCheck fc0(&bid1, qty1, b1Cost);
    uint32_t b2Cost = prc1 * qty1;
    SimpleFillCheck fc1(&bid2, qty1, b2Cost);
    uint32_t b3Cost = prc1 * qty1;
    SimpleFillCheck fc2(&bid3, qty1, b3Cost);
    Quantity b0Fill = qty6 - qty1 - qty1 - qty1;
    uint32_t b0Cost = b0Fill * prc0;
    SimpleFillCheck fc3(&bid0, b0Fill, b0Cost);
    uint32_t a1Cost = b0Cost + b1Cost +b2Cost + b3Cost;
    SimpleFillCheck fc4(&ask1, qty6, a1Cost);
    BOOST_CHECK(add_and_verify(order_book, &ask1, expectMatch, expectComplete, AON));
  // ); 
  }

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(prc0, 1, qty4));
  BOOST_CHECK(dc.verify_ask(prc2, 1, qty1));

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(1, order_book.asks().size());
}
///////////////////

BOOST_AUTO_TEST_CASE(TestOneAonBidOneAonAsk)
{
    SimpleOrderBook order_book;
    SimpleOrder bid1(buySide,prc1,qty1); // AON
    SimpleOrder ask1(sellSide,prc1,qty1); // AON

    // Prime the order book: No Matches
    BOOST_CHECK(add_and_verify(order_book,&bid1,expectNoMatch,expectNoComplete,AON));

    // Verify sizes
    BOOST_CHECK_EQUAL(1u,order_book.bids().size());
    BOOST_CHECK_EQUAL(0u,order_book.asks().size());

    // Verify depth
    DepthCheck<SimpleOrderBook> dc(order_book.depth());
    BOOST_CHECK(dc.verify_bid(prc1 , 1, qty1));

    // Add matching order
    {
        SimpleFillCheck fc1(&bid1,qty1, qty1 * prc1);
        SimpleFillCheck fc3(&ask1,qty1, qty1 * prc1);
        BOOST_CHECK(add_and_verify(order_book, &ask1, expectMatch, expectComplete, AON));
    }

    // Verify sizes
    BOOST_CHECK_EQUAL(0,order_book.bids().size());
    BOOST_CHECK_EQUAL(0,order_book.asks().size());
}

BOOST_AUTO_TEST_CASE(TestTwoAonBidOneAonAsk)
{
    SimpleOrderBook order_book;
    SimpleOrder bid1(buySide,prc1,qty1); // AON
    SimpleOrder bid2(buySide,prc1,qty2); // AON
    SimpleOrder ask1(sellSide,prc1,qty3); // AON

    // Prime the order book: No Matches
    BOOST_CHECK(add_and_verify(order_book, &bid1, expectNoMatch, expectNoComplete,AON));//AON)); //noConditions
    BOOST_CHECK(add_and_verify(order_book, &bid2,expectNoMatch,expectNoComplete,AON));

    // Verify sizes
    BOOST_CHECK_EQUAL(2u,order_book.bids().size());
    BOOST_CHECK_EQUAL(0u,order_book.asks().size());

    // Verify depth
    DepthCheck<SimpleOrderBook> dc(order_book.depth());
    BOOST_CHECK(dc.verify_bid(prc1, 2, qty1 + qty2));

    // Add matching order
    {
        SimpleFillCheck fc1(&bid1, qty1, qty1 * prc1);
        SimpleFillCheck fc2(&bid2, qty2, qty2 * prc1);
        SimpleFillCheck fc3(&ask1, qty3, qty3 * prc1);
        BOOST_CHECK(add_and_verify(order_book, &ask1, expectMatch, expectComplete, AON));
    }

    // Verify sizes
    BOOST_CHECK_EQUAL(0,order_book.bids().size());
    BOOST_CHECK_EQUAL(0,order_book.asks().size());

}

BOOST_AUTO_TEST_CASE(TestOneAonBidTwoAsk)
{
    SimpleOrderBook order_book;

    SimpleOrder bid1(buySide,prc1,qty3); // AON
    SimpleOrder ask1(sellSide,prc1,qty1); // No Conditions
    SimpleOrder ask2(sellSide,prc1,qty2); // No Conditions

    // Prime the order book: No Matches
    BOOST_CHECK(add_and_verify(order_book,&bid1,expectNoMatch,expectNoComplete,AON));//AON)); //noConditions

    // Add an order that does NOT meet the AON condition
    BOOST_CHECK(add_and_verify(order_book,&ask1,expectNoMatch,expectNoComplete,noConditions));
    // Verify sizes
    BOOST_CHECK_EQUAL(1u,order_book.bids().size());
    BOOST_CHECK_EQUAL(1u,order_book.asks().size());

    // Verify depth
    DepthCheck<SimpleOrderBook> dc(order_book.depth());
    BOOST_CHECK(dc.verify_bid(prc1,1,qty3));
    BOOST_CHECK(dc.verify_ask(prc1,1,qty1));

    // Add matching order
    {
        SimpleFillCheck fc1(&bid1,qty3,qty3 * prc1);
    SimpleFillCheck fc2(&ask1,qty1,qty1 * prc1);
    SimpleFillCheck fc3(&ask2,qty2,qty2 * prc1);
    BOOST_CHECK(add_and_verify(order_book,&ask2,expectMatch,expectComplete,noConditions));
    }

    // Verify sizes
    BOOST_CHECK_EQUAL(0,order_book.bids().size());
    BOOST_CHECK_EQUAL(0,order_book.asks().size());
}

BOOST_AUTO_TEST_CASE(TestOneBidTwoAonAsk)
{
    SimpleOrderBook order_book;

    SimpleOrder bid1(buySide,prc1,qty3); // noConditions
    SimpleOrder ask1(sellSide,prc1,qty1); // AON 
    SimpleOrder ask2(sellSide,prc1,qty2); // AON

    // Prime the order book: No Matches
    BOOST_CHECK(add_and_verify(order_book,&bid1,expectNoMatch,expectNoComplete,AON));

    // Verify sizes
    BOOST_CHECK_EQUAL(1u,order_book.bids().size());
    BOOST_CHECK_EQUAL(0u,order_book.asks().size());

    // Verify depth
    DepthCheck<SimpleOrderBook> dc(order_book.depth());
    BOOST_CHECK(dc.verify_bid(prc1,1,qty3));

    // Add matching order
    {
        SimpleFillCheck fc1(&bid1,qty3,qty3 * prc1);
        SimpleFillCheck fc2(&ask1,qty1,qty1 * prc1);
        SimpleFillCheck fc3(&ask2,qty2,qty2 * prc1);
        BOOST_CHECK(add_and_verify(order_book,&ask1,expectNoMatch,expectNoComplete,noConditions));
        BOOST_CHECK(add_and_verify(order_book,&ask2,expectMatch,expectComplete,noConditions));
    }

    // Verify sizes
    BOOST_CHECK_EQUAL(0,order_book.bids().size());
    BOOST_CHECK_EQUAL(0,order_book.asks().size());
}

BOOST_AUTO_TEST_CASE(TestTwoAonBidTwoAonAsk)
{
#if 1
    int todo_FixTestAonsTwoBidTwoAsk;
    std::cout << "***** WARNING TEST " << "TestAonsTwoBidTwoAsk" << " is disabled" << std::endl;
#else
  // The current match algorithm tries to match one order from one side of the market to "N" orders
  // from the other side.   This test won't pass because it requires two orders from each side.
  // I'm leaving the test here as a challenge to future developers who want to improve the matching
  // algorithm (good luck)

  SimpleOrderBook order_book;

  SimpleOrder ask1(sellSide,prc1,qty3); // AON
  SimpleOrder ask2(sellSide,prc1,qty2); // AON

  SimpleOrder bid1(buySide,prc1,qty1); // AON
  SimpleOrder bid2(buySide,prc1,qty4); // AON

                                        // Prime the order book: No Matches
  BOOST_CHECK(add_and_verify(order_book,&bid1,expectNoMatch,expectNoComplete,AON));
  BOOST_CHECK(add_and_verify(order_book,&bid2,expectNoMatch,expectNoComplete,AON));
  BOOST_CHECK(add_and_verify(order_book,&ask1,expectNoMatch,expectNoComplete,AON));

  // Verify sizes
  BOOST_CHECK_EQUAL(2u,order_book.bids().size());
  BOOST_CHECK_EQUAL(1u,order_book.asks().size());

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(prc1,2,qty1 + qty4));
  BOOST_CHECK(dc.verify_ask(prc1,1,qty3));

  // Add matching order
  {
      SimpleFillCheck fc1(&bid1,qty1,qty3 * prc1);
  SimpleFillCheck fc2(&bid2,qty4,qty3 * prc1);
  SimpleFillCheck fc3(&ask1,qty3,qty1 * prc1);
  SimpleFillCheck fc4(&ask2,qty2,qty2 * prc1);
  BOOST_CHECK(add_and_verify(order_book,&ask2,expectMatch,expectComplete,AON));
  }

  // Verify sizes
  BOOST_CHECK_EQUAL(0,order_book.bids().size());
  BOOST_CHECK_EQUAL(0,order_book.asks().size());
#endif
}

BOOST_AUTO_TEST_CASE(TestAonAskNoMatchMulti)
{
  SimpleOrderBook order_book;
  SimpleOrder ask0(sellSide, prc2, qty1); // no match (price)
  SimpleOrder ask1(sellSide, prc0, qty6); // AON

  SimpleOrder bid0(buySide, prc0, qty4); // AON no match
  SimpleOrder bid1(buySide, prc1, qty1); // AON
  SimpleOrder bid2(buySide, prc1, qty4);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &ask0, expectNoMatch));
  BOOST_CHECK(add_and_verify(order_book, &bid0, expectNoMatch, expectNoComplete,AON));
  BOOST_CHECK(add_and_verify(order_book, &bid1, expectNoMatch, expectNoComplete,AON));//AON)); //noConditions
  BOOST_CHECK(add_and_verify(order_book, &bid2, expectNoMatch));

  // Verify sizes
  BOOST_CHECK_EQUAL(3, order_book.bids().size());
  BOOST_CHECK_EQUAL(1, order_book.asks().size());

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(prc1, 2, qty1 + qty4));
  BOOST_CHECK(dc.verify_bid(prc0, 1, qty4));
  BOOST_CHECK(dc.verify_ask(prc2, 1, qty1));

  // This test was bogus -- testing a bug in the matching algorithm
  // I fixed the bug and the test started to fail.
  // So fixed the test to expect:
  // Ask1 (600 AON) should match bid0 (400 AON) + bid1(100) + bid 2(100 of 400)
  //
  // Now we need a new test of an AON that should NOT match!

  // No match
  { 
//  ASSERT_NO_THROW(
    SimpleFillCheck fc0(&bid0, qty4, prc0 * qty4);
    SimpleFillCheck fc1(&bid1, qty1, qty1 * prc1);
    SimpleFillCheck fc2(&bid2, qty1, prc1 * qty1);
    SimpleFillCheck fc3(&ask1, qty6, prc0 * qty4 + qty1 * prc1 + prc1 * qty1);
    BOOST_CHECK(add_and_verify(order_book, &ask1, expectMatch, expectComplete, AON));
  //); 
  }

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(prc1, 1, qty4 - qty1));

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(1, order_book.asks().size());
}

BOOST_AUTO_TEST_CASE(TestAonAskMatchAon)
{
  SimpleOrderBook order_book;
  SimpleOrder ask0(sellSide, prc2, qty1);
  SimpleOrder ask1(sellSide, prc1, qty2); // AON
  SimpleOrder bid1(buySide, prc1, qty2); // AON
  SimpleOrder bid0(buySide, prc0, qty4);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &ask0, expectNoMatch));
  BOOST_CHECK(add_and_verify(order_book, &bid0, expectNoMatch));
  BOOST_CHECK(add_and_verify(order_book, &bid1, expectNoMatch, expectNoComplete, AON));

  // Verify sizes
  BOOST_CHECK_EQUAL(2, order_book.bids().size());
  BOOST_CHECK_EQUAL(1, order_book.asks().size());

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_ask(prc2, 1, qty1));
  BOOST_CHECK(dc.verify_bid(prc1, 1, qty2));
  BOOST_CHECK(dc.verify_bid(prc0, 1, qty4));

  // Match complete
  {
    SimpleFillCheck fc1(&bid1, qty2, prc1 * qty2);
    SimpleFillCheck fc3(&ask1, qty2, prc1 * qty2);
    BOOST_CHECK(add_and_verify(order_book, &ask1, expectMatch, expectComplete, AON));
  }

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_ask(prc2, 1, qty1));
  BOOST_CHECK(dc.verify_bid(prc0, 1, qty4));

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(1, order_book.asks().size());
}

BOOST_AUTO_TEST_CASE(TestReplaceAonBidSmallerMatch)
{
  SimpleOrderBook order_book;
  SimpleOrder ask2(sellSide, prc3, qty1);
  SimpleOrder ask1(sellSide, prc2, qty1);
  SimpleOrder ask0(sellSide, prc1, qty1);
  SimpleOrder bid1(buySide, prc1, qty2); // AON
  SimpleOrder bid0(buySide, prc0, qty1);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &bid0, expectNoMatch));
  BOOST_CHECK(add_and_verify(order_book, &bid1, expectNoMatch, expectNoComplete, AON));
  BOOST_CHECK(add_and_verify(order_book, &ask0, expectNoMatch));
  BOOST_CHECK(add_and_verify(order_book, &ask1, expectNoMatch));
  BOOST_CHECK(add_and_verify(order_book, &ask2, expectNoMatch));

  // Verify sizes
  BOOST_CHECK_EQUAL(2, order_book.bids().size());
  BOOST_CHECK_EQUAL(3, order_book.asks().size());

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(prc1, 1, qty2));
  BOOST_CHECK(dc.verify_bid(prc0, 1, qty1));
  BOOST_CHECK(dc.verify_ask(prc1, 1, qty1));
  BOOST_CHECK(dc.verify_ask(prc2, 1, qty1));
  BOOST_CHECK(dc.verify_ask(prc3, 1, qty1));

  // Match - complete
  {
    SimpleFillCheck fc2(&ask0, qty1, prc1 * qty1);
    BOOST_CHECK(replace_and_verify(
        order_book, &bid1, -(int32_t)qty1, PRICE_UNCHANGED, simple::os_complete, qty1));
  }

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(prc0, 1, qty1));
  BOOST_CHECK(dc.verify_ask(prc2, 1, qty1));
  BOOST_CHECK(dc.verify_ask(prc3, 1, qty1));

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());
}

BOOST_AUTO_TEST_CASE(TestReplaceAonBidPriceMatch)
{
  SimpleOrderBook order_book;
  SimpleOrder ask2(sellSide, prc3, qty1);
  SimpleOrder ask1(sellSide, prc2, qty1);
  SimpleOrder ask0(sellSide, prc1, qty1);
  SimpleOrder bid1(buySide, prc1, qty2); // AON
  SimpleOrder bid0(buySide, prc0, qty1);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &bid0, expectNoMatch));
  BOOST_CHECK(add_and_verify(order_book, &bid1, expectNoMatch, expectNoComplete, AON));
  BOOST_CHECK(add_and_verify(order_book, &ask0, expectNoMatch));
  BOOST_CHECK(add_and_verify(order_book, &ask1, expectNoMatch));
  BOOST_CHECK(add_and_verify(order_book, &ask2, expectNoMatch));

  // Verify sizes
  BOOST_CHECK_EQUAL(2, order_book.bids().size());
  BOOST_CHECK_EQUAL(3, order_book.asks().size());

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(prc1, 1, qty2));
  BOOST_CHECK(dc.verify_bid(prc0, 1, qty1));
  BOOST_CHECK(dc.verify_ask(prc1, 1, qty1));
  BOOST_CHECK(dc.verify_ask(prc2, 1, qty1));
  BOOST_CHECK(dc.verify_ask(prc3, 1, qty1));

  // Match - complete
  {
    SimpleFillCheck fc1(&ask0, qty1, prc1 * qty1);
    SimpleFillCheck fc2(&ask1, qty1, prc2 * qty1);
    BOOST_CHECK(replace_and_verify(
        order_book, &bid1, qtyNone, prc2, simple::os_complete, qty2));
  }

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(prc0, 1, qty1));
  BOOST_CHECK(dc.verify_ask(prc3, 1, qty1));

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(1, order_book.asks().size());
}

BOOST_AUTO_TEST_CASE(TestReplaceBidLargerMatchAon)
{
  SimpleOrderBook order_book;
  SimpleOrder ask2(sellSide, prc3, qty1);
  SimpleOrder ask1(sellSide, prc2, qty1);
  SimpleOrder ask0(sellSide, prc1, qty2); // AON
  SimpleOrder bid1(buySide, prc1, qty1);
  SimpleOrder bid0(buySide, prc0, qty1);

  // No match
  BOOST_CHECK(add_and_verify(order_book, &bid0, expectNoMatch));
  BOOST_CHECK(add_and_verify(order_book, &bid1, expectNoMatch));
  BOOST_CHECK(add_and_verify(order_book, &ask0, expectNoMatch, expectNoComplete, AON));
  BOOST_CHECK(add_and_verify(order_book, &ask1, expectNoMatch));
  BOOST_CHECK(add_and_verify(order_book, &ask2, expectNoMatch));

  // Verify sizes
  BOOST_CHECK_EQUAL(2, order_book.bids().size());
  BOOST_CHECK_EQUAL(3, order_book.asks().size());

  // Verify depth
  DepthCheck<SimpleOrderBook> dc(order_book.depth());
  BOOST_CHECK(dc.verify_bid(prc1, 1, qty1));
  BOOST_CHECK(dc.verify_bid(prc0, 1, qty1));
  BOOST_CHECK(dc.verify_ask(prc1, 1, qty2));
  BOOST_CHECK(dc.verify_ask(prc2, 1, qty1));
  BOOST_CHECK(dc.verify_ask(prc3, 1, qty1));

  // Match - complete
  {
    SimpleFillCheck fc2(&ask0, qty2, qty2 * prc1);
    BOOST_CHECK(replace_and_verify(
        order_book, &bid1, qty1, PRICE_UNCHANGED, simple::os_complete, qty2));
  }

  // Verify depth
  dc.reset();
  BOOST_CHECK(dc.verify_bid(prc0, 1, qty1));
  BOOST_CHECK(dc.verify_ask(prc2, 1, qty1));
  BOOST_CHECK(dc.verify_ask(prc3, 1, qty1));

  // Verify sizes
  BOOST_CHECK_EQUAL(1, order_book.bids().size());
  BOOST_CHECK_EQUAL(2, order_book.asks().size());
}

} // Namespace
