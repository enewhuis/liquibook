#ifndef example_exchange_h
#define example_exchange_h

#include "order.h"
#include "book/depth_order_book.h"
#include "order.h"

#include <string>
#include <map>
#include <boost/shared_ptr.hpp>

namespace liquibook { namespace examples {

typedef boost::shared_ptr<Order> OrderPtr;
typedef book::DepthOrderBook<OrderPtr> ExampleOrderBook;
typedef ExampleOrderBook::TypedDepthListener MyDepthListener;

class Exchange {
public:
  Exchange(MyDepthListener* listener);
  void add_order_book(const std::string& symbol);
  void add_order(const std::string& symbol, OrderPtr& order);
private:
  typedef std::map<std::string, ExampleOrderBook> OrderBookMap;
  OrderBookMap order_books_;
  MyDepthListener* listener_;
};

} }

#endif
