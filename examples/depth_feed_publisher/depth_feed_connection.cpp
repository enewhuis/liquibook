
#include <iomanip>
#include <boost/bind.hpp>
#include "depth_feed_connection.h"

using namespace boost::asio::ip;

namespace liquibook { namespace examples {

DepthSession::DepthSession(boost::asio::io_service& ios,
                           DepthFeedConnection* connection)
: connected_(false),
  ios_(ios),
  socket_(ios),
  connection_(connection)
{
  std::cout << "DepthSession ctor" << std::endl;
}

DepthSession::~DepthSession()
{
  std::cout << "DepthSession dtor" << std::endl;
}

void
DepthSession::accept(tcp::endpoint address)
{
  std::cout << "DS accept" << std::endl;
/*
  boost::system::error_code ec;
  tcp::endpoint endpoint(tcp::v4(), 10003);
  acceptor_.reset(new tcp::acceptor(ios_));
  acceptor_->open(endpoint.protocol());
  acceptor_->set_option(boost::asio::socket_base::reuse_address(true), ec);
  acceptor_->bind(endpoint);
  acceptor_->listen();
*/
  acceptor_.reset(new tcp::acceptor(ios_, address));
  acceptor_->async_accept(socket_, 
                          boost::bind(&DepthSession::on_accept, this, _1));
}

void
DepthSession::on_accept(const boost::system::error_code& error)
{
  std::cout << "DS on_accept, error " << error << std::endl;
  if (!error) {
    connected_ = true;
  }
  connection_->on_accept(this, error);
}

bool
DepthSession::send_incr_update(const std::string& symbol,
                               WorkingBufferPtr& buf)
{
std::cout << "send_incr_update" << std::endl;
  bool sent = false;
  // If the session has been started for this symbol
  if (sent_symbols_.find(symbol) != sent_symbols_.end()) {
    SendHandler send_handler = boost::bind(&DepthSession::on_send,
                                           this, buf, _1, _2);
    boost::asio::const_buffers_1 buffer(
        boost::asio::buffer(buf->begin(), buf->size()));
    socket_.async_send(buffer, 0, send_handler);
    sent = true;
  }
  return sent;
}

void
DepthSession::send_full_update(const std::string& symbol,
                               WorkingBufferPtr& buf)
{
std::cout << "send_full_update" << std::endl;
  // Mark this symbols as sent
  std::pair<StringSet::iterator, bool> result = sent_symbols_.insert(symbol);

  // If this symbol is new for the session
  if (result.second) {
    size_t i = 0;
    const unsigned char* start = buf->begin();
    while (i < buf->size()) {
      unsigned short byte = start[i++];
      std::cout << byte << " ";
    }
    // Perform the send
    SendHandler send_handler = boost::bind(&DepthSession::on_send,
                                           this, buf, _1, _2);
    boost::asio::const_buffers_1 buffer(
        boost::asio::buffer(buf->begin(), buf->size()));
    socket_.async_send(buffer, 0, send_handler);
  }
}

void
DepthSession::on_send(WorkingBufferPtr wb,
                      const boost::system::error_code& error,
                      std::size_t bytes_transferred)
{
  if (error) {
    std::cout << "Error " << error << " sending message" << std::endl;
    connected_ = false;
  } else {
    std::cout << "Sent" << std::endl;
  }

  // Keep buffer for later
  connection_->on_send(wb, error, bytes_transferred);
}

DepthFeedConnection::DepthFeedConnection(int argc, const char* argv[])
: connected_(false),
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
  tcp::endpoint endpoint(address::from_string("127.0.0.1"), 10003);
  socket_.async_connect(endpoint, boost::bind(&DepthFeedConnection::on_connect,
                                              this, _1));
}

void
DepthFeedConnection::accept()
{

  std::cout << "DFC accept" << std::endl;
  DepthSession* session = new DepthSession(ios_, this);
  tcp::endpoint address(address::from_string("127.0.0.1"), 10003);
  session->accept(address);

/*
  tcp::endpoint endpoint(address::from_string("127.0.0.1"), 10003);
  tcp::acceptor acceptor(ios_, endpoint);
  acceptor.async_accept(socket_, boost::bind(&DepthFeedConnection::on_accept,
                                             this, _1));
*/
}

void
DepthFeedConnection::run()
{
  std::cout << "DFC run" << std::endl;

  // Keep on running
  work_ptr_.reset(new boost::asio::io_service::work(ios_));
  ios_.run();
  std::cout << "DFC run returning" << std::endl;
}

void
DepthFeedConnection::set_message_handler(MessageHandler handler)
{
  msg_handler_ = handler;
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
DepthFeedConnection::send_buffer(WorkingBufferPtr& buf)
{
  if (connected_) {
    SendHandler send_handler = boost::bind(&DepthFeedConnection::on_send,
                                           this, buf, _1, _2);
    boost::asio::const_buffers_1 buffer(
        boost::asio::buffer(buf->begin(), buf->size()));
    socket_.async_send(buffer, 0, send_handler);
  } else {
    // Just put back in unused
    unused_send_buffers_.push_back(buf);
  }
}

bool
DepthFeedConnection::send_incr_update(const std::string& symbol,
                                      WorkingBufferPtr& buf)
{
  bool any_new = false;
  // For each session
  Sessions::iterator session;
  for (session = sessions_.begin(); session != sessions_.end(); ) {
    // If the session is connected
    if ((*session)->connected()) {
      // send on that session
      if (!(*session)->send_incr_update(symbol, buf)) {
        any_new = true;
      }
      ++session;
    } else {
      // Remove the session
      session = sessions_.erase(session);
    }
  }
}

void
DepthFeedConnection::send_full_update(const std::string& symbol,
                                      WorkingBufferPtr& buf)
{
  // For each session
  Sessions::iterator session;
  for (session = sessions_.begin(); session != sessions_.end(); ) {
    // If the session is connected
    if ((*session)->connected()) {
      // conditionally send on that session
      (*session)->send_full_update(symbol, buf);
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
    std::cout << "on_connect" << std::endl;
    connected_ = true;
    // TODO - if handler
    issue_read();
  } else {
    std::cout << "on_connect, error=" << error << std::endl;
    sleep(3);
    // Try again
    connect();
  }
}

void
DepthFeedConnection::on_accept(DepthSession* session,
                               const boost::system::error_code& error)
{
  if (!error) {
    std::cout << "on_accept" << std::endl;
    sessions_.push_back(session);
  } else {
    std::cout << "DFC on_accept, error=" << error << std::endl;
    delete session;
    sleep(2);
  }
  // TODO - accept again
  //accept();
}

void
DepthFeedConnection::on_receive(BufferPtr bp,
                                const boost::system::error_code& error,
                                std::size_t bytes_transferred)
{
  if (!error) {
    // Next read
    issue_read();

    unsigned char* start = bp->c_array();
    //std::cout << std::hex << std::setfill('0');
    size_t i = 0;
    while (i < bytes_transferred) {
      unsigned short byte = start[i++];
      std::cout << byte << " ";
    }
    std::cout << std::endl;
    //std::cout << std::setfill(' ') << std::dec << std::endl;;
    // Handle the buffer
    msg_handler_(bp, bytes_transferred);

    // Restore buffer
    unused_recv_buffers_.push_back(bp);
  } else {
    std::cout << "Error " << error << " receiving message" << std::endl;
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
