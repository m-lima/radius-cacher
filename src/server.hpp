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
   * Callback wrapper to execute once the buffer is ready
   * Will offload to the parser to know how to take action
   */
  struct Listener {
    boostUdp::endpoint mEndpoint;
    Buffer mBuffer;
    Cache mCache;

    explicit Listener(const Config::Cache & config)
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
          mCache.set(*(action.key), *(action.value));
          break;
        case Action::REMOVE:
          mCache.remove(*(action.key));
          break;
      }
    }
  };

public:

  /**
   * Starts listening and offloading packets to P. This method will block
   *
   * @tparam P the packet parser type
   * @param parser the packet parser
   */
  template <typename P>
  void run(const Config & config, const P & parser) {

    LOG(logger::LOG, "Server::run: launching listener on UDP {:d} on a single thread", config.server.port);
    boost::asio::io_context ioConstext;
    boostUdp::socket socket{ioConstext, boostUdp::endpoint{boostUdp::v4(), config.server.port}};
    Cache cache{config.cache};

    Listener listener(config.cache);
    LOG(logger::DEBUG, "Server::run: listener built");

    for (;;) {
      try {
        boost::system::error_code error;
        auto bytes = socket.receive_from(boost::asio::buffer(listener.mBuffer), listener.mEndpoint, 0, error);
        LOG(logger::DEBUG, "Server::run: {:d} bytes received", bytes);

        if (error && error != boost::asio::error::message_size) {
          LOG(logger::WARN, "Server::run: error returned when executing receive: ({:d}) {:s}",
              error.value(),
              error.message());
        }

        listener(bytes, parser);
      } catch (const std::exception & e) {
        LOG(logger::WARN, "Server::run: exception caught when executing receive: {:s}", e.what());
      }
    }

    LOG(logger::LOG, "Server::run: server stopped");
  }
};
