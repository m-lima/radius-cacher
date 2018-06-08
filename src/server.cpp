//
// Created by Marcelo Lima on 8/6/18.
//

#include "server.hpp"

#include <string>

#include <mfl/out.hpp>

using boostUdp = boost::asio::ip::udp;

namespace {
  template <typename E, typename I>
  void print(const E &,
             const boost::system::error_code & errorCode,
             std::size_t bytesReceived,
             I begin,
             I end)
  {
    mfl::out::println(": Buffer: {:s}", std::string(begin, end));
    mfl::out::println(": Error code: {}", errorCode);
    mfl::out::println(": Bytes: {:d}", bytesReceived);
    mfl::out::println();
  }

  static void receive(boostUdp::socket & socket,
                      Server::CallbackList::iterator callbackCurrent,
                      Server::CallbackList::iterator callbackBegin,
                      Server::CallbackList::iterator callbackEnd) {
    socket.async_receive_from(
        boost::asio::buffer(callbackCurrent->mBuffer, Server::BUFFER_SIZE),
        callbackCurrent->mEndpoint,
        [&socket, callbackCurrent, callbackBegin, callbackEnd]
            (const boost::system::error_code & errorCode, std::size_t bytesReceived){
          auto callbackNext = callbackCurrent + 1;
          receive(socket,
                  (callbackNext != callbackEnd)
                  ? callbackNext
                  : callbackBegin,
                  callbackBegin,
                  callbackEnd);
          (*callbackCurrent)(errorCode,
                             bytesReceived,
                             print<decltype(callbackCurrent->mEndpoint), Server::Buffer::const_iterator>);
        }
    );
  }
}

Server::Server(boost::asio::io_service & ioService, unsigned short port)
  : mSocket{ioService, boostUdp::endpoint{boostUdp::v4(), port}} {
    receive(mSocket, mCallbackList.begin(), mCallbackList.begin(), mCallbackList.end());
}

