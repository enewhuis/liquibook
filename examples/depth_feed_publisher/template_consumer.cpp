
#include <boost/cstdint.hpp>
#include "template_consumer.h"
#include <fstream>
#include <Codecs/XMLTemplateParser.h>

namespace liquibook { namespace examples {

using namespace QuickFAST::Messages;

const QuickFAST::Messages::FieldIdentityCPtr
TemplateConsumer::id_seq_num_(new FieldIdentity("SequenceNumber"));

const QuickFAST::Messages::FieldIdentityCPtr
TemplateConsumer::id_msg_type_(new FieldIdentity("MessageType"));

const QuickFAST::Messages::FieldIdentityCPtr
TemplateConsumer::id_timestamp_(new FieldIdentity("Timestamp"));

const QuickFAST::Messages::FieldIdentityCPtr
TemplateConsumer::id_symbol_(new FieldIdentity("Symbol"));

const QuickFAST::Messages::FieldIdentityCPtr
TemplateConsumer::id_bids_(new FieldIdentity("Bids"));

const QuickFAST::Messages::FieldIdentityCPtr
TemplateConsumer::id_bids_length_(new FieldIdentity("BidsLength"));

const QuickFAST::Messages::FieldIdentityCPtr
TemplateConsumer::id_asks_(new FieldIdentity("Asks"));

const QuickFAST::Messages::FieldIdentityCPtr
TemplateConsumer::id_asks_length_(new FieldIdentity("AsksLength"));

const QuickFAST::Messages::FieldIdentityCPtr
TemplateConsumer::id_level_num_(new FieldIdentity("LevelNum"));

const QuickFAST::Messages::FieldIdentityCPtr
TemplateConsumer::id_order_count_(new FieldIdentity("OrderCount"));

const QuickFAST::Messages::FieldIdentityCPtr
TemplateConsumer::id_size_(new FieldIdentity("AggregateQty"));

const QuickFAST::Messages::FieldIdentityCPtr
TemplateConsumer::id_price_(new FieldIdentity("Price"));

const QuickFAST::Messages::FieldIdentityCPtr
TemplateConsumer::id_qty_(new FieldIdentity("Quantity"));

const QuickFAST::Messages::FieldIdentityCPtr
TemplateConsumer::id_cost_(new FieldIdentity("Cost"));

TemplateConsumer::TemplateConsumer()
{
}

QuickFAST::Codecs::TemplateRegistryPtr 
TemplateConsumer::parse_templates(const std::string& template_filename)
{
  std::ifstream template_stream(template_filename.c_str());
  QuickFAST::Codecs::XMLTemplateParser parser;
  return parser.parse(template_stream);
}

} } 
