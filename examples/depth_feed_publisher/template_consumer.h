
#include <boost/shared_ptr.hpp>
#include <Codecs/TemplateRegistry_fwd.h>
#include <Messages/FieldIdentity.h>

namespace liquibook { namespace examples {

class TemplateConsumer {
public:
  static QuickFAST::Codecs::TemplateRegistryPtr 
             parse_templates(const std::string& template_filename);
protected:
  TemplateConsumer();

  // Field identities
  QuickFAST::Messages::FieldIdentityCPtr id_seq_num_;
  QuickFAST::Messages::FieldIdentityCPtr id_msg_type_;
  QuickFAST::Messages::FieldIdentityCPtr id_timestamp_;
  QuickFAST::Messages::FieldIdentityCPtr id_symbol_;

  QuickFAST::Messages::FieldIdentityCPtr id_bids_length_;
  QuickFAST::Messages::FieldIdentityCPtr id_bids_;
  QuickFAST::Messages::FieldIdentityCPtr id_asks_length_;
  QuickFAST::Messages::FieldIdentityCPtr id_asks_;

  QuickFAST::Messages::FieldIdentityCPtr id_level_num_;
  QuickFAST::Messages::FieldIdentityCPtr id_order_count_;
  QuickFAST::Messages::FieldIdentityCPtr id_price_;
  QuickFAST::Messages::FieldIdentityCPtr id_size_;

  // Trade field identities
  QuickFAST::Messages::FieldIdentityCPtr id_qty_;
  QuickFAST::Messages::FieldIdentityCPtr id_cost_;

};

} }

