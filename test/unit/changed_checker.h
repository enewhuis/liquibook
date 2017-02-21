// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <book/depth.h>
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

  bool verify_bid_changed(bool l0, bool l1, bool l2, bool l3, bool l4)
  {
    return verify_side_changed(depth_.bids(), l0, l1, l2, l3, l4);
  }
  bool verify_ask_changed(bool l0, bool l1, bool l2, bool l3, bool l4)
  {
    return verify_side_changed(depth_.asks(), l0, l1, l2, l3, l4);
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

  bool verify_bbo_changed(bool bid_changed, bool ask_changed)
  {
    bool matched = true;
    
    if (depth_.bids()->changed_since(last_change_))
    {
      if(! bid_changed) {
        std::cout << "best bid unexpected change" << std::endl;
        matched = false;
      }
    }
    else if(bid_changed){
      std::cout << "best bid expected change" << std::endl;
      matched = false;
    }
    if (depth_.asks()->changed_since(last_change_)){
      if(!ask_changed) {
        std::cout << "best ask unexpected change" << std::endl;
        matched = false;
      }
    }
    else if(ask_changed){
      std::cout << "best ask expected change" << std::endl;
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

  bool verify_level(const DepthLevel* levels, size_t index, bool expected)
  {
    bool matched = true;
    if (levels[index].changed_since(last_change_) != expected) 
    {
      matched = false;
      if(expected)
      {
        std::cout << "expected change level[" << index << "] " << levels[index].price() << std::endl;
      }
      else
      {
        std::cout << "unexpected change level[" << index << "] " << levels[index].price() << std::endl;
      }
    }
    return matched;
  } 

  bool verify_side_changed(const DepthLevel* start,
                           bool l0, bool l1, bool l2, bool l3, bool l4)
  {
    bool matched = true;
    matched = verify_level(start, 0, l0) && matched;
    matched = verify_level(start, 1, l1) && matched;
    matched = verify_level(start, 2, l2) && matched;
    matched = verify_level(start, 3, l3) && matched;
    matched = verify_level(start, 4, l4) && matched;
    return matched;
  }

  private:
  SizedDepth& depth_;
};

} } // namespace
