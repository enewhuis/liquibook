#ifndef example_depth_feed_publisher_h
#define example_depth_feed_publisher_h

#include <stdexcept>
#include <boost/cstdint.hpp>
#include <boost/operators.hpp>
#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>
#include <cstring>
#include <sstream>
#include <vector>

#include <Codecs/Encoder.h>
#include <Codecs/TemplateRegistry_fwd.h>
#include "example_order_book.h"
#include "book/depth_listener.h"
#include "depth_feed_connection.h"
#include "template_consumer.h"

namespace liquibook { namespace examples {

class DepthFeedPublisher : public ExampleOrderBook::TypedDepthListener,
                           public TemplateConsumer {
public:
  DepthFeedPublisher(const std::string& template_filename);
  void set_message_handler(DepthFeedConnection* connection);

  virtual void on_depth_change(
      const book::DepthOrderBook<OrderPtr>* order_book,
      const book::DepthOrderBook<OrderPtr>::DepthTracker* tracker);
private:
  uint32_t sequence_num_;
  QuickFAST::Codecs::Encoder encoder_;

  const QuickFAST::template_id_t tid_depth_message_;

  DepthFeedConnection* connection_;

  // Build an incremental depth message
  void build_depth_message(
      QuickFAST::Codecs::DataDestination& dest,
      const std::string& symbol,
      const book::DepthOrderBook<OrderPtr>::DepthTracker* tracker);
  // Build a full depth message
  void build_full_depth_message(
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
