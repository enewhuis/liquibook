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
  tid_depth_message_(1),
  connection_(NULL)
{
}

void
DepthFeedPublisher::set_message_handler(DepthFeedConnection* connection)
{
  connection_ = connection;
}

void
DepthFeedPublisher::on_depth_change(
    const book::DepthOrderBook<OrderPtr>* order_book,
    const book::DepthOrderBook<OrderPtr>::DepthTracker* tracker)
{
  // Published changed levels of order book
  //QuickFAST::Codecs::DataDestination dest;
  QuickFAST::Messages::FieldSet message(20);
  const ExampleOrderBook* exob = 
          dynamic_cast<const ExampleOrderBook*>(order_book);
  build_depth_message(message, exob->symbol(), tracker);
  // TODO move
  //encoder_.encodeMessage(dest, tid_depth_message_, message);

  //WorkingBufferPtr wb = connection_->reserve_send_buffer();
  //dest.toWorkingBuffer(*wb);
  //std::cout << "incr message working buffer size " << wb->size() << std::endl;
  if (!connection_->send_incr_update(exob->symbol(), message)) {
    QuickFAST::Messages::FieldSet full_message(20);
    build_full_depth_message(full_message, exob->symbol(), tracker);
    connection_->send_full_update(exob->symbol(), full_message);
  }
/*
  if (!connection_->send_incr_update(exob->symbol(), wb)) {
    QuickFAST::Messages::FieldSet full_message(20);
    build_full_depth_message(full_message, exob->symbol(), tracker);

    WorkingBufferPtr full_wb = connection_->reserve_send_buffer();
    full_message.toWorkingBuffer(*full_wb);
    std::cout << "full message working buffer size " << full_wb->size()
              << std::endl;
    // send full buffer
    QuickFAST::Codecs::DataDestination full_message;
    encoder_.encodeMessage(dest, tid_depth_message_, message);
    connection_->send_full_update(exob->symbol(), full_wb);  
  }
*/
  sleep(5);
}
 
void
DepthFeedPublisher::build_depth_message(
    QuickFAST::Messages::FieldSet& message,
    const std::string& symbol,
    const book::DepthOrderBook<OrderPtr>::DepthTracker* tracker)
{
  size_t bid_count(0), ask_count(0);

  message.addField(id_seq_num_, FieldUInt32::create(++sequence_num_));
  message.addField(id_timestamp_, FieldUInt32::create(time_stamp()));
  message.addField(id_symbol_, FieldString::create(symbol));

  // Build the changed levels
  book::ChangeId last_published_change = tracker->last_published_change();
  
  // Build changed bids
  {
    SequencePtr bids(new Sequence(id_bids_length_, 1));
    int index = 0;
    // Create sequence of bids
    const book::DepthLevel* bid = tracker->bids();
    do {
      if (bid->changed_since(last_published_change)) {
        build_depth_level(bids, bid, index);
        ++bid_count;
      }
      ++index;
    } while (++bid != tracker->last_bid_level());
    message.addField(id_bids_, FieldSequence::create(bids));
  }

  // Build changed asks
  {
    SequencePtr asks(new Sequence(id_asks_length_, 1));
    int index = 0;
    // Create sequence of asks
    const book::DepthLevel* ask = tracker->asks();
    do {
      if (ask->changed_since(last_published_change)) {
        build_depth_level(asks, ask, index);
        ++ask_count;
      }
      ++index;
    } while (++ask != tracker->last_ask_level());
    message.addField(id_asks_, FieldSequence::create(asks));
  }
  std::cout << "Encoding depth message for symbol " << symbol 
            << " with " << bid_count << " bids, "
            << ask_count << " asks" << std::endl;
}

void
DepthFeedPublisher::build_full_depth_message(
    QuickFAST::Messages::FieldSet& message,
    const std::string& symbol,
    const book::DepthOrderBook<OrderPtr>::DepthTracker* tracker)
{
  size_t bid_count(0), ask_count(0);

  message.addField(id_seq_num_, FieldUInt32::create(++sequence_num_));
  message.addField(id_timestamp_, FieldUInt32::create(time_stamp()));
  message.addField(id_symbol_, FieldString::create(symbol));

  // Build all bids
  {
    SequencePtr bids(new Sequence(id_bids_length_, 1));
    int index = 0;
    // Create sequence of bids
    const book::DepthLevel* bid = tracker->bids();
    do {
      build_depth_level(bids, bid, index);
      ++bid_count;
      ++index;
    } while (++bid != tracker->last_bid_level());
    message.addField(id_bids_, FieldSequence::create(bids));
  }

  // Build all asks
  {
    SequencePtr asks(new Sequence(id_asks_length_, 1));
    int index = 0;
    // Create sequence of asks
    const book::DepthLevel* ask = tracker->asks();
    do {
      build_depth_level(asks, ask, index);
      ++ask_count;
      ++index;
    } while (++ask != tracker->last_ask_level());
    message.addField(id_asks_, FieldSequence::create(asks));
  }
  std::cout << "Encoding depth message for symbol " << symbol 
            << " with " << bid_count << " bids, "
            << ask_count << " asks" << std::endl;
}

void
DepthFeedPublisher::build_depth_level(
    QuickFAST::Messages::SequencePtr& level_seq,
    const book::DepthLevel* level,
    int level_index)
{
  //std::cout << "Depth level " << level_index << " changed" << std::endl;
  FieldSetPtr level_fields(new FieldSet(4));
  level_fields->addField(id_level_num_, FieldUInt8::create(level_index));
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
