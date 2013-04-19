
#include "depth_feed_subscriber.h"


namespace liquibook { namespace examples {

DepthFeedSubscriber::DepthFeedSubscriber(const std::string& template_filename)
: decoder_(parse_templates(template_filename))
{
}

} }
