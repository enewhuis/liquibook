// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include "order.h"

namespace liquibook { namespace book {
bool
Order::is_limit() const 
{
  return (price() > 0);
}

Price
Order::stop_price() const
{
  // default to not a stop order
  return 0;
}

bool
Order::all_or_none() const
{
  // default to normal
  return false;
}

bool
Order::immediate_or_cancel() const
{
  // default to normal
  return false;
}

OrderConditions
Order::conditions() const
{
  return OrderCondition::oc_no_conditions;
}

} }
