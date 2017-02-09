// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include "assertiv/assertiv.h"
#include "changed_checker.h"
#include "book/order_book.h"
#include "impl/simple_order.h"
#include "impl/simple_order_book.h"

using namespace liquibook::book;

namespace liquibook {

using book::DepthLevel;
using book::OrderBook;
using book::OrderTracker;
using impl::SimpleOrder;

typedef OrderTracker<SimpleOrder*> SimpleTracker;
typedef impl::SimpleOrderBook<1> SimpleOrderBook;
typedef test::ChangedChecker<1> ChangedChecker;
typedef SimpleOrderBook::DepthTracker SimpleDepth;

template <class OrderBook, class OrderPtr>
bool add_and_verify(OrderBook& order_book,
                    const OrderPtr& order,
                    const bool match_expected,
                    const bool complete_expected = false)
{
  const bool matched = order_book.add(order);
  if (matched == match_expected) {
    order_book.perform_callbacks();
    if (complete_expected) {
      // State should be complete
      return impl::os_complete == order->state();
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
                        impl::OrderState expected_state = impl::os_accepted)
{
  // Calculate
  Quantity expected_order_qty = order->order_qty() + size_change;
  Quantity expected_open_qty = order->open_qty() + size_change;
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
  if (level.is_excess()) {
    std::cout << "Marked as excess" << std::endl;
    matched = false;
  }
  return matched;
}

template <class OrderPtr>
class FillCheck {
public:
  FillCheck(OrderPtr order, 
            Quantity filled_qty,
            Cost filled_cost)
  : order_(order),
    expected_filled_qty_(order->filled_qty() + filled_qty),
    expected_open_qty_(order->order_qty() - expected_filled_qty_),
    expected_filled_cost_(order->filled_cost() + (filled_cost))
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
    if (order_->state() != impl::os_complete && !expected_open_qty_) {
      std::cout << "state " << order_->state() 
                << " expected " << impl::os_complete << std::endl;
      throw std::runtime_error("Unexpected state with no open quantity");
    }
    if (order_->state() != impl::os_accepted && expected_open_qty_) {
      std::cout << "state " << order_->state() 
                << " expected " << impl::os_accepted << std::endl;
      throw std::runtime_error("Unexpected state with open quantity");
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
      ASSERT_EQ(MARKET_ORDER_BID_SORT_PRICEx, bid->first);
    } else {
      ASSERT_EQ(expected_order[index]->price(), bid->first);
    }
    ASSERT_EQ(expected_order[index], bid->second.ptr());
  }

  // Should be able to search and find
  ASSERT_TRUE((bids.upper_bound(book::ComparablePrice(true, 1245)))->second.ptr()->price() == 1240);
  ASSERT_TRUE((bids.lower_bound(book::ComparablePrice(true, 1245)))->second.ptr()->price() == 1245);
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
      ASSERT_EQ(MARKET_ORDER_ASK_SORT_PRICEx, ask->first);
    } else {
      ASSERT_EQ(expected_order[index]->price(), ask->first);
    }
    ASSERT_EQ(expected_order[index], ask->second.ptr());
  }

  ASSERT_TRUE((asks.upper_bound(book::ComparablePrice(false, 3235)))->second.ptr()->price() == 3245);
  ASSERT_TRUE((asks.lower_bound(book::ComparablePrice(false, 3235)))->second.ptr()->price() == 3235);
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
  ASSERT_TRUE(dc.verify_ask(9251, 2, 500));

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
  ASSERT_TRUE(dc.verify_ask(7252, 1, 100));

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

  // Match - repeated
  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&bid1, 100, 125100);
    SimpleFillCheck fc2(&ask0, 100, 125100);
    ASSERT_TRUE(add_and_verify(order_book, &ask0, true, true));
  ); }

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1251, 1, 800));

  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&bid1, 300, 1251 * 300);
    SimpleFillCheck fc2(&ask1, 300, 1251 * 300);
    ASSERT_TRUE(add_and_verify(order_book, &ask1, true, true));
  ); }

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1251, 1, 500));

  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&bid1, 200, 1251 * 200);
    SimpleFillCheck fc2(&ask2, 200, 1251 * 200);
    ASSERT_TRUE(add_and_verify(order_book, &ask2, true, true));
  ); }

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1251, 1, 300));

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

  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&ask1, 300, 1251 * 300);
    SimpleFillCheck fc2(&bid1, 300, 1251 * 300);
    ASSERT_TRUE(add_and_verify(order_book, &bid1, true, true));
  ); }

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_ask(1251, 1, 500));

  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&ask1, 200, 1251 * 200);
    SimpleFillCheck fc2(&bid2, 200, 1251 * 200);
    ASSERT_TRUE(add_and_verify(order_book, &bid2, true, true));
  ); }

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_ask(1251, 1, 300));

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
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));
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
  ASSERT_TRUE(dc.verify_bid(   0, 0,   0));
}

TEST(TestCancelBid)
{
  SimpleOrderBook order_book;
  SimpleOrder ask1(false, 1252, 100);
  SimpleOrder ask0(false, 1251, 100);
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

  // Cancel bid
  ASSERT_TRUE(cancel_and_verify(order_book, &bid0, impl::os_cancelled));

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(0,    0,   0));
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));

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

  // Cancel bid
  ASSERT_TRUE(cancel_and_verify(order_book, &ask0, impl::os_cancelled));

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1250, 2, 200));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));

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
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));

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
  ASSERT_TRUE(dc.verify_bid(   0, 0,   0));

  // Cancel a filled order
  ASSERT_TRUE(cancel_and_verify(order_book, &bid0, impl::os_complete));

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_ask(1251, 1, 100));
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
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));

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
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));

  // Cancel a filled order
  ASSERT_TRUE(cancel_and_verify(order_book, &ask0, impl::os_complete));

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_ask(1252, 1, 100));
  ASSERT_TRUE(dc.verify_bid(1250, 1, 100));
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
  ASSERT_TRUE(dc.verify_ask(1250, 1,  500));

  // Cancel a bid level (erase)
  ASSERT_TRUE(cancel_and_verify(order_book, &bid3, impl::os_cancelled));

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1249, 3,  500));
  ASSERT_TRUE(dc.verify_ask(1250, 1,  500));
  
  // Cancel common bid levels (not erased)
  ASSERT_TRUE(cancel_and_verify(order_book, &bid7, impl::os_cancelled));
  ASSERT_TRUE(cancel_and_verify(order_book, &bid4, impl::os_cancelled));

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1249, 3,  500));
  ASSERT_TRUE(dc.verify_ask(1250, 1,  500));

  // Cancel the best bid level (erased)
  ASSERT_TRUE(cancel_and_verify(order_book, &bid1, impl::os_cancelled));
  ASSERT_TRUE(cancel_and_verify(order_book, &bid0, impl::os_cancelled));
  ASSERT_TRUE(cancel_and_verify(order_book, &bid2, impl::os_cancelled));

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1246, 1,  500));
  ASSERT_TRUE(dc.verify_ask(1250, 1,  500));
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
  ASSERT_TRUE(dc.verify_ask(1250, 1,  500));

  // Cancel an ask level (erase)
  ASSERT_TRUE(cancel_and_verify(order_book, &ask1, impl::os_cancelled));

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1249, 3,  500));
  ASSERT_TRUE(dc.verify_ask(1250, 1,  500));

  // Cancel common ask levels (not erased)
  ASSERT_TRUE(cancel_and_verify(order_book, &ask2, impl::os_cancelled));
  ASSERT_TRUE(cancel_and_verify(order_book, &ask6, impl::os_cancelled));

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1249, 3,  500));
  ASSERT_TRUE(dc.verify_ask(1250, 1,  500));

  // Cancel the best ask level (erased)
  ASSERT_TRUE(cancel_and_verify(order_book, &ask0, impl::os_cancelled));

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1249, 3,  500));
  ASSERT_TRUE(dc.verify_ask(1252, 1,  200));
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
  ASSERT_TRUE(dc.verify_ask(1250, 1,  500));

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
  ASSERT_TRUE(dc.verify_ask(1249, 1,  300)); // Inserted
  
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
  ASSERT_TRUE(dc.verify_ask(1249, 1,  300));

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
  ASSERT_TRUE(dc.verify_ask(1246, 1, 1300));

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
  ASSERT_TRUE(dc.verify_ask(1246, 1, 1300));
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
  ASSERT_TRUE(dc.verify_ask(1250, 1,  500));

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
  ASSERT_TRUE(dc.verify_ask(1251, 1,  400));

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
  ASSERT_TRUE(dc.verify_ask(1252, 2,  300));

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
  ASSERT_TRUE(dc.verify_ask(1254, 1,  300));

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
  ASSERT_TRUE(dc.verify_ask(1255, 2,  700));

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
  ASSERT_TRUE(dc.verify_ask(1255, 1,  150)); // 1 filled, 1 reduced
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
  ASSERT_TRUE(dc.verify_ask(1252, 2, 500));

  // Verify changed stamps
  ASSERT_TRUE(cc.verify_bbo_changed(1, 1));
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
  ASSERT_TRUE(dc.verify_ask(1252, 2, 350));

  // Verify changed stamps
  ASSERT_TRUE(cc.verify_bbo_changed(0, 1));
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
  ASSERT_TRUE(dc.verify_bid(1251, 1, 400));
  ASSERT_TRUE(dc.verify_ask(1252, 2, 500));

  // Partial Fill existing book
  SimpleOrder cross_bid(true,  1252, 125);
  SimpleOrder cross_ask(false, 1251, 100);
  
  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&cross_bid, 125, 1252 * 125);
    SimpleFillCheck fc2(&ask0,      125, 1252 * 125);
    ASSERT_TRUE(add_and_verify(order_book, &cross_bid, true, true));
  ); }
  { ASSERT_NO_THROW(
    SimpleFillCheck fc1(&cross_ask, 100, 1251 * 100);
    SimpleFillCheck fc2(&bid1,      100, 1251 * 100);
    ASSERT_TRUE(add_and_verify(order_book, &cross_ask, true, true));
  ); }

  // Verify quantity
  ASSERT_EQ(175, ask0.open_qty());
  ASSERT_EQ(300, bid1.open_qty());

  // Verify depth
  dc.reset();
  ASSERT_TRUE(dc.verify_bid(1251, 1, 300));
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
}

TEST(TestReplaceBidPriceChange)
{
  SimpleOrderBook order_book;
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
  ASSERT_TRUE(dc.verify_ask(1252, 1, 200));

  // Replace price increase
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

  // Replace price decrease
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
  ASSERT_TRUE(dc.verify_ask(1252, 1, 200));
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

  // Verify depth
  DepthCheck dc(order_book.depth());
  ASSERT_TRUE(dc.verify_bid(1251, 1, 140));
  ASSERT_TRUE(dc.verify_ask(1252, 1, 200));

  // Replace price increase 1252 -> 1253
  ASSERT_TRUE(replace_and_verify(order_book, &ask1, SIZE_UNCHANGED, 1253));

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
  ASSERT_TRUE(dc.verify_ask(1252, 1, 300));
}
} // namespace
