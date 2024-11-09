
#include "order.h"
#include <boost/scoped_ptr.hpp>
#include "depth_feed_subscriber.h"
#include <Codecs/DataSourceBuffer.h>
#include <Codecs/GenericMessageBuilder.h>
#include <Codecs/SingleMessageConsumer.h>
#include <Messages/MessageAccessor.h>

namespace liquibook { namespace examples {

const uint64_t DepthFeedSubscriber::MSG_TYPE_DEPTH(11);
const uint64_t DepthFeedSubscriber::MSG_TYPE_TRADE(22);

using QuickFAST::ValueType;

DepthFeedSubscriber::DepthFeedSubscriber(
        const QuickFAST::Codecs::TemplateRegistryPtr& templates)
: decoder_(templates),
  expected_seq_(1)
{
}

void
DepthFeedSubscriber::handle_reset()
{
  expected_seq_ = 1;
}

bool
DepthFeedSubscriber::handle_message(BufferPtr& bp, size_t bytes_transferred)
{
  // Decode the message
  QuickFAST::Codecs::DataSourceBuffer source(bp->c_array(), bytes_transferred);
  QuickFAST::Codecs::SingleMessageConsumer consumer;
  QuickFAST::Codecs::GenericMessageBuilder builder(consumer);
  decoder_.decodeMessage(source, builder);
  QuickFAST::Messages::Message& msg(consumer.message());

  // Examine message contents
  uint64_t seq_num, msg_type, timestamp;
  const QuickFAST::StringBuffer* string_buffer;
  std::string symbol;
  if (!msg.getUnsignedInteger(id_seq_num_, ValueType::UINT32, seq_num)) {
    std::cout << "Could not get seq num from msg" << std::endl;
    return false;
  }
  if (seq_num != expected_seq_) {
    std::cout << "ERROR: Got Seq num " << seq_num << ", expected " 
              << expected_seq_ << std::endl;
    return false;
  }
  if (!msg.getUnsignedInteger(id_msg_type_, ValueType::UINT32, msg_type)) {
    std::cout << "Could not get msg type from msg" << std::endl;
    return false;
  }
  if (!msg.getString(id_symbol_, ValueType::ASCII, string_buffer)) {
    std::cout << "Could not get symbol from msg" << std::endl;
    return false;
  }
  if (!msg.getUnsignedInteger(id_timestamp_, ValueType::UINT32, timestamp)) {
    std::cout << "Could not get timestamp from msg" << std::endl;
    return false;
  }
  bool result = false;
  symbol = (std::string)*string_buffer;
  switch (msg_type) {
  case MSG_TYPE_DEPTH:
    result = handle_depth_message(symbol, seq_num, timestamp, msg);
    break;
  case MSG_TYPE_TRADE:
    result = handle_trade_message(symbol, seq_num, timestamp, msg);
    break;
  default:
    std::cout << "ERROR: Unknown message type " << msg_type 
              << " seq num " << seq_num << std::endl;
    return false;
  }
  ++expected_seq_;
  return result;
}

void
DepthFeedSubscriber::log_depth(book::Depth<5>& depth)
{
  book::DepthLevel* bid = depth.bids();
  book::DepthLevel* ask = depth.asks();
  printf("----------BID----------    ----------ASK----------\n");
  while (bid || ask) {
    if (bid && bid->order_count()) {
      printf("%8.2f %9ld [%2d]",
             (double)bid->price() / Order::precision_,
             bid->aggregate_qty(), bid->order_count());
      if (bid == depth.last_bid_level()) {
        bid = NULL;
      } else {
        ++bid;
      }
    } else {
      // Blank lines
      printf("                       ");
      bid = NULL;
    }

    if (ask && ask->order_count()) {
      printf("    %8.2f %9ld [%2d]\n",
             (double)ask->price() / Order::precision_,
             ask->aggregate_qty(), ask->order_count());
      if (ask == depth.last_ask_level()) {
        ask = NULL;
      } else {
        ++ask;
      }
    } else {
      // Newline
      printf("\n");
      ask = NULL;
    }
  }
}

bool
DepthFeedSubscriber::handle_depth_message(
  const std::string& symbol,
  uint64_t& seq_num,
  uint64_t& timestamp,
  QuickFAST::Messages::Message& msg)
{
  size_t bids_length, asks_length;
  std::cout << timestamp
            << " Got depth msg " << seq_num 
            << " for symbol " << symbol << std::endl;

  // Create or find depth
  std::pair<DepthMap::iterator, bool> results = depth_map_.insert(
      std::make_pair(symbol, book::Depth<5>()));
  book::Depth<5>& depth = results.first->second;

  if (msg.getSequenceLength(id_bids_, bids_length)) {
    for (size_t i = 0; i < bids_length; ++i) {
      const QuickFAST::Messages::MessageAccessor* accessor;
      if (msg.getSequenceEntry(id_bids_, i, accessor)) {
        uint64_t level_num, price, order_count, aggregate_qty;
        if (!accessor->getUnsignedInteger(id_level_num_, ValueType::UINT8,
                                         level_num)) {
          std::cout << "Could not get Bid level from depth msg" << std::endl;
          return false;
        }
        if (!accessor->getUnsignedInteger(id_price_, ValueType::UINT32,
                                         price)) {
          std::cout << "Could not get Bid price from depth msg" << std::endl;
          return false;
        }
        if (!accessor->getUnsignedInteger(id_order_count_, ValueType::UINT32,
                                         order_count)) {
          std::cout << "Could not get Bid count from depth msg" << std::endl;
          return false;
        }
        if (!accessor->getUnsignedInteger(id_size_, ValueType::UINT32,
                                         aggregate_qty)) {
          std::cout << "Could not get Bid agg qty  from depth msg" << std::endl;
          return false;
        }

        book::DepthLevel& level = depth.bids()[level_num];
        level.set(book::Price(price), book::Quantity(aggregate_qty), uint32_t(order_count));

      } else {
        std::cout << "Failed to get bid " << i << std::endl;
        return false;
      }
      msg.endSequenceEntry(id_bids_, i, accessor);
    }
  }
  if (msg.getSequenceLength(id_asks_, asks_length)) {
    for (size_t i = 0; i < asks_length; ++i) {
      const QuickFAST::Messages::MessageAccessor* accessor;
      if (msg.getSequenceEntry(id_asks_, i, accessor)) {
        uint64_t level_num, price, order_count, aggregate_qty;
        if (!accessor->getUnsignedInteger(id_level_num_, ValueType::UINT8,
                                         level_num)) {
          std::cout << "Could not get Ask level from depth msg " << std::endl;
          return false;
        }
        if (!accessor->getUnsignedInteger(id_price_, ValueType::UINT32,
                                         price)) {
          std::cout << "Could not get Ask price  from depth msg" << std::endl;
          return false;
        }
        if (!accessor->getUnsignedInteger(id_order_count_, ValueType::UINT32,
                                         order_count)) {
          std::cout << "Could not get Ask count from depth msg " << std::endl;
          return false;
        }
        if (!accessor->getUnsignedInteger(id_size_, ValueType::UINT32,
                                         aggregate_qty)) {
          std::cout << "Could not get Ask agg qty from depth msg " << std::endl;
          return false;
        }

        book::DepthLevel& level = depth.asks()[level_num];
        level.set(book::Price(price), book::Quantity(aggregate_qty), uint32_t(order_count));

      } else {
        std::cout << "Failed to get ask " << i << std::endl;
        return false;
      }
      msg.endSequenceEntry(id_asks_, i, accessor);
    }
  }
  log_depth(depth);
  return true;
}

bool
DepthFeedSubscriber::handle_trade_message(
  const std::string& symbol,
  uint64_t& seq_num,
  uint64_t& timestamp,
  QuickFAST::Messages::Message& msg)
{
  uint64_t qty, cost;
  // Get trade fields
  if (!msg.getUnsignedInteger(id_qty_, ValueType::UINT32, qty)) {
    std::cout << "Could not qty from trade msg" << std::endl;
    return false;
  }
  if (!msg.getUnsignedInteger(id_cost_, ValueType::UINT32, cost)) {
    std::cout << "Could not get cost from trade msg" << std::endl;
    return false;
  }

  double price = (double) cost / (qty * Order::precision_);
  std::cout << timestamp
            << " Got trade msg " << seq_num 
            << " for symbol " << symbol 
            << ": " << qty << "@" << price
            << std::endl;

  return true;
}

} }
