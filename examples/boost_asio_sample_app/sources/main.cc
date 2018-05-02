#include <iostream>
#include <boost/shared_ptr.hpp>

#include "include/app/common/common.h"
#include "include/app/common/random.h"
#include "include/app/common/uuid.h"
#include "include/app/network/internal/asio_server.h"
#include "include/app/network/server.h"
#include "include/app/network/protocol.h"

#include <array>
#include <thread>


using app::Handler;
using app::ServerProperty;
using app::asio::Buffer;
using app::asio::TcpClient;
using app::asio::TcpServer;
using app::AppServer;

using boost::shared_ptr;

namespace app {


std::vector<boost::shared_ptr<TcpClient> > the_clients;


void OnConnectionReplied(const shared_ptr<TcpClient> &tcp_client,
                         const shared_ptr<Buffer> &buffer,
                         const boost::system::error_code &error,
                         const size_t read_bytes) {
  DLOG("client - OnConnectionReplied.");

  sc::ConnectionReply reply;
  memcpy(&reply, buffer->data(), sizeof(reply));
  DLOG("replied. id=" + std::to_string(reply.id));
}


void OnReceived(const shared_ptr<TcpClient> &tcp_client,
                const shared_ptr<Buffer> &buffer,
                const boost::system::error_code &error,
                const size_t read_bytes) {
  DLOG("client - OnReceived.");
  size_t size = 0;
  char *buffer_ptr = reinterpret_cast<char *> (buffer->data());
  memcpy(&size, buffer_ptr, sizeof(size_t));

  // TODO: check size and read_byte(has more recv data)
  BOOST_ASSERT(size > 0);

  int32_t header = 0;
  memcpy(&header, buffer_ptr + sizeof(size_t), sizeof(int32_t));

  switch (header) {
    case HeaderType::kConnectionReply:
      OnConnectionReplied(tcp_client, buffer, error, read_bytes);
      return;
  }
}



void OnConnected(const shared_ptr<TcpClient> &tcp_client,
                 const shared_ptr<Buffer> &buffer,
                 const boost::system::error_code &error,
                 const size_t bytes_transferred) {
  DLOG("client - OnConnected.");
  shared_ptr<asio::Buffer> read_buffer =
      boost::make_shared<asio::Buffer>();
  tcp_client->Receive(read_buffer, 256,
                      bind(&OnReceived, tcp_client, _1, _2, _3));

  cs::ConnectionRequest request;
  request.size = sizeof(request);
  request.header = HeaderType::kConnectionRequest;
  request.success = true;

  shared_ptr<Buffer> send_buffer =
      boost::make_shared<Buffer>();

  void *buffer_ptr = reinterpret_cast<void *>(send_buffer->data());
  memcpy(buffer_ptr, &request, request.size);
  tcp_client->Send(send_buffer, request.size);
}

}  // namespace app


int main() {
  std::ios::sync_with_stdio(true);
  ServerProperty property("127.0.0.1", 9999, true);
  AppServer app_server;
  app_server.Start(property);

  DLOG("started tcp server.");

  std::thread th([]() {
    sleep(3);
    DLOG("add client.");
    boost::shared_ptr<TcpClient> client =
        boost::make_shared<TcpClient>();
    client->Connect("127.0.0.1", 9999,
                    bind(&app::OnConnected, client, _1, _2, _3));
    app::the_clients.push_back(client);

    while(true) {
      sleep(3);
    }
  });

  app::asio::InitializeAsio();

  return 0;
}