//
// Created by Marcelo Lima on 08/06/2018.
//

#pragma once

#include <array>
#include <memory>
#include <string>

#include <boost/asio.hpp>

#include "config.hpp"
#include "logger.hpp"

namespace server::listener {
  template <typename P, typename I>
  void receive(boost::asio::ip::udp::socket & socket,
               I callbackCurrent,
               I callbackBegin,
               I callbackEnd,
               std::shared_ptr<P> parser);
}

/**
 * Callback wrapper to execute once the buffer is ready
 *
 * @tparam B The buffer type
 */
template <typename B>
struct Callback {
  boost::asio::ip::udp::endpoint mEndpoint;
  B mBuffer;

  template <typename C>
  auto operator()(const boost::system::error_code & errorCode,
                  std::size_t byteCount,
                  std::shared_ptr<C> callback) const {
    auto bufferBegin = std::cbegin(mBuffer);
    auto bufferEnd = std::cend(mBuffer);
    return (*callback)(
        mEndpoint,
        errorCode,
        byteCount,
        bufferBegin,
        bufferBegin +
        std::min(byteCount, static_cast<std::size_t>(std::max(0L, std::distance(bufferBegin, bufferEnd))))
    );
  }

};

/**
 * Main server to handle UDP connections
 *
 * Works with 10 rolling callback handlers shared across all threads
 * Each callback handler has 8KB buffer for the packet. This is currently not customizable to
 * harness std::array compile-time stack allocation
 */
class Server {
public:
  static constexpr auto BUFFER_SIZE = 1024 * 8;
  static constexpr unsigned short CALLBACK_COUNT = 10;

  using boostUdp = boost::asio::ip::udp;
  using Buffer = std::array<std::uint8_t, BUFFER_SIZE>;
  using CallbackList = std::array<Callback<Buffer>, CALLBACK_COUNT>;

private:

  class Listener {
  public:

    /**
     * Attaches a boost::asio::io_service to this listener and starts listening
     *
     * @tparam P the packet parser type
     * @param ioService the listening service
     * @param port the port to listen in
     * @param parser the packet parser
     */
    template <typename P>
    Listener(boost::asio::io_service & ioService, unsigned short port, std::shared_ptr<P> parser)
        : mSocket{ioService, boostUdp::endpoint{boostUdp::v4(), port}} {
      server::listener::receive(mSocket, mCallbackList.begin(), mCallbackList.begin(), mCallbackList.end(), parser);
    }

    boostUdp::socket mSocket;
    Server::CallbackList mCallbackList;
  };

public:
  Server(config::Server config)
      : mConfig{std::move(config)} {}

  /**
   * Starts listening and offloading packets to P. This method will block
   *
   * @tparam P the packet parser type
   * @param parser the packet parser
   */
  template <typename P>
  void run(std::shared_ptr<P> parser) {
    boost::asio::io_service ioService;

    Listener listener{ioService, mConfig.port, parser};

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

  /**
   * Must declare since compiler will omit due to copy constructor deletion
   */
  ~Server() = default;

  /**
   * Delete copy constructor
   */
  Server(const Server &) = delete;

  /**
   * Delete copy
   */
  void operator=(const Server &) = delete;

private:
  const config::Server mConfig;
};

namespace server::listener {
/**
 * A rolling receiver of packets that offloads to the callback wrapper rolling list
 *
 * @tparam P the packet parser type
 * @tparam I the iterator type for the callback wrapper rolling list
 * @param socket the socket for the connection
 * @param callbackCurrent the current callback wrapper
 * @param callbackBegin the first callback wrapper in the rolling list
 * @param callbackEnd the end-of-list for the callback wrapper rolling list
 * @param parser the packet parser
 */
  template <typename P, typename I>
  void receive(boost::asio::ip::udp::socket & socket,
               I callbackCurrent,
               I callbackBegin,
               I callbackEnd,
               std::shared_ptr<P> parser) {
    try {
      socket.async_receive_from(
          boost::asio::buffer(callbackCurrent->mBuffer, Server::BUFFER_SIZE),
          callbackCurrent->mEndpoint,
          [&socket, callbackCurrent, callbackBegin, callbackEnd, parser]
              (const boost::system::error_code & errorCode, std::size_t bytesReceived) {
            auto callbackNext = callbackCurrent + 1;

            // Infinite loop for listening
            receive(socket,
                    (callbackNext != callbackEnd)
                    ? callbackNext
                    : callbackBegin,
                    callbackBegin,
                    callbackEnd,
                    parser);

            (*callbackCurrent)(errorCode,
                               bytesReceived,
                               parser);
//                               parse<decltype(callbackCurrent->mEndpoint), Server::Buffer::const_iterator>);
          }
      );
    } catch (std::exception & e) {
      logger::errPrintln<logger::WARNING>("Server::Listener::receive: "
                                          "exception caught when executing receive: {:s}",
                                          e.what());
    }
  }
}
