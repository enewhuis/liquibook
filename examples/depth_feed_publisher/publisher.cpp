#include <boost/cstdint.hpp>
#include <map>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <stdexcept>
#include <boost/operators.hpp>
#include <cstring>
#include <sstream>
#include <boost/scoped_array.hpp>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <Codecs/Encoder.h>
#include <Codecs/DataDestination.h>
#include <Codecs/TemplateRegistry.h>
#include <Codecs/XMLTemplateParser.h>
#include <Messages/FieldIdentity.h>
#include <Messages/FieldSet.h>
#include <Messages/FieldString.h>
#include <Messages/FieldUInt16.h>
#include <Messages/FieldUInt32.h>
#include <Messages/MessageBuilder.h>

using namespace QuickFAST::Messages;

void populateUInt32(QuickFAST::Messages::FieldSet& message,
                    const std::string& fieldName,
                    QuickFAST::uint32 fieldValue)
{
  QuickFAST::Messages::FieldIdentityCPtr fieldNamePtr(new QuickFAST::Messages::FieldIdentity(fieldName));
  QuickFAST::Messages::FieldCPtr fieldValuePtr = QuickFAST::Messages::FieldUInt32::create(fieldValue);
  message.addField(fieldNamePtr,fieldValuePtr);
}

int main(int argc, const char** argv) {
  std::ifstream templateStream("./templates/Simple.xml");
  QuickFAST::Codecs::XMLTemplateParser parser;
  QuickFAST::Codecs::TemplateRegistryPtr templateRegistry =
      parser.parse(templateStream);

  QuickFAST::Codecs::Encoder encoder(templateRegistry);
  QuickFAST::template_id_t templateId = 1;
  QuickFAST::Messages::FieldSet message(20); // allocate space for 20 message fields

  QuickFAST::Messages::FieldIdentityCPtr seqNumId(new FieldIdentity("SequenceNumber"));
  QuickFAST::Messages::FieldIdentityCPtr timestampId(new FieldIdentity("Timestamp"));
  QuickFAST::Messages::FieldIdentityCPtr orderIdId(new FieldIdentity("OrderId"));
  QuickFAST::Messages::FieldIdentityCPtr symbolId(new FieldIdentity("Symbol"));
  QuickFAST::Messages::FieldIdentityCPtr sizeId(new FieldIdentity("Size"));
  QuickFAST::Messages::FieldIdentityCPtr priceId(new FieldIdentity("Price"));
  // Build message
  message.addField(seqNumId, FieldUInt32::create(1));
  message.addField(timestampId, FieldUInt32::create(239879098));
  message.addField(orderIdId, FieldUInt32::create(202));
  message.addField(symbolId, FieldString::create("MSFT"));
  message.addField(sizeId, FieldUInt16::create(100));
  message.addField(priceId, FieldUInt32::create(2000));

  // Populate sequence number
  QuickFAST::Codecs::DataDestination dest;
  encoder.encodeMessage(dest, templateId, message);
  std::cout << "hello" << std::endl;
  return 0;
};
