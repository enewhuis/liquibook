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

namespace
{
  const bool sideBuy = true;
  const bool sideSell = false;

  const Price prcMkt = 0;
  const Price prc0 = 9900;

  const Quantity q100 = 100;

  const bool expectMatch = true;
  const bool expectNoMatch = false;

  const bool expectComplete = true;
  const bool expectNoComplete = false;
}

typedef OrderTracker<SimpleOrder*> SimpleTracker;
typedef test::ChangedChecker<5> ChangedChecker;

typedef FillCheck<SimpleOrder*> SimpleFillCheck;

TEST(TestNoMktToMktWithoutPreviousTrade)
{
  SimpleOrderBook order_book;
  SimpleOrder order0(sideBuy, prcMkt, q100);
  SimpleOrder order1(sideSell, prcMkt, q100);

  // Check that no market-to-market trade happens without a prior trade
  ASSERT_TRUE(add_and_verify(order_book, &order0, expectNoMatch));
  ASSERT_TRUE(add_and_verify(order_book, &order1, expectNoMatch));

  SimpleFillCheck fc0(&order0, 0, 0);
  SimpleFillCheck fc1(&order1, 0, 0);

  ASSERT_EQ(0U, order_book.market_price());
}

TEST(TestTradeSetsMarketPrice)
{
  SimpleOrderBook order_book;
  SimpleOrder order0(sideBuy, prcMkt, q100);
  SimpleOrder order1(sideSell, prcMkt, q100);

  // Check that no market-to-market trade happens without a prior trade
  ASSERT_TRUE(add_and_verify(order_book, &order0, expectNoMatch));
  ASSERT_TRUE(add_and_verify(order_book, &order1, expectNoMatch));

  ASSERT_EQ(0U, order_book.market_price());

  SimpleOrder order2(sideBuy, prc0, q100);

  // Scope for fill checks
  {
    SimpleFillCheck fc0(&order0, 0, 0);
    SimpleFillCheck fc1(&order1, q100, q100 * prc0);
    SimpleFillCheck fc2(&order2, q100, q100 * prc0);
    ASSERT_TRUE(add_and_verify(order_book, &order2, expectMatch, expectComplete));
  }
  ASSERT_EQ(prc0, order_book.market_price());
  
  SimpleOrder order3(sideSell, prcMkt, q100);
  // Scope for fill checks
  {
    SimpleFillCheck fc0(&order0, q100, q100 * prc0);
    SimpleFillCheck fc3(&order3, q100, q100 * prc0);
    ASSERT_TRUE(add_and_verify(order_book, &order3, expectMatch, expectComplete));
  }
  ASSERT_EQ(prc0, order_book.market_price());
}

TEST(TestExplicitlySettingMarketPriceAllowsMarketToMarketTrades)
{
  SimpleOrderBook order_book;
  SimpleOrder order0(sideBuy, prcMkt, q100);
  SimpleOrder order1(sideSell, prcMkt, q100);

  // Check that no market-to-market trade happens without a prior trade
  ASSERT_TRUE(add_and_verify(order_book, &order0, expectNoMatch));
  ASSERT_TRUE(add_and_verify(order_book, &order1, expectNoMatch));

  // IF WE MAKE THE MANUAL SETTING OF MARKET PRICE RETROACTIVE
  // FIX THIS TEST TO REFLECT THAT
  
  // Scope for fill checks
  {
    SimpleFillCheck fc0(&order0, 0, 0);
    SimpleFillCheck fc1(&order1, 0, 0);
    order_book.set_market_price(prc0);
  }

  SimpleOrder order2(sideBuy, prcMkt, q100);
  SimpleOrder order3(sideSell, prcMkt, q100);
  // Scope for fill checks
  {
    SimpleFillCheck fc0(&order0, q100, q100 * prc0);
    SimpleFillCheck fc1(&order1, q100, q100 * prc0);
    SimpleFillCheck fc2(&order2, q100, q100 * prc0);
    SimpleFillCheck fc3(&order3, q100, q100 * prc0);
    ASSERT_TRUE(add_and_verify(order_book, &order2, expectMatch, expectComplete));
    ASSERT_TRUE(add_and_verify(order_book, &order3, expectMatch, expectComplete));
  }
}

} // namespace
