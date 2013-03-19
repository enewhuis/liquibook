// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include "order.h"

namespace liquibook { namespace book {
bool
Order::is_limit() const {
  return (price() > 0);
}

} }
