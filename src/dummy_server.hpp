//
// Created by Marcelo Lima on 9/6/18.
//

#include <boost/asio.hpp>

struct DummyServer {
  DummyServer(boost::asio::io_service & ioService, unsigned short port);
};
