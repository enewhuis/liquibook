#include "example_order_book.h"

namespace liquibook { namespace examples {

ExampleOrderBook::ExampleOrderBook(const std::string& symbol)
: symbol_(symbol)
{
}

const std::string&
ExampleOrderBook::symbol() const
{
  return symbol_;
}

} } // End namespace

