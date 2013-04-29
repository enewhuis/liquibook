
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
  
}

void
DepthSession::accept()
{
  tcp::endpoint endpoint(address::from_string("127.0.0.1"), 10003);
  tcp::acceptor acceptor(ios_, endpoint);
  acceptor.async_accept(socket_, boost::bind(&DepthSession::on_accept,
                                             this, _1));
}

void
DepthSession::on_accept(const boost::system::error_code& error)
{
  if (!error) {
    std::cout << "on_accept" << std::endl;
    connected_ = true;
  } else {
    std::cout << "on_accept, error=" << error << std::endl;
  }
  // Accept next connection
  connection_->accept();
}

DepthFeedConnection::DepthFeedConnection(int argc, const char* argv[])
: connected_(false),
  socket_(ios_)
{
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
  
  tcp::endpoint endpoint(address::from_string("127.0.0.1"), 10003);
  tcp::acceptor acceptor(ios_, endpoint);
  acceptor.async_accept(socket_, boost::bind(&DepthFeedConnection::on_accept,
                                             this, _1));
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
DepthFeedConnection::send_incremental_update(WorkingBufferPtr& buf)
{
  return false;
/*
  bool any_new = false;
  // For each session
  Sessions::iterator session;
  for (session = sessions_.begin(); session != sessions_.end(); ) {
    // If the session is connected
    if (session->connected()) {
      // send on that session
// TODO SEND BUFFER
      //if (*session->send_incremental_buffer(buf)) {
        any_new = false;
      //}
      ++session;
    } else {
      // Remove the session
      session = sessions_.erase(session);
    }
  }
*/

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
DepthFeedConnection::on_accept(const boost::system::error_code& error)
{
  if (!error) {
    std::cout << "on_accept" << std::endl;
    connected_ = true;
  } else {
    std::cout << "on_accept, error=" << error << std::endl;
  }
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
    int i = 0;
    while (i < bytes_transferred) {
      unsigned short byte = start[i++];
      std::cout << byte << " ";
    }
    std::cout << std::endl;
    //std::cout << std::setfill(' ') << std::dec << std::endl;;
    // Handle the buffer
    msg_handler_(bp);

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
  if (error) {
    std::cout << "Error " << error << " sending message" << std::endl;
  }

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
