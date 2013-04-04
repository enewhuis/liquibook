#include "exchange.h"

namespace liquibook { namespace examples {

Exchange::Exchange()
{
}

void
Exchange::add_order_book(const std::string& symbol)
{
  order_books_.insert(std::make_pair(symbol, ExampleOrderBook()));
}


} } // End namespace
