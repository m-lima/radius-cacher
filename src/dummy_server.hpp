//
// Created by Marcelo Lima on 9/6/18.
//

#include <array>

#include <mfl/out.hpp>
#include <boost/asio.hpp>

#include <ctime>
#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::udp;

std::string make_daytime_string()
{
  using namespace std; // For time_t, time and ctime;
  time_t now = time(0);
  return ctime(&now);
}

class DummyServer {
public:
  DummyServer(boost::asio::io_service & ioService, unsigned short port) {
    try
    {
      udp::socket socket(ioService, udp::endpoint(udp::v4(), port));

      for (;;)
      {
        boost::array<char, 1> recv_buf;
        udp::endpoint remote_endpoint;
        boost::system::error_code error;
        socket.receive_from(boost::asio::buffer(recv_buf),
                            remote_endpoint, 0, error);

        if (error && error != boost::asio::error::message_size)
          throw boost::system::system_error(error);

        std::string message = make_daytime_string();

        boost::system::error_code ignored_error;
        socket.send_to(boost::asio::buffer(message),
                       remote_endpoint, 0, ignored_error);
      }
    }
    catch (std::exception& e)
    {
      std::cerr << e.what() << std::endl;
    }
//    boost::asio::ip::tcp::socket socket{ioService, boost::asio::ip::tcp::endpoint{boost::asio::ip::tcp::v4(), port}};
//
//    for (;;) {
//      std::array<char, 8> recv_buf;
//      boost::asio::ip::tcp::endpoint remote_endpoint;
//      boost::system::error_code error;
//      socket.receive(boost::asio::buffer(recv_buf));
////      socket.receive_from(boost::asio::buffer(recv_buf),
////                          remote_endpoint, 0, error);
//
//      if (error && error != boost::asio::error::message_size)
//        throw boost::system::system_error(error);
//
//      boost::system::error_code ignored_error;
//      socket.send(boost::asio::buffer("yo!"));
////      socket.send_to(boost::asio::buffer("yo!"),
////                     remote_endpoint, 0, ignored_error);
//
//      mfl::out::println("Received: {}", std::string(recv_buf.begin(), recv_buf.end()));
//    }
  }
};
