// Copyright (c) 2012 - 2017 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include "depth_constants.h"
#include "depth_level.h"
#include <stdexcept>
#include <map>
#include <cmath>
#include <string.h>
#include <functional>

namespace liquibook { namespace book {
/// @brief container of limit order data aggregated by price.  Designed so that
///    the depth levels themselves are easily copyable with a single memcpy
///    when used with a separate callback thread.
///
/// TODO: Fix the bid and ask methods to behave like a normal iterator (i.e. begin(), back(), and end()

template <int SIZE=5> 
class Depth {
public:
  /// @brief construct
  Depth();

  /// @brief get the first bid level (const)
  const DepthLevel* bids() const;
  /// @brief get the last bid level (const)
  const DepthLevel* last_bid_level() const;
  /// @brief get the first ask level (const)
  const DepthLevel* asks() const;
  /// @brief get the last ask level (const)
  const DepthLevel* last_ask_level() const;
  /// @brief get one past the last ask level (const)
  const DepthLevel* end() const;

  /// @brief get the first bid level (mutable)
  DepthLevel* bids();
  /// @brief get the last bid level (mutable)
  DepthLevel* last_bid_level();
  /// @brief get the first ask level (mutable)
  DepthLevel* asks();
  /// @brief get the last ask level (mutable)
  DepthLevel* last_ask_level();

  /// @brief add an order
  /// @param price the price level of the order
  /// @param qty the open quantity of the order
  /// @param is_bid indicator of bid or ask
  void add_order(Price price, Quantity qty, bool is_bid);

  /// @brief ignore future fill quantity on a side, due to a match at 
  ///        accept time for an order
  /// @param qty the open quantity to ignore
  /// @param is_bid indicator of bid or ask
  void ignore_fill_qty(Quantity qty, bool is_bid);

  /// @brief handle an order fill
  /// @param price the price level of the order
  /// @param fill_qty the quantity of this fill
  /// @param filled was this order completely filled?
  /// @param is_bid indicator of bid or ask
  void fill_order(Price price, 
                  Quantity fill_qty, 
                  bool filled,
                  bool is_bid);
  /// @brief cancel or fill an order
  /// @param price the price level of the order
  /// @param open_qty the open quantity of the order
  /// @param is_bid indicator of bid or ask
  /// @return true if the close erased a visible level
  bool close_order(Price price, Quantity open_qty, bool is_bid);

  /// @brief change quantity of an order
  /// @param price the price level of the order
  /// @param qty_delta the change in open quantity of the order (+ or -)
  /// @param is_bid indicator of bid or ask
  void change_qty_order(Price price, int32_t qty_delta, bool is_bid);

  /// @brief replace a order
  /// @param current_price the current price level of the order
  /// @param new_price the new price level of the order
  /// @param current_qty the current open quantity of the order
  /// @param new_qty the new open quantity of the order
  /// @param is_bid indicator of bid or ask
  /// @return true if the close erased a visible level
  bool replace_order(Price current_price,
                     Price new_price,
                     Quantity current_qty,
                     Quantity new_qty,
                     bool is_bid);

  /// @brief does this depth need bid restoration after level erasure
  /// @param restoration_price the price to restore after (out)
  /// @return true if restoration is needed (previously was full)
  bool needs_bid_restoration(Price& restoration_price);

  /// @brief does this depth need ask restoration after level erasure
  /// @param restoration_price the price to restore after (out)
  /// @return true if restoration is needed (previously was full)
  bool needs_ask_restoration(Price& restoration_price);

  /// @brief has the depth changed since the last publish
  bool changed() const;

  /// @brief what was the ID of the last change?
  ChangeId last_change() const;

  /// @brief what was the ID of the last published change?
  ChangeId last_published_change() const;

  /// @brief note the ID of last published change
  void published();

private:
  DepthLevel levels_[SIZE*2];
  ChangeId last_change_;
  ChangeId last_published_change_;
  Quantity ignore_bid_fill_qty_;
  Quantity ignore_ask_fill_qty_;

  typedef std::map<Price, DepthLevel, std::greater<Price> > BidLevelMap;
  typedef std::map<Price, DepthLevel, std::less<Price> > AskLevelMap;
  BidLevelMap excess_bid_levels_;
  AskLevelMap excess_ask_levels_;

  /// @brief find the level associated with the price
  /// @param price the price to find
  /// @param is_bid indicator of bid or ask
  /// @param should_create should a level for the price be created, if necessary
  /// @return the level, or nullptr if not found and full
  DepthLevel* find_level(Price price, bool is_bid, bool should_create = true);

  /// @brief insert a new level before this level and shift down
  /// @param level the level to insert before
  /// @param is_bid indicator of bid or ask
  /// @param price the price to initialize the level at
  void insert_level_before(DepthLevel* level,
                           bool is_bid,
                           Price price);

  /// @brief erase a level and shift up
  /// @param level the level to erase
  /// @param is_bid indicator of bid or ask
  void erase_level(DepthLevel* level, bool is_bid);
};

template <int SIZE> 
Depth<SIZE>::Depth()
: last_change_(0),
  last_published_change_(0),
  ignore_bid_fill_qty_(0),
  ignore_ask_fill_qty_(0)
{
  memset(levels_, 0, sizeof(DepthLevel) * SIZE * 2);
}

template <int SIZE> 
inline const DepthLevel* 
Depth<SIZE>::bids() const
{
  return levels_;
}

template <int SIZE> 
inline const DepthLevel* 
Depth<SIZE>::asks() const
{
  return levels_ + SIZE;
}

template <int SIZE> 
inline const DepthLevel*
Depth<SIZE>::last_bid_level() const
{
  return levels_ + (SIZE - 1);
}

template <int SIZE> 
inline const DepthLevel*
Depth<SIZE>::last_ask_level() const
{
  return levels_ + (SIZE * 2 - 1);
}

template <int SIZE> 
inline const DepthLevel* 
Depth<SIZE>::end() const
{
  return levels_ + (SIZE * 2);
}

template <int SIZE> 
inline DepthLevel* 
Depth<SIZE>::bids()
{
  return levels_;
}

template <int SIZE> 
inline DepthLevel* 
Depth<SIZE>::asks()
{
  return levels_ + SIZE;
}

template <int SIZE> 
inline DepthLevel*
Depth<SIZE>::last_bid_level()
{
  return levels_ + (SIZE - 1);
}

template <int SIZE> 
inline DepthLevel*
Depth<SIZE>::last_ask_level()
{
  return levels_ + (SIZE * 2 - 1);
}

template <int SIZE> 
inline void
Depth<SIZE>::add_order(Price price, Quantity qty, bool is_bid)
{
  ChangeId last_change_copy = last_change_;
  DepthLevel* level = find_level(price, is_bid);
  if (level) {
    level->add_order(qty);
    // If this is a visible level
    if (!level->is_excess()) {
      // The depth changed
      last_change_ = last_change_copy + 1; // Ensure incremented
      level->last_change(last_change_copy + 1);
    }
    // The level is not marked as changed if it is not visible
  }
}

template <int SIZE> 
inline void
Depth<SIZE>::ignore_fill_qty(Quantity qty, bool is_bid)
{
  if (is_bid) {
    if (ignore_bid_fill_qty_) {
      throw std::runtime_error("Unexpected ignore_bid_fill_qty_");
    }
    ignore_bid_fill_qty_ = qty;
  } else {
    if (ignore_ask_fill_qty_) {
      throw std::runtime_error("Unexpected ignore_ask_fill_qty_");
    }
    ignore_ask_fill_qty_ = qty;
  }  
}

template <int SIZE> 
inline void
Depth<SIZE>::fill_order(
  Price price, 
  Quantity fill_qty, 
  bool filled,
  bool is_bid)
{
  if (is_bid && ignore_bid_fill_qty_) {
    ignore_bid_fill_qty_ -= fill_qty;
  } else if ((!is_bid) && ignore_ask_fill_qty_) {
    ignore_ask_fill_qty_ -= fill_qty;
  } else if (filled) {
    close_order(price, fill_qty, is_bid);
  } else {
    change_qty_order(price, -(int32_t)fill_qty, is_bid);
  }
}

template <int SIZE> 
inline bool
Depth<SIZE>::close_order(Price price, Quantity open_qty, bool is_bid)
{
  DepthLevel* level = find_level(price, is_bid, false);
  if (level) {
    // If this is the last order on the level
    if (level->close_order(open_qty)) {
      erase_level(level, is_bid);
      return true;
    // Else, mark the level as changed
    } else {
      level->last_change(++last_change_);
    }
  }
  return false;
}

template <int SIZE> 
inline void
Depth<SIZE>::change_qty_order(Price price, int32_t qty_delta, bool is_bid)
{
  DepthLevel* level = find_level(price, is_bid, false);
  if (level && qty_delta) {
    if (qty_delta > 0) {
      level->increase_qty(Quantity(qty_delta));
    } else {
      level->decrease_qty(Quantity(std::abs(qty_delta)));
    }
    level->last_change(++last_change_);
  }
  // Ignore if not found - may be beyond our depth size
}
 
template <int SIZE> 
inline bool
Depth<SIZE>::replace_order(
  Price current_price,
  Price new_price,
  Quantity current_qty,
  Quantity new_qty,
  bool is_bid)
{
  bool erased = false;
  // If the price is unchanged, modify this level only
  if (current_price == new_price) {
    int32_t qty_delta = ((int32_t)new_qty) - current_qty;
    // Only change order qty.  If this closes order, a cancel callback will
    // also be fired
    change_qty_order(current_price, qty_delta, is_bid);
  // Else this is a price change
  } else {
    // Add the new order quantity first, and possibly insert a new level
    add_order(new_price, new_qty, is_bid);
    // Remove the old order quantity, and possibly erase a level
    erased = close_order(current_price, current_qty, is_bid);
  }
  return erased;
}

template <int SIZE> 
inline bool
Depth<SIZE>::needs_bid_restoration(Price& restoration_price)
{
  // If this depth has multiple levels
  if (SIZE > 1) {
    // Restore using the price before the last level
    restoration_price = (last_bid_level() - 1)->price();
    // Restore if that level was valid
    return restoration_price != INVALID_LEVEL_PRICE;
  // Else this depth is BBO only
  } else if (SIZE == 1) {
    // There is no earlier level to look at, restore using the first non-market
    // bid price
    restoration_price = MARKET_ORDER_BID_SORT_PRICE;
    // Always restore on BBO only
    return true;
  }
  throw std::runtime_error("Depth size less than one not allowed");
}

template <int SIZE> 
inline bool
Depth<SIZE>::needs_ask_restoration(Price& restoration_price)
{
  // If this depth has multiple levels
  if (SIZE > 1) {
    // Restore using the price before the last level
    restoration_price = (last_ask_level() - 1)->price();
    // Restore if that level was valid
    return restoration_price != INVALID_LEVEL_PRICE;
  // Else this depth is BBO only
  } else if (SIZE == 1) {
    // There is no earlier level to look at, restore the first non-market
    // ask price
    restoration_price =  MARKET_ORDER_ASK_SORT_PRICE;
    // Always restore on BBO only
    return true;
  }
  throw std::runtime_error("Depth size less than one not allowed");
}

template <int SIZE> 
DepthLevel*
Depth<SIZE>::find_level(Price price, bool is_bid, bool should_create)
{
  // Find starting and ending point
  DepthLevel* level = is_bid ? bids() : asks();
  const DepthLevel* past_end = is_bid ? asks() : end();
  // Linear search each level
  for ( ; level != past_end; ++level) {
    if (level->price() == price) {
      break;
    // Else if the level is blank
    } else if (should_create && level->price() == INVALID_LEVEL_PRICE) {
      level->init(price, false);  // Change ID will be assigned by caller
      break;  // Blank slot
    // Else if the bid level price is too low
    } else if (is_bid && should_create && level->price() < price) {
      // Insert a slot
      insert_level_before(level, is_bid, price);
      break;
    // Else if the ask level price is too high
    } else if ((!is_bid) && should_create && level->price() > price) {
      // Insert a slot
      insert_level_before(level, is_bid, price);
      break;
    }
  }
  // If level was not found
  if (level == past_end) {
    if (is_bid) {
      // Search in excess bid levels
      BidLevelMap::iterator find_result = excess_bid_levels_.find(price);
      // If found in excess levels, return location
      if (find_result != excess_bid_levels_.end()) {
        level = &find_result->second;
      // Else not found, insert if one should be created
      } else if (should_create) {
        DepthLevel new_level;
        new_level.init(price, true);
        std::pair<BidLevelMap::iterator, bool> insert_result;
        insert_result = excess_bid_levels_.insert(
            std::make_pair(price, new_level));
        level = &insert_result.first->second;
      }
    } else {
      // Search in excess ask levels
      AskLevelMap::iterator find_result = excess_ask_levels_.find(price);
      // If found in excess levels, return location
      if (find_result != excess_ask_levels_.end()) {
        level = &find_result->second;
      // Else not found, insert if one should be created
      } else if (should_create) {
        DepthLevel new_level;
        new_level.init(price, true);
        std::pair<AskLevelMap::iterator, bool> insert_result;
        insert_result = excess_ask_levels_.insert(
            std::make_pair(price, new_level));
        level = &insert_result.first->second;
      }
    }
  }
  return level;
}

template <int SIZE> 
void
Depth<SIZE>::insert_level_before(DepthLevel* level, 
                                 bool is_bid,
                                 Price price)
{
  DepthLevel* last_side_level = is_bid ? last_bid_level() : last_ask_level();

  // If the last level has valid data
  if (last_side_level->price() != INVALID_LEVEL_PRICE) {
    DepthLevel excess_level;
    excess_level.init(0, true);  // Will assign over price
    excess_level = *last_side_level;
    // Save it in excess levels
    if (is_bid) {
      excess_bid_levels_.insert(
      std::make_pair(last_side_level->price(), excess_level));
    } else {
      excess_ask_levels_.insert(
      std::make_pair(last_side_level->price(), excess_level));
    }
  }
  // Back from end
  DepthLevel* current_level = last_side_level - 1;
  // Increment only once
  ++last_change_;
  // Last level to process is one passed in
  while (current_level >= level) {
    // Copy level to level one lower
    *(current_level + 1) = *current_level;
    // If the level being copied is valid
    if (current_level->price() != INVALID_LEVEL_PRICE) {
      // Update change Id
      (current_level + 1)->last_change(last_change_);
    }
    // Move back one
    --current_level;
   }
   level->init(price, false);
}

template <int SIZE> 
void
Depth<SIZE>::erase_level(DepthLevel* level, bool is_bid)
{
  // If ther level being erased is from the excess, remove excess from map
  if (level->is_excess()) {
    if (is_bid) {
      excess_bid_levels_.erase(level->price());
    } else {
      excess_ask_levels_.erase(level->price());
    }
  // Else the level being erased is not excess, copy over from those worse
  } else {
    DepthLevel* last_side_level = is_bid ? last_bid_level() : last_ask_level();
    // Increment once
    ++last_change_;
    DepthLevel* current_level = level;
    // Level to end
    while (current_level < last_side_level) {
      // If this is the first level, or the level to be overwritten is valid
      // (must force first level, when called already should be invalidated)
      if ((current_level->price() != INVALID_LEVEL_PRICE) ||
          (current_level == level)) {
        // Copy to current level from one lower
        *current_level = *(current_level + 1);
        // Mark the current level as updated
        current_level->last_change(last_change_);
      }
      // Move forward one
      ++current_level;
    }

    // If I erased the last level, or the last level was valid
    if ((level == last_side_level) ||
        (last_side_level->price() != INVALID_LEVEL_PRICE)) {
      // Attempt to restore last level from excess
      if (is_bid) {
        BidLevelMap::iterator best_bid = excess_bid_levels_.begin();
        if (best_bid != excess_bid_levels_.end()) {
          *last_side_level = best_bid->second;
          excess_bid_levels_.erase(best_bid);
        } else {
          // Nothing to restore, last level is blank
          last_side_level->init(INVALID_LEVEL_PRICE, false);
          last_side_level->last_change(last_change_);
        }
      } else {
        AskLevelMap::iterator best_ask = excess_ask_levels_.begin();
        if (best_ask != excess_ask_levels_.end()) {
          *last_side_level = best_ask->second;
          excess_ask_levels_.erase(best_ask);
        } else {
          // Nothing to restore, last level is blank
          last_side_level->init(INVALID_LEVEL_PRICE, false);
          last_side_level->last_change(last_change_);
        }
      }
      last_side_level->last_change(last_change_);
    }
  }
}

template <int SIZE> 
bool
Depth<SIZE>::changed() const
{
  return last_change_ > last_published_change_;
}


template <int SIZE> 
ChangeId
Depth<SIZE>::last_change() const
{
  return last_change_;
}

template <int SIZE> 
ChangeId
Depth<SIZE>::last_published_change() const
{
  return last_published_change_;
}


template <int SIZE> 
void
Depth<SIZE>::published()
{
  last_published_change_ = last_change_;
}

} }
