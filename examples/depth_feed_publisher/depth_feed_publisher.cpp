
#include "depth_feed_publisher.h"

namespace liquibook { namespace examples { 

DepthFeedPublisher::DepthFeedPublisher()
{
}

void
DepthFeedPublisher::on_depth_change(const ExampleOrderBook* order_book,
                                    const Tracker* tracker)
{
  // Published changed levels of order book
}

} } // End namespace
