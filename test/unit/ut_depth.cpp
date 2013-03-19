// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include "assertiv/assertiv.h"
#include "book/depth.h"
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

TEST(TestAddBid)
{
  SizedDepth depth;
  depth.add_order(1234, 100, true);
  const DepthLevel* first_bid = depth.bids();
  ASSERT_TRUE(verify_level(first_bid, 1234, 1, 100));
  ChangedChecker cc(depth);
  ASSERT_TRUE(cc.verify_bid_changed(1, 0, 0, 0, 0));
}

TEST(TestAddBids)
{
  SizedDepth depth;
  depth.add_order(1234, 100, true);
  depth.add_order(1234, 200, true);
  depth.add_order(1234, 300, true);
  const DepthLevel* first_bid = depth.bids();
  ASSERT_TRUE(verify_level(first_bid, 1234, 3, 600));
  ChangedChecker cc(depth);
  ASSERT_TRUE(cc.verify_bid_changed(1, 0, 0, 0, 0));
}

TEST(TestAppendBidLevels)
{
  SizedDepth depth;
  depth.add_order(1236, 300, true);
  depth.add_order(1235, 200, true);
  depth.add_order(1232, 100, true);
  depth.add_order(1235, 400, true);
  const DepthLevel* bid = depth.bids();
  ASSERT_TRUE(verify_level(bid, 1236, 1, 300));
  ASSERT_TRUE(verify_level(bid, 1235, 2, 600));
  ASSERT_TRUE(verify_level(bid, 1232, 1, 100));
  ChangedChecker cc(depth);
  ASSERT_TRUE(cc.verify_bid_changed(1, 1, 1, 0, 0));
}

TEST(TestInsertBidLevels)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1234, 800, true);
  ASSERT_TRUE(cc.verify_bid_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.add_order(1232, 100, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 1, 0, 0, 0)); cc.reset();
  depth.add_order(1236, 300, true);
  ASSERT_TRUE(cc.verify_bid_changed(1, 1, 1, 0, 0)); cc.reset();
  depth.add_order(1235, 200, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 1, 1, 1, 0)); cc.reset();
  depth.add_order(1234, 900, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 0, 1, 0, 0)); cc.reset();
  depth.add_order(1231, 700, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 0, 0, 0, 1)); cc.reset();
  depth.add_order(1235, 400, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 1, 0, 0, 0)); cc.reset();
  depth.add_order(1231, 500, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 0, 0, 0, 1)); cc.reset();
  depth.add_order(1233, 200, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 0, 0, 1, 1)); cc.reset();
  const DepthLevel* bid = depth.bids();
  ASSERT_TRUE(verify_level(bid, 1236, 1,  300));
  ASSERT_TRUE(verify_level(bid, 1235, 2,  600));
  ASSERT_TRUE(verify_level(bid, 1234, 2, 1700));
  ASSERT_TRUE(verify_level(bid, 1233, 1,  200));
  ASSERT_TRUE(verify_level(bid, 1232, 1,  100));
}

TEST(TestInsertBidLevelsPast5)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1234, 800, true);
  ASSERT_TRUE(cc.verify_bid_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.add_order(1232, 100, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 1, 0, 0, 0)); cc.reset();
  depth.add_order(1236, 300, true);
  ASSERT_TRUE(cc.verify_bid_changed(1, 1, 1, 0, 0)); cc.reset();
  depth.add_order(1231, 700, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 0, 0, 1, 0)); cc.reset();
  depth.add_order(1234, 900, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 1, 0, 0, 0)); cc.reset();
  depth.add_order(1235, 400, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 1, 1, 1, 1)); cc.reset();
  depth.add_order(1235, 200, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 1, 0, 0, 0)); cc.reset();
  depth.add_order(1231, 500, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 0, 0, 0, 1)); cc.reset();
  depth.add_order(1230, 200, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 0, 0, 0, 0)); cc.reset();
  depth.add_order(1229, 200, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 0, 0, 0, 0)); cc.reset();
  const DepthLevel* bid = depth.bids();
  ASSERT_TRUE(verify_level(bid, 1236, 1,  300));
  ASSERT_TRUE(verify_level(bid, 1235, 2,  600));
  ASSERT_TRUE(verify_level(bid, 1234, 2, 1700));
  ASSERT_TRUE(verify_level(bid, 1232, 1,  100));
  ASSERT_TRUE(verify_level(bid, 1231, 2, 1200));
}

TEST(TestInsertBidLevelsTruncate5)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1234, 800, true);
  ASSERT_TRUE(cc.verify_bid_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.add_order(1232, 100, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 1, 0, 0, 0)); cc.reset();
  depth.add_order(1236, 300, true);
  ASSERT_TRUE(cc.verify_bid_changed(1, 1, 1, 0, 0)); cc.reset();
  depth.add_order(1231, 700, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 0, 0, 1, 0)); cc.reset();
  depth.add_order(1234, 900, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 1, 0, 0, 0)); cc.reset();
  depth.add_order(1235, 400, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 1, 1, 1, 1)); cc.reset();
  depth.add_order(1235, 200, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 1, 0, 0, 0)); cc.reset();
  depth.add_order(1231, 500, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 0, 0, 0, 1)); cc.reset();
  depth.add_order(1230, 200, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 0, 0, 0, 0)); cc.reset();
  depth.add_order(1238, 200, true);
  ASSERT_TRUE(cc.verify_bid_changed(1, 1, 1, 1, 1)); cc.reset();
  depth.add_order(1238, 250, true);
  ASSERT_TRUE(cc.verify_bid_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.add_order(1237, 500, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 1, 1, 1, 1)); cc.reset();
  const DepthLevel* bid = depth.bids();
  ASSERT_TRUE(verify_level(bid, 1238, 2,  450));
  ASSERT_TRUE(verify_level(bid, 1237, 1,  500));
  ASSERT_TRUE(verify_level(bid, 1236, 1,  300));
  ASSERT_TRUE(verify_level(bid, 1235, 2,  600));
  ASSERT_TRUE(verify_level(bid, 1234, 2, 1700));
}

TEST(TestCloseBid)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1234, 300, true);
  ASSERT_TRUE(cc.verify_bid_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.add_order(1234, 500, true);
  ASSERT_TRUE(cc.verify_bid_changed(1, 0, 0, 0, 0)); cc.reset();
  ASSERT_TRUE(!depth.close_order(1234, 300, true)); // Does not erase
  ASSERT_TRUE(cc.verify_bid_changed(1, 0, 0, 0, 0)); cc.reset();
  const DepthLevel* first_bid = depth.bids();
  ASSERT_TRUE(verify_level(first_bid, 1234, 1, 500));
}

TEST(TestCloseEraseBid)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1235, 300, true);
  ASSERT_TRUE(cc.verify_bid_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.add_order(1235, 400, true);
  ASSERT_TRUE(cc.verify_bid_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.add_order(1234, 500, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 1, 0, 0, 0)); cc.reset();
  depth.add_order(1233, 200, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 0, 1, 0, 0)); cc.reset();
  ASSERT_TRUE(!depth.close_order(1235, 300, true)); // Does not erase
  ASSERT_TRUE(cc.verify_bid_changed(1, 0, 0, 0, 0)); cc.reset();
  ASSERT_TRUE(depth.close_order(1235, 400, true)); // Erase
  ASSERT_TRUE(cc.verify_bid_changed(1, 1, 1, 0, 0)); cc.reset();
  const DepthLevel* bid = depth.bids();
  ASSERT_TRUE(verify_level(bid, 1234, 1, 500));
  ASSERT_TRUE(verify_level(bid, 1233, 1, 200));
}

TEST(TestAddCloseAddBid)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1234, 300, true);
  ASSERT_TRUE(cc.verify_bid_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.close_order(1234, 300, true);
  ASSERT_TRUE(cc.verify_bid_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.add_order(1233, 200, true);
  ASSERT_TRUE(cc.verify_bid_changed(1, 0, 0, 0, 0)); cc.reset();
  const DepthLevel* bid = depth.bids();
  ASSERT_TRUE(verify_level(bid, 1233, 1, 200));
  ASSERT_TRUE(verify_level(bid, 0, 0, 0));
}

TEST(TestAddCloseAddHigherBid)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1234, 300, true);
  ASSERT_TRUE(cc.verify_bid_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.close_order(1234, 300, true);
  ASSERT_TRUE(cc.verify_bid_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.add_order(1235, 200, true);
  ASSERT_TRUE(cc.verify_bid_changed(1, 0, 0, 0, 0)); cc.reset();
  const DepthLevel* bid = depth.bids();
  ASSERT_TRUE(verify_level(bid, 1235, 1, 200));
  ASSERT_TRUE(verify_level(bid, 0, 0, 0));
}

TEST(TestCloseBidsFreeLevels)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1234, 800, true);
  ASSERT_TRUE(cc.verify_bid_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.add_order(1232, 100, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 1, 0, 0, 0)); cc.reset();
  depth.add_order(1236, 300, true);
  ASSERT_TRUE(cc.verify_bid_changed(1, 1, 1, 0, 0)); cc.reset();
  depth.add_order(1235, 200, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 1, 1, 1, 0)); cc.reset();
  depth.add_order(1234, 900, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 0, 1, 0, 0)); cc.reset();
  depth.add_order(1231, 700, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 0, 0, 0, 1)); cc.reset();
  depth.add_order(1235, 400, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 1, 0, 0, 0)); cc.reset();
  depth.add_order(1231, 500, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 0, 0, 0, 1)); cc.reset();
  depth.close_order(1234, 900, true); // No erase
  ASSERT_TRUE(cc.verify_bid_changed(0, 0, 1, 0, 0)); cc.reset();
  depth.close_order(1232, 100, true); // Erase
  ASSERT_TRUE(cc.verify_bid_changed(0, 0, 0, 1, 1)); cc.reset();
  depth.close_order(1236, 300, true); // Erase
  ASSERT_TRUE(cc.verify_bid_changed(1, 1, 1, 1, 0)); cc.reset();
  const DepthLevel* bid = depth.bids();
  ASSERT_TRUE(verify_level(bid, 1235, 2,  600));
  ASSERT_TRUE(verify_level(bid, 1234, 1,  800));
  ASSERT_TRUE(verify_level(bid, 1231, 2, 1200));
  ASSERT_TRUE(verify_level(bid,    0, 0,    0));
  ASSERT_TRUE(verify_level(bid,    0, 0,    0));
  depth.add_order(1233, 350, true); // Insert
  ASSERT_TRUE(cc.verify_bid_changed(0, 0, 1, 1, 0)); cc.reset();
  depth.add_order(1236, 300, true); // Insert
  ASSERT_TRUE(cc.verify_bid_changed(1, 1, 1, 1, 1)); cc.reset();
  depth.add_order(1231, 700, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 0, 0, 0, 1)); cc.reset();
  bid = depth.bids();  // reset
  ASSERT_TRUE(verify_level(bid, 1236, 1,  300));
  ASSERT_TRUE(verify_level(bid, 1235, 2,  600));
  ASSERT_TRUE(verify_level(bid, 1234, 1,  800));
  ASSERT_TRUE(verify_level(bid, 1233, 1,  350));
  ASSERT_TRUE(verify_level(bid, 1231, 3, 1900));
}

TEST(TestIncreaseBid)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1236, 300, true);
  ASSERT_TRUE(cc.verify_bid_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.add_order(1235, 200, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 1, 0, 0, 0)); cc.reset();
  depth.add_order(1232, 100, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 0, 1, 0, 0)); cc.reset();
  depth.add_order(1235, 400, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 1, 0, 0, 0)); cc.reset();
  depth.change_qty_order(1232, 37, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 0, 1, 0, 0)); cc.reset();
  depth.change_qty_order(1232, 41, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 0, 1, 0, 0)); cc.reset();
  depth.change_qty_order(1235, 201, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 1, 0, 0, 0)); cc.reset();
  const DepthLevel* bid = depth.bids();
  ASSERT_TRUE(verify_level(bid, 1236, 1, 300));
  ASSERT_TRUE(verify_level(bid, 1235, 2, 801));
  ASSERT_TRUE(verify_level(bid, 1232, 1, 178));
}

TEST(TestDecreaseBid)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1236, 300, true);
  ASSERT_TRUE(cc.verify_bid_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.add_order(1235, 200, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 1, 0, 0, 0)); cc.reset();
  depth.add_order(1232, 100, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 0, 1, 0, 0)); cc.reset();
  depth.add_order(1235, 400, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 1, 0, 0, 0)); cc.reset();
  depth.change_qty_order(1236, -37, true);
  ASSERT_TRUE(cc.verify_bid_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.change_qty_order(1236, -41, true);
  ASSERT_TRUE(cc.verify_bid_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.change_qty_order(1235, -201, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 1, 0, 0, 0)); cc.reset();
  const DepthLevel* bid = depth.bids();
  ASSERT_TRUE(verify_level(bid, 1236, 1, 222));
  ASSERT_TRUE(verify_level(bid, 1235, 2, 399));
  ASSERT_TRUE(verify_level(bid, 1232, 1, 100));
}

TEST(TestIncreaseDecreaseBid)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1236, 300, true);
  ASSERT_TRUE(cc.verify_bid_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.add_order(1235, 200, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 1, 0, 0, 0)); cc.reset();
  depth.add_order(1232, 100, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 0, 1, 0, 0)); cc.reset();
  depth.add_order(1235, 400, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 1, 0, 0, 0)); cc.reset();
  depth.change_qty_order(1236, 37, true);
  ASSERT_TRUE(cc.verify_bid_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.change_qty_order(1235, -41, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 1, 0, 0, 0)); cc.reset();
  depth.change_qty_order(1232, 60, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 0, 1, 0, 0)); cc.reset();
  depth.change_qty_order(1236, -41, true);
  ASSERT_TRUE(cc.verify_bid_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.change_qty_order(1236, 210, true);
  ASSERT_TRUE(cc.verify_bid_changed(1, 0, 0, 0, 0)); cc.reset();
  const DepthLevel* bid = depth.bids();
  ASSERT_TRUE(verify_level(bid, 1236, 1, 506));
  ASSERT_TRUE(verify_level(bid, 1235, 2, 559));
  ASSERT_TRUE(verify_level(bid, 1232, 1, 160));
}

TEST(TestAddAsk)
{
  SizedDepth depth;
  depth.add_order(1234, 100, false);
  const DepthLevel* first_ask = depth.asks();
  ASSERT_TRUE(verify_level(first_ask, 1234, 1, 100));
  ChangedChecker cc(depth);
  ASSERT_TRUE(cc.verify_ask_changed(1, 0, 0, 0, 0)); cc.reset();
}

TEST(TestAddAsks)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1234, 100, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.add_order(1234, 200, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.add_order(1234, 300, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 0, 0, 0, 0)); cc.reset();
  const DepthLevel* first_ask = depth.asks();
  ASSERT_TRUE(verify_level(first_ask, 1234, 3, 600));
}

TEST(TestAppendAskLevels)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1236, 300, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.add_order(1235, 200, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 1, 0, 0, 0)); cc.reset();
  depth.add_order(1232, 100, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 1, 1, 0, 0)); cc.reset();
  depth.add_order(1235, 400, false);
  ASSERT_TRUE(cc.verify_ask_changed(0, 1, 0, 0, 0)); cc.reset();
  const DepthLevel* ask = depth.asks();
  ASSERT_TRUE(verify_level(ask, 1232, 1, 100));
  ASSERT_TRUE(verify_level(ask, 1235, 2, 600));
  ASSERT_TRUE(verify_level(ask, 1236, 1, 300));
}

TEST(TestInsertAskLevels)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1234, 800, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.add_order(1232, 100, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 1, 0, 0, 0)); cc.reset();
  depth.add_order(1236, 300, false);
  ASSERT_TRUE(cc.verify_ask_changed(0, 0, 1, 0, 0)); cc.reset();
  depth.add_order(1235, 200, false);
  ASSERT_TRUE(cc.verify_ask_changed(0, 0, 1, 1, 0)); cc.reset();
  depth.add_order(1234, 900, false);
  ASSERT_TRUE(cc.verify_ask_changed(0, 1, 0, 0, 0)); cc.reset();
  depth.add_order(1231, 700, false);
  depth.add_order(1235, 400, false);
  depth.add_order(1231, 500, false);
  const DepthLevel* ask = depth.asks();
  ASSERT_TRUE(verify_level(ask, 1231, 2, 1200));
  ASSERT_TRUE(verify_level(ask, 1232, 1,  100));
  ASSERT_TRUE(verify_level(ask, 1234, 2, 1700));
  ASSERT_TRUE(verify_level(ask, 1235, 2,  600));
  ASSERT_TRUE(verify_level(ask, 1236, 1,  300));
}

TEST(TestInsertAskLevelsPast5)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1234, 800, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.add_order(1232, 100, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 1, 0, 0, 0)); cc.reset();
  depth.add_order(1236, 300, false);
  ASSERT_TRUE(cc.verify_ask_changed(0, 0, 1, 0, 0)); cc.reset();
  depth.add_order(1231, 700, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 1, 1, 1, 0)); cc.reset();
  depth.add_order(1234, 900, false);
  ASSERT_TRUE(cc.verify_ask_changed(0, 0, 1, 0, 0)); cc.reset();
  depth.add_order(1235, 400, false);
  ASSERT_TRUE(cc.verify_ask_changed(0, 0, 0, 1, 1)); cc.reset();
  depth.add_order(1235, 200, false);
  ASSERT_TRUE(cc.verify_ask_changed(0, 0, 0, 1, 0)); cc.reset();
  depth.add_order(1231, 500, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.add_order(1230, 200, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 1, 1, 1, 1)); cc.reset();
  depth.add_order(1229, 200, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 1, 1, 1, 1)); cc.reset();
  const DepthLevel* ask = depth.asks();
  ASSERT_TRUE(verify_level(ask, 1229, 1,  200));
  ASSERT_TRUE(verify_level(ask, 1230, 1,  200));
  ASSERT_TRUE(verify_level(ask, 1231, 2, 1200));
  ASSERT_TRUE(verify_level(ask, 1232, 1,  100));
  ASSERT_TRUE(verify_level(ask, 1234, 2, 1700));
}

TEST(TestInsertAskLevelsTruncate5)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1234, 800, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.add_order(1232, 100, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 1, 0, 0, 0)); cc.reset();
  depth.add_order(1236, 300, false);
  ASSERT_TRUE(cc.verify_ask_changed(0, 0, 1, 0, 0)); cc.reset();
  depth.add_order(1231, 700, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 1, 1, 1, 0)); cc.reset();
  depth.add_order(1234, 900, false);
  ASSERT_TRUE(cc.verify_ask_changed(0, 0, 1, 0, 0)); cc.reset();
  depth.add_order(1235, 400, false);
  ASSERT_TRUE(cc.verify_ask_changed(0, 0, 0, 1, 1)); cc.reset();
  depth.add_order(1235, 200, false);
  ASSERT_TRUE(cc.verify_ask_changed(0, 0, 0, 1, 0)); cc.reset();
  depth.add_order(1231, 500, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.add_order(1230, 200, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 1, 1, 1, 1)); cc.reset();
  depth.add_order(1238, 200, false);
  ASSERT_TRUE(cc.verify_ask_changed(0, 0, 0, 0, 0)); cc.reset();
  depth.add_order(1232, 250, false);
  ASSERT_TRUE(cc.verify_ask_changed(0, 0, 1, 0, 0)); cc.reset();
  depth.add_order(1237, 500, false);
  ASSERT_TRUE(cc.verify_ask_changed(0, 0, 0, 0, 0)); cc.reset();
  const DepthLevel* ask = depth.asks();
  ASSERT_TRUE(verify_level(ask, 1230, 1,  200));
  ASSERT_TRUE(verify_level(ask, 1231, 2, 1200));
  ASSERT_TRUE(verify_level(ask, 1232, 2,  350));
  ASSERT_TRUE(verify_level(ask, 1234, 2, 1700));
  ASSERT_TRUE(verify_level(ask, 1235, 2,  600));
}

TEST(TestCloseAsk)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1234, 300, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.add_order(1234, 500, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 0, 0, 0, 0)); cc.reset();
  ASSERT_TRUE(!depth.close_order(1234, 300, false)); // Does not erase
  const DepthLevel* first_ask = depth.asks();
  ASSERT_TRUE(verify_level(first_ask, 1234, 1, 500));
}

TEST(TestCloseEraseAsk)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1233, 300, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.add_order(1234, 500, false);
  ASSERT_TRUE(cc.verify_ask_changed(0, 1, 0, 0, 0)); cc.reset();
  depth.add_order(1233, 400, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 0, 0, 0, 0)); cc.reset();
  ASSERT_TRUE(!depth.close_order(1233, 300, false)); // Does not erase
  ASSERT_TRUE(cc.verify_ask_changed(1, 0, 0, 0, 0)); cc.reset();
  ASSERT_TRUE(depth.close_order(1233, 400, false)); // Erase
  ASSERT_TRUE(cc.verify_ask_changed(1, 1, 0, 0, 0)); cc.reset();
  const DepthLevel* first_ask = depth.asks();
  ASSERT_TRUE(verify_level(first_ask, 1234, 1, 500));
}

TEST(TestAddCloseAddAsk)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1234, 300, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.close_order(1234, 300, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.add_order(1233, 200, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 0, 0, 0, 0)); cc.reset();
  const DepthLevel* ask = depth.asks();
  ASSERT_TRUE(verify_level(ask, 1233, 1, 200));
  ASSERT_TRUE(verify_level(ask, 0, 0, 0));
}

TEST(TestAddCloseAddHigherAsk)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1234, 300, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.close_order(1234, 300, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.add_order(1235, 200, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 0, 0, 0, 0)); cc.reset();
  const DepthLevel* ask = depth.asks();
  ASSERT_TRUE(verify_level(ask, 1235, 1, 200));
  ASSERT_TRUE(verify_level(ask, 0, 0, 0));
}

TEST(TestCloseAsksFreeLevels)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1234, 800, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.add_order(1232, 100, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 1, 0, 0, 0)); cc.reset();
  depth.add_order(1236, 300, false);
  ASSERT_TRUE(cc.verify_ask_changed(0, 0, 1, 0, 0)); cc.reset();
  depth.add_order(1235, 200, false);
  ASSERT_TRUE(cc.verify_ask_changed(0, 0, 1, 1, 0)); cc.reset();
  depth.add_order(1234, 900, false);
  ASSERT_TRUE(cc.verify_ask_changed(0, 1, 0, 0, 0)); cc.reset();
  depth.add_order(1231, 700, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 1, 1, 1, 1)); cc.reset();
  depth.add_order(1235, 400, false);
  ASSERT_TRUE(cc.verify_ask_changed(0, 0, 0, 1, 0)); cc.reset();
  depth.add_order(1231, 500, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.close_order(1234, 900, false); // does not erase
  ASSERT_TRUE(cc.verify_ask_changed(0, 0, 1, 0, 0)); cc.reset();
  depth.close_order(1232, 100, false); // erase
  ASSERT_TRUE(cc.verify_ask_changed(0, 1, 1, 1, 1)); cc.reset();
  depth.close_order(1236, 100, false);
  ASSERT_TRUE(cc.verify_ask_changed(0, 0, 0, 1, 0)); cc.reset();
  const DepthLevel* ask = depth.asks();
  ASSERT_TRUE(verify_level(ask, 1231, 2, 1200));
  ASSERT_TRUE(verify_level(ask, 1234, 1,  800));
  ASSERT_TRUE(verify_level(ask, 1235, 2,  600));
  ASSERT_TRUE(verify_level(ask,    0, 0,    0));
  ASSERT_TRUE(verify_level(ask,    0, 0,    0));
  depth.add_order(1233, 350, false);
  depth.add_order(1236, 300, false);
  depth.add_order(1231, 700, false);
  ask = depth.asks();  // reset
  ASSERT_TRUE(verify_level(ask, 1231, 3, 1900));
  ASSERT_TRUE(verify_level(ask, 1233, 1,  350));
  ASSERT_TRUE(verify_level(ask, 1234, 1,  800));
  ASSERT_TRUE(verify_level(ask, 1235, 2,  600));
  ASSERT_TRUE(verify_level(ask, 1236, 1,  300));
}

TEST(TestIncreaseAsk)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1236, 300, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.add_order(1235, 200, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 1, 0, 0, 0)); cc.reset();
  depth.add_order(1232, 100, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 1, 1, 0, 0)); cc.reset();
  depth.add_order(1235, 400, false);
  ASSERT_TRUE(cc.verify_ask_changed(0, 1, 0, 0, 0)); cc.reset();
  depth.change_qty_order(1232, 37, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.change_qty_order(1232, 41, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.change_qty_order(1235, 201, false);
  ASSERT_TRUE(cc.verify_ask_changed(0, 1, 0, 0, 0)); cc.reset();
  const DepthLevel* ask = depth.asks();
  ASSERT_TRUE(verify_level(ask, 1232, 1, 178));
  ASSERT_TRUE(verify_level(ask, 1235, 2, 801));
  ASSERT_TRUE(verify_level(ask, 1236, 1, 300));
}

TEST(TestDecreaseAsk)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1236, 300, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.add_order(1235, 200, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 1, 0, 0, 0)); cc.reset();
  depth.add_order(1232, 100, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 1, 1, 0, 0)); cc.reset();
  depth.add_order(1235, 400, false);
  ASSERT_TRUE(cc.verify_ask_changed(0, 1, 0, 0, 0)); cc.reset();
  depth.change_qty_order(1236, -37, false);
  ASSERT_TRUE(cc.verify_ask_changed(0, 0, 1, 0, 0)); cc.reset();
  depth.change_qty_order(1236, -41, false);
  ASSERT_TRUE(cc.verify_ask_changed(0, 0, 1, 0, 0)); cc.reset();
  depth.change_qty_order(1235, -201, false);
  ASSERT_TRUE(cc.verify_ask_changed(0, 1, 0, 0, 0)); cc.reset();
  const DepthLevel* ask = depth.asks();
  ASSERT_TRUE(verify_level(ask, 1232, 1, 100));
  ASSERT_TRUE(verify_level(ask, 1235, 2, 399));
  ASSERT_TRUE(verify_level(ask, 1236, 1, 222));
}

TEST(TestIncreaseDecreaseAsk)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1236, 300, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.add_order(1235, 200, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 1, 0, 0, 0)); cc.reset();
  depth.add_order(1232, 100, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 1, 1, 0, 0)); cc.reset();
  depth.add_order(1235, 400, false);
  ASSERT_TRUE(cc.verify_ask_changed(0, 1, 0, 0, 0)); cc.reset();
  depth.change_qty_order(1236, 37, false);
  ASSERT_TRUE(cc.verify_ask_changed(0, 0, 1, 0, 0)); cc.reset();
  depth.change_qty_order(1235, -41, false);
  ASSERT_TRUE(cc.verify_ask_changed(0, 1, 0, 0, 0)); cc.reset();
  depth.change_qty_order(1232, 51, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.change_qty_order(1236, -41, false);
  ASSERT_TRUE(cc.verify_ask_changed(0, 0, 1, 0, 0)); cc.reset();
  depth.change_qty_order(1236, 201, false);
  ASSERT_TRUE(cc.verify_ask_changed(0, 0, 1, 0, 0)); cc.reset();
  const DepthLevel* ask = depth.asks();
  ASSERT_TRUE(verify_level(ask, 1232, 1, 151));
  ASSERT_TRUE(verify_level(ask, 1235, 2, 559));
  ASSERT_TRUE(verify_level(ask, 1236, 1, 497));
}

TEST(TestReplaceBid)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1236, 300, true);
  ASSERT_TRUE(cc.verify_bid_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.add_order(1235, 200, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 1, 0, 0, 0)); cc.reset();
  depth.add_order(1232, 100, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 0, 1, 0, 0)); cc.reset();
  depth.add_order(1235, 400, true);
  ASSERT_TRUE(cc.verify_bid_changed(0, 1, 0, 0, 0)); cc.reset();

  // Verify Levels 
  const DepthLevel* bid = depth.bids();
  ASSERT_TRUE(verify_level(bid, 1236, 1, 300));
  ASSERT_TRUE(verify_level(bid, 1235, 2, 600));
  ASSERT_TRUE(verify_level(bid, 1232, 1, 100));

  // Replace bid
  depth.replace_order(1235, 1237, 200, 200, true);

  // Verify Levels 
  bid = depth.bids();
  ASSERT_TRUE(verify_level(bid, 1237, 1, 200));
  ASSERT_TRUE(verify_level(bid, 1236, 1, 300));
  ASSERT_TRUE(verify_level(bid, 1235, 1, 400));
  ASSERT_TRUE(verify_level(bid, 1232, 1, 100));

  ASSERT_TRUE(cc.verify_bid_changed(1, 1, 1, 1, 0)); cc.reset();
}

TEST(TestReplaceAsk)
{
  SizedDepth depth;
  ChangedChecker cc(depth);
  depth.add_order(1236, 300, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 0, 0, 0, 0)); cc.reset();
  depth.add_order(1235, 200, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 1, 0, 0, 0)); cc.reset();
  depth.add_order(1232, 100, false);
  ASSERT_TRUE(cc.verify_ask_changed(1, 1, 1, 0, 0)); cc.reset();
  depth.add_order(1235, 400, false);
  ASSERT_TRUE(cc.verify_ask_changed(0, 1, 0, 0, 0)); cc.reset();

  // Verify Levels 
  const DepthLevel* ask = depth.asks();
  ASSERT_TRUE(verify_level(ask, 1232, 1, 100));
  ASSERT_TRUE(verify_level(ask, 1235, 2, 600));
  ASSERT_TRUE(verify_level(ask, 1236, 1, 300));

  // Replace ask
  depth.replace_order(1235, 1237, 200, 200, false);

  // Verify Levels 
  ask = depth.asks();
  ASSERT_TRUE(verify_level(ask, 1232, 1, 100));
  ASSERT_TRUE(verify_level(ask, 1235, 1, 400));
  ASSERT_TRUE(verify_level(ask, 1236, 1, 300));
  ASSERT_TRUE(verify_level(ask, 1237, 1, 200));

  ASSERT_TRUE(cc.verify_ask_changed(0, 1, 0, 1, 0)); cc.reset();
}

} // namespace
