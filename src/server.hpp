//
// Created by Marcelo Lima on 8/6/18.
//

#pragma once

#include <boost/asio.hpp>

class Server {
private:
  boost::asio::ip::udp::socket mSocket;

public:
  Server(boost::asio::io_service & ioService, unsigned short port);
};
