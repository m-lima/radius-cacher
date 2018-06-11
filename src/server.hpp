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

/*
template <typename T, int N, typename ... Args>
auto filled(const T & ... t, const Args & ... args) {
  return std::array<T, N>{args...};
}

template <typename T, int N, typename ... Args>
auto filled<T, N, 0, Args ... >(const T & ... t, const Args & ... args) {
  return std::array<T, N>{t ...};
}

template <typename T, int N, int I, typename ... Args>
auto filled(const Args & ... args) {
  static_assert(I > 0);
  return filled<T, N, I - 1, Args ...>(T{args ...}, args ...);
}

template <typename T, int N, typename ... Args>
struct ArrayFiller<T, N, 0, Args ...> {
  constexpr static std::array<T, N> fill(const T & ... t) {
  }
};

template <typename T, int N, int I, typename ... Args>
struct ArrayFiller {
  constexpr static std::array<T, N> fill(const T & ... t, const Args & ... args) {
    static_assert(I > 0);
    return ArrayFiller<T, N, I - 1, Args ...>::fill(T{args ...}, args ...);
  }
};
*/

/*
#include <array>

template <typename T, int I, int N, typename ... Args>
struct ArrayFiller {
  constexpr static auto fill(const Args & ... args, const T & t ...) {
    static_assert(I > 0);
    return ArrayFiller<T, I - 1, N, Args ...>::fill(args ..., T{args ...}, t);
  }

  constexpr static auto fill(const Args & ... args) {
    static_assert(I > 0);
    return ArrayFiller<T, I - 1, N, Args ...>::fill(args ..., T{args ...});
  }

};

template <typename T, int N, typename ... Args>
struct ArrayFiller<T, 1, N, Args ...> {
  constexpr static auto fill(const Args & ... args, const T & t ...) {
    return ArrayFiller<T, 0, N>::fill(T{args...}, t);
  }
};

template <typename T, int N>
struct ArrayFiller<T, 0, N> {
  constexpr static auto fill( const T & t ...) {
    return std::array<T, N>{t};
  }
};

template <typename T, int N, typename ... Args>
struct ArrayFiller {
    constexpr static std::array<T, N> fill(const Args & ... args) {
        return {args ...};
    }
};

struct A {
  int a;
  int b;
  A() = delete;
  A(int b, int a)
      : a(a), b(b) {}

  template <int N, int I>
  struct ArrayFiller {
    static std::array<A, N> getArray(int b, int a, const A & args ...) {
      static_assert(N > 0);
      return ArrayFiller<N, I - 1>::getArray(b, a, A{a, b}, args);
    }
  };

  template<int N>
  struct ArrayFiller<N, 0> {
    static std::array<A, N> getArray(int b, int a, const A & args ...) {
      return std::array<A, N> {args};
    }
  };
};

int main(int argc, char * argv[]) {
  //std::array<A, 3> a = {std::array<A, 3>{A{1, 2}, A{3, 4}}, A{5, 6}};
  //std::array<A, 3> a = {{A{1,2}...}};
  //auto a = ArrayFiller<A, 1>::fill(1, 2);

  auto yo = ArrayFiller<A, 1, 1>::fill(1, 2);

  return A{1, 3}.b;
}
 */

/**
 * Callback wrapper to execute once the buffer is ready
 *
 * @tparam B The buffer type
 */
template <typename B>
struct Callback {
  boost::asio::ip::udp::endpoint mEndpoint;
  B mBuffer;
  Cache mCache;

  Callback() = delete;
  explicit Callback(const config::Cache & config)
      : mCache{config} {}

  template <typename C>
  auto operator()(const boost::system::error_code & errorCode,
                  std::size_t byteCount,
                  const C & callback) {
    auto bufferBegin = std::cbegin(mBuffer);
    auto bufferEnd = std::cend(mBuffer);
    return callback(
        byteCount,
        bufferBegin,
        bufferBegin +
        std::min(byteCount, static_cast<std::size_t>(std::max(0L, std::distance(bufferBegin, bufferEnd)))),
        &mCache
    );
  }

};

/**
 * Main server to handle UDP connections
 *
 * Works with 16 rolling callback handlers shared across all threads
 * Each callback handler has 8KB buffer for the packet by default.
 *
 * This is currently customizable at compile-time to harness std::array stack allocation
 * Use:
 * cmake -DRC_BUFFER_SIZE=<value> -DRC_CALLBACK_COUNT=<value> (...)
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
     * @param ioService the listening service
     * @param port the port to listen in
     * @param parser the packet parser
     */
    template <typename P>
    Listener(boost::asio::io_service & ioService,
             unsigned short port,
             const P & parser,
             unsigned short callbackPoolSize,
             const config::Cache & config)
        : mSocket{ioService, boostUdp::endpoint{boostUdp::v4(), port}} {
      mCallbackList.reserve(callbackPoolSize);
      for (unsigned short i = 0; i < callbackPoolSize; ++i) {
        mCallbackList.emplace_back(config);
      }
      receive(mSocket, mCallbackList.begin(), mCallbackList.begin(), mCallbackList.end(), parser);
    }

    boostUdp::socket mSocket;
    std::vector<Callback<Server::Buffer>> mCallbackList;
  };

public:
  explicit Server(config::Server config)
      : mConfig{std::move(config)} {}

  /**
   * Starts listening and offloading packets to P. This method will block
   *
   * @tparam P the packet parser type
   * @param parser the packet parser
   */
  template <typename P>
  void run(const P & parser, const config::Cache & config) {
    boost::asio::io_service ioService;

    Listener listener{ioService, mConfig.port, parser, mConfig.threadPoolSize, config};
    logger::println<logger::DEBUG>("Server::run: listener built");

    if (mConfig.threadPoolSize == 1) {
      logger::println<logger::LOG>("Server::run: launching listener on UDP {:d} on a single thread", mConfig.port);
      ioService.run();
    } else {
      std::vector<std::thread> threadPool;
      threadPool.reserve(mConfig.threadPoolSize);

      logger::println<logger::LOG>("Server::run: launching listeners on UDP {:d} on {:d} threads",
                                   mConfig.port,
                                   mConfig.threadPoolSize);

      for (unsigned short i = 0; i < mConfig.threadPoolSize; ++i) {
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
  const config::Server mConfig;
};
