
#include <boost/shared_ptr.hpp>
#include <Codecs/TemplateRegistry_fwd.h>
#include <Messages/FieldIdentity.h>

namespace liquibook { namespace examples {

class TemplateConsumer {
public:
  static QuickFAST::Codecs::TemplateRegistryPtr 
             parse_templates(const std::string& template_filename);

  TemplateConsumer();

  // Field identities
  static const QuickFAST::Messages::FieldIdentityCPtr id_seq_num_;
  static const QuickFAST::Messages::FieldIdentityCPtr id_msg_type_;
  static const QuickFAST::Messages::FieldIdentityCPtr id_timestamp_;
  static const QuickFAST::Messages::FieldIdentityCPtr id_symbol_;

  static const QuickFAST::Messages::FieldIdentityCPtr id_bids_length_;
  static const QuickFAST::Messages::FieldIdentityCPtr id_bids_;
  static const QuickFAST::Messages::FieldIdentityCPtr id_asks_length_;
  static const QuickFAST::Messages::FieldIdentityCPtr id_asks_;

  static const QuickFAST::Messages::FieldIdentityCPtr id_level_num_;
  static const QuickFAST::Messages::FieldIdentityCPtr id_order_count_;
  static const QuickFAST::Messages::FieldIdentityCPtr id_price_;
  static const QuickFAST::Messages::FieldIdentityCPtr id_size_;

  // Trade field identities
  static const QuickFAST::Messages::FieldIdentityCPtr id_qty_;
  static const QuickFAST::Messages::FieldIdentityCPtr id_cost_;
};

} }

