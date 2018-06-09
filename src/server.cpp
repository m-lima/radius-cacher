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

  /**
   * Possible actions to be taken given a certain packet
   */
  enum Action {
    DO_NOTHING,
    STORE,
    REMOVE
  };

  /**
   * Interpret what kind of Account-Request this packet is to decide the action to be taken
   *
   * @tparam I the itarator for the buffer
   * @param begin the start of the buffer
   * @param end the end of the buffer
   * @return the action to be taken
   */
  template <typename I>
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

  /**
   * Parse the incoming buffer for packet and call for action
   *
   * @tparam E enpoint type
   * @tparam I the itarator for the buffer
   * @param errorCode error code given by the socket
   * @param bytesReceived number of bytes received in current packet
   * @param begin the start of the buffer
   * @param end the end of the buffer
   */
  template <typename E, typename I>
  void parse(const E &,
             const boost::system::error_code & errorCode,
             std::size_t bytesReceived,
             I begin,
             I end) {

    // Read the header
    auto header = radius::Header::extract(begin, end);
    if (header.code != radius::Header::REQUEST) return; // Type is not a request. Break away

    auto packetEnd = begin + header.length;
    logger::println<logger::DEBUG>("\n"
                                   "Header--\n"
                                   ":: Code:   {:d}\n"
                                   ":: ID:     {:d}\n"
                                   ":: Length: {:d}",
                                   header.code,
                                   header.id,
                                   header.length);
    begin += radius::Header::SIZE;

    // Prepare the cache action and data
    auto action = DO_NOTHING;
    std::optional<std::string> key;
    std::optional<std::string> value;

    // Slide through the attributes
    logger::println<logger::DEBUG>("Start attribute iteration");
    while (begin < end && begin < packetEnd) {
      auto attribute = radius::Attribute::extract(begin, end);
      auto valueBegin = begin + radius::Attribute::SIZE;

      if (attribute.length > radius::Attribute::SIZE) {
        logger::println<logger::DEBUG>("\n"
                                       "Attribute--\n"
                                       ":: Type:   {:d}\n"
                                       ":: Length: {:d}\n"
                                       ":: Value:  {:s}",
                                       attribute.type,
                                       attribute.length,
                                       radius::ValueReader::getString(valueBegin, end, begin + attribute.length));
      } else {
        logger::println<logger::DEBUG>("\n"
                                       "Attribute--\n"
                                       ":: Type:   {:d}\n"
                                       ":: Length: {:d}",
                                       attribute.type,
                                       attribute.length);
      }

      switch (attribute.type) {

        case radius::Attribute::ACCT_STATUS_TYPE:
          if ((action = extractAction(valueBegin, end)) == DO_NOTHING) {
            logger::println<logger::INFO>("Got action DO_NOTHING. Breaking away");
            return; // Free the buffer stack and callback ASAP
          }
          logger::println<logger::DEBUG>("Got action {:s}", action == STORE ? "STORE" : "REMOVE");
          break;

        case radius::Attribute::FRAMED_IP_ADDRESS: {
          auto ip = radius::ValueReader::getAddress(valueBegin, end);
          key = std::make_optional(ip.ip);
          logger::println<logger::DEBUG>("Key = {:s}", *key);
        }
          break;

        case radius::Attribute::USER_NAME:
          value = std::make_optional(radius::ValueReader::getString(valueBegin, end, begin + attribute.length));
          logger::println<logger::DEBUG>("Value = {:s}", *value);
          break;

      }

      if (key && value && action != DO_NOTHING) {
        logger::println<logger::DEBUG>("Got all fields. Breaking loop");
        break; // Break the loop ASAP
      }
      begin += attribute.length;
    }

    if (!key || !value) {
      logger::println<logger::INFO>("Missing fields. Breaking away");
      return; // Free the buffer stack and callback ASAP
    }

    logger::println<logger::INFO>("{:s} {:s} with {:s}", action == STORE ? "Storing" : "Removing", *key, *value);
  }

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
      logger::errPrintln<logger::WARNING>("exception caught when executing receive: {:s}", e.what());
    }
  }
}

/**
 * Attaches a boost::asio::io_service to this listener and starts listening
 *
 * @param ioService the listening service
 * @param port the port to listen in
 */
Server::Server(boost::asio::io_service & ioService, unsigned short port)
    : mSocket{ioService, boostUdp::endpoint{boostUdp::v4(), port}} {
  receive(mSocket, mCallbackList.begin(), mCallbackList.begin(), mCallbackList.end());
}

