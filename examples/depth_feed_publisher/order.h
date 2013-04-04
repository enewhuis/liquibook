#ifndef example_order_h
#define example_order_h

#include "book/order.h"

namespace liquibook { namespace examples {

class Order : public book::Order {
public:
  Order(bool buy,
        const double& price,          
        book::Quantity qty);

  virtual bool is_buy() const;
  virtual book::Price price() const;
  virtual book::Quantity order_qty() const;
private:
  bool is_buy_;
  double price_;
  book::Quantity qty_;

  static const uint8_t precision_;
};

} }

#endif
