
#include <boost/bind.hpp>
#include "depth_feed_connection.h"
#include "depth_feed_subscriber.h"

int main(int argc, const char* argv[])
{
  try
  {
    // Create the connection
    liquibook::examples::DepthFeedConnection connection(argc, argv);

    // Create feed subscriber
    liquibook::examples::DepthFeedSubscriber feed(connection.get_templates());

    // Set up handlers
    liquibook::examples::MessageHandler msg_handler =
        boost::bind(&liquibook::examples::DepthFeedSubscriber::handle_message,
                    &feed, _1, _2);
    liquibook::examples::ResetHandler reset_handler =
        boost::bind(&liquibook::examples::DepthFeedSubscriber::handle_reset,
                    &feed);
    connection.set_message_handler(msg_handler);
    connection.set_reset_handler(reset_handler);

    // Connect to server
    connection.connect();
    connection.run();
  }
  catch (const std::exception & ex)
  {
    std::cerr << "Exception caught at main level: " << ex.what() << std::endl;
    return -1;
  }
  return 0;
}
