#ifndef example_depth_feed_publisher_h
#define example_depth_feed_publisher_h

#include "exchange.h"

namespace liquibook { namespace examples {

typedef ExampleOrderBook::DepthTracker Tracker;

class DepthFeedPublisher : public book::DepthListener<ExampleOrderBook> {
public:
  DepthFeedPublisher();

  virtual void on_depth_change(const ExampleOrderBook* order_book,
                               const Tracker* tracker);
};

} } // End namespace
#endif
