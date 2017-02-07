// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include "types.h"

namespace liquibook { namespace book {
  const Price INVALID_LEVEL_PRICE(0);
  const Price MARKET_ORDER_PRICE(0);
  const Price MARKET_ORDER_BID_SORT_PRICE(UINT32_MAX);
  const Price MARKET_ORDER_ASK_SORT_PRICE(0);
  const Price PRICE_UNCHANGED(0);

  const int32_t SIZE_UNCHANGED(0);
} }
