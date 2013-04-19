#include <iomanip>
#include <fstream>
#include "depth_feed_publisher.h"
#include <Codecs/DataDestination.h>
#include <Codecs/XMLTemplateParser.h>
#include <Messages/FieldIdentity.h>
#include <Messages/FieldSet.h>
#include <Messages/FieldSequence.h>
#include <Messages/FieldString.h>
#include <Messages/FieldUInt8.h>
#include <Messages/FieldUInt32.h>
#include <Messages/Sequence.h>

namespace liquibook { namespace examples { 

using namespace QuickFAST::Messages;

DepthFeedPublisher::DepthFeedPublisher(const std::string& template_filename)
: sequence_num_(0),
  encoder_(parse_templates(template_filename)),
  id_seq_num_(new FieldIdentity("SequenceNumber")),
  id_timestamp_(new FieldIdentity("Timestamp")),
  id_symbol_(new FieldIdentity("Symbol")),
  id_bids_(new FieldIdentity("Bids")),
  id_bids_length_(new FieldIdentity("BidsLength")),
  id_asks_(new FieldIdentity("Asks")),
  id_asks_length_(new FieldIdentity("AsksLength")),
  id_level_num_(new FieldIdentity("LevelNum")),
  id_order_count_(new FieldIdentity("OrderCount")),
  id_size_(new FieldIdentity("AggregateQty")),
  id_price_(new FieldIdentity("Price")),
  tid_depth_message_(1)
{
}

void
DepthFeedPublisher::on_depth_change(
    const book::DepthOrderBook<OrderPtr>* order_book,
    const book::DepthOrderBook<OrderPtr>::DepthTracker* tracker)
{
  // Published changed levels of order book
  std::cout << "Depth changed" << std::endl;
  QuickFAST::Codecs::DataDestination message;
  const ExampleOrderBook* exob = 
          dynamic_cast<const ExampleOrderBook*>(order_book);
  build_depth_message(message, exob->symbol(), tracker);
  QuickFAST::WorkingBuffer wb;
  message.toWorkingBuffer(wb);
  wb.hexDisplay(std::cout);
}
 
void
DepthFeedPublisher::build_depth_message(
    QuickFAST::Codecs::DataDestination& dest,
    const std::string& symbol,
    const book::DepthOrderBook<OrderPtr>::DepthTracker* tracker)
{
  QuickFAST::Messages::FieldSet message(20); // allocate space for 20 fields
  message.addField(id_seq_num_, FieldUInt32::create(++sequence_num_));
  message.addField(id_timestamp_, FieldUInt32::create(time_stamp()));
  message.addField(id_symbol_, FieldString::create(symbol));

  // Build the changed levels
  book::ChangeId last_published_change = tracker->last_published_change();
  
  // Build changed bids
  {
    SequencePtr bids(new Sequence(id_bids_length_, 1));
    int index = 0;
    // Todo - create sequende of bids
    const book::DepthLevel* bid = tracker->bids();
    do {
      if (bid->changed_since(last_published_change)) {
        build_depth_level(bids, bid, index);
      }
      ++index;
    } while (++bid != tracker->last_bid_level());
    message.addField(id_bids_, FieldSequence::create(bids));
  }
  
  // Build changed asks
  {
    SequencePtr asks(new Sequence(id_asks_length_, 1));
    int index = 0;
    // Todo - create sequende of asks
    const book::DepthLevel* ask = tracker->asks();
    do {
      if (ask->changed_since(last_published_change)) {
        build_depth_level(asks, ask, index);
      }
      ++index;
    } while (++ask != tracker->last_ask_level());
    message.addField(id_asks_, FieldSequence::create(asks));
  }
  encoder_.encodeMessage(dest, tid_depth_message_, message);
}

void
DepthFeedPublisher::build_depth_level(
    QuickFAST::Messages::SequencePtr& level_seq,
    const book::DepthLevel* level,
    int level_index)
{
  std::cout << "Depth level " << level_index << " changed" << std::endl;
  FieldSetPtr level_fields(new FieldSet(4));
  level_fields->addField(id_level_num_, FieldUInt8::create(level_index + 1));
  level_fields->addField(id_order_count_, 
                         FieldUInt32::create(level->order_count()));
  level_fields->addField(id_price_,
                         FieldUInt32::create(level->price()));
  level_fields->addField(id_size_,
                         FieldUInt32::create(level->aggregate_qty()));
  level_seq->addEntry(level_fields);
}

uint32_t
DepthFeedPublisher::time_stamp()
{
  time_t now;
  time(&now);
  return now;
}

QuickFAST::Codecs::TemplateRegistryPtr
DepthFeedPublisher::parse_templates(const std::string& template_filename)
{
  std::ifstream template_stream("./templates/Depth.xml");
  QuickFAST::Codecs::XMLTemplateParser parser;
  return parser.parse(template_stream);
}

} } // End namespace
