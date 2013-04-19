#include "template_consumer.h"
#include <fstream>
#include <Codecs/XMLTemplateParser.h>

namespace liquibook { namespace examples {

QuickFAST::Codecs::TemplateRegistryPtr 
TemplateConsumer::parse_templates(const std::string& template_fileanme)
{
  std::ifstream template_stream("./templates/Depth.xml");
  QuickFAST::Codecs::XMLTemplateParser parser;
  return parser.parse(template_stream);
}

} } 
