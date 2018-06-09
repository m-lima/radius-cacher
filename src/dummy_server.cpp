//
// Created by Marcelo Lima on 9/6/18.
//

#include "dummy_server.hpp"

#include <array>

#include <mfl/out.hpp>
#include <boost/asio.hpp>

#include <ctime>
#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::udp;

DummyServer::DummyServer(boost::asio::io_service & ioService, unsigned short port) {
  try
  {
    udp::socket socket(ioService, udp::endpoint(udp::v4(), port));

    for (;;)
    {
      boost::array<char, 4096> recv_buf;
      udp::endpoint remote_endpoint;
      boost::system::error_code error;
      socket.receive_from(boost::asio::buffer(recv_buf),
          remote_endpoint, 0, error);

      if (error && error != boost::asio::error::message_size)
        throw boost::system::system_error(error);

      boost::system::error_code ignored_error;

      mfl::out::println("Received: {}", std::string(recv_buf.begin(), recv_buf.end()));
    }
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
}
