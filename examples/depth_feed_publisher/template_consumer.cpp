
#include <boost/cstdint.hpp>
#include "template_consumer.h"
#include <fstream>
#include <Codecs/XMLTemplateParser.h>

namespace liquibook { namespace examples {

using namespace QuickFAST::Messages;


TemplateConsumer::TemplateConsumer()
: id_seq_num_(new FieldIdentity("SequenceNumber")),
  id_msg_type_(new FieldIdentity("MessageType")),
  id_timestamp_(new FieldIdentity("Timestamp")),
  id_symbol_(new FieldIdentity("Symbol")),
  id_bids_(new FieldIdentity("Bids")),
  id_bids_length_(new FieldIdentity("BidsLength")),
  id_asks_(new FieldIdentity("Asks")),
  id_asks_length_(new FieldIdentity("AsksLength")),
  id_level_num_(new FieldIdentity("LevelNum")),
  id_order_count_(new FieldIdentity("OrderCount")),
  id_size_(new FieldIdentity("AggregateQty")),
  id_price_(new FieldIdentity("Price")),
  id_qty_(new FieldIdentity("Quantity")),
  id_cost_(new FieldIdentity("Cost"))
{
}

QuickFAST::Codecs::TemplateRegistryPtr 
TemplateConsumer::parse_templates(const std::string& template_fileanme)
{
  std::ifstream template_stream("./templates/Depth.xml");
  QuickFAST::Codecs::XMLTemplateParser parser;
  return parser.parse(template_stream);
}

} } 
