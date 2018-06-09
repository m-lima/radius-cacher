//
// Created by Marcelo Lima on 08/06/2018.
//

#pragma once

#include <array>

#include <boost/asio.hpp>

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
 * harness std::array compile time stack allocation
 */
class Server {
public:
  static constexpr auto BUFFER_SIZE = 1024 * 8;
  static constexpr unsigned short CALLBACK_COUNT = 10;

  using Buffer = std::array<std::uint8_t, BUFFER_SIZE>;
  using CallbackList = std::array<Callback<Buffer>, CALLBACK_COUNT>;

  Server(boost::asio::io_service & ioService, unsigned short port);

private:
  boost::asio::ip::udp::socket mSocket;
  CallbackList mCallbackList;

};
