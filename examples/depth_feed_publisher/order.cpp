#include "order.h"

namespace liquibook { namespace examples {

const uint8_t Order::precision_(100);

Order::Order(bool buy, const double& price, book::Quantity qty)
: is_buy_(buy),
  price_(price),
  qty_(qty)
{
}

bool
Order::is_buy() const
{
  return is_buy_;
}

book::Price
Order::price() const
{
  return price_ / precision_;
}

book::Quantity
Order::order_qty() const
{
  return qty_;
}

} } // End namespace

