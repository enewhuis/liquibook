#ifndef example_exchange_h
#define example_exchange_h

#include "order.h"
#include "book/depth_order_book.h"

#include <string>
#include <map>

namespace liquibook { namespace examples {

typedef book::DepthOrderBook<Order*> ExampleOrderBook;
typedef ExampleOrderBook::TypedDepthListener MyDepthListener;

class Exchange {
public:
  Exchange(MyDepthListener* listener);
  void add_order_book(const std::string& symbol);
private:
  typedef std::map<std::string, ExampleOrderBook> OrderBookMap;
  OrderBookMap order_books_;
  MyDepthListener* listener_;
};

} }

#endif
