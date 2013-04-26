
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
DepthFeedSubscriber::handle_message(BufferPtr& bp)
{
  QuickFAST::Codecs::DataSourceBuffer source(bp->c_array(), bp->size());
  QuickFAST::Codecs::SingleMessageConsumer consumer;
  QuickFAST::Codecs::GenericMessageBuilder builder(consumer);
  decoder_.decodeMessage(source, builder);
  QuickFAST::Messages::Message& msg(consumer.message());

  uint64_t seq_num, timestamp;
  const QuickFAST::StringBuffer* string_buffer;
  size_t bids_length, asks_length;
  if (msg.getUnsignedInteger(*id_seq_num_, ValueType::UINT32, seq_num)) {
    std::cout << "Got depth message seq num " << seq_num << std::endl;
  }
  if (msg.getString(*id_symbol_, ValueType::ASCII, string_buffer)) {
    std::cout << "Got depth message for symbol " 
              << (std::string)*string_buffer << std::endl;
  }
  if (msg.getUnsignedInteger(*id_timestamp_, ValueType::UINT32, timestamp)) {
    std::cout << "Got depth message timestamp " << timestamp << std::endl;
  }
  if (msg.getSequenceLength(*id_bids_, bids_length)) {
    std::cout << "Got " << bids_length << " bids " << std::endl;
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
        std::cout << "Bid level " << level_num
                  << ": price " << price
                  << ", qty " << aggregate_qty
                  << ", count " << order_count << std::endl;
      } else {
        std::cout << "Failed to get bid " << i << std::endl;
      }
      msg.endSequenceEntry(*id_bids_, i, accessor);
    }
  }
  if (msg.getSequenceLength(*id_asks_, asks_length)) {
    std::cout << "Got " << asks_length << " asks " << std::endl;
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
        std::cout << "Ask level " << level_num
                  << ": price " << price
                  << ", qty " << aggregate_qty
                  << ", count " << order_count << std::endl;
      } else {
        std::cout << "Failed to get ask " << i << std::endl;
      }
      msg.endSequenceEntry(*id_asks_, i, accessor);
    }
  }
}

} }
