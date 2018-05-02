#ifndef INCLUDE_NETWORK_INTERNAL_ASIO_SERVER_H_
#define INCLUDE_NETWORK_INTERNAL_ASIO_SERVER_H_

#include <boost/asio.hpp>

#include "include/app/common/common.h"
#include "include/app/common/uuid.h"

#include <array>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/uuid/uuid.hpp>


namespace app {

struct ServerProperty {
  ServerProperty(const std::string &ip, const uint16_t port,
                 const bool is_tcp);

  std::string ip;
  uint16_t port;
  bool is_tcp;
};


namespace asio {

typedef boost::asio::ip::tcp::acceptor TcpAsyncAcceptor;
typedef boost::asio::ip::tcp::endpoint TcpEndpoint;
typedef boost::asio::ip::tcp::socket TcpSocket;

typedef std::array<char, 256> Buffer;


enum ErrorCode {
  kSuccess = 0,
  kInvalidProperty = 1001,
  kInvalidIoService = 1002,
};


class TcpClient : public boost::enable_shared_from_this<TcpClient> {
 public:
  typedef function<void(
      const shared_ptr<Buffer> &buffer,
      const boost::system::error_code &/*error*/,
      const size_t /*bytes_transferred*/)> Callback;

  static const Callback kNullCallback;

  TcpClient();

  void Close();
  void Connect(const string &ip, const uint16_t port,
               const Callback &cb=kNullCallback);

  void Receive(const shared_ptr<Buffer> &buffer, const size_t buffer_size,
               const Callback &cb=kNullCallback);
  void Send(const shared_ptr<Buffer> &buffer, const size_t buffer_size,
            const Callback &cb=kNullCallback);

  void set_uuid(const Uuid &uuid);

  TcpSocket &socket();
  const Uuid &uuid() const;
 private:
  void OnReceived(const shared_ptr<Buffer> &buffer,
                  const boost::system::error_code &error,
                  const size_t read_bytes,
                  const Callback &cb);
  void OnSent(const shared_ptr<Buffer> &buffer,
              const boost::system::error_code &error,
              const size_t sent_bytes,
              const Callback &cb);

  TcpSocket socket_;
  bool is_connected_;
  Uuid uuid_;
};


class TcpServer {
 public:
  typedef std::map<boost::uuids::uuid, shared_ptr<TcpClient> >
      TcpClientMap;

  typedef function<void(
      const shared_ptr<TcpClient> &/*tcp_client*/,
      const boost::system::error_code &/*error*/)> AcceptHandler;

  typedef function<void(
      const shared_ptr<TcpClient> &/*tcp_client*/,
      const boost::shared_ptr<Buffer> &/*buffer*/,
      const boost::system::error_code &/*error*/,
      const size_t /*read_bytes*/)> ReceiveHandler;

  TcpServer(const ServerProperty &property);

  ErrorCode Start();

  void RegisterAcceptHandler(const AcceptHandler &handler);
  void RegisterReceiveHandler(const ReceiveHandler &handler);
  const shared_ptr<TcpClient> Find(const Uuid &uuid);
  void Finalize();

 private:
  void AsyncAccept();
  void OnAccepted(const shared_ptr<TcpClient> &client,
                  const boost::system::error_code &error);

  void OnReceived(const shared_ptr<TcpClient> &client,
                  const shared_ptr<Buffer> &buffer,
                  const boost::system::error_code &error,
                  const size_t read_bytes);

  AcceptHandler accept_handler_;
  ReceiveHandler receive_handler_;

  ServerProperty property_;

  boost::mutex mutex_;
  TcpClientMap clients_;
  shared_ptr<TcpAsyncAcceptor> acceptor_;
};


void InitializeAsio();

}  // namespace asio

}  // namespace app


#endif  // INCLUDE_NETWORK_INTERNAL_ASIO_SERVER_H_