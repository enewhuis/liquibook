// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include "book/depth.h"
#include <iostream>

namespace liquibook {

using book::Depth;
using book::DepthLevel;
using book::ChangeId;

namespace test {

template <int SIZE = 5>
class ChangedChecker {
public:
  typedef Depth<SIZE> SizedDepth;
  ChangedChecker(SizedDepth& depth)
  : depth_(depth)
  {
    reset();
  }

  void reset() {
    last_change_ = depth_.last_change();
  }

  bool verify_bid_changed(int l0, int l1, int l2, int l3, int l4)
  {
    return verify_side_changed(depth_.bids(), bool(l0 == 1), bool(l1 ==1),
                               bool(l2 == 1), bool(l3 == 1), bool(l4 == 1));
  }
  bool verify_ask_changed(int l0, int l1, int l2, int l3, int l4)
  {
    return verify_side_changed(depth_.asks(), bool(l0 == 1), bool(l1 == 1),
                               bool(l2 == 1), bool(l3 == 1), bool(l4 == 1));
  }

  bool verify_bid_stamps(ChangeId l0, ChangeId l1, ChangeId l2, 
                  ChangeId l3, ChangeId l4)
  {
    return verify_side_stamps(depth_.bids(), l0, l1, l2, l3, l4);
  }

  bool verify_ask_stamps(ChangeId l0, ChangeId l1, ChangeId l2, 
                  ChangeId l3, ChangeId l4)
  {
    return verify_side_stamps(depth_.asks(), l0, l1, l2, l3, l4);
  }

  bool verify_bbo_changed(int bid_changed, int ask_changed)
  {
    bool matched = true;
    
    if (depth_.bids()->changed_since(last_change_) != bool(bid_changed == 1)) {
      std::cout << "best bid changed incorrect" << std::endl;
      matched = false;
    }
    if (depth_.asks()->changed_since(last_change_) != bool(ask_changed == 1)) {
      std::cout << "best ask changed incorrect" << std::endl;
      matched = false;
    }
    return matched;
  }

  bool verify_bbo_stamps(ChangeId bid_stamp, ChangeId ask_stamp)
  {
    bool matched = true;
    if (depth_.bids()->last_change() != bid_stamp) {
      std::cout << "best bid change " 
                << depth_.bids()->last_change() << std::endl;
      matched = false;
    }
    if (depth_.asks()->last_change() != ask_stamp) {
      std::cout << "best ask change " 
                << depth_.asks()->last_change() << std::endl;
      matched = false;
    }
    return matched;
  }
  private:
  ChangeId last_change_;
  bool verify_side_stamps(const DepthLevel* start, 
                   ChangeId l0, ChangeId l1, ChangeId l2, 
                   ChangeId l3, ChangeId l4)
  {
    bool matched = true;
    if (start[0].last_change() != l0) {
      std::cout << "change id[0] " << start[0].last_change() << std::endl;
      matched = false;
    }
    if (start[1].last_change() != l1) {
      std::cout << "change id[1] " << start[1].last_change() << std::endl;
      matched = false;
    }
    if (start[2].last_change() != l2) {
      std::cout << "change id[2] " << start[2].last_change() << std::endl;
      matched = false;
    }
    if (start[3].last_change() != l3) {
      std::cout << "change id[3] " << start[3].last_change() << std::endl;
      matched = false;
    }
    if (start[4].last_change() != l4) {
      std::cout << "change id[4] " << start[4].last_change() << std::endl;
      matched = false;
    }
    return matched;
  }

  bool verify_side_changed(const DepthLevel* start,
                           bool l0, bool l1, bool l2, bool l3, bool l4)
  {
    bool matched = true;
    
    if (start[0].changed_since(last_change_) != l0) {
      std::cout << "changed[0] mismatch" << std::endl;
      matched = false;
    }
    if (start[1].changed_since(last_change_) != l1) {
      std::cout << "changed[1] mismatch" << std::endl;
      matched = false;
    }
    if (start[2].changed_since(last_change_) != l2) {
      std::cout << "changed[2] mismatch" << std::endl;
      matched = false;
    }
    if (start[3].changed_since(last_change_) != l3) {
      std::cout << "changed[3] mismatch" << std::endl;
      matched = false;
    }
    if (start[4].changed_since(last_change_) != l4) {
      std::cout << "changed[4] mismatch" << std::endl;
      matched = false;
    }
    return matched;
  }

  private:
  SizedDepth& depth_;
};

} } // namespace
