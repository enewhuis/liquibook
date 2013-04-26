
#include <boost/bind.hpp>
#include "depth_feed_connection.h"
#include "depth_feed_subscriber.h"

int main(int argc, const char* argv[])
{
  // Create feed subscriber
  liquibook::examples::DepthFeedSubscriber feed("./templates/Depth.xml");

  // Connect to server
  liquibook::examples::DepthFeedConnection connection;
  liquibook::examples::MessageHandler msg_handler =
      boost::bind(&liquibook::examples::DepthFeedSubscriber::handle_message,
                  &feed, _1);
  connection.set_message_handler(msg_handler);
  connection.connect(argc, argv);

  return 0;
}
