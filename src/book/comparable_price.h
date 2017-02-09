// Copyright (c) 2017 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include "types.h"
#include <iostream>

namespace liquibook { namespace book {

/// @brief A price that knows which side of the market it is on
/// Designed to be compared to other prices on the same side using
/// standard comparison operators (< > <= >= == !=) and to
/// compared to prices on the other side using the match() method.
///
/// Using the  '<' operation to sort a set of prices will result in the
/// prices being partially ordered from most liquid to least liquid.
/// i.e:
///   Market prices always sort first since they will match any counter price.
///   Sell side low prices sort before high prices because they match more buys.
///   Buy side high prices sort before low prices because they match more asks.
///   
class ComparablePrice
{
  Price price_;
  bool buySide_;

public:
  /// @brief construct given side and price
  /// @param buySide controls whether price comparison is normal or reversed
  /// @param price is the price for this key, or 0 (MARKET_ORDER_PRICE) for market
  ComparablePrice(bool buySide, Price price)
    : price_(price)
    , buySide_(buySide)
  {
  }

  /// @brief Check possible trade
  /// Assumes rhs is on the opposite side
  bool matches(Price rhs) const
  {
    if(price_ == rhs)
    {
      return true;
    }
    if(buySide_)
    {
      return rhs < price_  || price_ == MARKET_ORDER_PRICE ;
    }
    return price_ < rhs || rhs == MARKET_ORDER_PRICE;
  }

  /// @brief less than compare key to a price
  /// Assumes both prices are on the same side.
  /// Uses side to determine the sense of the comparison.
  bool operator <(Price rhs) const
  {
    // Compare difficulty finding a match.  (easy is less than hard)
    if(price_ == MARKET_ORDER_PRICE)
    {
      return rhs != MARKET_ORDER_PRICE;
    }
    else if(rhs == MARKET_ORDER_PRICE)
    {
      return false;
    }
    else if(buySide_)
    {
      // Buying: Highest prices first. 
      return rhs < price_ ;
    }
    else
    {
      // Selling: lowest prices first
      return price_ < rhs;
    }
  }

  /// @brief equality compare key to a price
  bool operator ==(Price rhs) const
  {
    // compares equal without regard to side
    return price_ == rhs;
  }

  /// @brief inequality compare key to a price
  bool operator !=(Price rhs) const
  {
    return ! price_ == rhs;
  }

  /// @brief greater than compare key to a price
  /// Assumes both prices are on the same side.
  bool operator > (Price rhs) const
  {
    return price_!= MARKET_ORDER_PRICE && ((rhs == MARKET_ORDER_PRICE) || (buySide_ ? (rhs > price_) : (price_ > rhs)));
  }

  /// @brief less than or equal to compare key to a price
  /// Assumes both prices are on the same side.
  bool operator <=(Price rhs) const
  {
    return *this < rhs || *this == rhs;
  }

  /// @brief greater than or equal to compare key to a price
  /// Assumes both prices are on the same side.
  bool operator >=(Price rhs) const
  {
    return *this > rhs || *this == rhs;
  }

  /// @brief less than compare order map keys
  /// compares the prices assuming they are on the same side
  bool operator <(const ComparablePrice & rhs) const
  {
    return *this < rhs.price_;
  }

  /// @brief equality compare order map keys
  bool operator ==(const ComparablePrice & rhs) const
  {
    return *this == rhs.price_;
  }

  /// @brief inequality compare order map keys
  bool operator !=(const ComparablePrice & rhs) const
  {
    return *this != rhs.price_;
  }

  /// @brief greater than compare order map keys
  /// Assumes both prices are on the same side.
  bool operator >(const ComparablePrice & rhs) const
  {
    return *this > rhs.price_;
  }

  /// @brief access price.
  Price price() const
  {
    return price_;
  }

  /// @brief access side.
  bool isBuy() const
  {
    return buySide_;
  }

  /// @brief check to see if this is market price
  bool isMarket() const
  {
    return price_ == MARKET_ORDER_PRICE;
  }
};

/// @brief less than compare price to key
inline
bool operator < (Price price, const ComparablePrice & key)
{
  return key > price;
}

/// @brief greater than compare price to key
inline
bool operator > (Price price, const ComparablePrice & key)
{
  return key < price;
}

/// @brief equality compare price to key
inline
bool operator == (Price price, const ComparablePrice & key)
{
  return key == price;
}

/// @brief inequality compare price to key
inline
bool operator != (Price price, const ComparablePrice & key)
{
  return key != price;
}

/// @brief less than or equal to compare price to key
inline
bool operator <= (Price price, const ComparablePrice & key)
{
  return key >= price;
}

/// @brief greater or equal to than compare price to key
inline
bool operator >= (Price price, const ComparablePrice & key)
{
  return key <= price;
}

inline
std::ostream & operator << (std::ostream & out, const ComparablePrice & key)
{
  out << (key.isBuy() ? "Buy at " : "Sell at ");
  if(key.isMarket())
  {
    out << "Market";
  }
  else
  {
    out << key.price();
  }
  return out;
}

}}