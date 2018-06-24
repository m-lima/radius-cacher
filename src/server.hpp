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
#include "action.hpp"

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
private:

#ifndef RC_BUFFER_SIZE
  #define RC_BUFFER_SIZE 1024 * 8
#endif

  static constexpr unsigned int BUFFER_SIZE = RC_BUFFER_SIZE;

  using boostUdp = boost::asio::ip::udp;
  using Buffer = std::array<std::uint8_t, BUFFER_SIZE>;

  /**
   * Executor for once the buffer is ready
   * Will offload to the parser to know how to take action
   */
  struct Executor {
    boostUdp::endpoint mEndpoint;
    Buffer mBuffer;
    Cache mCache;

    explicit Executor(const Config::Cache & config)
        : mCache{config} {}

    template <typename P>
    auto operator()(std::size_t byteCount,
                    const P & parser) {
      auto bufferBegin = std::cbegin(mBuffer);
      auto bufferEnd = std::cend(mBuffer);
      auto action = parser(
          byteCount,
          bufferBegin,
          bufferBegin +
          std::min(byteCount, static_cast<std::size_t>(std::max(0L, std::distance(bufferBegin, bufferEnd))))
      );

      switch (action.action) {
        case Action::STORE:
          LOG(logger::INFO, "Server::Executor: Storing {:s} with {:s}", *action.key, *action.value);
          mCache.set(*action.key, *action.value);
          break;
        case Action::REMOVE:
          LOG(logger::INFO, "Server::Executor: Removing {:s} with {:s}", *action.key, *action.value);
          mCache.remove(*action.key);
          break;
        case Action::FILTER:
          LOG(logger::INFO, "Server::Executor: Filtering {:s}", *action.value);
          break;
      }
    }
  };

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
    LOG(logger::DEBUG, "Server::receive: ready to receive");
    try {
      socket.async_receive_from(
          boost::asio::buffer(callbackCurrent->mBuffer, Server::BUFFER_SIZE),
          callbackCurrent->mEndpoint,
          [&socket, callbackCurrent, callbackBegin, callbackEnd, parser]
              (const boost::system::error_code & error, std::size_t bytesReceived) {

            if (error && error != boost::asio::error::message_size) {
              LOG(logger::WARN, "Server::receive::lambda: error returned when executing receive: ({:d}) {:s}",
                  error.value(),
                  error.message());
            }

            LOG(logger::DEBUG, "Server::receive::lambda: packet received");
            auto callbackNext = callbackCurrent + 1;

            // Infinite loop for listening
            receive(socket,
                    (callbackNext != callbackEnd)
                    ? callbackNext
                    : callbackBegin,
                    callbackBegin,
                    callbackEnd,
                    parser);

            (*callbackCurrent)(bytesReceived, parser);
          }
      );
    } catch (const std::exception & e) {
      LOG(logger::WARN, "Server::receive: exception caught when executing receive: {:s}", e.what());
    }
  }

  struct Listener {

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
    std::vector<Executor> mCallbackList;
  };

  /**
   * Starts listening and offloading packets to P. This method will block
   *
   * @tparam P the packet parser type
   * @param parser the packet parser
   */
  template <typename P>
  static void runSingleCore(const Config & config, const P & parser) {

    LOG(logger::LOG, "Server::runSingleCore: launching listener on UDP {:d} on a single core", config.server.port);
    boost::asio::io_context ioContext;
    boostUdp::socket socket{ioContext, boostUdp::endpoint{boostUdp::v4(), config.server.port}};

    Executor executor(config.cache);
    LOG(logger::INFO, "Server::runSingleCore: executor built");

    for (;;) {
      try {
        boost::system::error_code error;
        auto bytes = socket.receive_from(boost::asio::buffer(executor.mBuffer), executor.mEndpoint, 0, error);
        LOG(logger::DEBUG, "Server::runSingleCore: {:d} bytes received", bytes);

        if (error && error != boost::asio::error::message_size) {
          LOG(logger::WARN, "Server::runSingleCore: error returned when executing receive: ({:d}) {:s}",
              error.value(),
              error.message());
        }

        executor(bytes, parser);
      } catch (const std::exception & e) {
        LOG(logger::WARN, "Server::runSingleCore: exception caught when executing receive: {:s}", e.what());
      }
    }

    LOG(logger::LOG, "Server::runSingleCore: server stopped");
  }

  /**
   * Starts listening and offloading packets to P. This method will block
   *
   * @tparam P the packet parser type
   * @param parser the packet parser
   */
  template <typename P>
  static void runMultiCore(const Config & config, const P & parser) {
    boost::asio::io_service ioService;

    Listener listener{config, ioService, parser};
    LOG(logger::DEBUG, "Server::runMultiCore: listener built");

    if (config.server.threadPoolSize == 1) {
      LOG(logger::LOG, "Server::runMultiCore: launching listener on UDP {:d} on a single thread", config.server.port);
      ioService.run();
    } else {
      std::vector<std::thread> threadPool;
      threadPool.reserve(config.server.threadPoolSize);

      LOG(logger::LOG,
          "Server::runMultiCore: launching listeners on UDP {:d} on {:d} threads",
          config.server.port,
          config.server.threadPoolSize);

      for (unsigned short i = 0; i < config.server.threadPoolSize; ++i) {
        threadPool.emplace_back([&ioService]() { ioService.run(); });
      }

      for (auto & t : threadPool) {
        t.join();
      }
    }
    LOG(logger::LOG, "Server::runMultiCore: server stopped");
  }

public:

  template <typename P>
  static void run(const Config & config, const P & parser) {
    if (config.server.singleCore) {
      if (config.server.threadPoolSize > 1) {
        LOG(logger::WARN,
            "Server::run: SINGLE_CORE option set. Ignoring THREAD_POOL_SIZE={:d}",
            config.server.threadPoolSize);
      }
      runSingleCore(config, parser);
    } else {
      runMultiCore(config, parser);
    }
  }
};
