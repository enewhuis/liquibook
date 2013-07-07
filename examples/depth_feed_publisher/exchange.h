#ifndef example_exchange_h
#define example_exchange_h

#include "order.h"
#include "example_order_book.h"

#include <string>
#include <map>
#include <boost/shared_ptr.hpp>

namespace liquibook { namespace examples {

class Exchange {
public:
  Exchange(ExampleOrderBook::TypedDepthListener* depth_listener,
           ExampleOrderBook::TypedTradeListener* trade_listener);

  // Permanently add an order book to the exchange
  void add_order_book(const std::string& symbol);

  // Handle an incoming order
  void add_order(const std::string& symbol, OrderPtr& order);
private:
  typedef std::map<std::string, ExampleOrderBook> OrderBookMap;
  OrderBookMap order_books_;
  ExampleOrderBook::TypedDepthListener* depth_listener_;
  ExampleOrderBook::TypedTradeListener* trade_listener_;
};

} }

#endif
