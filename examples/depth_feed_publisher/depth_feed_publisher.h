#ifndef example_depth_feed_publisher_h
#define example_depth_feed_publisher_h

#include "example_order_book.h"
#include "book/depth_listener.h"
#include <boost/cstdint.hpp>
#include <boost/operators.hpp>
#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>
#include <cstring>
#include <sstream>

#include <Messages/FieldIdentity.h>
#include <Codecs/Encoder.h>
#include <Codecs/TemplateRegistry_fwd.h>

namespace liquibook { namespace examples {

class DepthFeedPublisher : public ExampleOrderBook::TypedDepthListener {
public:
  DepthFeedPublisher(const std::string& template_filename);

  virtual void on_depth_change(
      const book::DepthOrderBook<OrderPtr>* order_book,
      const book::DepthOrderBook<OrderPtr>::DepthTracker* tracker);
private:
  uint32_t sequence_num_;
  QuickFAST::Codecs::Encoder encoder_;
  // Field identities
  QuickFAST::Messages::FieldIdentityCPtr id_seq_num_;
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

  const QuickFAST::template_id_t tid_depth_message_;

  // Build a depth message
  void build_depth_message(
      QuickFAST::Codecs::DataDestination& dest,
      const std::string& symbol,
      const book::DepthOrderBook<OrderPtr>::DepthTracker* tracker);
  void build_depth_level(
      QuickFAST::Messages::SequencePtr& level_seq,
      const book::DepthLevel* level,
      int level_index);
  uint32_t time_stamp();

  static QuickFAST::Codecs::TemplateRegistryPtr 
             parse_templates(const std::string& template_fileanme);
};

} } // End namespace
#endif
