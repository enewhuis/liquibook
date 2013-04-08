#include "exchange.h"

namespace liquibook { namespace examples {

Exchange::Exchange(MyDepthListener* listener)
: listener_(listener)
{
}

void
Exchange::add_order_book(const std::string& symbol)
{
  std::pair<OrderBookMap::iterator, bool> result;
  result = order_books_.insert(std::make_pair(symbol, ExampleOrderBook()));
  result.first->second.set_depth_listener(listener_);
}


} } // End namespace
