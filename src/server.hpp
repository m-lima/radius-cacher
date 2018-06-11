//
// Created by Marcelo Lima on 08/06/2018.
//

#pragma once

#include <array>
#include <vector>
#include <memory>
#include <string>

#include <boost/asio.hpp>

#include "config.hpp"
#include "logger.hpp"
#include "cache.hpp"

/**
 * Callback wrapper to execute once the buffer is ready
 * Will offload to the parser to know how to take action
 *
 * @tparam B The buffer type
 */
template <typename B>
struct Callback {
  boost::asio::ip::udp::endpoint mEndpoint;
  B mBuffer;
  Cache mCache;

  explicit Callback(const Config::Cache & config)
      : mCache{config} {}

  template <typename P>
  auto operator()(const boost::system::error_code & errorCode,
                  std::size_t byteCount,
                  const P & parser) {
    auto bufferBegin = std::cbegin(mBuffer);
    auto bufferEnd = std::cend(mBuffer);
    return parser(byteCount,
                  bufferBegin,
                  bufferBegin +
                  std::min(byteCount,
                           static_cast<std::size_t>(std::max(0L, std::distance(bufferBegin, bufferEnd)))),
                  &mCache
    );
  }

};

/**
 * Main server to handle UDP connections
 *
 * Works with rolling callback handlers shared across all threads.
 * Each callback handler has 8KB buffer for the packet by default.
 *
 * This is currently customizable at compile-time to harness std::array stack allocation
 * Use:
 * cmake -DRC_BUFFER_SIZE=<value> (...)
 */
class Server {
public:

#ifndef RC_BUFFER_SIZE
  #define RC_BUFFER_SIZE 1024 * 8
#endif

  static constexpr unsigned int BUFFER_SIZE = RC_BUFFER_SIZE;

  using boostUdp = boost::asio::ip::udp;
  using Buffer = std::array<std::uint8_t, BUFFER_SIZE>;

private:

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
  static void receive(boost::asio::ip::udp::socket & socket,
                      I callbackCurrent,
                      I callbackBegin,
                      I callbackEnd,
                      const P & parser) {
    logger::println<logger::DEBUG>("Server::Listener::receive: ready to receive");
    try {
      socket.async_receive_from(
          boost::asio::buffer(callbackCurrent->mBuffer, Server::BUFFER_SIZE),
          callbackCurrent->mEndpoint,
          [&socket, callbackCurrent, callbackBegin, callbackEnd, parser]
              (const boost::system::error_code & errorCode, std::size_t bytesReceived) {
            logger::println<logger::DEBUG>("Server::Listener::receive::lambda: packet received");
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
          }
      );
    } catch (std::exception & e) {
      logger::errPrintln<logger::WARN>("Server::Listener::receive: "
                                       "exception caught when executing receive: {:s}",
                                       e.what());
    }
  }

  class Listener {
  public:

    /**
     * Attaches a boost::asio::io_service to this listener and starts listening
     *
     * @tparam P the packet parser type
     * @param config configuration for inbound and outbound connections
     * @param ioService the listening service
     * @param parser the packet parser
     */
    template <typename P>
    Listener(const Config & config,
             boost::asio::io_service & ioService,
             const P & parser)
        : mSocket{ioService, boostUdp::endpoint{boostUdp::v4(), config.server.port}} {

      mCallbackList.reserve(config.server.threadPoolSize);
      for (unsigned short i = 0; i < config.server.threadPoolSize; ++i) {
        mCallbackList.emplace_back(config.cache);
      }

      receive(mSocket, mCallbackList.begin(), mCallbackList.begin(), mCallbackList.end(), parser);
    }

    boostUdp::socket mSocket;
    std::vector<Callback<Server::Buffer>> mCallbackList;
  };

public:
  explicit Server(Config config)
      : mConfig{std::move(config)} {}

  /**
   * Starts listening and offloading packets to P. This method will block
   *
   * @tparam P the packet parser type
   * @param parser the packet parser
   */
  template <typename P>
  void run(const P & parser) {
    boost::asio::io_service ioService;

    Listener listener{mConfig, ioService, parser};
    logger::println<logger::DEBUG>("Server::run: listener built");

    if (mConfig.server.threadPoolSize == 1) {
      logger::println<logger::LOG>("Server::run: launching listener on UDP {:d} on a single thread",
                                   mConfig.server.port);
      ioService.run();
    } else {
      std::vector<std::thread> threadPool;
      threadPool.reserve(mConfig.server.threadPoolSize);

      logger::println<logger::LOG>("Server::run: launching listeners on UDP {:d} on {:d} threads",
                                   mConfig.server.port,
                                   mConfig.server.threadPoolSize);

      for (unsigned short i = 0; i < mConfig.server.threadPoolSize; ++i) {
        threadPool.emplace_back([&ioService]() { ioService.run(); });
      }

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
  const Config mConfig;
};
