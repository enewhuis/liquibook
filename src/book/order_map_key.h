// Copyright (c) 2012 - 2017 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include "types.h"

namespace liquibook { namespace book {

class OrderMapKey
{
  Price price_;
  bool buySide_;

public:
  OrderMapKey(bool buySide, Price price)
    : price_(price != MARKET_ORDER_PRICE ? price : buySide ? MARKET_ORDER_BID_SORT_PRICE : MARKET_ORDER_ASK_SORT_PRICE)
    , buySide_(buySide)
  {
  }

  bool operator <(const OrderMapKey & rhs) const
  {
    return *this < rhs.price_;
  }

  bool operator <(Price rhs) const
  {
    // Sorted in order of most likely to trade
    // Buying: Highest prices first. 
    // Selling: lowest prices first
    return buySide_ ? (price_ < rhs) : (rhs < price_);
  }


  bool isMarket() const
  {
    return price_ == MARKET_ORDER_BID_SORT_PRICE || price_ == MARKET_ORDER_ASK_SORT_PRICE;
  }
};

}}