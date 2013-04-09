#include "depth_feed_publisher.h"
#include <Codecs/XMLTemplateParser.h>
#include <fstream>

namespace liquibook { namespace examples { 

DepthFeedPublisher::DepthFeedPublisher(const std::string& template_filename)
: encoder_(parse_templates(template_filename))
{
}

void
DepthFeedPublisher::on_depth_change(const ExampleOrderBook* order_book,
                                    const Tracker* tracker)
{
  // Published changed levels of order book
  std::cout << "Depth changed" << std::endl;
}

QuickFAST::Codecs::TemplateRegistryPtr
DepthFeedPublisher::parse_templates(const std::string& template_filename)
{
  std::ifstream template_stream("./templates/Simple.xml");
  QuickFAST::Codecs::XMLTemplateParser parser;
  return parser.parse(template_stream);
}

} } // End namespace
