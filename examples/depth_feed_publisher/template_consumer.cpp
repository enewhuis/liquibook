
#include <boost/cstdint.hpp>
#include "template_consumer.h"
#include <fstream>
#include <Codecs/XMLTemplateParser.h>

namespace liquibook { namespace examples {

using namespace QuickFAST::Messages;

const FieldIdentity TemplateConsumer::id_seq_num_("SequenceNumber");

const FieldIdentity TemplateConsumer::id_msg_type_("MessageType");

const FieldIdentity TemplateConsumer::id_timestamp_("Timestamp");

const FieldIdentity TemplateConsumer::id_symbol_("Symbol");

const FieldIdentity TemplateConsumer::id_bids_("Bids");

const FieldIdentity TemplateConsumer::id_bids_length_("BidsLength");

const FieldIdentity TemplateConsumer::id_asks_("Asks");

const FieldIdentity TemplateConsumer::id_asks_length_("AsksLength");

const FieldIdentity TemplateConsumer::id_level_num_("LevelNum");

const FieldIdentity TemplateConsumer::id_order_count_("OrderCount");

const FieldIdentity TemplateConsumer::id_size_("AggregateQty");

const FieldIdentity TemplateConsumer::id_price_("Price");

const FieldIdentity TemplateConsumer::id_qty_("Quantity");

const FieldIdentity TemplateConsumer::id_cost_("Cost");

QuickFAST::Codecs::TemplateRegistryPtr 
TemplateConsumer::parse_templates(const std::string& template_filename)
{
  std::ifstream template_stream(template_filename.c_str());
  QuickFAST::Codecs::XMLTemplateParser parser;
  return parser.parse(template_stream);
}

} } 
