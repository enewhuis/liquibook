#ifndef example_order_book_h
#define example_order_book_h

#include "order.h"
#include "book/depth_order_book.h"
#include <boost/shared_ptr.hpp>

namespace liquibook { namespace examples {

typedef boost::shared_ptr<Order> OrderPtr;

class ExampleOrderBook : public book::DepthOrderBook<OrderPtr> {
public:
  ExampleOrderBook(const std::string& symbol);
  const std::string& symbol() const;

private:
  std::string symbol_;
};

} } // End namespace

#endif
