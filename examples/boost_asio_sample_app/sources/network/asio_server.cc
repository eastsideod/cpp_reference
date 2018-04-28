#include "include/app/network/internal/asio_server.h"

#include <boost/uuid/uuid_generators.hpp>


namespace app {

namespace asio {

namespace {

const Buffer kEmptyBuffer;


boost::asio::io_service the_io_service;


TcpSocket CreateSocket() {
  return TcpSocket(the_io_service);
}


void NullCallback(const Buffer &,const boost::system::error_code &,
                  const size_t) {
}

} // unnamed namespace


const TcpClient::Callback TcpClient::kNullCallback = NullCallback;


TcpClient::TcpClient() : socket_(CreateSocket()) {
}


void TcpClient::Close() {
  BOOST_ASSERT_MSG(false, "Not Implemented.");
}


void TcpClient::Connect(const string &ip, const uint16_t port,
                        const Callback &cb) {
  boost::asio::ip::tcp::endpoint ep(
    boost::asio::ip::address::from_string(ip), port);

  // TODO: endpoint condition check
  socket_.async_connect(
      ep, bind(cb, kEmptyBuffer,
               boost::asio::placeholders::error, 0));
}


void TcpClient::Receive(Buffer *buffer, const Callback &cb) {
  BOOST_ASSERT(buffer && cb);
  BOOST_ASSERT(not buffer->empty());

  socket_.async_read_some(
      boost::asio::buffer(*buffer, buffer->size()),
      bind(cb, *buffer, _1, _2));
}


void TcpClient::Send(const Buffer &buffer, const Callback &cb) {
  BOOST_ASSERT(cb);
  BOOST_ASSERT(not buffer.empty());

  // TODO: replacement buffer not copyable
  std::vector<char> copied_buffer(buffer.begin(), buffer.end());

  socket_.async_write_some(
      boost::asio::buffer(copied_buffer, buffer.size()),
      bind(cb, buffer, _1, _2));
}


TcpSocket &TcpClient::socket() {
  return socket_;
}


ErrorCode TcpServer::Initialize(const ServerProperty &property) {
  property_ = property;
  return kSuccess;
}


ErrorCode TcpServer::Start() {
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
  // TODO: make multiple sockets waiting for accept;
  shared_ptr<TcpClient> tcp_client =
      boost::make_shared<TcpClient>();

  BOOST_ASSERT(tcp_client);

  TcpEndpoint endpoint(boost::asio::ip::tcp::v4(), property_.port);
  TcpAsyncAcceptor acceptor(the_io_service, endpoint);

  acceptor.async_accept(
      tcp_client->socket(),
      bind(&TcpServer::OnAccepted, this, tcp_client,
           boost::asio::placeholders::error));
}


void TcpServer::OnAccepted(const shared_ptr<TcpClient> &client,
                           const boost::system::error_code &error) {

  boost::uuids::random_generator gen;
  boost::uuids::uuid uuid = gen();
  bool inserted = false;

  {
    boost::mutex::scoped_lock lock(mutex_);
    inserted = clients_.insert(std::make_pair(uuid, client)).second;
  }

  AsyncAccept();

  // TODO: error handling.
  if (not inserted) {
    return;
  }

  // TODO: replace buffer.
  std::vector<char> buffer;
  buffer.reserve(256);

  client->Receive(
      &buffer, bind(&TcpServer::OnReceived, this,
                    client, _1, _2, _3));

  accept_handler_(client, error);

}


void TcpServer::OnReceived(const shared_ptr<TcpClient> &client,
                           const Buffer &buffer,
                           const boost::system::error_code &error,
                           const size_t read_bytes) {
  // TODO: need pooling buffer.
  std::vector<char> new_buffer;
  new_buffer.reserve(256);

  client->Receive(
      &new_buffer,
      bind(&TcpServer::OnReceived, this, client, _1, _2, _3));

  receive_handler_(client, error, read_bytes);
}


void InitializeAsio() {
  the_io_service.run();
}

}  // namespace asio


ServerProperty::ServerProperty(const std::string &_ip,
                               const uint16_t _port,
                               const bool _is_tcp)
    : ip(_ip), port(_port), is_tcp(_is_tcp) {
}

}  // namespace app