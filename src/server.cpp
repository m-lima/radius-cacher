//
// Created by Marcelo Lima on 8/6/18.
//

#include "server.hpp"

#include <string>
#include <optional>

#include <libmemcached/memcached.hpp>

#include <mfl/out.hpp>

#include "radius.hpp"

using boostUdp = boost::asio::ip::udp;

namespace {

  enum Action {
    DO_NOTHING,
    STORE,
    REMOVE
  };

  template<typename I>
  auto extractAction(I begin, I end) {
    auto type = radius::ValueReader::getUnsignedInt(begin, end);
    switch (type) {
      case radius::START:
      case radius::UPDATE:
        return STORE;
      case radius::STOP:
        return REMOVE;
      default:
        return DO_NOTHING;
    }
  }

  template<typename E, typename I>
  void parse(const E &,
             const boost::system::error_code & errorCode,
             std::size_t bytesReceived,
             I begin,
             I end,
             unsigned short callbackId) {

    // Read the header
    auto header = radius::Header::extract(begin, end);
    if (header.code != radius::Header::REQUEST) return; // Type is not a request. Break away

    auto packetEnd = begin + header.length;
    mfl::out::println("DEBUG: Header--\n"
                      "DEBUG: :: Code:   {:d}\n"
                      "DEBUG: :: ID:     {:d}\n"
                      "DEBUG: :: Length: {:d}",
                      header.code,
                      header.id,
                      header.length);
    begin += radius::Header::SIZE;

    // Prepare the cache action and data
    auto action = DO_NOTHING;
    std::optional < std::string > key;
    std::optional < std::string > value;

    // Slide through the attributes
    mfl::out::println("DEBUG: Start attribute iteration");
    while (begin < end && begin < packetEnd) {
      auto attribute = radius::Attribute::extract(begin, end);
      auto valueBegin = begin + radius::Attribute::SIZE;

      if (attribute.length > radius::Attribute::SIZE) {
        mfl::out::println("DEBUG: Attribute--\n"
                          "DEBUG: :: Type:   {:d}\n"
                          "DEBUG: :: Length: {:d}\n"
                          "DEBUG: :: Value:  {:s}",
                          attribute.type,
                          attribute.length,
                          radius::ValueReader::getString(valueBegin, end, begin + attribute.length));
      } else {
        mfl::out::println("DEBUG: Attribute--\n"
                          "DEBUG: :: Type:   {:d}\n"
                          "DEBUG: :: Length: {:d}",
                          attribute.type,
                          attribute.length);
      }

      switch (attribute.type) {

        case radius::Attribute::ACCT_STATUS_TYPE:
          if ((action = extractAction(valueBegin, end)) == DO_NOTHING) {
            mfl::out::println("DEBUG: Got action DO_NOTHING. Breaking away");
            return; // Free the buffer stack and callback ASAP
          }
          mfl::out::println("DEBUG: Got action {:s}", action == STORE ? "STORE" : "REMOVE");
          break;

        case radius::Attribute::FRAMED_IP_ADDRESS: {
          auto ip = radius::ValueReader::getAddress(valueBegin, end);
          key = std::make_optional(ip.ip);
          mfl::out::println("DEBUG: Key = {:s}", *key);
        }
          break;

        case radius::Attribute::USER_NAME:
          value = std::make_optional(radius::ValueReader::getString(valueBegin, end, begin + attribute.length));
          mfl::out::println("DEBUG: Value = {:s}", *value);
          break;

      }

      if (key && value && action != DO_NOTHING) {
        mfl::out::println("DEBUG: Got all fields. Breaking loop");
        break; // Break the loop ASAP
      }
      begin += attribute.length;
    }

    if (!key || !value) {
      mfl::out::println("DEBUG: Missing fields. Breaking away");
      return; // Free the buffer stack and callback ASAP
    }

    mfl::out::println("{:s} {:s} with {:s}", action == STORE ? "Storing" : "Removing", *key, *value);
  }

  static void receive(boostUdp::socket & socket,
                      Server::CallbackList::iterator callbackCurrent,
                      Server::CallbackList::iterator callbackBegin,
                      Server::CallbackList::iterator callbackEnd) {
    socket.async_receive_from(
        boost::asio::buffer(callbackCurrent->mBuffer, Server::BUFFER_SIZE),
        callbackCurrent->mEndpoint,
        [&socket, callbackCurrent, callbackBegin, callbackEnd]
            (const boost::system::error_code & errorCode, std::size_t bytesReceived) {
          auto callbackNext = callbackCurrent + 1;
          receive(socket,
                  (callbackNext != callbackEnd)
                  ? callbackNext
                  : callbackBegin,
                  callbackBegin,
                  callbackEnd);
          try {
            (*callbackCurrent)(errorCode,
                               bytesReceived,
                               parse<decltype(callbackCurrent->mEndpoint), Server::Buffer::const_iterator>);
          } catch (std::exception & e) {
            mfl::out::println(stderr, "exception caught when executing callback: {:s}", e.what());
          }
        }
    );
  }
}

Server::Server(boost::asio::io_service & ioService, unsigned short port)
    : mSocket{ioService, boostUdp::endpoint{boostUdp::v4(), port}} {
  for (short i = 0; i < CALLBACK_COUNT; ++i) {
    mCallbackList[i].callbackId = i;
  }
  receive(mSocket, mCallbackList.begin(), mCallbackList.begin(), mCallbackList.end());
}

