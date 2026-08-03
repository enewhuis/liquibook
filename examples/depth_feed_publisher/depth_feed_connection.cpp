#include "depth_feed_connection.h"
#include <iomanip>
#include <boost/bind.hpp>
#include "template_consumer.h"
#include <Codecs/DataDestination.h>
#include <Messages/FieldSet.h>
#include <Messages/FieldUInt32.h>

using namespace boost::asio::ip;

namespace liquibook { namespace examples {

QuickFAST::template_id_t DepthFeedSession::TID_TRADE_MESSAGE(1);
QuickFAST::template_id_t DepthFeedSession::TID_DEPTH_MESSAGE(2);

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

void
DepthFeedSession::send_trade(QuickFAST::Messages::FieldSet& message)
{
  // Add or update sequence number in message
  set_sequence_num(message);
                            
  std::cout << "sending trade message with " << message.size() << " fields" << std::endl;

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
    // Add or update sequence number in message
    set_sequence_num(message);
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
    // Add or update sequence number in message
    set_sequence_num(message);
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
DepthFeedSession::set_sequence_num(QuickFAST::Messages::FieldSet& message)
{
  // Create the field
  QuickFAST::Messages::FieldCPtr value = 
      QuickFAST::Messages::FieldUInt32::create(++seq_num_);
  // Update the sequence number
  if (!message.replaceField(TemplateConsumer::id_seq_num_, value)) {
    // Not found, add the sequence number
    message.addField(TemplateConsumer::id_seq_num_, value);
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
: template_filename_(template_file_from_args(argc, argv)),
  host_(host_from_args(argc, argv)),
  port_(port_from_args(argc, argv)),
  templates_(TemplateConsumer::parse_templates(template_filename_)),
  socket_(ios_)
{
}

void
DepthFeedConnection::connect()
{
  std::cout << "Connecting to feed" << std::endl;
  tcp::endpoint endpoint(address::from_string(host_), port_);
  socket_.async_connect(endpoint, boost::bind(&DepthFeedConnection::on_connect,
                                              this, _1));
}

void
DepthFeedConnection::accept()
{
  if (!acceptor_) {
    acceptor_.reset(new tcp::acceptor(ios_));
    tcp::endpoint endpoint(tcp::v4(), port_);
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
std::cout << "DepthFeedConnection::run()" << std::endl;
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
  } else {
    std::cout << "Error " << error << " receiving message" << std::endl;
    socket_.close();
    sleep(3);
    connect();
  }
  // Restore buffer
  unused_recv_buffers_.push_back(bp);
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

const char*
DepthFeedConnection::template_file_from_args(int argc, const char* argv[])
{
  bool next_is_name = false;
  for (int i = 0; i < argc; ++i) {
    if (next_is_name) {
      return argv[i];
    } else if (strcmp(argv[i], "-t") == 0) {
      next_is_name = true;
    }
  }
  return "./templates/depth.xml";
}

const char*
DepthFeedConnection::host_from_args(int argc, const char* argv[])
{
  bool next_is_host = false;
  for (int i = 0; i < argc; ++i) {
    if (next_is_host) {
      return argv[i];
    } else if (strcmp(argv[i], "-h") == 0) {
      next_is_host = true;
    }
  }
  return "127.0.0.1";
}

int
DepthFeedConnection::port_from_args(int argc, const char* argv[])
{
  bool next_is_port = false;
  for (int i = 0; i < argc; ++i) {
    if (next_is_port) {
      return atoi(argv[i]);
    } else if (strcmp(argv[i], "-p") == 0) {
      next_is_port = true;
    }
  }
  return 10003;
}

} } // End namespace
