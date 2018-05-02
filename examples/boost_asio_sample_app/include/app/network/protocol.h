#ifndef INCLUDE_APP_NETWORK_PROTOCOL_H_
#define INCLUDE_APP_NETWORK_PROTOCOL_H_

#include <cstdlib>
#include <stdint.h>

#include <vector>

#include <boost/assert.hpp>


namespace app {

typedef int32_t Header;


#pragma pack(push, 1)


struct Packet {
  size_t size;
  Header header;
};


enum HeaderType {
  kConnectionRequest = 1,
  kConnectionReply = 2
};


// server-to-client
namespace sc {

struct ConnectionReply : public Packet {
  ConnectionReply() : id(0) {
  }

  int64_t id;
};

} // namespace sc


// client-to-server
namespace cs {

struct ConnectionRequest : public Packet {
  ConnectionRequest() : success(false) {
  }

  bool success;
};

}  // namespace cs

#pragma pack(pop)

}  // namespace app

#endif  // INCLUDE_APP_NETWORK_PROTOCOL_H_