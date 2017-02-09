// Copyright (c) 2012 - 2017 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include "order_tracker.h"
#include "callback.h"
#include "order_listener.h"
#include "order_book_listener.h"
#include "trade_listener.h"
#include "comparable_price.h"

#include <map>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <list>
#include <functional>
#include <algorithm>

namespace liquibook { namespace book {

template<class OrderPtr>
class OrderListener;

template<class OrderBook>
class OrderBookListener;

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
  typedef TradeListener<MyClass > TypedTradeListener;
  typedef OrderBookListener<MyClass > TypedOrderBookListener;
  typedef std::vector<TypedCallback > Callbacks;
  typedef std::multimap<ComparablePrice, Tracker> TrackerMap;
  typedef std::vector<Tracker> TrackerVec;
  // Keep this around briefly for compatibility.
  typedef TrackerMap Bids;
  typedef TrackerMap Asks;

  // TODO: GET RID OF THIS! Or fix it so it behaves correctly
  // Q: When do items get removed from these lists?
  typedef std::list<typename TrackerMap::iterator> DeferredBidCrosses;
  typedef std::list<typename TrackerMap::iterator> DeferredAskCrosses;

  /// @brief construct
  OrderBook(const std::string & symbol = "unknown");

  /// @brief Set symbol for orders in this book.
  void set_symbol(const std::string & symbol);

  /// @ Get the symbol for orders in this book
  /// @return the symbol.
  const std::string & symbol() const;

  /// @brief set the order listener
  void set_order_listener(TypedOrderListener* listener);

  /// @brief set the trade listener
  void set_trade_listener(TypedTradeListener* listener);

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

  /// @brief Set the current market price
  /// Intended to be used during initialization to establish the market
  /// price before this order book has generated any exceptions.
  ///
  /// If price is zero (or never set) no market-to-market trades can happen.
  /// @param price is the current market price for this security.
  void set_market_price(Price price);

  /// @brief Get current market price.
  /// The market price is normally the price at which the last trade happened.
  Price market_price()const;

  /// @brief access the bids container
  const TrackerMap& bids() const { return bids_; };

  /// @brief access the asks container
  const TrackerMap& asks() const { return asks_; };

  /// @brief access stop bid orders
  const TrackerMap & stopBids() const { return stopBids_;}

  /// @brief access stop ask orders
  const TrackerMap & stopAsks() const { return stopAsks_;}

  /// @brief move callbacks to another thread's container
  void move_callbacks(Callbacks& target);

  /// @brief perform all callbacks in the queue
  virtual void perform_callbacks();

  /// @brief perform an individual callback
  virtual void perform_callback(TypedCallback& cb);

  /// @brief log the orders in the book.
  std::ostream & log(std::ostream & out) const;

protected:
  /// @brief match a new order to current orders
  /// @param inbound_order the inbound order
  /// @param inbound_price price of the inbound order
  /// @param current current open orders
  /// @return true if a match occurred 
  virtual bool match_order(Tracker& inbound_order, 
    const Price& inbound_price, 
    TrackerMap& current);

  /// @brief perform fill on two orders
  /// @param inbound_tracker the new (or changed) order tracker
  /// @param current_tracker the current order tracker
  /// @return true if the trade was successful.
  bool cross_orders(Tracker& inbound_tracker, 
                    Tracker& current_tracker);

  /// @brief perform validation on the order, and create reject callbacks if not
  /// @param order the order to validate
  /// @return true if the order is valid
  virtual bool is_valid(const OrderPtr& order, OrderConditions conditions);

  /// @brief find an order in a container
  /// @param order is the the order we are looking for
  /// @param sideMap contains the container where we will look
  /// @param[OUT] result will point to the entry in the container if we find a match
  /// @returns true: match, false: no match
  bool find_on_market(
    const OrderPtr& order,
    typename TrackerMap::iterator& result);


  /// @brief match an inbound with a current order
  virtual bool matches(const Tracker& inbound_order, 
                       const Price& inbound_price, 
                       const Quantity inbound_open_qty,
                       const Tracker& current_order,
                       const ComparablePrice& key);

  /// @brief add incoming stop order to stops colletion unless it's already
  /// on the market.
  /// @return true if added to stops, false if it should go directly to the order book.
  bool add_stop_order(Tracker & tracker);

  /// @brief See if any stop orders should go on the market.
  void check_stop_orders(bool side, Price price, TrackerMap & stops);

  /// @brief accept pending (formerly stop) orders.
  void submit_pending_orders();

private:
//    Price sort_price(const OrderPtr& order);
    bool submit_order(Tracker & inbound);
    bool add_order(Tracker& order_tracker, Price order_price);
private:
  std::string symbol_;
  TrackerMap bids_;
  TrackerMap asks_;
  DeferredBidCrosses deferred_bid_crosses_; // TODO go away
  DeferredAskCrosses deferred_ask_crosses_; // TODO go away

  TrackerMap stopBids_;
  TrackerMap stopAsks_;
  TrackerVec pendingOrders_;

  Callbacks callbacks_;
  TypedOrderListener* order_listener_;
  TypedTradeListener* trade_listener_;
  TypedOrderBookListener* order_book_listener_;
  TransId trans_id_;
  Price marketPrice_;
};

template <class OrderPtr>
OrderBook<OrderPtr>::OrderBook(const std::string & symbol)
: symbol_(symbol),
  order_listener_(nullptr),
  trade_listener_(nullptr),
  order_book_listener_(nullptr),
  trans_id_(0),
  marketPrice_(MARKET_ORDER_PRICE)
{
  callbacks_.reserve(16);
}

template <class OrderPtr>
void 
OrderBook<OrderPtr>::set_symbol(const std::string & symbol)
{
    symbol_ = symbol;
}

template <class OrderPtr>
const std::string &
OrderBook<OrderPtr>::symbol() const
{
    return symbol_;
}

template <class OrderPtr>
void
OrderBook<OrderPtr>:: set_market_price(Price price)
{
  Price oldMarketPrice = marketPrice_;
  marketPrice_ = price;
  if(price > oldMarketPrice || oldMarketPrice == MARKET_ORDER_PRICE)
  {
    // price has gone up: check stop bids
    bool buySide = true;
    check_stop_orders(buySide, price, stopBids_);
  }
  else if(price < oldMarketPrice || oldMarketPrice == MARKET_ORDER_PRICE)
  {
    // price has gone down: check stop asks
    bool buySide = false;
    check_stop_orders(buySide, price, stopAsks_);
  }
}

template <class OrderPtr>
bool
OrderBook<OrderPtr>::add_stop_order(Tracker & tracker)
{
  bool isBuy = tracker.ptr()->is_buy();
  ComparablePrice key(isBuy, tracker.ptr()->stop_price());
  ComparablePrice market(isBuy, marketPrice_);
  bool isStopped = market < key;
  if(isStopped)
  {
    if(isBuy)
    {
      stopBids_.emplace(key, std::move(tracker));
    }
    else
    {
      stopAsks_.emplace(key, std::move(tracker));
    }
  }
  return isStopped;
}


template <class OrderPtr>
void
OrderBook<OrderPtr>::check_stop_orders(bool side, Price price, TrackerMap & stops)
{
  ComparablePrice until(side, price);
  auto pos = stops.begin(); 
  while(pos != stops.end())
  {
    auto here = pos++;
    if(until < here->first)
    {
      break;
    }
    pendingOrders_.push_back(std::move(here->second));
    stops.erase(here);
  }
}

template <class OrderPtr>
void
OrderBook<OrderPtr>::submit_pending_orders()
{
  TrackerVec pending;
  pending.swap(pendingOrders_);
  for(auto pos = pending.begin(); pos != pending.end(); ++pos)
  {
    Tracker & tracker = *pos;
    submit_order(tracker);
  }
}

/// @brief Get current market price.
/// The market price is normally the price at which the last trade happened.
template <class OrderPtr>
Price
OrderBook<OrderPtr>::market_price() const
{
  return marketPrice_;
}


template <class OrderPtr>
void
OrderBook<OrderPtr>::set_order_listener(TypedOrderListener* listener)
{
  order_listener_ = listener;
}

template <class OrderPtr>
void
OrderBook<OrderPtr>::set_trade_listener(TypedTradeListener* listener)
{
  trade_listener_ = listener;
}

template <class OrderPtr>
void
OrderBook<OrderPtr>::set_order_book_listener(TypedOrderBookListener* listener)
{
  order_book_listener_ = listener;
}

template <class OrderPtr>
bool
OrderBook<OrderPtr>::add(const OrderPtr& order, OrderConditions conditions)
{
  // Increment transacion ID
  ++trans_id_;  

  bool matched = false;

  // If the order is invalid, exit
  if (!is_valid(order, conditions)) 
  {
    // reject created by is_valid
  } 
  else 
  {
    callbacks_.push_back(TypedCallback::accept(order, trans_id_));
    TypedCallback& accept_cb = callbacks_.back();
    Tracker inbound(order, conditions);
    if(inbound.ptr()->stop_price() != 0 && add_stop_order(inbound))
    {
      // The order has been added to stops
    }
    else
    {
      matched = submit_order(inbound);
      // Note the filled qty in the accept callback
      accept_cb.accept_match_qty = inbound.filled_qty();
      // Cancel any unfilled IOC order
      if (inbound.immediate_or_cancel() && !inbound.filled()) 
      {
        // NOTE - this may need he actual open qty???
        callbacks_.push_back(TypedCallback::cancel(order, 0, trans_id_));
      }
    }
    // If adding this order triggered any stops
    // handle those stops now
    while(!pendingOrders_.empty())
    {
      submit_pending_orders();
    }
    callbacks_.push_back(TypedCallback::book_update(this, trans_id_));
  }
  return matched;
}

template <class OrderPtr>
bool
OrderBook<OrderPtr>::submit_order(Tracker & inbound)
{
  Price order_price = inbound.ptr()->price();//sort_price(inbound.ptr());
  return add_order(inbound, order_price);
}

template <class OrderPtr>
void
OrderBook<OrderPtr>::cancel(const OrderPtr& order)
{
  // Increment transacion ID
  ++trans_id_;  

  bool found = false;
  Quantity open_qty;
  // If the cancel is a buy order
  if (order->is_buy()) {
    typename TrackerMap::iterator bid;
    find_on_market(order, bid);
    if (bid != bids_.end()) {
      open_qty = bid->second.open_qty();
      // Remove from container for cancel
      bids_.erase(bid);
      found = true;
    }
  // Else the cancel is a sell order
  } else {
    typename TrackerMap::iterator ask;
    find_on_market(order, ask);
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
bool
OrderBook<OrderPtr>::replace(
  const OrderPtr& order, 
  int32_t size_delta,
  Price new_price)
{
  // Increment transacion ID
  ++trans_id_;  

  bool matched = false;
  bool price_change = new_price && (new_price != order->price());

  Price price = (new_price == PRICE_UNCHANGED) ? order->price() : new_price;

  // If the order to replace is a buy order
  TrackerMap & market = order->is_buy() ? bids_ : asks_;
  typename TrackerMap::iterator pos;
  if(find_on_market(order, pos))
  {
    // If this is a valid replace
    const Tracker& tracker = pos->second;
    // If there is not enough open quantity for the size reduction
    if (size_delta < 0 && ((int)tracker.open_qty() < -size_delta)) 
    {
      // get rid of as much as we can
      size_delta = -int(tracker.open_qty());
      if(size_delta == 0)
      {
        // if there is nothing to get rid of
        // Reject the replace
        callbacks_.push_back(TypedCallback::replace_reject(tracker.ptr(), 
          "order is already filled", 
          trans_id_));
        return false;
      }
    }

    // Accept the replace
    callbacks_.push_back(
        TypedCallback::replace(order, pos->second.open_qty(), size_delta, 
                                price, trans_id_));
    Quantity new_open_qty = pos->second.open_qty() + size_delta;
    pos->second.change_qty(size_delta);  // Update my copy
    // If the size change will close the order
    if (!new_open_qty) 
    {
      // Cancel with NO open qty (should be zero after replace)
      callbacks_.push_back(TypedCallback::cancel(order, 0, trans_id_));
      market.erase(pos); // Remove order
    } 
    else 
    {
      // Else rematch the new order - there could be a price change
      // or size change - that could cause all or none match
      market.erase(pos); // Remove old order order
      matched = add_order(pos->second, price); // Add order
    }
    // If replace any order this order triggered any trades
    // which triggered any stops
    // handle those stops now
    while(!pendingOrders_.empty())
    {
      submit_pending_orders();
    }
    callbacks_.push_back(TypedCallback::book_update(this, trans_id_));
  }
  else
  {
    // not found
    callbacks_.push_back(
          TypedCallback::replace_reject(order, "not found", trans_id_));
  }
  return matched;
}

template <class OrderPtr>
bool
OrderBook<OrderPtr>::match_order(Tracker& inbound, 
  const Price& inbound_price, 
  TrackerMap& current)
{
  bool matched = false;
  bool is_buy = false;
  Quantity matched_qty = 0;
  Quantity inbound_qty = inbound.open_qty();

  typename TrackerMap::iterator pos = current.begin(); 
  while(pos != current.end()) {
    auto bid = pos++;
    ComparablePrice key = bid->first;
    Tracker & counter_order = bid->second;
    // If the inbound order matches the current order
    if (matches(inbound, 
                inbound_price, 
                inbound.open_qty() - matched_qty, 
                counter_order, 
                key)) {
      // If the inbound order is an all or none order
      if (inbound.all_or_none()) {
        // Track how much of the inbound order has been matched
        matched_qty += counter_order.open_qty();
        // If we have matched enough quantity to fill the inbound order
        if (matched_qty >= inbound_qty) {
          matched =  true;

          // Unwind the deferred crosses
          typename DeferredBidCrosses::iterator dbc;
          for (dbc = deferred_bid_crosses_.begin(); 
               dbc != deferred_bid_crosses_.end(); ++dbc) {
            // Adjust tracking values for cross
            if(cross_orders(inbound, (*dbc)->second))
            {
              // If the existing order was filled, remove it
              if ((*dbc)->second.filled()) {
                current.erase(*dbc);
              }
            }
          }
        // Else we have to defer crossing this order
        } else {
          deferred_bid_crosses_.push_back(bid);
        }
      } else {
        matched =  true;
      }

      if (matched) {
        // Adjust tracking values for cross
        matched = cross_orders(inbound,counter_order);
      }

      if (matched) {
        // If the existing order was filled, remove it
        if (counter_order.filled()) {
          current.erase(bid);
        }

        // if the inbound order is filled, no more matches are possible
        if (inbound.filled()) {
          break;
        }
      }
    // Didn't match, exit loop if this was because of price
    } else if (!key.matches(inbound_price)) {
      break;
    }
  }

  return matched;
}

template <class OrderPtr>
bool
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
  if(MARKET_ORDER_PRICE == cross_price)
  {
    cross_price = marketPrice_;
  }
  if(MARKET_ORDER_PRICE == cross_price)
  {
    // No price available for this order
    return false;    
  }
  inbound_tracker.fill(fill_qty);
  current_tracker.fill(fill_qty);
  set_market_price(cross_price);

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
  return true;
}

template <class OrderPtr>
void
OrderBook<OrderPtr>::move_callbacks(Callbacks& target)
{
  // optimize for common case
  if(target.empty())
  {
    callbacks_.swap(target);
  }
  else
  {
    target.insert(target.end(), callbacks_.begin(), callbacks_.end());
    callbacks_.clear();
  }
}

template <class OrderPtr>
void
OrderBook<OrderPtr>::perform_callbacks()
{
  typename Callbacks::iterator cb;
  for (cb = callbacks_.begin(); cb != callbacks_.end(); ++cb) {
    perform_callback(*cb);
  }
  callbacks_.clear();
}

template <class OrderPtr>
void
OrderBook<OrderPtr>::perform_callback(TypedCallback& cb)
{
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
        order_listener_->on_replace(cb.order, 
                                    cb.repl_size_delta,
                                    cb.repl_new_price);
        break;
      case TypedCallback::cb_order_replace_reject:
        order_listener_->on_replace_reject(cb.order, cb.reject_reason);
        break;
      case TypedCallback::cb_book_update:
      case TypedCallback::cb_unknown:
        // Error
        std::runtime_error("Unexpected callback type for order");
        break;
    }
  // Else if this was an order book update and there is an order book listener
  } else if (cb.type == TypedCallback::cb_book_update && order_book_listener_) {
    order_book_listener_->on_order_book_change(this);
  }
  // If this was a trade and there is a trade listener
  if (cb.type == TypedCallback::cb_order_fill && trade_listener_) {
    Cost fill_cost = cb.fill_price * cb.fill_qty;
    trade_listener_->on_trade(this, cb.fill_qty, fill_cost);
  }
}

template <class OrderPtr>
std::ostream &
OrderBook<OrderPtr>::log(std::ostream & out) const
{
  for(auto ask = asks_.rbegin(); ask != asks_.rend(); ++ask) {
    out << "  Ask " << ask->second.open_qty() << " @ " << ask->first
                          << std::endl;
  }

  for(auto bid = bids_.begin(); bid != bids_.end(); ++bid) {
    out << "  Bid " << bid->second.open_qty() << " @ " << bid->first
                          << std::endl;
  }
  return out;
}

template <class OrderPtr>
bool
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
bool
OrderBook<OrderPtr>::find_on_market(
  const OrderPtr& order,
  typename TrackerMap::iterator& result)
{
  const ComparablePrice key(order->is_buy(), order->price());
  TrackerMap & sideMap = order->is_buy() ? bids_ : asks_;

  for (result = sideMap.find(key); result != sideMap.end(); ++result) {
    // If this is the correct bid
    if (result->second.ptr() == order) 
    {
      return true;
    } 
    else if (key < result->first) 
    {
      // exit early if result is beyond the matching prices
      result = sideMap.end();
      return false;
    }
  }
  return false;
}

template <class OrderPtr>
bool
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
      // Insert into current
      bids_.insert(std::make_pair(ComparablePrice(true, order_price), inbound));
    // Else this is a sell order
    } else {
      // Insert into asks
      asks_.insert(std::make_pair(ComparablePrice(false, order_price), inbound));
    }
  }
  return matched;
}

template <class OrderPtr>
bool
OrderBook<OrderPtr>::matches(
  const Tracker& /*inbound_order*/,
  const Price& inbound_price, 
  const Quantity inbound_open_qty,
  const Tracker& current_order,
  const ComparablePrice& key)
{
  // Check for price mismatch
  if(!key.matches(inbound_price))
  {
      return false;
  }

  // If the inbound order is all or none, we can only check quantity after
  // all matches take place
  if (current_order.all_or_none()) {
    // Don't match current if not completely filled
    if (current_order.open_qty() > inbound_open_qty) {
      return false;
    }
  }
  return true;
}

} }

