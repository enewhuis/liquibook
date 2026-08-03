// Copyright (c) 2012 - 2017 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.

#define BOOST_TEST_NO_MAIN LiquibookTest
#include <boost/test/unit_test.hpp>

#include <book/depth.h>
#include "changed_checker.h"
#include <iostream>

namespace liquibook {

using book::Depth;
using book::DepthLevel;
typedef Depth<5> SizedDepth;
typedef test::ChangedChecker<5> ChangedChecker;

bool verify_level(const DepthLevel*& level, 
                  book::Price price, 
                  uint32_t order_count, 
                  book::Quantity aggregate_qty)
{
  bool matched = true;
  if (price != level->price()) {
    std::cout << "Level price " << level->price() << std::endl;
    matched = false;
  }
  if (order_count != level->order_count()) {
    std::cout << "Level order count " << level->order_count() << std::endl;
    matched = false;
  }
  if (aggregate_qty != level->aggregate_qty()) {
    std::cout << "Level aggregate qty " << level->aggregate_qty() << std::endl;
    matched = false;
  }
  ++level;
  return matched;
}

BOOST_AUTO_TEST_CASE(TestAddBid)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1234, 100, true);
  const DepthLevel* first_bid = depth.bids();
  BOOST_CHECK(verify_level(first_bid, 1234, 1, 100));
  BOOST_CHECK(cc.verify_bid_changed(true, false, false, false, false));
}

BOOST_AUTO_TEST_CASE(TestAddBids)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1234, 100, true);
  depth.add_order(1234, 200, true);
  depth.add_order(1234, 300, true);
  const DepthLevel* first_bid = depth.bids();
  BOOST_CHECK(verify_level(first_bid, 1234, 3, 600));
  BOOST_CHECK(cc.verify_bid_changed(true,false, false, false, false));
}

BOOST_AUTO_TEST_CASE(TestAppendBidLevels)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1236, 300, true);
  depth.add_order(1235, 200, true);
  depth.add_order(1232, 100, true);
  depth.add_order(1235, 400, true);
  const DepthLevel* bid = depth.bids();
  BOOST_CHECK(verify_level(bid, 1236, 1, 300));
  BOOST_CHECK(verify_level(bid, 1235, 2, 600));
  BOOST_CHECK(verify_level(bid, 1232, 1, 100));
  BOOST_CHECK(cc.verify_bid_changed(true, true, true, false, false));
}

BOOST_AUTO_TEST_CASE(TestInsertBidLevels)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1234, 800, true);

  BOOST_CHECK(cc.verify_bid_changed(true, false, false, false, false)); 
  cc.reset();

  depth.add_order(1232, 100, true);
  BOOST_CHECK(cc.verify_bid_changed(false, true, false, false, false));
  cc.reset();

  depth.add_order(1236, 300, true);
  BOOST_CHECK(cc.verify_bid_changed(true, true, true, false, false)); 
  cc.reset();

  depth.add_order(1235, 200, true);
  BOOST_CHECK(cc.verify_bid_changed(false, true, true, true, false));
  cc.reset();

  depth.add_order(1234, 900, true);
  BOOST_CHECK(cc.verify_bid_changed(false, false, true, false, false));
  cc.reset();

  depth.add_order(1231, 700, true);
  BOOST_CHECK(cc.verify_bid_changed(false, false, false, false, true));
  cc.reset();

  depth.add_order(1235, 400, true);
  BOOST_CHECK(cc.verify_bid_changed(false, true, false, false, false));
  cc.reset();

  depth.add_order(1231, 500, true);
  BOOST_CHECK(cc.verify_bid_changed(false, false, false, false, true));
  cc.reset();

  depth.add_order(1233, 200, true);
  BOOST_CHECK(cc.verify_bid_changed(false, false, false, true, true));
  cc.reset();

  const DepthLevel* bid = depth.bids();
  BOOST_CHECK(verify_level(bid, 1236, 1,  300));
  BOOST_CHECK(verify_level(bid, 1235, 2,  600));
  BOOST_CHECK(verify_level(bid, 1234, 2, 1700));
  BOOST_CHECK(verify_level(bid, 1233, 1,  200));
  BOOST_CHECK(verify_level(bid, 1232, 1,  100));
}

BOOST_AUTO_TEST_CASE(TestInsertBidLevelsPast5)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1234, 800, true);
  BOOST_CHECK(cc.verify_bid_changed(true, false, false, false, false));
  cc.reset();

  depth.add_order(1232, 100, true);
  BOOST_CHECK(cc.verify_bid_changed(false, true, false, false, false));
  cc.reset();

  depth.add_order(1236, 300, true);
  BOOST_CHECK(cc.verify_bid_changed(true, true, true, false, false));
  cc.reset();

  depth.add_order(1231, 700, true);
  BOOST_CHECK(cc.verify_bid_changed(false, false, false, true, false));
  cc.reset();

  depth.add_order(1234, 900, true);
  BOOST_CHECK(cc.verify_bid_changed(false, true, false, false, false));
  cc.reset();

  depth.add_order(1235, 400, true);
  BOOST_CHECK(cc.verify_bid_changed(false, true, true, true, true));
  cc.reset();

  depth.add_order(1235, 200, true);
  BOOST_CHECK(cc.verify_bid_changed(false, true, false, false, false));
  cc.reset();

  depth.add_order(1231, 500, true);
  BOOST_CHECK(cc.verify_bid_changed(false, false, false, false, true));
  cc.reset();

  depth.add_order(1230, 200, true);

  BOOST_CHECK(cc.verify_bid_changed(false, false, false, false, false));
  cc.reset();
  depth.add_order(1229, 200, true);

  BOOST_CHECK(cc.verify_bid_changed(false, false, false, false, false));
  cc.reset();
  const DepthLevel* bid = depth.bids();
  BOOST_CHECK(verify_level(bid, 1236, 1,  300));
  BOOST_CHECK(verify_level(bid, 1235, 2,  600));
  BOOST_CHECK(verify_level(bid, 1234, 2, 1700));
  BOOST_CHECK(verify_level(bid, 1232, 1,  100));
  BOOST_CHECK(verify_level(bid, 1231, 2, 1200));
}

BOOST_AUTO_TEST_CASE(TestInsertBidLevelsTruncate5)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1234, 800, true);
  BOOST_CHECK(cc.verify_bid_changed(true, false, false, false, false));
  cc.reset();

  depth.add_order(1232, 100, true);
  BOOST_CHECK(cc.verify_bid_changed(false, true, false, false, false));
  cc.reset();

  depth.add_order(1236, 300, true);
  BOOST_CHECK(cc.verify_bid_changed(true, true, true, false, false));
  cc.reset();

  depth.add_order(1231, 700, true);
  BOOST_CHECK(cc.verify_bid_changed(false, false, false, true, false));
  cc.reset();

  depth.add_order(1234, 900, true);
  BOOST_CHECK(cc.verify_bid_changed(false, true, false, false, false));
  cc.reset();

  depth.add_order(1235, 400, true);
  BOOST_CHECK(cc.verify_bid_changed(false, true, true, true, true));
  cc.reset();

  depth.add_order(1235, 200, true);
  BOOST_CHECK(cc.verify_bid_changed(false, true, false, false, false));
  cc.reset();

  depth.add_order(1231, 500, true);
  BOOST_CHECK(cc.verify_bid_changed(false, false, false, false, true));
  cc.reset();

  depth.add_order(1230, 200, true);
  BOOST_CHECK(cc.verify_bid_changed(false, false, false, false, false));
  cc.reset();

  depth.add_order(1238, 200, true);
  BOOST_CHECK(cc.verify_bid_changed(true, true, true, true, true));
  cc.reset();

  depth.add_order(1238, 250, true);
  BOOST_CHECK(cc.verify_bid_changed(true, false, false, false, false));
  cc.reset();

  depth.add_order(1237, 500, true);
  BOOST_CHECK(cc.verify_bid_changed(false, true, true, true, true));
  cc.reset();
  const DepthLevel* bid = depth.bids();
  BOOST_CHECK(verify_level(bid, 1238, 2,  450));
  BOOST_CHECK(verify_level(bid, 1237, 1,  500));
  BOOST_CHECK(verify_level(bid, 1236, 1,  300));
  BOOST_CHECK(verify_level(bid, 1235, 2,  600));
  BOOST_CHECK(verify_level(bid, 1234, 2, 1700));
}

BOOST_AUTO_TEST_CASE(TestCloseBid)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1234, 300, true);
  BOOST_CHECK(cc.verify_bid_changed(true, false, false, false, false));
  cc.reset();

  depth.add_order(1234, 500, true);
  BOOST_CHECK(cc.verify_bid_changed(true, false, false, false, false));
  cc.reset();

  BOOST_CHECK(!depth.close_order(1234, 300, true)); // Does not erase
  BOOST_CHECK(cc.verify_bid_changed(true, false, false, false, false));

  const DepthLevel* first_bid = depth.bids();
  BOOST_CHECK(verify_level(first_bid, 1234, 1, 500));
}

BOOST_AUTO_TEST_CASE(TestCloseEraseBid)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1235, 300, true);
  BOOST_CHECK(cc.verify_bid_changed(true, false, false, false, false));
  cc.reset();

  depth.add_order(1235, 400, true);
  BOOST_CHECK(cc.verify_bid_changed(true, false, false, false, false));
  cc.reset();

  depth.add_order(1234, 500, true);
  BOOST_CHECK(cc.verify_bid_changed(false, true, false, false, false));
  cc.reset();

  depth.add_order(1233, 200, true);
  BOOST_CHECK(cc.verify_bid_changed(false, false, true, false, false));
  cc.reset();

  BOOST_CHECK(!depth.close_order(1235, 300, true)); // Does not erase
  BOOST_CHECK(cc.verify_bid_changed(true, false, false, false, false));
  cc.reset();

  BOOST_CHECK(depth.close_order(1235, 400, true)); // Erase
  BOOST_CHECK(cc.verify_bid_changed(true, true, true, false, false));

  const DepthLevel* bid = depth.bids();
  BOOST_CHECK(verify_level(bid, 1234, 1, 500));
  BOOST_CHECK(verify_level(bid, 1233, 1, 200));
}

BOOST_AUTO_TEST_CASE(TestAddCloseAddBid)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1234, 300, true);
  BOOST_CHECK(cc.verify_bid_changed(true, false, false, false, false));
  cc.reset();
  
  depth.close_order(1234, 300, true);
  BOOST_CHECK(cc.verify_bid_changed(true, false, false, false, false));
  cc.reset();
  
  depth.add_order(1233, 200, true);
  BOOST_CHECK(cc.verify_bid_changed(true, false, false, false, false));
  cc.reset();
  const DepthLevel* bid = depth.bids();
  BOOST_CHECK(verify_level(bid, 1233, 1, 200));
  BOOST_CHECK(verify_level(bid, false, false, false));
}

BOOST_AUTO_TEST_CASE(TestAddCloseAddHigherBid)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1234, 300, true);
  BOOST_CHECK(cc.verify_bid_changed(true, false, false, false, false));
  cc.reset();

  depth.close_order(1234, 300, true);
  BOOST_CHECK(cc.verify_bid_changed(true, false, false, false, false));
  cc.reset();

  depth.add_order(1235, 200, true);
  BOOST_CHECK(cc.verify_bid_changed(true, false, false, false, false));
  cc.reset();

  const DepthLevel* bid = depth.bids();
  BOOST_CHECK(verify_level(bid, 1235, 1, 200));
  BOOST_CHECK(verify_level(bid, false, false, false));
}

BOOST_AUTO_TEST_CASE(TestCloseBidsFreeLevels)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1234, 800, true);
  BOOST_CHECK(cc.verify_bid_changed(true, false, false, false, false));
  cc.reset();

  depth.add_order(1232, 100, true);
  BOOST_CHECK(cc.verify_bid_changed(false, true, false, false, false));
  cc.reset();

  depth.add_order(1236, 300, true);
  BOOST_CHECK(cc.verify_bid_changed(true, true, true, false, false));
  cc.reset();

  depth.add_order(1235, 200, true);
  BOOST_CHECK(cc.verify_bid_changed(false, true, true, true, false));
  cc.reset();

  depth.add_order(1234, 900, true);
  BOOST_CHECK(cc.verify_bid_changed(false, false, true, false, false));
  cc.reset();

  depth.add_order(1231, 700, true);
  BOOST_CHECK(cc.verify_bid_changed(false, false, false, false, true));
  cc.reset();

  depth.add_order(1235, 400, true);
  BOOST_CHECK(cc.verify_bid_changed(false, true, false, false, false));
  cc.reset();

  depth.add_order(1231, 500, true);
  BOOST_CHECK(cc.verify_bid_changed(false, false, false, false, true));
  cc.reset();

  depth.close_order(1234, 900, true); // No erase
  BOOST_CHECK(cc.verify_bid_changed(false, false, true, false, false));
  cc.reset();

  depth.close_order(1232, 100, true); // Erase
  BOOST_CHECK(cc.verify_bid_changed(false, false, false, true, true));
  cc.reset();

  depth.close_order(1236, 300, true); // Erase
  BOOST_CHECK(cc.verify_bid_changed(true, true, true, true, false));
  cc.reset();

  const DepthLevel* bid = depth.bids();
  BOOST_CHECK(verify_level(bid, 1235, 2,  600));
  BOOST_CHECK(verify_level(bid, 1234, 1,  800));
  BOOST_CHECK(verify_level(bid, 1231, 2, 1200));
  BOOST_CHECK(verify_level(bid,    0, 0,    0));
  BOOST_CHECK(verify_level(bid,    0, 0,    0));

  depth.add_order(1233, 350, true); // Insert
  BOOST_CHECK(cc.verify_bid_changed(false, false, true, true, false));
  cc.reset();

  depth.add_order(1236, 300, true); // Insert
  BOOST_CHECK(cc.verify_bid_changed(true, true, true, true, true));
  cc.reset();

  depth.add_order(1231, 700, true);
  BOOST_CHECK(cc.verify_bid_changed(false, false, false, false, true));
  cc.reset();
  bid = depth.bids();  // reset
  BOOST_CHECK(verify_level(bid, 1236, 1,  300));
  BOOST_CHECK(verify_level(bid, 1235, 2,  600));
  BOOST_CHECK(verify_level(bid, 1234, 1,  800));
  BOOST_CHECK(verify_level(bid, 1233, 1,  350));
  BOOST_CHECK(verify_level(bid, 1231, 3, 1900));
}

BOOST_AUTO_TEST_CASE(TestIncreaseBid)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1236, 300, true);
  BOOST_CHECK(cc.verify_bid_changed(true, false, false, false, false));
  cc.reset();

  depth.add_order(1235, 200, true);
  BOOST_CHECK(cc.verify_bid_changed(false, true, false, false, false));
  cc.reset();

  depth.add_order(1232, 100, true);
  BOOST_CHECK(cc.verify_bid_changed(false, false, true, false, false));
  cc.reset();

  depth.add_order(1235, 400, true);
  BOOST_CHECK(cc.verify_bid_changed(false, true, false, false, false));
  cc.reset();

  depth.change_qty_order(1232, 37, true);
  BOOST_CHECK(cc.verify_bid_changed(false, false, true, false, false));
  cc.reset();

  depth.change_qty_order(1232, 41, true);
  BOOST_CHECK(cc.verify_bid_changed(false, false, true, false, false));
  cc.reset();

  depth.change_qty_order(1235, 201, true);
  BOOST_CHECK(cc.verify_bid_changed(false, true, false, false, false));
  cc.reset();

  const DepthLevel* bid = depth.bids();
  BOOST_CHECK(verify_level(bid, 1236, 1, 300));
  BOOST_CHECK(verify_level(bid, 1235, 2, 801));
  BOOST_CHECK(verify_level(bid, 1232, 1, 178));
}

BOOST_AUTO_TEST_CASE(TestDecreaseBid)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1236, 300, true);
  BOOST_CHECK(cc.verify_bid_changed(true, false, false, false, false));
  cc.reset();

  depth.add_order(1235, 200, true);
  BOOST_CHECK(cc.verify_bid_changed(false, true, false, false, false));
  cc.reset();

  depth.add_order(1232, 100, true);
  BOOST_CHECK(cc.verify_bid_changed(false, false, true, false, false));
  cc.reset();

  depth.add_order(1235, 400, true);
  BOOST_CHECK(cc.verify_bid_changed(false, true, false, false, false));
  cc.reset();

  depth.change_qty_order(1236, -37, true);
  BOOST_CHECK(cc.verify_bid_changed(true, false, false, false, false));
  cc.reset();

  depth.change_qty_order(1236, -41, true);
  BOOST_CHECK(cc.verify_bid_changed(true, false, false, false, false));
  cc.reset();

  depth.change_qty_order(1235, -201, true);
  BOOST_CHECK(cc.verify_bid_changed(false, true, false, false, false));
  cc.reset();

  const DepthLevel* bid = depth.bids();
  BOOST_CHECK(verify_level(bid, 1236, 1, 222));
  BOOST_CHECK(verify_level(bid, 1235, 2, 399));
  BOOST_CHECK(verify_level(bid, 1232, 1, 100));
}

BOOST_AUTO_TEST_CASE(TestIncreaseDecreaseBid)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1236, 300, true);
  BOOST_CHECK(cc.verify_bid_changed(true, false, false, false, false));
  cc.reset();

  depth.add_order(1235, 200, true);
  BOOST_CHECK(cc.verify_bid_changed(false, true, false, false, false));
  cc.reset();

  depth.add_order(1232, 100, true);
  BOOST_CHECK(cc.verify_bid_changed(false, false, true, false, false));
  cc.reset();

  depth.add_order(1235, 400, true);
  BOOST_CHECK(cc.verify_bid_changed(false, true, false, false, false));
  cc.reset();

  depth.change_qty_order(1236, 37, true);
  BOOST_CHECK(cc.verify_bid_changed(true, false, false, false, false));
  cc.reset();

  depth.change_qty_order(1235, -41, true);
  BOOST_CHECK(cc.verify_bid_changed(false, true, false, false, false));
  cc.reset();

  depth.change_qty_order(1232, 60, true);
  BOOST_CHECK(cc.verify_bid_changed(false, false, true, false, false));
  cc.reset();

  depth.change_qty_order(1236, -41, true);
  BOOST_CHECK(cc.verify_bid_changed(true, false, false, false, false));
  cc.reset();

  depth.change_qty_order(1236, 210, true);
  BOOST_CHECK(cc.verify_bid_changed(true, false, false, false, false));
  cc.reset();
  const DepthLevel* bid = depth.bids();
  BOOST_CHECK(verify_level(bid, 1236, 1, 506));
  BOOST_CHECK(verify_level(bid, 1235, 2, 559));
  BOOST_CHECK(verify_level(bid, 1232, 1, 160));
}

BOOST_AUTO_TEST_CASE(TestAddAsk)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1234, 100, false);
  const DepthLevel* first_ask = depth.asks();
  BOOST_CHECK(verify_level(first_ask, 1234, 1, 100));
  BOOST_CHECK(cc.verify_ask_changed(true, false, false, false, false));
}

BOOST_AUTO_TEST_CASE(TestAddAsks)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1234, 100, false);
  BOOST_CHECK(cc.verify_ask_changed(true, false, false, false, false));
  cc.reset();

  depth.add_order(1234, 200, false);
  BOOST_CHECK(cc.verify_ask_changed(true, false, false, false, false));
  cc.reset();

  depth.add_order(1234, 300, false);
  BOOST_CHECK(cc.verify_ask_changed(true, false, false, false, false));

  const DepthLevel* first_ask = depth.asks();
  BOOST_CHECK(verify_level(first_ask, 1234, 3, 600));
}

BOOST_AUTO_TEST_CASE(TestAppendAskLevels)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1236, 300, false);
  BOOST_CHECK(cc.verify_ask_changed(true, false, false, false, false));
  cc.reset();

  depth.add_order(1235, 200, false);
  BOOST_CHECK(cc.verify_ask_changed(true, true, false, false, false));
  cc.reset();

  depth.add_order(1232, 100, false);
  BOOST_CHECK(cc.verify_ask_changed(true, true, true, false, false));
  cc.reset();

  depth.add_order(1235, 400, false);
  BOOST_CHECK(cc.verify_ask_changed(false, true, false, false, false));
  cc.reset();
  const DepthLevel* ask = depth.asks();
  BOOST_CHECK(verify_level(ask, 1232, 1, 100));
  BOOST_CHECK(verify_level(ask, 1235, 2, 600));
  BOOST_CHECK(verify_level(ask, 1236, 1, 300));
}

BOOST_AUTO_TEST_CASE(TestInsertAskLevels)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1234, 800, false);
  BOOST_CHECK(cc.verify_ask_changed(true, false, false, false, false));
  cc.reset();

  depth.add_order(1232, 100, false);
  BOOST_CHECK(cc.verify_ask_changed(true, true, false, false, false));
  cc.reset();

  depth.add_order(1236, 300, false);
  BOOST_CHECK(cc.verify_ask_changed(false, false, true, false, false));
  cc.reset();

  depth.add_order(1235, 200, false);
  BOOST_CHECK(cc.verify_ask_changed(false, false, true, true, false));
  cc.reset();

  depth.add_order(1234, 900, false);
  BOOST_CHECK(cc.verify_ask_changed(false, true, false, false, false));
  cc.reset();
  depth.add_order(1231, 700, false);
  depth.add_order(1235, 400, false);
  depth.add_order(1231, 500, false);
  const DepthLevel* ask = depth.asks();
  BOOST_CHECK(verify_level(ask, 1231, 2, 1200));
  BOOST_CHECK(verify_level(ask, 1232, 1,  100));
  BOOST_CHECK(verify_level(ask, 1234, 2, 1700));
  BOOST_CHECK(verify_level(ask, 1235, 2,  600));
  BOOST_CHECK(verify_level(ask, 1236, 1,  300));
}

BOOST_AUTO_TEST_CASE(TestInsertAskLevelsPast5)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1234, 800, false);
  BOOST_CHECK(cc.verify_ask_changed(true, false, false, false, false));
  cc.reset();

  depth.add_order(1232, 100, false);
  BOOST_CHECK(cc.verify_ask_changed(true, true, false, false, false));
  cc.reset();

  depth.add_order(1236, 300, false);
  BOOST_CHECK(cc.verify_ask_changed(false, false, true, false, false));
  cc.reset();

  depth.add_order(1231, 700, false);
  BOOST_CHECK(cc.verify_ask_changed(true, true, true, true, false));
  cc.reset();

  depth.add_order(1234, 900, false);
  BOOST_CHECK(cc.verify_ask_changed(false, false, true, false, false));
  cc.reset();

  depth.add_order(1235, 400, false);
  BOOST_CHECK(cc.verify_ask_changed(false, false, false, true, true));
  cc.reset();

  depth.add_order(1235, 200, false);
  BOOST_CHECK(cc.verify_ask_changed(false, false, false, true, false));
  cc.reset();

  depth.add_order(1231, 500, false);
  BOOST_CHECK(cc.verify_ask_changed(true, false, false, false, false));
  cc.reset();

  depth.add_order(1230, 200, false);
  BOOST_CHECK(cc.verify_ask_changed(true, true, true, true, true));
  cc.reset();

  depth.add_order(1229, 200, false);
  BOOST_CHECK(cc.verify_ask_changed(true, true, true, true, true));
  cc.reset();

  const DepthLevel* ask = depth.asks();
  BOOST_CHECK(verify_level(ask, 1229, 1,  200));
  BOOST_CHECK(verify_level(ask, 1230, 1,  200));
  BOOST_CHECK(verify_level(ask, 1231, 2, 1200));
  BOOST_CHECK(verify_level(ask, 1232, 1,  100));
  BOOST_CHECK(verify_level(ask, 1234, 2, 1700));
}

BOOST_AUTO_TEST_CASE(TestInsertAskLevelsTruncate5)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1234, 800, false);
  BOOST_CHECK(cc.verify_ask_changed(true, false, false, false, false));
  cc.reset();

  depth.add_order(1232, 100, false);
  BOOST_CHECK(cc.verify_ask_changed(true, true, false, false, false));
  cc.reset();

  depth.add_order(1236, 300, false);
  BOOST_CHECK(cc.verify_ask_changed(false, false, true, false, false));
  cc.reset();

  depth.add_order(1231, 700, false);
  BOOST_CHECK(cc.verify_ask_changed(true, true, true, true, false));
  cc.reset();

  depth.add_order(1234, 900, false);
  BOOST_CHECK(cc.verify_ask_changed(false, false, true, false, false));
  cc.reset();

  depth.add_order(1235, 400, false);
  BOOST_CHECK(cc.verify_ask_changed(false, false, false, true, true));
  cc.reset();

  depth.add_order(1235, 200, false);
  BOOST_CHECK(cc.verify_ask_changed(false, false, false, true, false));
  cc.reset();

  depth.add_order(1231, 500, false);
  BOOST_CHECK(cc.verify_ask_changed(true, false, false, false, false));
  cc.reset();

  depth.add_order(1230, 200, false);
  BOOST_CHECK(cc.verify_ask_changed(true, true, true, true, true));
  cc.reset();

  depth.add_order(1238, 200, false);
  BOOST_CHECK(cc.verify_ask_changed(false, false, false, false, false));
  cc.reset();

  depth.add_order(1232, 250, false);
  BOOST_CHECK(cc.verify_ask_changed(false, false, true, false, false));
  cc.reset();

  depth.add_order(1237, 500, false);
  BOOST_CHECK(cc.verify_ask_changed(false, false, false, false, false));
  cc.reset();
  const DepthLevel* ask = depth.asks();
  BOOST_CHECK(verify_level(ask, 1230, 1,  200));
  BOOST_CHECK(verify_level(ask, 1231, 2, 1200));
  BOOST_CHECK(verify_level(ask, 1232, 2,  350));
  BOOST_CHECK(verify_level(ask, 1234, 2, 1700));
  BOOST_CHECK(verify_level(ask, 1235, 2,  600));
}

BOOST_AUTO_TEST_CASE(TestCloseAsk)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1234, 300, false);
  BOOST_CHECK(cc.verify_ask_changed(true, false, false, false, false));
  cc.reset();

  depth.add_order(1234, 500, false);
  BOOST_CHECK(cc.verify_ask_changed(true, false, false, false, false));
  cc.reset();
  BOOST_CHECK(!depth.close_order(1234, 300, false)); // Does not erase
  const DepthLevel* first_ask = depth.asks();
  BOOST_CHECK(verify_level(first_ask, 1234, 1, 500));
}

BOOST_AUTO_TEST_CASE(TestCloseEraseAsk)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1233, 300, false);
  BOOST_CHECK(cc.verify_ask_changed(true, false, false, false, false));
  cc.reset();

  depth.add_order(1234, 500, false);
  BOOST_CHECK(cc.verify_ask_changed(false, true, false, false, false));
  cc.reset();

  depth.add_order(1233, 400, false);
  BOOST_CHECK(cc.verify_ask_changed(true, false, false, false, false));
  cc.reset();

  BOOST_CHECK(!depth.close_order(1233, 300, false)); // Does not erase
  BOOST_CHECK(cc.verify_ask_changed(true, false, false, false, false));
  cc.reset();

  BOOST_CHECK(depth.close_order(1233, 400, false)); // Erase
  BOOST_CHECK(cc.verify_ask_changed(true, true, false, false, false));
  cc.reset();
  const DepthLevel* first_ask = depth.asks();
  BOOST_CHECK(verify_level(first_ask, 1234, 1, 500));
}

BOOST_AUTO_TEST_CASE(TestAddCloseAddAsk)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1234, 300, false);
  BOOST_CHECK(cc.verify_ask_changed(true, false, false, false, false));
  cc.reset();
  
  depth.close_order(1234, 300, false);
  BOOST_CHECK(cc.verify_ask_changed(true, false, false, false, false));
  cc.reset();
  
  depth.add_order(1233, 200, false);
  BOOST_CHECK(cc.verify_ask_changed(true, false, false, false, false));
  cc.reset();
  const DepthLevel* ask = depth.asks();
  BOOST_CHECK(verify_level(ask, 1233, 1, 200));
  BOOST_CHECK(verify_level(ask, false, false, false));
}

BOOST_AUTO_TEST_CASE(TestAddCloseAddHigherAsk)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1234, 300, false);
  BOOST_CHECK(cc.verify_ask_changed(true, false, false, false, false));
  cc.reset();
  
  depth.close_order(1234, 300, false);
  BOOST_CHECK(cc.verify_ask_changed(true, false, false, false, false));
  cc.reset();
  
  depth.add_order(1235, 200, false);
  BOOST_CHECK(cc.verify_ask_changed(true, false, false, false, false));  
  cc.reset();
  const DepthLevel* ask = depth.asks();
  BOOST_CHECK(verify_level(ask, 1235, 1, 200));
  BOOST_CHECK(verify_level(ask, false, false, false));
}

BOOST_AUTO_TEST_CASE(TestCloseAsksFreeLevels)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1234, 800, false);
  BOOST_CHECK(cc.verify_ask_changed(true, false, false, false, false));
  cc.reset();

  depth.add_order(1232, 100, false);
  BOOST_CHECK(cc.verify_ask_changed(true, true, false, false, false));
  cc.reset();

  depth.add_order(1236, 300, false);
  BOOST_CHECK(cc.verify_ask_changed(false, false, true, false, false));
  cc.reset();

  depth.add_order(1235, 200, false);
  BOOST_CHECK(cc.verify_ask_changed(false, false, true, true, false));
  cc.reset();

  depth.add_order(1234, 900, false);
  BOOST_CHECK(cc.verify_ask_changed(false, true, false, false, false));
  cc.reset();

  depth.add_order(1231, 700, false);
  BOOST_CHECK(cc.verify_ask_changed(true, true, true, true, true));
  cc.reset();

  depth.add_order(1235, 400, false);
  BOOST_CHECK(cc.verify_ask_changed(false, false, false, true, false));
  cc.reset();

  depth.add_order(1231, 500, false);
  BOOST_CHECK(cc.verify_ask_changed(true, false, false, false, false));
  cc.reset();

  depth.close_order(1234, 900, false); // does not erase
  BOOST_CHECK(cc.verify_ask_changed(false, false, true, false, false));
  cc.reset();

  depth.close_order(1232, 100, false); // erase
  BOOST_CHECK(cc.verify_ask_changed(false, true, true, true, true));
  cc.reset();
  depth.close_order(1236, 100, false);
  BOOST_CHECK(cc.verify_ask_changed(false, false, false, true, false));
  cc.reset();
  const DepthLevel* ask = depth.asks();
  BOOST_CHECK(verify_level(ask, 1231, 2, 1200));
  BOOST_CHECK(verify_level(ask, 1234, 1,  800));
  BOOST_CHECK(verify_level(ask, 1235, 2,  600));
  BOOST_CHECK(verify_level(ask,    0, 0,    0));
  BOOST_CHECK(verify_level(ask,    0, 0,    0));
  depth.add_order(1233, 350, false);
  depth.add_order(1236, 300, false);
  depth.add_order(1231, 700, false);
  ask = depth.asks();  // reset
  BOOST_CHECK(verify_level(ask, 1231, 3, 1900));
  BOOST_CHECK(verify_level(ask, 1233, 1,  350));
  BOOST_CHECK(verify_level(ask, 1234, 1,  800));
  BOOST_CHECK(verify_level(ask, 1235, 2,  600));
  BOOST_CHECK(verify_level(ask, 1236, 1,  300));
}

BOOST_AUTO_TEST_CASE(TestIncreaseAsk)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1236, 300, false);
  BOOST_CHECK(cc.verify_ask_changed(true, false, false, false, false));
  cc.reset();

  depth.add_order(1235, 200, false);
  BOOST_CHECK(cc.verify_ask_changed(true, true, false, false, false));
  cc.reset();

  depth.add_order(1232, 100, false);
  BOOST_CHECK(cc.verify_ask_changed(true, true, true, false, false));
  cc.reset();

  depth.add_order(1235, 400, false);
  BOOST_CHECK(cc.verify_ask_changed(false, true, false, false, false));
  cc.reset();

  depth.change_qty_order(1232, 37, false);
  BOOST_CHECK(cc.verify_ask_changed(true, false, false, false, false));
  cc.reset();

  depth.change_qty_order(1232, 41, false);
  BOOST_CHECK(cc.verify_ask_changed(true, false, false, false, false));
  cc.reset();

  depth.change_qty_order(1235, 201, false);
  BOOST_CHECK(cc.verify_ask_changed(false, true, false, false, false));
  cc.reset();
  const DepthLevel* ask = depth.asks();
  BOOST_CHECK(verify_level(ask, 1232, 1, 178));
  BOOST_CHECK(verify_level(ask, 1235, 2, 801));
  BOOST_CHECK(verify_level(ask, 1236, 1, 300));
}

BOOST_AUTO_TEST_CASE(TestDecreaseAsk)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1236, 300, false);
  BOOST_CHECK(cc.verify_ask_changed(true, false, false, false, false));
  cc.reset();

  depth.add_order(1235, 200, false);
  BOOST_CHECK(cc.verify_ask_changed(true, true, false, false, false));
  cc.reset();

  depth.add_order(1232, 100, false);
  BOOST_CHECK(cc.verify_ask_changed(true, true, true, false, false));
  cc.reset();

  depth.add_order(1235, 400, false);
  BOOST_CHECK(cc.verify_ask_changed(false, true, false, false, false));
  cc.reset();

  depth.change_qty_order(1236, -37, false);
  BOOST_CHECK(cc.verify_ask_changed(false, false, true, false, false));
  cc.reset();

  depth.change_qty_order(1236, -41, false);
  BOOST_CHECK(cc.verify_ask_changed(false, false, true, false, false));
  cc.reset();

  depth.change_qty_order(1235, -201, false);
  BOOST_CHECK(cc.verify_ask_changed(false, true, false, false, false));
  cc.reset();
  const DepthLevel* ask = depth.asks();
  BOOST_CHECK(verify_level(ask, 1232, 1, 100));
  BOOST_CHECK(verify_level(ask, 1235, 2, 399));
  BOOST_CHECK(verify_level(ask, 1236, 1, 222));
}

BOOST_AUTO_TEST_CASE(TestIncreaseDecreaseAsk)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1236, 300, false);
  BOOST_CHECK(cc.verify_ask_changed(true, false, false, false, false));
  cc.reset();

  depth.add_order(1235, 200, false);
  BOOST_CHECK(cc.verify_ask_changed(true, true, false, false, false));
  cc.reset();

  depth.add_order(1232, 100, false);
  BOOST_CHECK(cc.verify_ask_changed(true, true, true, false, false));
  cc.reset();

  depth.add_order(1235, 400, false);
  BOOST_CHECK(cc.verify_ask_changed(false, true, false, false, false));
  cc.reset();

  depth.change_qty_order(1236, 37, false);
  BOOST_CHECK(cc.verify_ask_changed(false, false, true, false, false));
  cc.reset();

  depth.change_qty_order(1235, -41, false);
  BOOST_CHECK(cc.verify_ask_changed(false, true, false, false, false));
  cc.reset();

  depth.change_qty_order(1232, 51, false);
  BOOST_CHECK(cc.verify_ask_changed(true, false, false, false, false));
  cc.reset();

  depth.change_qty_order(1236, -41, false);
  BOOST_CHECK(cc.verify_ask_changed(false, false, true, false, false));
  cc.reset();

  depth.change_qty_order(1236, 201, false);
  BOOST_CHECK(cc.verify_ask_changed(false, false, true, false, false));
  cc.reset();
  const DepthLevel* ask = depth.asks();
  BOOST_CHECK(verify_level(ask, 1232, 1, 151));
  BOOST_CHECK(verify_level(ask, 1235, 2, 559));
  BOOST_CHECK(verify_level(ask, 1236, 1, 497));
}

BOOST_AUTO_TEST_CASE(TestReplaceBid)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1236, 300, true);
  BOOST_CHECK(cc.verify_bid_changed(true, false, false, false, false));
  cc.reset();

  depth.add_order(1235, 200, true);
  BOOST_CHECK(cc.verify_bid_changed(false, true, false, false, false));
  cc.reset();

  depth.add_order(1232, 100, true);
  BOOST_CHECK(cc.verify_bid_changed(false, false, true, false, false));
  cc.reset();

  depth.add_order(1235, 400, true);
  BOOST_CHECK(cc.verify_bid_changed(false, true, false, false, false));
  cc.reset();


  // Verify Levels 
  const DepthLevel* bid = depth.bids();
  BOOST_CHECK(verify_level(bid, 1236, 1, 300));
  BOOST_CHECK(verify_level(bid, 1235, 2, 600));
  BOOST_CHECK(verify_level(bid, 1232, 1, 100));

  // Replace bid
  depth.replace_order(1235, 1237, 200, 200, true);

  // Verify Levels 
  bid = depth.bids();
  BOOST_CHECK(verify_level(bid, 1237, 1, 200));
  BOOST_CHECK(verify_level(bid, 1236, 1, 300));
  BOOST_CHECK(verify_level(bid, 1235, 1, 400));
  BOOST_CHECK(verify_level(bid, 1232, 1, 100));

  BOOST_CHECK(cc.verify_bid_changed(true, true, true, true, false));
  cc.reset();
}

BOOST_AUTO_TEST_CASE(TestReplaceAsk)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1236, 300, false);
  BOOST_CHECK(cc.verify_ask_changed(true, false, false, false, false));
  cc.reset();

  depth.add_order(1235, 200, false);
  BOOST_CHECK(cc.verify_ask_changed(true, true, false, false, false));
  cc.reset();

  depth.add_order(1232, 100, false);
  BOOST_CHECK(cc.verify_ask_changed(true, true, true, false, false));
  cc.reset();

  depth.add_order(1235, 400, false);
  BOOST_CHECK(cc.verify_ask_changed(false, true, false, false, false));
  cc.reset();

  // Verify Levels 
  const DepthLevel* ask = depth.asks();
  BOOST_CHECK(verify_level(ask, 1232, 1, 100));
  BOOST_CHECK(verify_level(ask, 1235, 2, 600));
  BOOST_CHECK(verify_level(ask, 1236, 1, 300));

  // Replace ask
  depth.replace_order(1235, 1237, 200, 200, false);

  // Verify Levels 
  ask = depth.asks();
  BOOST_CHECK(verify_level(ask, 1232, 1, 100));
  BOOST_CHECK(verify_level(ask, 1235, 1, 400));
  BOOST_CHECK(verify_level(ask, 1236, 1, 300));
  BOOST_CHECK(verify_level(ask, 1237, 1, 200));

  BOOST_CHECK(cc.verify_ask_changed(false, true, false, true, false));
  cc.reset();
}

} // namespace
