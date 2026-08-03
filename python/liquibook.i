// Copyright (c) Mike Kipnis
%module(directors="1") liquibook

%feature("director:oldvarinit") "";

#define DEPTH 5

%{
#define SWIG_FILE_WITH_INIT

#include <memory>
#include <map>
#include <vector>

#include <book/types.h>
#include <book/depth_constants.h>
#include <book/order_tracker.h>
#include <book/comparable_price.h>
#include <book/order.h>
#include <book/logger.h>
#include <book/depth.h>
#include <book/depth_level.h>
#include <book/order_book_listener.h>
#include <book/order_book.h>
#include <book/bbo_listener.h>
#include <book/depth_listener.h>
#include <book/depth_order_book.h>

#include <simple/simple_order.h>
#include <simple/simple_order.cpp>
#include <simple/simple_order_book.h>

using namespace liquibook;
using namespace liquibook::book;
using namespace liquibook::simple;

%}


%feature("director") liquibook::book::OrderBook;
%feature("director") liquibook::book::DepthOrderBook;
%feature("director") liquibook::book::OrderListener;
%feature("director") liquibook::book::OrderBookListener;
%feature("director") liquibook::book::BboListener;
%feature("director") liquibook::book::DepthListener;
%feature("director") liquibook::simple::SimpleOrder;
%feature("director") liquibook::simple::SimpleOrderBook;

%include std_string.i
%include std_shared_ptr.i
%include std_multimap.i
%include std_vector.i
%include std_map.i
%include stdint.i

%shared_ptr(liquibook::simple::SimpleOrder)

%include "../src/book/types.h"
%include "../src/book/depth_constants.h"
%include "../src/book/comparable_price.h"
%include "../src/book/order.h"
%include "../src/book/order_listener.h"
%include "../src/book/order_book_listener.h"
%include "../src/book/order_book.h"
%include "../src/book/bbo_listener.h"
%include "../src/book/depth_order_book.h"
%include "../src/book/depth_level.h"
%include "../src/book/depth_listener.h"
%include "../src/book/comparable_price.h"
%include "../src/book/order_tracker.h"
%include "../src/book/logger.h"
%include "../src/book/depth.h"
%include "../src/simple/simple_order.h"
%include "../src/simple/simple_order_book.h"

%template(liquibook_book_OrderBook) liquibook::book::OrderBook<std::shared_ptr<liquibook::simple::SimpleOrder>>;
%template(liquibook_book_OrderBookListener) liquibook::book::OrderBookListener< liquibook::book::DepthOrderBook<std::shared_ptr<liquibook::simple::SimpleOrder>,DEPTH>  >;

%template(liquibook_book_Depth) liquibook::book::Depth<DEPTH>;

%template(depth_level_vector) std::vector<liquibook::book::DepthLevel*>;


%extend liquibook::book::Depth<DEPTH> {
  
  std::vector<liquibook::book::DepthLevel*> get_bid_levels() 
  {
    std::vector<liquibook::book::DepthLevel*> bids;

    auto top_level = self->bids();
    
    for ( auto bid = 0; bid<DEPTH; ++bid )
	bids.push_back(top_level++);
   
    return bids;  
  }

  std::vector<liquibook::book::DepthLevel*> get_ask_levels()
  {
    std::vector<liquibook::book::DepthLevel*> asks;

    auto top_level = self->asks();

    for ( auto ask = 0; ask<DEPTH; ++ask )
        asks.push_back(top_level++);

    return asks;
  }

}

%template(DepthOrderBook) liquibook::book::DepthOrderBook<std::shared_ptr<liquibook::simple::SimpleOrder>,DEPTH>;
%template(OrderListener) liquibook::book::OrderListener<std::shared_ptr<liquibook::simple::SimpleOrder>>;
%template(DepthListener) liquibook::book::DepthListener<liquibook::book::DepthOrderBook<std::shared_ptr<liquibook::simple::SimpleOrder>,DEPTH>>;
%template(OrderBookListener) liquibook::book::OrderBookListener< liquibook::simple::SimpleOrderBook<DEPTH> >;
%template(OrderTracker) liquibook::book::OrderTracker<std::shared_ptr<liquibook::simple::SimpleOrder>>;
%template(OrderTrackerMap) std::multimap<liquibook::book::ComparablePrice, liquibook::book::OrderTracker<std::shared_ptr<liquibook::simple::SimpleOrder>>>;
%template(DepthOrderBookBboListener) liquibook::book::BboListener<liquibook::book::DepthOrderBook<std::shared_ptr<liquibook::simple::SimpleOrder>,DEPTH>>;

%template(SimpleOrderBook) liquibook::simple::SimpleOrderBook<DEPTH>;
