// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#ifndef order_book_h
#define order_book_h

#include "callback.h"
#include "order.h"
#include "order_listener.h"
#include "order_book_listener.h"
#include "depth_level.h"
#include <map>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <list>
#include <functional>

namespace liquibook { namespace book {

template<class OrderPtr>
class OrderListener;

template<class OrderBook>
class OrderBookListener;

/// @brief Tracker of an order's state, to keep inside the OrderBook.  
///   Kept separate from the order itself.
template <class OrderPtr = Order*>
class OrderTracker {
public:
  /// @brief construct
  OrderTracker(const OrderPtr& order, OrderConditions conditions = 0);

  /// @brief modify the order quantity
  void change_qty(int32_t delta);

  /// @brief fill an order
  /// @param qty the number of shares filled in this fill
  void fill(Quantity qty); 

  /// @brief is there no remaining open quantity in this order?
  bool filled() const;

  /// @brief get the total filled quantity of this order
  Quantity filled_qty() const;

  /// @brief get the open quantity of this order
  Quantity open_qty() const;

  /// @brief get the order pointer
  const OrderPtr& ptr() const;

  /// @brief get the order pointer
  OrderPtr& ptr();

  /// @ brief is this order marked all or none?
  bool all_or_none() const;

  /// @ brief is this order marked immediate or cancel?
  bool immediate_or_cancel() const;

private:
  OrderPtr order_;
  Quantity open_qty_;
  OrderConditions conditions_;
};

/// @brief The limit order book of a security.  Template implementation allows
///        user to supply common or smart pointers, and to provide a different
///        Order class completely (as long as interface is obeyed).
template <class OrderPtr = Order*>
class OrderBook {
public:
  typedef OrderTracker<OrderPtr > Tracker;
  typedef Callback<OrderPtr > TypedCallback;
  typedef OrderListener<OrderPtr > TypedOrderListener;
  typedef OrderBook<OrderPtr > MyClass;
  typedef OrderBookListener<MyClass > TypedOrderBookListener;
  typedef std::vector<TypedCallback > Callbacks;
  typedef std::multimap<Price, Tracker, std::greater<Price> >  Bids;
  typedef std::multimap<Price, Tracker, std::less<Price> >     Asks;
  typedef std::list<typename Bids::iterator> DeferredBidCrosses;
  typedef std::list<typename Asks::iterator> DeferredAskCrosses;

  /// @brief construct
  OrderBook();

  /// @brief set the order listener
  void set_order_listener(TypedOrderListener* listener);

  /// @brief set the order book listener
  void set_order_book_listener(TypedOrderBookListener* listener);

  /// @brief add an order to book
  /// @param order the order to add
  /// @param conditions special conditions on the order
  /// @return true if the add resulted in a fill
  virtual bool add(const OrderPtr& order, OrderConditions conditions = 0);

  /// @brief cancel an order in the book
  virtual void cancel(const OrderPtr& order);

  /// @brief replace an order in the book
  /// @param order the order to replace
  /// @param size_delta the change in size for the order (positive or negative)
  /// @param new_price the new order price, or PRICE_UNCHANGED
  /// @return true if the replace resulted in a fill
  virtual bool replace(const OrderPtr& order, 
                       int32_t size_delta = SIZE_UNCHANGED,
                       Price new_price = PRICE_UNCHANGED);

  /// @brief access the bids container
  const Bids& bids() const { return bids_; };

  /// @brief access the asks container
  const Asks& asks() const { return asks_; };

  /// @brief move callbacks to another thread's container
  void move_callbacks(Callbacks& target);

  /// @brief perform all callbacks in the queue
  virtual void perform_callbacks();

  /// @brief perform an individual callback
  virtual void perform_callback(TypedCallback& cb);

  /// @brief log the orders in the book.
  void log() const;

protected:
  /// @brief match a new ask to current bids
  /// @param inbound_order the inbound order
  /// @param inbound_price price of the inbound order
  /// @param bids current bids
  /// @return true if a match occurred 
  virtual bool match_order(Tracker& inbound_order, 
                           const Price& inbound_price, 
                           Bids& bids);

  /// @brief match a new bid to current asks
  /// @param inbound_order the inbound order
  /// @param inbound_price price of the inbound order
  /// @param asks current asks
  /// @return true if a match occurred 
  virtual bool match_order(Tracker& inbound_order, 
                           const Price& inbound_price, 
                           Asks& asks);

  /// @brief perform fill on two orders
  /// @param inbound_tracker the new (or changed) order tracker
  /// @param current_tracker the current order tracker
  void cross_orders(Tracker& inbound_tracker, 
                    Tracker& current_tracker);

  /// @brief perform validation on the order, and create reject callbacks if not
  /// @param order the order to validate
  /// @return true if the order is valid
  virtual bool is_valid(const OrderPtr& order, OrderConditions conditions);

  /// @brief perform validation on the order replace, and create reject 
  ///   callbacks if not
  /// @param order the order to validate
  /// @param size_delta the change in size (+ or -)
  /// @param new_price the new order price
  /// @return true if the order replace is valid
  virtual bool is_valid_replace(const Tracker& order,
                                int32_t size_delta,
                                Price new_price);

  /// @brief find a bid
  void find_bid(const OrderPtr& order, typename Bids::iterator& result);

  /// @brief find an ask
  void find_ask(const OrderPtr& order, typename Asks::iterator& result);

  /// @brief match an inbound with a current order
  virtual bool matches(const Tracker& inbound_order, 
                       const Price& inbound_price, 
                       const Quantity inbound_open_qty,
                       const Tracker& current_order,
                       const Price& current_price,
                       bool inbound_is_buy);
private:
  Bids bids_;
  Asks asks_;
  DeferredBidCrosses deferred_bid_crosses_;
  DeferredAskCrosses deferred_ask_crosses_;
  Callbacks callbacks_;
  TypedOrderListener* order_listener_;
  TypedOrderBookListener* order_book_listener_;
  TransId trans_id_;

  Price sort_price(const OrderPtr& order);
  bool add_order(Tracker& order_tracker, Price order_price);
};

template <class OrderPtr>
inline
OrderTracker<OrderPtr>::OrderTracker(
  const OrderPtr& order, 
  OrderConditions conditions)
: order_(order),
  open_qty_(order_->open_qty()),
  conditions_(conditions)
{
}

template <class OrderPtr>
inline void
OrderTracker<OrderPtr>::change_qty(int32_t delta)
{
  if ((delta < 0 && 
      (int)open_qty_ < std::abs(delta))) {
    throw 
        std::runtime_error("Replace size reduction larger than open quantity");
  }
  open_qty_ += delta;
}

template <class OrderPtr>
inline void
OrderTracker<OrderPtr>::fill(Quantity qty) 
{
  if (qty > open_qty_) {
    throw std::runtime_error("Fill size larger than open quantity");
  }
  open_qty_ -= qty;
}

template <class OrderPtr>
inline bool
OrderTracker<OrderPtr>::filled() const
{
  return open_qty_ == 0;
}

template <class OrderPtr>
inline Quantity
OrderTracker<OrderPtr>::filled_qty() const
{
  return order_->order_qty() - open_qty();
}

template <class OrderPtr>
inline Quantity
OrderTracker<OrderPtr>::open_qty() const
{
  return open_qty_;
}

template <class OrderPtr>
inline const OrderPtr&
OrderTracker<OrderPtr>::ptr() const
{
  return order_;
}

template <class OrderPtr>
inline OrderPtr&
OrderTracker<OrderPtr>::ptr()
{
  return order_;
}

template <class OrderPtr>
inline bool
OrderTracker<OrderPtr>::all_or_none() const
{
  return bool(conditions_ & oc_all_or_none);
}

template <class OrderPtr>
inline bool
OrderTracker<OrderPtr>::immediate_or_cancel() const
{
  return bool((conditions_ & oc_immediate_or_cancel) != 0);
}

template <class OrderPtr>
OrderBook<OrderPtr>::OrderBook()
: order_listener_(NULL),
  order_book_listener_(NULL),
  trans_id_(0)
{
  callbacks_.reserve(16);
}

template <class OrderPtr>
void
OrderBook<OrderPtr>::set_order_listener(TypedOrderListener* listener)
{
  order_listener_ = listener;
}

template <class OrderPtr>
void
OrderBook<OrderPtr>::set_order_book_listener(TypedOrderBookListener* listener)
{
  order_book_listener_ = listener;
}

template <class OrderPtr>
inline bool
OrderBook<OrderPtr>::add(const OrderPtr& order, OrderConditions conditions)
{
  // Increment transacion ID
  ++trans_id_;  

  bool matched = false;

  // If the order is invalid, exit
  if (!is_valid(order, conditions)) {
    // reject created by is_valid
  } else {
    callbacks_.push_back(TypedCallback::accept(order, trans_id_));
    TypedCallback& accept_cb = callbacks_.back();

    Price order_price = sort_price(order);

    Tracker inbound(order, conditions);
    matched = add_order(inbound, order_price);
    if (matched) {
      // Note the filled qty in the callback
      accept_cb.match_qty = inbound.filled_qty();
    }
    // Cancel any unfilled IOC order
    if (inbound.immediate_or_cancel() && !inbound.filled()) {
      // NOTE - this may need he actual open qty???
      callbacks_.push_back(TypedCallback::cancel(order, 0, trans_id_));
    }
    callbacks_.push_back(TypedCallback::book_update(this, trans_id_));
  }
  return matched;
}

template <class OrderPtr>
inline void
OrderBook<OrderPtr>::cancel(const OrderPtr& order)
{
  // Increment transacion ID
  ++trans_id_;  

  bool found = false;
  Quantity open_qty;
  // If the cancel is a buy order
  if (order->is_buy()) {
    typename Bids::iterator bid;
    find_bid(order, bid);
    if (bid != bids_.end()) {
      open_qty = bid->second.open_qty();
      // Remove from container for cancel
      bids_.erase(bid);
      found = true;
    }
  // Else the cancel is a sell order
  } else {
    typename Asks::iterator ask;
    find_ask(order, ask);
    if (ask != asks_.end()) {
      open_qty = ask->second.open_qty();
      // Remove from container for cancel
      asks_.erase(ask);
      found = true;
    }
  } 
  // If the cancel was found, issue callback
  if (found) {
    callbacks_.push_back(TypedCallback::cancel(order, open_qty, trans_id_));
    callbacks_.push_back(TypedCallback::book_update(this, trans_id_));
  } else {
    callbacks_.push_back(
        TypedCallback::cancel_reject(order, "not found", trans_id_));
  }
}

template <class OrderPtr>
inline bool
OrderBook<OrderPtr>::replace(
  const OrderPtr& order, 
  int32_t size_delta,
  Price new_price)
{
  // Increment transacion ID
  ++trans_id_;  

  bool matched = false;
  bool found = false;
  bool price_change = new_price && (new_price != order->price());

  Price price = (new_price == PRICE_UNCHANGED) ? order->price() : new_price;
  // TODO can we use value in tracker?
  Quantity new_order_qty = order->order_qty() + size_delta;

  // If the order to replace is a buy order
  if (order->is_buy()) {
    typename Bids::iterator bid;
    find_bid(order, bid);
    // If the order was found
    if (bid != bids_.end()) {
      found = true;
      // If this is a valid replace
      if (is_valid_replace(bid->second, size_delta, new_price)) {
        // Accept the replace
        callbacks_.push_back(
            TypedCallback::replace(order, new_order_qty, price, trans_id_));
        callbacks_.push_back(TypedCallback::book_update(this, trans_id_));
        Quantity new_open_qty = bid->second.open_qty() + size_delta;
        bid->second.change_qty(size_delta);  // Update my copy
        // If the size change will close the order
        if (!new_open_qty) {
          callbacks_.push_back(TypedCallback::cancel(order, -size_delta, trans_id_));
          bids_.erase(bid); // Remove order
        // Else rematch the new order - there could be a price change
        // or size change - that could cause all or none match
        } else {
          matched = add_order(bid->second, price); // Add order
          bids_.erase(bid); // Remove order
        }
      }
    }
  // Else the order to replace is a sell order
  } else {
    typename Asks::iterator ask;
    find_ask(order, ask);
    // If the order was found
    if (ask != asks_.end()) {
      found = true;
      // If this is a valid replace
      if (is_valid_replace(ask->second, size_delta, new_price)) {
        // Accept the replace
        callbacks_.push_back(
            TypedCallback::replace(order, new_order_qty, price, trans_id_));
        callbacks_.push_back(TypedCallback::book_update(this, trans_id_));
        Quantity new_open_qty = ask->second.open_qty() + size_delta;
        ask->second.change_qty(size_delta);  // Update my copy
        // If the size change will close the order
        if (!new_open_qty) {
          callbacks_.push_back(TypedCallback::cancel(order, -size_delta, trans_id_));
          asks_.erase(ask); // Remove order
        // Else rematch the new order if there is a price change or the order
        // is all or none (for which a size change could cause it to match)
        } else if (price_change || ask->second.all_or_none()) {
          matched = add_order(ask->second, price); // Add order
          asks_.erase(ask); // Remove order
        }
      }
    } 

    if (!found) {
      callbacks_.push_back(
          TypedCallback::replace_reject(order, "not found", trans_id_));
    }
  }

  return matched;
}

template <class OrderPtr>
inline bool
OrderBook<OrderPtr>::match_order(Tracker& inbound, 
                                 const Price& inbound_price, 
                                 Bids& bids)
{
  bool matched = false;
  typename Bids::iterator bid;
  Quantity matched_qty = 0;
  Quantity inbound_qty = inbound.open_qty();

  for (bid = bids.begin(); bid != bids.end(); ) {
    // If the inbound order matches the current order
    if (matches(inbound, 
                inbound_price, 
                inbound.open_qty() - matched_qty, 
                bid->second, 
                bid->first, 
                false)) {
      // If the inbound order is an all or none order
      if (inbound.all_or_none()) {
        // Track how much of the inbound order has been matched
        matched_qty += bid->second.ptr()->open_qty();
        // If we have matched enough quantity to fill the inbound order
        if (matched_qty >= inbound_qty) {
          matched =  true;

          // Unwind the deferred crosses
          typename DeferredBidCrosses::iterator dbc;
          for (dbc = deferred_bid_crosses_.begin(); 
               dbc != deferred_bid_crosses_.end(); ++dbc) {
            // Adjust tracking values for cross
            cross_orders(inbound, (*dbc)->second);

            // If the existing order was filled, remove it
            if ((*dbc)->second.filled()) {
              bids.erase(*dbc);
            }
          }
        // Else we have to defer crossing this order
        } else {
          deferred_bid_crosses_.push_back(bid);
          ++bid;
        }
      } else {
        matched =  true;
      }

      if (matched) {
        // Adjust tracking values for cross
        cross_orders(inbound, bid->second);

        // If the existing order was filled, remove it
        if (bid->second.filled()) {
          bids.erase(bid++);
        } else {
          ++bid;
        }

        // if the inbound order is filled, no more matches are possible
        if (inbound.filled()) {
          break;
        }
      }
    // Didn't match, exit loop if this was because of price
    } else if (bid->first < inbound_price) {
      break;
    } else {
      ++bid;
    }
  }

  return matched;
}

template <class OrderPtr>
inline bool
OrderBook<OrderPtr>::match_order(Tracker& inbound, 
                                 const Price& inbound_price, 
                                 Asks& asks)
{
  bool matched = false;
  typename Asks::iterator ask;
  Quantity matched_qty = 0;
  Quantity inbound_qty = inbound.open_qty();

  for (ask = asks.begin(); ask != asks.end(); ) {
    // If the inbound order matches the current order
    if (matches(inbound, 
                inbound_price, 
                inbound.open_qty() - matched_qty, 
                ask->second, 
                ask->first, 
                true)) {
      // If the inbound order is an all or none order
      if (inbound.all_or_none()) {
        // Track how much of the inbound order has been matched
        matched_qty += ask->second.ptr()->open_qty();
        // If we have matched enough quantity to fill the inbound order
        if (matched_qty >= inbound_qty) {
          matched =  true;

          // Unwind the deferred crosses
          typename DeferredAskCrosses::iterator dac;
          for (dac = deferred_ask_crosses_.begin(); 
               dac != deferred_ask_crosses_.end(); ++dac) {
            // Adjust tracking values for cross
            cross_orders(inbound, (*dac)->second);

            // If the existing order was filled, remove it
            if ((*dac)->second.filled()) {
              asks.erase(*dac);
            }
          }
        // Else we have to defer crossing this order
        } else {
          deferred_ask_crosses_.push_back(ask);
          ++ask;
        }
      } else {
        matched =  true;
      }

      if (matched) {
        // Adjust tracking values for cross
        cross_orders(inbound, ask->second);

        // If the existing order was filled, remove it
        if (ask->second.filled()) {
          asks.erase(ask++);
        } else {
          ++ask;
        }

        // if the inbound order is filled, no more matches are possible
        if (inbound.filled()) {
          break;
        }
      }
    // Didn't match, exit loop if this was because of price
    } else if (ask->first > inbound_price) {
      break;
    } else {
      ++ask;
    }
  }
  return matched;
}

template <class OrderPtr>
inline void
OrderBook<OrderPtr>::cross_orders(Tracker& inbound_tracker, 
                                  Tracker& current_tracker)
{
  Quantity fill_qty = std::min(inbound_tracker.open_qty(), 
                               current_tracker.open_qty());
  Price cross_price = current_tracker.ptr()->price();
  // If current order is a market order, cross at inbound price
  if (MARKET_ORDER_PRICE == cross_price) {
    cross_price = inbound_tracker.ptr()->price();
  }
  inbound_tracker.fill(fill_qty);
  current_tracker.fill(fill_qty);

  typename TypedCallback::FillFlags fill_flags = 
                              TypedCallback::ff_neither_filled;
  if (!inbound_tracker.open_qty()) {
    fill_flags = (typename TypedCallback::FillFlags)(
                     fill_flags | TypedCallback::ff_inbound_filled);
  }
  if (!current_tracker.open_qty()) {
    fill_flags = (typename TypedCallback::FillFlags)(
                     fill_flags | TypedCallback::ff_matched_filled);
  }

  callbacks_.push_back(TypedCallback::fill(inbound_tracker.ptr(),
                                           current_tracker.ptr(),
                                           fill_qty,
                                           cross_price,
                                           fill_flags,
                                           trans_id_));
}

template <class OrderPtr>
inline void
OrderBook<OrderPtr>::move_callbacks(Callbacks& target)
{
  target.insert(target.end(), callbacks_.begin(), callbacks_.end());
  callbacks_.erase(callbacks_.begin(), callbacks_.end());
}

template <class OrderPtr>
inline void
OrderBook<OrderPtr>::perform_callbacks()
{
  typename Callbacks::iterator cb;
  for (cb = callbacks_.begin(); cb != callbacks_.end(); ++cb) {
    perform_callback(*cb);
  }
  callbacks_.erase(callbacks_.begin(), callbacks_.end());
}

template <class OrderPtr>
inline void
OrderBook<OrderPtr>::perform_callback(TypedCallback& cb)
{
  // NOTE - this is not yet handled in the parent class.  You are requried to
  //        override in the child.
  // If this is an order callback and I know of an order listener
  if (cb.order && order_listener_) {
    switch (cb.type) {
      case TypedCallback::cb_order_fill: {
        Cost fill_cost = cb.fill_price * cb.fill_qty;
        order_listener_->on_fill(cb.order, cb.matched_order, 
                                 cb.fill_qty, fill_cost);
        break;
      }
      case TypedCallback::cb_order_accept:
        order_listener_->on_accept(cb.order);
        break;
      case TypedCallback::cb_order_reject:
        order_listener_->on_reject(cb.order, cb.reject_reason);
        break;
      case TypedCallback::cb_order_cancel:
        order_listener_->on_cancel(cb.order);
        break;
      case TypedCallback::cb_order_cancel_reject:
        order_listener_->on_cancel_reject(cb.order, cb.reject_reason);
        break;
      case TypedCallback::cb_order_replace:
        order_listener_->on_replace(cb.order, cb.new_order_qty, cb.new_price);
        break;
      case TypedCallback::cb_order_replace_reject:
        order_listener_->on_replace_reject(cb.order, cb.reject_reason);
        break;
      case TypedCallback::cb_unknown:
        // Error
        std::runtime_error("Unexpected callback type for order");
        break;
    }
  } else if (cb.type == TypedCallback::cb_book_update && order_book_listener_) {
    order_book_listener_->on_order_book_change(this);
  }
}

template <class OrderPtr>
inline void
OrderBook<OrderPtr>::log() const
{
  typename Asks::const_reverse_iterator ask;
  typename Bids::const_iterator bid;
  for (ask = asks_.rbegin(); ask != asks_.rend(); ++ask) {
    std::cout << "  Ask " << ask->second.open_qty() << " @ " << ask->first
                          << std::endl;
  }
  for (bid = bids_.begin(); bid != bids_.end(); ++bid) {
    std::cout << "  Bid " << bid->second.open_qty() << " @ " << bid->first
                          << std::endl;
  }
}

template <class OrderPtr>
inline bool
OrderBook<OrderPtr>::is_valid(const OrderPtr& order, OrderConditions )
{
  if (order->order_qty() == 0) {
    callbacks_.push_back(TypedCallback::reject(order, "size must be positive", trans_id_));
    return false;
  } else {
    return true;
  }
}

template <class OrderPtr>
inline bool
OrderBook<OrderPtr>::is_valid_replace(
  const Tracker& order,
  int32_t size_delta,
  Price /*new_price*/)
{
  bool size_decrease = size_delta < 0;
  // If there is not enough open quantity for the size reduction
  if (size_decrease && 
      ((int)order.open_qty() < std::abs(size_delta))) {
    // Reject the replace
    callbacks_.push_back(TypedCallback::replace_reject(order.ptr(), 
                                                       "not enough open qty", 
                                                       trans_id_));
    return false;
  }
  return true;
}

template <class OrderPtr>
inline void
OrderBook<OrderPtr>::find_bid(
  const OrderPtr& order,
  typename Bids::iterator& result)
{
  // Find the order search price
  Price search_price = sort_price(order);
  for (result = bids_.find(search_price); result != bids_.end(); ++result) {
    // If this is the correct bid
    if (result->second.ptr() == order) {
      break;
    // Else if this bid's price is too low to match the search price
    } else if (result->first < search_price) {
      break; // No more possible
    }
  }
}

template <class OrderPtr>
inline void
OrderBook<OrderPtr>::find_ask(
  const OrderPtr& order,
  typename Asks::iterator& result)
{
  // Find the order search price
  Price search_price = sort_price(order);
  for (result = asks_.find(search_price); result != asks_.end(); ++result) {
    // If this is the correct ask
    if (result->second.ptr() == order) {
      break;
    // Else if this ask's price is too high to match the search price
    } else if (result->first > search_price) {
      break; // No more possible
    }
  }
} 

template <class OrderPtr>
inline Price
OrderBook<OrderPtr>::sort_price(const OrderPtr& order)
{
  Price result_price = order->price();
  if (MARKET_ORDER_PRICE == result_price) {
    result_price = (order->is_buy() ? MARKET_ORDER_BID_SORT_PRICE :
                                      MARKET_ORDER_ASK_SORT_PRICE);
  }
  return result_price;
}

template <class OrderPtr>
inline bool
OrderBook<OrderPtr>::add_order(Tracker& inbound, Price order_price)
{
  bool matched = false;
  OrderPtr& order = inbound.ptr();

  // Try to match with current orders
  if (order->is_buy()) {
    matched = match_order(inbound, order_price, asks_);
  } else {
    matched = match_order(inbound, order_price, bids_);
  }

  // If order has remaining open quantity and is not immediate or cancel
  if (inbound.open_qty() && !inbound.immediate_or_cancel()) {
    // If this is a buy order
    if (order->is_buy()) {
      // Insert into bids
      bids_.insert(std::make_pair(order_price, inbound));
    // Else this is a sell order
    } else {
      // Insert into asks
      asks_.insert(std::make_pair(order_price, inbound));
    }
  }
  return matched;
}

template <class OrderPtr>
inline bool
OrderBook<OrderPtr>::matches(
  const Tracker& /*inbound_order*/,
  const Price& inbound_price, 
  const Quantity inbound_open_qty,
  const Tracker& current_order,
  const Price& current_price,
  bool inbound_is_buy)
{
  // Check for price mismatch
  if (inbound_is_buy) {
    // If the inbound buy is not as high as the existing sell
    if (inbound_price < current_price) {
      return false;
    }
  } else {
    // Else if the inbound sell is not as low as the existing buy
    if (inbound_price > current_price) {
      return false;
    }
  }

  if (current_order.all_or_none()) {
    // Don't match current if not completely filled
    if (current_order.open_qty() > inbound_open_qty) {
      return false;
    }
  }

  // If the inbound order is all or none, we can only check quantity after
  // all matches take place

  return true;
}

} }

#endif
