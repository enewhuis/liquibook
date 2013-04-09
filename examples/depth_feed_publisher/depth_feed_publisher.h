#ifndef example_depth_feed_publisher_h
#define example_depth_feed_publisher_h

#include "exchange.h"
#include <boost/cstdint.hpp>
#include <boost/operators.hpp>
#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>
#include <cstring>
#include <sstream>

#include <Codecs/Encoder.h>
#include <Codecs/TemplateRegistry_fwd.h>

namespace liquibook { namespace examples {

typedef ExampleOrderBook::DepthTracker Tracker;

class DepthFeedPublisher : public book::DepthListener<ExampleOrderBook> {
public:
  DepthFeedPublisher(const std::string& template_filename);

  virtual void on_depth_change(const ExampleOrderBook* order_book,
                               const Tracker* tracker);
private:
  QuickFAST::Codecs::Encoder encoder_;
  static QuickFAST::Codecs::TemplateRegistryPtr 
             parse_templates(const std::string& template_fileanme);
};

} } // End namespace
#endif
