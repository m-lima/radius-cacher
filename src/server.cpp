//
// Created by Marcelo Lima on 08/06/2018.
//

#include "server.hpp"

#include <string>
#include <optional>

#include <libmemcached/memcached.hpp>

#include "radius.hpp"
#include "logger.hpp"

using boostUdp = boost::asio::ip::udp;

namespace {
  template <typename E, typename I>
  void parse(const E &,
             const boost::system::error_code & errorCode,
             std::size_t bytesReceived,
             I begin,
             I end) {}

  /**
   * A rolling receiver of packets that offloads to the callback wrapper rolling list
   *
   * @param socket the socket for the connection
   * @param callbackCurrent the current callback wrapper
   * @param callbackBegin the first callback wrapper in the rolling list
   * @param callbackEnd the end-of-list for the callback wrapper rolling list
   */
  void receive(boostUdp::socket & socket,
               Server::CallbackList::iterator callbackCurrent,
               Server::CallbackList::iterator callbackBegin,
               Server::CallbackList::iterator callbackEnd) {
    try {
      socket.async_receive_from(
          boost::asio::buffer(callbackCurrent->mBuffer, Server::BUFFER_SIZE),
          callbackCurrent->mEndpoint,
          [&socket, callbackCurrent, callbackBegin, callbackEnd]
              (const boost::system::error_code & errorCode, std::size_t bytesReceived) {
            auto callbackNext = callbackCurrent + 1;

            // Infinite loop for listening
            receive(socket,
                    (callbackNext != callbackEnd)
                    ? callbackNext
                    : callbackBegin,
                    callbackBegin,
                    callbackEnd);

            (*callbackCurrent)(errorCode,
                               bytesReceived,
                               parse<decltype(callbackCurrent->mEndpoint), Server::Buffer::const_iterator>);
          }
      );
    } catch (std::exception & e) {
      logger::errPrintln<logger::WARNING>("Server::Listener::receive: "
                                          "exception caught when executing receive: {:s}",
                                          e.what());
    }
  }
}

/**
 * Attaches a boost::asio::io_service to this listener and starts listening
 *
 * @param ioService the listening service
 * @param port the port to listen in
 */
Server::Listener::Listener(boost::asio::io_service & ioService, unsigned short port)
    : mSocket{ioService, boostUdp::endpoint{boostUdp::v4(), port}} {
  receive(mSocket, mCallbackList.begin(), mCallbackList.begin(), mCallbackList.end());
}

/**
 * Starts listening. This method will block
 */
void Server::run() {
  boost::asio::io_service ioService;

  Listener listener{ioService, mConfig.port};

  if (mConfig.threadPoolSize == 1) {
    logger::println<logger::LOG>("Server::run: listening on UDP {:d} on a single thread", mConfig.port);
    ioService.run();
  } else {
    std::vector<std::thread> threadPool;
    threadPool.reserve(mConfig.threadPoolSize);

    for (unsigned short i = 0; i < mConfig.threadPoolSize; ++i) {
      threadPool[i] = std::thread{[&ioService]() { ioService.run(); }};
    }

    logger::println<logger::LOG>("Server::run: listening on UDP {:d} on {:d} threads",
                                 mConfig.port,
                                 mConfig.threadPoolSize);

    for (auto & t : threadPool) {
      t.join();
    }
  }
  logger::println<logger::LOG>("Server::run: server stopped");
}

