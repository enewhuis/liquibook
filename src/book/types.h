// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <cstdlib>
#include <cstdint>
#include <stdexcept>

namespace liquibook { namespace book {
  // Types used in Liquibook
  typedef uint32_t Price;
  typedef uint32_t Quantity;
  typedef uint32_t Cost;
  typedef uint32_t FillId;
  typedef uint32_t ChangeId;
  typedef uint32_t OrderConditions;

  enum OrderCondition {
    oc_no_conditions = 0,
    oc_all_or_none = 1,
    oc_immediate_or_cancel = oc_all_or_none << 1,
    oc_fill_or_kill = oc_all_or_none | oc_immediate_or_cancel,
    oc_stop = oc_immediate_or_cancel << 1
  };

  namespace {
  // Constants used in liquibook API
  const Price MARKET_ORDER_PRICE(0);
  const Price PRICE_UNCHANGED(0);
  const int32_t SIZE_UNCHANGED(0);
  }

} } // namespace
