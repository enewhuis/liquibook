
#include "depth_feed_connection.h"
#include "depth_feed_subscriber.h"

int main(int argc, const char* argv[])
{
  // Create feed subscriber
  liquibook::examples::DepthFeedSubscriber feed("./templates/Depth.xml");

  // Connect to server
  liquibook::examples::DepthFeedConnection connection;
  connection.set_message_handler(&feed);
  connection.connect(argc, argv);
  
  return 0;
}
