//
// Created by Marcelo Lima on 08/06/2018.
//

#pragma once

#include <array>

#include <boost/asio.hpp>

#include "config.hpp"
#include "logger.hpp"
#include "cacher.hpp"

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
                  C callback) const {
    auto bufferBegin = std::cbegin(mBuffer);
    auto bufferEnd = std::cend(mBuffer);
    return callback(
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

  using Buffer = std::array<std::uint8_t, BUFFER_SIZE>;
  using CallbackList = std::array<Callback<Buffer>, CALLBACK_COUNT>;

private:

  class Listener {
  public:
    Listener(boost::asio::io_service & ioService, unsigned short port);

    boost::asio::ip::udp::socket mSocket;
    Server::CallbackList mCallbackList;
  };

public:
  Server(config::Server config)
  : mConfig{std::move(config)} {}

  /**
   * Starts listening. This method will block
   */
  void run();

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

