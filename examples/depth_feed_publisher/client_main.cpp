
#include "depth_feed_subscriber.h"

int main(int argc, const char* argv[])
{
  // Create feed subscriber
  liquibook::examples::DepthFeedSubscriber feed("./templates/Depth.xml");

  return 0;
}
