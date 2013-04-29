#ifndef example_depth_feed_connection_h
#define example_depth_feed_connection_h

#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>
#include <Common/WorkingBuffer.h>
#include <deque>
#include <set>

namespace liquibook { namespace examples {
  typedef boost::shared_ptr<QuickFAST::WorkingBuffer> WorkingBufferPtr;
  typedef std::deque<WorkingBufferPtr> WorkingBuffers;
  typedef boost::array<unsigned char, 128> Buffer;
  typedef boost::shared_ptr<Buffer> BufferPtr;
  typedef boost::function<void (BufferPtr)> MessageHandler;
  typedef boost::function<void (const boost::system::error_code& error,
                                std::size_t bytes_transferred)> SendHandler;
  typedef boost::function<void (const boost::system::error_code& error,
                                std::size_t bytes_transferred)> RecvHandler;

  class DepthFeedConnection;
  // Socket connection
  class DepthSession {
  public:
    DepthSession(boost::asio::io_service& ios,
                 DepthFeedConnection* connection);

    ~DepthSession();

    void accept();
  private:       
    bool connected_;
    boost::asio::io_service& ios_;
    boost::asio::ip::tcp::socket socket_;
    DepthFeedConnection* connection_;

    typedef std::set<std::string> StringSet;
    StringSet sent_symbols_;

    void on_accept(const boost::system::error_code& error);
  };

  // Create a Session when accepting or connecting.  Then if fails, delete.

  class DepthFeedConnection {
  public:
    DepthFeedConnection(int argc, const char* argv[]);
    ~DepthFeedConnection();

    void connect();
    void accept();
    void run();

    void set_message_handler(MessageHandler msg_handler);

    BufferPtr        reserve_recv_buffer();
    WorkingBufferPtr reserve_send_buffer();

    void send_buffer(WorkingBufferPtr& buf);
    bool send_incremental_update(WorkingBufferPtr& buf);
    void send_full_update(WorkingBufferPtr& buf);

    void on_connect(const boost::system::error_code& error);
    void on_accept(DepthSession* session,
                   const boost::system::error_code& error);
    void on_receive(BufferPtr bp,
                    const boost::system::error_code& error,
                    std::size_t bytes_transferred);
    void on_send(WorkingBufferPtr wb,
                 const boost::system::error_code& error,
                 std::size_t bytes_transferred);

  private:
    typedef std::deque<BufferPtr> Buffers;
    typedef std::vector<DepthSession*> Sessions;
    bool connected_;
    MessageHandler msg_handler_;

    Buffers        unused_recv_buffers_;
    WorkingBuffers unused_send_buffers_;
    Sessions sessions_;
    boost::asio::io_service ios_;
    boost::asio::ip::tcp::socket socket_;
    boost::shared_ptr<boost::asio::io_service::work> work_ptr_;

    void issue_read();
  };
} } // End namespace

#endif
