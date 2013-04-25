
#include <boost/scoped_ptr.hpp>
#include "depth_feed_subscriber.h"
#include <Codecs/DataSourceBuffer.h>
#include <Codecs/GenericMessageBuilder.h>
#include <Codecs/SingleMessageConsumer.h>


namespace liquibook { namespace examples {

DepthFeedSubscriber::DepthFeedSubscriber(const std::string& template_filename)
: decoder_(parse_templates(template_filename))
{
}

void
DepthFeedSubscriber::handle_message(BufferPtr& bp)
{
  QuickFAST::Codecs::DataSourceBuffer source(bp->c_array(), bp->size());
  QuickFAST::Codecs::SingleMessageConsumer consumer;
  QuickFAST::Codecs::GenericMessageBuilder builder(consumer);
  decoder_.decodeMessage(source, builder);
  QuickFAST::Messages::Message& message(consumer.message());

}


} }
