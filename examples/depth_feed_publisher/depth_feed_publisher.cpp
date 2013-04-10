#include <iomanip>
#include <fstream>
#include "depth_feed_publisher.h"
#include <Codecs/DataDestination.h>
#include <Codecs/XMLTemplateParser.h>
#include <Messages/FieldIdentity.h>
#include <Messages/FieldSet.h>
#include <Messages/FieldString.h>
#include <Messages/FieldUInt32.h>

namespace liquibook { namespace examples { 

using namespace QuickFAST::Messages;

DepthFeedPublisher::DepthFeedPublisher(const std::string& template_filename)
: sequence_num_(0),
  encoder_(parse_templates(template_filename)),
  id_seq_num_(new FieldIdentity("SequenceNumber")),
  id_timestamp_(new FieldIdentity("Timestamp")),
  id_symbol_(new FieldIdentity("Symbol")),
  id_size_(new FieldIdentity("Size")),
  id_price_(new FieldIdentity("Price"))
{
}

void
DepthFeedPublisher::on_depth_change(
    const book::DepthOrderBook<OrderPtr>* order_book,
    const book::DepthOrderBook<OrderPtr>::DepthTracker* tracker)
{
  // Published changed levels of order book
  std::cout << "Depth changed" << std::endl;
  QuickFAST::Codecs::DataDestination message;
  const ExampleOrderBook* exob = 
          dynamic_cast<const ExampleOrderBook*>(order_book);
  build_depth_message(message, exob->symbol());
}
 
void
DepthFeedPublisher::build_depth_message(
    QuickFAST::Codecs::DataDestination& dest,
    const std::string& symbol)
{
  QuickFAST::Messages::FieldSet message(20); // allocate space for 20 fields
  message.addField(id_seq_num_, FieldUInt32::create(++sequence_num_));
  message.addField(id_timestamp_, FieldUInt32::create(time_stamp()));
  message.addField(id_symbol_, FieldString::create(symbol));
/*
  message.addField(orderIdId, FieldUInt32::create(202));
  message.addField(sizeId, FieldUInt16::create(100));
  message.addField(priceId, FieldUInt32::create(2000));
*/
  encoder_.encodeMessage(dest, 1, message);
}

uint32_t
DepthFeedPublisher::time_stamp()
{
  time_t now;
  time(&now);
  return now;
}

QuickFAST::Codecs::TemplateRegistryPtr
DepthFeedPublisher::parse_templates(const std::string& template_filename)
{
  std::ifstream template_stream("./templates/Depth.xml");
  QuickFAST::Codecs::XMLTemplateParser parser;
  return parser.parse(template_stream);
}

} } // End namespace
