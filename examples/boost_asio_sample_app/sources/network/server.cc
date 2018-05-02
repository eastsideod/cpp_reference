#include "include/app/network/server.h"

#include "include/app/network/protocol.h"
#include "include/app/common/random.h"


using app::ServerProperty;
using app::asio::Buffer;
using app::asio::TcpClient;
using app::asio::TcpServer;

using boost::shared_ptr;


namespace app {

namespace {

struct ConnectionRequestHandler : public Handler<cs::ConnectionRequest> {
  virtual void Handle(
      const boost::shared_ptr<TcpClient> &client,
      const cs::ConnectionRequest &request) const override {
    if (not request.success) {
      DLOG("failed to connect.");
      return;
    }

    int64_t id = GenerateRandomInt64();
    DLOG("generate id=" + std::to_string(id));

    sc::ConnectionReply reply;
    reply.header = HeaderType::kConnectionReply;
    reply.id = id;
    reply.size = sizeof(reply);

    shared_ptr<Buffer> buffer = boost::make_shared<Buffer>();
    memcpy(reinterpret_cast<void *>(buffer->data()),
          &reply, sizeof(reply));

    client->Send(buffer, sizeof(reply));
  }
};

}  // unnamed namespace


void AppServer::OnAccepted(const shared_ptr<TcpClient> &client,
                           const boost::system::error_code &error) {
  DLOG("OnAccepted.");
}


void AppServer::OnReceived(const shared_ptr<TcpClient> &client,
                           const shared_ptr<Buffer> &buffer,
                           const boost::system::error_code &error,
                           const size_t read_bytes) {
  DLOG("OnReceived");

  size_t size = 0;
  char *buffer_ptr = reinterpret_cast<char *> (buffer->data());
  memcpy(&size, buffer_ptr, sizeof(size_t));

  // TODO: check size and read_byte(has more recv data)
  BOOST_ASSERT(size > 0);

  int32_t header = 0;
  memcpy(&header, buffer_ptr + sizeof(size_t), sizeof(int32_t));

  DeserializerMap::const_iterator itr = deserializer_map_.find(header);

  if (itr == deserializer_map_.cend()) {
    return;
  }

  BOOST_ASSERT(itr->second);
  itr->second->Deserialize(header, client, buffer);
}


void AppServer::Start(const ServerProperty &property) {
  server_ = boost::make_shared<asio::TcpServer>(property);
  server_->RegisterAcceptHandler(
      bind(&AppServer::OnAccepted, this, _1, _2));

  server_->RegisterReceiveHandler(
      bind(&AppServer::OnReceived, this,  _1, _2, _3, _4));

  boost::shared_ptr<Handler<cs::ConnectionRequest> > conn_req_handler =
      boost::make_shared<ConnectionRequestHandler>();
  RegisterHandler(HeaderType::kConnectionRequest, conn_req_handler);
  server_->Start();
}

}  // namespace app