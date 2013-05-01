
#include <iomanip>
#include <boost/bind.hpp>
#include "depth_feed_connection.h"
#include "template_consumer.h"
#include <Codecs/DataDestination.h>
#include <Messages/FieldSet.h>
#include <Messages/FieldUInt32.h>

#define TID_DEPTH_MESSAGE QuickFAST::template_id_t(1)
#define TID_TRADE_MESSAGE QuickFAST::template_id_t(2)

using namespace boost::asio::ip;

namespace liquibook { namespace examples {

DepthFeedSession::DepthFeedSession(
    boost::asio::io_service& ios,
    DepthFeedConnection* connection,
    QuickFAST::Codecs::TemplateRegistryPtr& templates)
: connected_(false),
  seq_num_(0),
  ios_(ios),
  socket_(ios),
  connection_(connection),
  encoder_(templates)
{
}

DepthFeedSession::~DepthFeedSession()
{
}

void
DepthFeedSession::send_trade(QuickFAST::Messages::FieldSet& message)
{
  std::cout << "sending trade message with " << message.size() << " fields" << std::endl;
  message.addField(TemplateConsumer::id_seq_num_,
                   QuickFAST::Messages::FieldUInt32::create(++seq_num_));
  QuickFAST::Codecs::DataDestination dest;
  encoder_.encodeMessage(dest, TID_TRADE_MESSAGE, message);
  WorkingBufferPtr wb = connection_->reserve_send_buffer();
  dest.toWorkingBuffer(*wb);

  // Perform the send
  SendHandler send_handler = boost::bind(&DepthFeedSession::on_send,
                                         this, wb, _1, _2);
  boost::asio::const_buffers_1 buffer(
      boost::asio::buffer(wb->begin(), wb->size()));
  socket_.async_send(buffer, 0, send_handler);
}

bool
DepthFeedSession::send_incr_update(const std::string& symbol,
                                   QuickFAST::Messages::FieldSet& message)
{
  bool sent = false;
  // If the session has been started for this symbol
  if (sent_symbols_.find(symbol) != sent_symbols_.end()) {
    QuickFAST::Codecs::DataDestination dest;
    message.addField(TemplateConsumer::id_seq_num_,
                     QuickFAST::Messages::FieldUInt32::create(++seq_num_));
    encoder_.encodeMessage(dest, TID_DEPTH_MESSAGE, message);
    WorkingBufferPtr wb = connection_->reserve_send_buffer();
    dest.toWorkingBuffer(*wb);
    SendHandler send_handler = boost::bind(&DepthFeedSession::on_send,
                                           this, wb, _1, _2);
    boost::asio::const_buffers_1 buffer(
        boost::asio::buffer(wb->begin(), wb->size()));
    socket_.async_send(buffer, 0, send_handler);
    sent = true;
  }
  return sent;
}

void
DepthFeedSession::send_full_update(const std::string& symbol,
                                   QuickFAST::Messages::FieldSet& message)
{
  // Mark this symbols as sent
  std::pair<StringSet::iterator, bool> result = sent_symbols_.insert(symbol);

  // If this symbol is new for the session
  if (result.second) {
    QuickFAST::Codecs::DataDestination dest;
    message.addField(TemplateConsumer::id_seq_num_,
                     QuickFAST::Messages::FieldUInt32::create(++seq_num_));
    encoder_.encodeMessage(dest, TID_DEPTH_MESSAGE, message);
    WorkingBufferPtr wb = connection_->reserve_send_buffer();
    dest.toWorkingBuffer(*wb);

    // Perform the send
    SendHandler send_handler = boost::bind(&DepthFeedSession::on_send,
                                           this, wb, _1, _2);
    boost::asio::const_buffers_1 buffer(
        boost::asio::buffer(wb->begin(), wb->size()));
    socket_.async_send(buffer, 0, send_handler);
  }
}

void
DepthFeedSession::on_send(WorkingBufferPtr wb,
                          const boost::system::error_code& error,
                      std::size_t bytes_transferred)
{
  if (error) {
    std::cout << "Error " << error << " sending message" << std::endl;
    connected_ = false;
  }

  // Keep buffer for later
  connection_->on_send(wb, error, bytes_transferred);
}

DepthFeedConnection::DepthFeedConnection(int argc, const char* argv[])
: connected_(false),
  templates_(TemplateConsumer::parse_templates("./templates/Depth.xml")),
  socket_(ios_)
{
}

DepthFeedConnection::~DepthFeedConnection()
{
  std::cout << "DepthFeedConnection dtor" << std::endl;
}

void
DepthFeedConnection::connect()
{
  std::cout << "Connecting to feed" << std::endl;
  tcp::endpoint endpoint(address::from_string("127.0.0.1"), 10003);
  socket_.async_connect(endpoint, boost::bind(&DepthFeedConnection::on_connect,
                                              this, _1));
}

void
DepthFeedConnection::accept()
{
  if (!acceptor_) {
    acceptor_.reset(new tcp::acceptor(ios_));
    tcp::endpoint endpoint(tcp::v4(), 10003);
    acceptor_->open(endpoint.protocol());
    boost::system::error_code ec;
    acceptor_->set_option(boost::asio::socket_base::reuse_address(true), ec);
    acceptor_->bind(endpoint);
    acceptor_->listen();
  }
  SessionPtr session(new DepthFeedSession(ios_, this, templates_));
  acceptor_->async_accept(
      session->socket(), 
      boost::bind(&DepthFeedConnection::on_accept, this, session, _1));
}

void
DepthFeedConnection::run()
{
  // Keep on running
  work_ptr_.reset(new boost::asio::io_service::work(ios_));
  ios_.run();
}

void
DepthFeedConnection::set_message_handler(MessageHandler handler)
{
  msg_handler_ = handler;
}

void
DepthFeedConnection::set_reset_handler(ResetHandler handler)
{
  reset_handler_ = handler;
}

WorkingBufferPtr
DepthFeedConnection::reserve_send_buffer()
{
  if (unused_send_buffers_.empty()) {
    return WorkingBufferPtr(new QuickFAST::WorkingBuffer());
  } else {
    WorkingBufferPtr wb = unused_send_buffers_.front();
    unused_send_buffers_.pop_front();
    return wb;
  }
}

BufferPtr
DepthFeedConnection::reserve_recv_buffer()
{
  if (unused_recv_buffers_.empty()) {
    return BufferPtr(new Buffer());
  } else {
    BufferPtr bp = unused_recv_buffers_.front();
    unused_recv_buffers_.pop_front();
    return bp;
  }
}

void
DepthFeedConnection::send_trade(QuickFAST::Messages::FieldSet& message)
{
  // For each session
  Sessions::iterator session;
  for (session = sessions_.begin(); session != sessions_.end(); ) {
    // If the session is connected
    if ((*session)->connected()) {
      // conditionally send on that session
      (*session)->send_trade(message);
      ++session;
    } else {
      // Remove the session
      session = sessions_.erase(session);
    }
  }
}

bool
DepthFeedConnection::send_incr_update(const std::string& symbol,
                                      QuickFAST::Messages::FieldSet& message)
{
  bool none_new = true;
  // For each session
  Sessions::iterator session;
  for (session = sessions_.begin(); session != sessions_.end(); ) {
    // If the session is connected
    if ((*session)->connected()) {
      // send on that session
      if (!(*session)->send_incr_update(symbol, message)) {
        none_new = false;
      }
      ++session;
    } else {
      // Remove the session
      session = sessions_.erase(session);
    }
  }
  return none_new;
}

void
DepthFeedConnection::send_full_update(const std::string& symbol,
                                      QuickFAST::Messages::FieldSet& message)
{
  // For each session
  Sessions::iterator session;
  for (session = sessions_.begin(); session != sessions_.end(); ) {
    // If the session is connected
    if ((*session)->connected()) {
      // conditionally send on that session
      (*session)->send_full_update(symbol, message);
      ++session;
    } else {
      // Remove the session
      session = sessions_.erase(session);
    }
  }
}

void
DepthFeedConnection::on_connect(const boost::system::error_code& error)
{
  if (!error) {
    std::cout << "connected to feed" << std::endl;
    connected_ = true;
    reset_handler_();
    issue_read();
  } else {
    std::cout << "on_connect, error=" << error << std::endl;
    socket_.close();
    sleep(3);
    // Try again
    connect();
  }
}

void
DepthFeedConnection::on_accept(SessionPtr session,
                               const boost::system::error_code& error)
{
  if (!error) {
    std::cout << "accepted client connection" << std::endl;
    sessions_.push_back(session);
    session->set_connected();
  } else {
    std::cout << "on_accept, error=" << error << std::endl;
    session.reset();
    sleep(2);
  }
  // accept again
  accept();
}

void
DepthFeedConnection::on_receive(BufferPtr bp,
                                const boost::system::error_code& error,
                                std::size_t bytes_transferred)
{
  if (!error) {
    // Next read
    issue_read();

    // Handle the buffer
    if (!msg_handler_(bp, bytes_transferred)) {
      socket_.close();
    }

    // Restore buffer
    unused_recv_buffers_.push_back(bp);
  } else {
    std::cout << "Error " << error << " receiving message" << std::endl;
    socket_.close();
    sleep(3);
    connect();
  }
}

void
DepthFeedConnection::on_send(WorkingBufferPtr wb,
                             const boost::system::error_code& error,
                             std::size_t bytes_transferred)
{
  // Keep buffer for later
  unused_send_buffers_.push_back(wb);
}

void
DepthFeedConnection::issue_read()
{
  BufferPtr bp = reserve_recv_buffer();
  RecvHandler recv_handler = boost::bind(&DepthFeedConnection::on_receive, 
                                         this, bp, _1, _2);
  boost::asio::mutable_buffers_1 buffer(
      boost::asio::buffer(*bp, bp->size()));
  socket_.async_receive(buffer, 0, recv_handler);
}

} } // End namespace
