#include <boost/cstdint.hpp>
#include <boost/operators.hpp>
#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>
#include <stdexcept>
#include <cstring>
#include <sstream>
#include <vector>
#include <Application/QuickFAST.h>
#include <Codecs/Decoder.h>

#include "template_consumer.h"
#include "depth_feed_connection.h"
#include "book/depth.h"

namespace QuickFAST { namespace Messages {
  class Message;
} }

namespace liquibook { namespace examples {

  class DepthFeedSubscriber : public TemplateConsumer {
  public:
    DepthFeedSubscriber(
        const QuickFAST::Codecs::TemplateRegistryPtr& templates);

    // Handle a reset of the connection
    void handle_reset();

    // Handle a message
    // return false if failure
    bool handle_message(BufferPtr& bp, size_t bytes_transferred);

  private:
    QuickFAST::Codecs::Decoder decoder_;
    typedef std::map<std::string, book::Depth<5> > DepthMap;
    DepthMap depth_map_;
    uint64_t expected_seq_;

    static const uint64_t MSG_TYPE_DEPTH;
    static const uint64_t MSG_TYPE_TRADE;

    void log_depth(book::Depth<5>& depth);
    bool handle_trade_message(const std::string& symbol,
                              uint64_t& seq_num,
                              uint64_t& timestamp,
                              QuickFAST::Messages::Message& msg);
    bool handle_depth_message(const std::string& symbol,
                              uint64_t& seq_num,
                              uint64_t& timestamp,
                              QuickFAST::Messages::Message& msg);
  };
} }
