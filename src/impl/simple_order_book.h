// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#ifndef simple_order_book_h
#define simple_order_book_h

#include "simple_order.h"
#include "book/depth_order_book.h"
#include <iostream>

namespace liquibook { namespace impl {

// @brief binding of DepthOrderBook template with SimpleOrder* order pointer.
template <int SIZE = 5>
class SimpleOrderBook : public book::DepthOrderBook<SimpleOrder*, SIZE> {
};

} }

#endif
