#include "include/app/network/internal/asio_server.h"

#include <boost/uuid/uuid_generators.hpp>


namespace app {

namespace asio {

namespace {

const shared_ptr<Buffer> kEmptyBuffer;
const size_t kEmptyBufferLength = 0;
const shared_ptr<TcpClient> kNullTcpClient;

boost::asio::io_service the_io_service(4);


TcpSocket CreateSocket() {
  return TcpSocket(the_io_service);
}


void NullCallback(const shared_ptr<Buffer> &,
                  const boost::system::error_code &,
                  const size_t) {
}

}  // unnamed namespace


const TcpClient::Callback TcpClient::kNullCallback = NullCallback;


TcpClient::TcpClient() : socket_(CreateSocket()), uuid_(Uuid::kNilUuid) {
}


void TcpClient::Close() {
  socket_.close();
}


void TcpClient::Connect(const string &ip, const uint16_t port,
                        const Callback &cb) {
  // TODO: endpoint condition check
  boost::asio::ip::tcp::endpoint ep(
      boost::asio::ip::address::from_string(ip), port);

  auto on_connected = [cb](
      const boost::system::error_code &error) {
    cb(boost::make_shared<Buffer>(), error, 0);
  };

  // TODO: append timeout.
  socket_.async_connect(ep, on_connected);
}


void TcpClient::Receive(const shared_ptr<Buffer> &buffer,
                        const size_t buffer_size,
                        const Callback &cb) {
  typedef function<void(
      const boost::system::error_code &,
      const size_t)> ReceiveCb;

  BOOST_ASSERT(buffer);
  BOOST_ASSERT(buffer_size > 0);
  BOOST_ASSERT(cb);

  ReceiveCb on_received =
      bind(&TcpClient::OnReceived, shared_from_this(),
           buffer, _1, _2, cb);

  socket_.async_read_some(
      boost::asio::buffer(
          buffer->data(),
          buffer_size),
      on_received);
}


void TcpClient::Send(const shared_ptr<Buffer> &buffer,
                     const size_t buffer_size,
                     const Callback &cb) {
  typedef function<void(
      const boost::system::error_code &,
      const size_t)> SendCb;

  BOOST_ASSERT(buffer && cb);
  BOOST_ASSERT(buffer_size > 0);
  BOOST_ASSERT(not buffer->empty());

  SendCb on_sent =
      bind(&TcpClient::OnSent, shared_from_this(), buffer, _1, _2, cb);

  socket_.async_write_some(
      boost::asio::buffer(
          buffer->data(), buffer_size),
      on_sent);
}


void TcpClient::OnReceived(const shared_ptr<Buffer> &buffer,
                           const boost::system::error_code &error,
                           const size_t read_bytes,
                           const Callback &cb) {
  if (error != boost::system::errc::success) {
    DLOG(error.message());
    cb(kEmptyBuffer, error, 0);
    return;
  }

  function<void()> cb_wrapper = bind(cb, buffer, error, read_bytes);
  cb_wrapper();
}


void TcpClient::OnSent(const shared_ptr<Buffer> &buffer,
                       const boost::system::error_code &error,
                       const size_t sent_bytes,
                       const Callback &cb) {
  if (error != boost::system::errc::success) {
    DLOG(error.message());
    cb(kEmptyBuffer, error, 0);
    return;
  }

  function<void()> cb_wrapper = bind(cb, buffer, error, sent_bytes);
  cb_wrapper();
}


void TcpClient::set_uuid(const Uuid &uuid) {
  uuid_ = uuid;
}


TcpSocket &TcpClient::socket() {
  return socket_;
}


const Uuid &TcpClient::uuid() const {
  return uuid_;
}



TcpServer::TcpServer(const ServerProperty &property)
    : property_(property) {

}


ErrorCode TcpServer::Start() {
  BOOST_ASSERT(not acceptor_);

  TcpEndpoint endpoint(boost::asio::ip::tcp::v4(), property_.port);
  acceptor_ = boost::make_shared<TcpAsyncAcceptor>(
      the_io_service, endpoint, true);


  AsyncAccept();
  return kSuccess;
}


void TcpServer::RegisterAcceptHandler(
    const AcceptHandler &handler) {
  BOOST_ASSERT(handler);

  boost::mutex::scoped_lock lock(mutex_);
  accept_handler_ = handler;
}


void TcpServer::RegisterReceiveHandler(
    const ReceiveHandler &handler) {
  BOOST_ASSERT(handler);

  boost::mutex::scoped_lock lock(mutex_);
  receive_handler_ = handler;
}


const shared_ptr<TcpClient> TcpServer::Find(const Uuid &uuid) {
  BOOST_ASSERT(not uuid.IsNil());

  TcpClientMap::const_iterator itr;

  {
    boost::mutex::scoped_lock lock(mutex_);
    itr = clients_.find(uuid.raw_uuid());

    if (itr == clients_.end()) {
      return nullptr;
    }
  }

  return itr->second;
}


void TcpServer::Finalize() {
  BOOST_ASSERT_MSG(false, "Not Implemented.");
}


void TcpServer::AsyncAccept() {
  shared_ptr<TcpClient> tcp_client =
      boost::make_shared<TcpClient>();

  BOOST_ASSERT(tcp_client);

  acceptor_->async_accept(
      tcp_client->socket(),
      bind(&TcpServer::OnAccepted, this, tcp_client,
           boost::asio::placeholders::error));
}


void TcpServer::OnAccepted(const shared_ptr<TcpClient> &client,
                           const boost::system::error_code &error) {
  AsyncAccept();

  if (error != boost::system::errc::success) {
    accept_handler_(kNullTcpClient, error);
    return;
  }

  boost::uuids::random_generator gen;
  boost::uuids::uuid uuid = gen();
  client->set_uuid(uuid);
  bool inserted = false;

  {
    boost::mutex::scoped_lock lock(mutex_);
    inserted = clients_.insert(std::make_pair(uuid, client)).second;
  }

  BOOST_ASSERT(inserted);

  // TODO: replace buffer use pool.
  shared_ptr<Buffer> buffer = boost::make_shared<Buffer>();

  client->Receive(
      buffer, 256,
      bind(&TcpServer::OnReceived, this, client, buffer, _2, _3));

  accept_handler_(client, error);
}


void TcpServer::OnReceived(const shared_ptr<TcpClient> &client,
                           const shared_ptr<Buffer> &buffer,
                           const boost::system::error_code &error,
                           const size_t read_bytes) {
  if (error != boost::system::errc::success) {
    BOOST_ASSERT(false);
  }

  BOOST_ASSERT(error == boost::system::errc::success);

  shared_ptr<Buffer> new_buffer = boost::make_shared<Buffer>();

  client->Receive(
      new_buffer, 256,
      bind(&TcpServer::OnReceived, this, client, _1, _2, _3));

  receive_handler_(client, buffer, error, read_bytes);
}


void InitializeAsio() {
  const size_t result = the_io_service.run();

  BOOST_ASSERT(result > 0);
}

}  // namespace asio


ServerProperty::ServerProperty(const std::string &_ip,
                               const uint16_t _port,
                               const bool _is_tcp)
    : ip(_ip), port(_port), is_tcp(_is_tcp) {
}

}  // namespace app