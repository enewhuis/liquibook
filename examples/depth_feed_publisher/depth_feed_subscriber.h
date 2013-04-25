#include <boost/cstdint.hpp>
#include <boost/operators.hpp>
#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>
#include <stdexcept>
#include <cstring>
#include <sstream>
#include <vector>
#include <Codecs/Decoder.h>

#include "template_consumer.h"
#include "depth_feed_connection.h"

namespace liquibook { namespace examples {

  class DepthFeedSubscriber : public TemplateConsumer {
  public:
    DepthFeedSubscriber(const std::string& template_filename);
    void handle_message(BufferPtr& bp);

  private:
    QuickFAST::Codecs::Decoder decoder_;
  };

} }
