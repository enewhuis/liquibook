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

DepthFeedPublisher::DepthFeedPublisher()
: connection_(NULL)
{
}

void
DepthFeedPublisher::set_connection(DepthFeedConnection* connection)
{
  connection_ = connection;
}

void
DepthFeedPublisher::on_trade(
    const book::OrderBook<OrderPtr>* order_book,
    book::Quantity qty,
    book::Cost cost)
{
  // Publish trade
  QuickFAST::Messages::FieldSet message(20);
  const ExampleOrderBook* exob = 
          dynamic_cast<const ExampleOrderBook*>(order_book);
  std::cout << "Got trade for " << exob->symbol() 
            << " qty " << qty
            << " cost " << cost << std::endl;
  build_trade_message(message, exob->symbol(), qty, cost);
  connection_->send_trade(message);
}

void
DepthFeedPublisher::on_depth_change(
    const book::DepthOrderBook<OrderPtr>* order_book,
    const book::DepthOrderBook<OrderPtr>::DepthTracker* tracker)
{
  // Publish changed levels of order book
  QuickFAST::Messages::FieldSet message(20);
  const ExampleOrderBook* exob = 
          dynamic_cast<const ExampleOrderBook*>(order_book);
  build_depth_message(message, exob->symbol(), tracker, false);
  if (!connection_->send_incr_update(exob->symbol(), message)) {
    // Publish all levels of order book
    QuickFAST::Messages::FieldSet full_message(20);
    build_depth_message(full_message, exob->symbol(), tracker, true);
    connection_->send_full_update(exob->symbol(), full_message);
  }
}
 
void
DepthFeedPublisher::build_trade_message(
    QuickFAST::Messages::FieldSet& message,
    const std::string& symbol,
    book::Quantity qty,
    book::Cost cost)
{
  message.addField(id_timestamp_, FieldUInt32::create(time_stamp()));
  message.addField(id_symbol_, FieldString::create(symbol));
  message.addField(id_qty_, FieldUInt32::create(qty));
  message.addField(id_cost_, FieldUInt32::create(cost));
}

void
DepthFeedPublisher::build_depth_message(
    QuickFAST::Messages::FieldSet& message,
    const std::string& symbol,
    const book::DepthOrderBook<OrderPtr>::DepthTracker* tracker,
    bool full_message)
{
  size_t bid_count(0), ask_count(0);

  message.addField(id_timestamp_, FieldUInt32::create(time_stamp()));
  message.addField(id_symbol_, FieldString::create(symbol));

  // Build the changed levels
  book::ChangeId last_published_change = tracker->last_published_change();
  
  // Build changed bids
  {
    SequencePtr bids(new Sequence(id_bids_length_, 1));
    int index = 0;
    const book::DepthLevel* bid = tracker->bids();
    // Create sequence of bids
    while (true) {
      if (full_message || bid->changed_since(last_published_change)) {
        build_depth_level(bids, bid, index);
        ++bid_count;
      }
      ++index;
      if (bid == tracker->last_bid_level()) {
        break;
      } else {
        ++bid;
      }
    }
    message.addField(id_bids_, FieldSequence::create(bids));
  }

  // Build changed asks
  {
    SequencePtr asks(new Sequence(id_asks_length_, 1));
    int index = 0;
    const book::DepthLevel* ask = tracker->asks();
    // Create sequence of asks
    while (true) {
      if (full_message || ask->changed_since(last_published_change)) {
        build_depth_level(asks, ask, index);
        ++ask_count;
      }
      ++index;
      if (ask == tracker->last_ask_level()) {
        break;
      } else {
        ++ask;
      }
    }
    message.addField(id_asks_, FieldSequence::create(asks));
  }
  std::cout << "Encoding " << (full_message ? "full" : "incr")
            << " depth message for symbol " << symbol 
            << " with " << bid_count << " bids, "
            << ask_count << " asks" << std::endl;
}

void
DepthFeedPublisher::build_depth_level(
    QuickFAST::Messages::SequencePtr& level_seq,
    const book::DepthLevel* level,
    int level_index)
{
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
  return (uint32_t) now;
}

} } // End namespace
