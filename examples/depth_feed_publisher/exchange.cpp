#include "exchange.h"

namespace liquibook { namespace examples {

Exchange::Exchange(ExampleOrderBook::TypedDepthListener* depth_listener,
                   ExampleOrderBook::TypedTradeListener* trade_listener)
: depth_listener_(depth_listener),
  trade_listener_(trade_listener)
{
}

void
Exchange::add_order_book(const std::string& sym)
{
  std::pair<OrderBookMap::iterator, bool> result;
  result = order_books_.insert(std::make_pair(sym, ExampleOrderBook(sym)));
  result.first->second.set_depth_listener(depth_listener_);
  result.first->second.set_trade_listener(trade_listener_);
}

void
Exchange::add_order(const std::string& symbol, OrderPtr& order)
{
  OrderBookMap::iterator order_book = order_books_.find(symbol);
  if (order_book != order_books_.end()) {
    order_book->second.add(order);
    order_book->second.perform_callbacks();
  }
}

} } // End namespace
