
#include <boost/shared_ptr.hpp>
#include <Codecs/TemplateRegistry_fwd.h>

namespace liquibook { namespace examples {

class TemplateConsumer {
public:
  static QuickFAST::Codecs::TemplateRegistryPtr 
             parse_templates(const std::string& template_filename);
};

} }

