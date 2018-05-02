#ifndef INCLUDE_APP_NETWORK_SERVER_H_
#define INCLUDE_APP_NETWORK_SERVER_H_


#include "include/app/network/internal/asio_server.h"

#include "include/app/network/protocol.h"

#include <boost/thread/mutex.hpp>


namespace app {

template <typename T>
struct Handler {
  virtual void Handle(
      const boost::shared_ptr<asio::TcpClient> &, const T &) const {
    BOOST_ASSERT_MSG(false, "Not implemented handler.");
  }
};


class AppServer {
 public:
  void Start(const ServerProperty &property);

  template <typename T>
  void RegisterHandler(const int32_t header,
                       const boost::shared_ptr<Handler<T> > &handler) {
    bool result = false;
    boost::shared_ptr<Deserializer> deserializer =
        boost::make_shared<DeserializerImpl<T>> (handler);

    {
      boost::mutex::scoped_lock lock(mutex_);

      result = deserializer_map_.insert(
          std::make_pair(header, deserializer)).second;

      BOOST_ASSERT(result);
    }
  }

 private:
  struct Deserializer {
    virtual void Deserialize(
        const int32_t header,
        const boost::shared_ptr<asio::TcpClient> &client,
        const boost::shared_ptr<asio::Buffer> &buffer) = 0;
  };

  template <typename T>
  struct DeserializerImpl : public Deserializer {
    DeserializerImpl(const boost::shared_ptr<Handler<T> > &_handler)
        : handler(_handler) {
    }

    virtual void Deserialize(
        const int32_t header,
        const boost::shared_ptr<asio::TcpClient> &client,
        const boost::shared_ptr<asio::Buffer> &buffer) {
      T message;
      memcpy(&message, buffer->data(), sizeof(T));
      handler->Handle(client, message);
    }

    const boost::shared_ptr<Handler<T> > handler;
  };

  typedef std::map<const int32_t, boost::shared_ptr<Deserializer> >
      DeserializerMap;

  void OnAccepted(const boost::shared_ptr<asio::TcpClient> &client,
                  const boost::system::error_code &error);


  void OnReceived(const boost::shared_ptr<asio::TcpClient> &client,
                  const boost::shared_ptr<asio::Buffer> &buffer,
                  const boost::system::error_code &error,
                  const size_t read_bytes);

  boost::mutex mutex_;
  boost::shared_ptr<asio::TcpServer> server_;
  DeserializerMap deserializer_map_;
};

}  // namespace app


#endif  // INCLUDE_APP_NETWORK_SERVER_H_