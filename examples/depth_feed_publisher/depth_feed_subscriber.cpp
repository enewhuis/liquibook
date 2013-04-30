
#include <boost/scoped_ptr.hpp>
#include "depth_feed_subscriber.h"
#include <Codecs/DataSourceBuffer.h>
#include <Codecs/GenericMessageBuilder.h>
#include <Codecs/SingleMessageConsumer.h>
#include <Messages/MessageAccessor.h>

namespace liquibook { namespace examples {

using QuickFAST::ValueType;

DepthFeedSubscriber::DepthFeedSubscriber(const std::string& template_filename)
: decoder_(parse_templates(template_filename))
{
}

void
DepthFeedSubscriber::handle_message(BufferPtr& bp, size_t bytes_transferred)
{
  QuickFAST::Codecs::DataSourceBuffer source(bp->c_array(), bytes_transferred);
  QuickFAST::Codecs::SingleMessageConsumer consumer;
  QuickFAST::Codecs::GenericMessageBuilder builder(consumer);
  decoder_.decodeMessage(source, builder);
  QuickFAST::Messages::Message& msg(consumer.message());

  uint64_t seq_num, timestamp;
  const QuickFAST::StringBuffer* string_buffer;
  size_t bids_length, asks_length;
  std::string symbol;
  if (!msg.getUnsignedInteger(*id_seq_num_, ValueType::UINT32, seq_num)) {
    std::cout << "Could not get seq num from message" << std::endl;
    return;
  }
  if (!msg.getString(*id_symbol_, ValueType::ASCII, string_buffer)) {
    std::cout << "Could not get symbol from message" << std::endl;
    return;
  }

  symbol = (std::string)*string_buffer;
  std::cout << "Got depth message " << seq_num 
            << " for symbol " << symbol << std::endl;

  // Create or find depth
  std::pair<DepthMap::iterator, bool> results = depth_map_.insert(
      std::make_pair(symbol, book::Depth<5>()));
  book::Depth<5>& depth = results.first->second;

  if (msg.getSequenceLength(*id_bids_, bids_length)) {
    for (size_t i = 0; i < bids_length; ++i) {
      const QuickFAST::Messages::MessageAccessor* accessor;
      if (msg.getSequenceEntry(*id_bids_, i, accessor)) {
        uint64_t level_num, price, order_count, aggregate_qty;
        if (!accessor->getUnsignedInteger(*id_level_num_, ValueType::UINT8,
                                         level_num)) {
          std::cout << "FAILED TO GET Bid level " << std::endl;
          continue;
        }
        if (!accessor->getUnsignedInteger(*id_price_, ValueType::UINT32,
                                         price)) {
          std::cout << "FAILED TO GET Bid price " << std::endl;
          continue;
        }
        if (!accessor->getUnsignedInteger(*id_order_count_, ValueType::UINT32,
                                         order_count)) {
          std::cout << "FAILED TO GET Bid order count " << std::endl;
          continue;
        }
        if (!accessor->getUnsignedInteger(*id_size_, ValueType::UINT32,
                                         aggregate_qty)) {
          std::cout << "FAILED TO GET Bid aggregate qty " << std::endl;
          continue;
        }

        book::DepthLevel& level = depth.bids()[level_num];
        level.set(price, aggregate_qty, order_count);

      } else {
        std::cout << "Failed to get bid " << i << std::endl;
      }
      msg.endSequenceEntry(*id_bids_, i, accessor);
    }
  }
  if (msg.getSequenceLength(*id_asks_, asks_length)) {
    for (size_t i = 0; i < asks_length; ++i) {
      const QuickFAST::Messages::MessageAccessor* accessor;
      if (msg.getSequenceEntry(*id_asks_, i, accessor)) {
        uint64_t level_num, price, order_count, aggregate_qty;
        if (!accessor->getUnsignedInteger(*id_level_num_, ValueType::UINT8,
                                         level_num)) {
          std::cout << "FAILED TO GET Ask level " << std::endl;
          continue;
        }
        if (!accessor->getUnsignedInteger(*id_price_, ValueType::UINT32,
                                         price)) {
          std::cout << "FAILED TO GET Ask price " << std::endl;
          continue;
        }
        if (!accessor->getUnsignedInteger(*id_order_count_, ValueType::UINT32,
                                         order_count)) {
          std::cout << "FAILED TO GET Ask order count " << std::endl;
          continue;
        }
        if (!accessor->getUnsignedInteger(*id_size_, ValueType::UINT32,
                                         aggregate_qty)) {
          std::cout << "FAILED TO GET Ask aggregate qty " << std::endl;
          continue;
        }

        book::DepthLevel& level = depth.asks()[level_num];
        level.set(price, aggregate_qty, order_count);

      } else {
        std::cout << "Failed to get ask " << i << std::endl;
      }
      msg.endSequenceEntry(*id_asks_, i, accessor);
    }
  }
  log_depth(depth);
}

void
DepthFeedSubscriber::log_depth(book::Depth<5>& depth)
{
  book::DepthLevel* bid = depth.bids();
  book::DepthLevel* ask = depth.asks();
  printf("----------BID----------    ----------ASK----------\n");
  while (bid || ask) {
    if (bid && bid->order_count()) {
      printf("%8.2f %9d [%2d]", 
             (double)bid->price() / 10.0, 
             bid->aggregate_qty(), bid->order_count());
      if (bid == depth.last_bid_level()) {
        bid = NULL;
      } else {
        ++bid;
      }
    } else {
      // Blanklines
      printf("                       ");
      bid = NULL;
    }

    if (ask && ask->order_count()) {
      printf("    %8.2f %9d [%2d]\n",
             (double)ask->price() / 10.0, 
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

} }
