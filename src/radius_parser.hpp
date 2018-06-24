//
// Created by Marcelo Lima on 10/06/2018.
//

#pragma once

#include <optional>

#include "action.hpp"
#include "radius.hpp"
#include "logger.hpp"
#include "filter.hpp"

class RadiusParser {
private:

  /**
   * Interpret what kind of Account-Request this packet is to decide the action to be taken
   *
   * @tparam I the itarator for the buffer
   * @param begin the start of the buffer
   * @param end the end of the buffer
   * @return the action to be taken
   */
  template <typename I>
  static inline auto extractAction(I begin, I end) {
    auto type = radius::ValueReader::getUnsignedInt(begin, end);
    switch (type) {
      case radius::START:
      case radius::UPDATE:
        return Action::STORE;
      case radius::STOP:
        return Action::REMOVE;
      default:
        return Action::DO_NOTHING;
    }
  }

  const Filter mFilter;

public:

  /**
   * Build the parser with a filter
   *
   * The parser must be built with the filter to avoid processing of packets before
   * the filter is ready
   */
  RadiusParser(std::string filterFilePath, std::chrono::minutes refreshMinutes)
      : mFilter{std::move(filterFilePath), refreshMinutes} {}

  ~RadiusParser() = default;
  RadiusParser(const RadiusParser &) = delete;
  RadiusParser(RadiusParser &&) = default;
  RadiusParser & operator = (const RadiusParser &) = delete;

  /**
   * Parse the incoming buffer for packet and call for action
   *
   * @tparam I the itarator for the buffer
   * @param bytesReceived number of bytes received in current packet
   * @param begin the start of the buffer
   * @param end the end of the buffer
   * @return the action to be taken by the server
   */
  template <typename I>
  Action operator()(std::size_t bytesReceived, I begin, I end) const {

    // Read the header
    auto header = radius::Header::extract(begin, end);

    // Break aways
    if (header.code != radius::Header::REQUEST) return {}; // Type is not a request
    if (header.length < 20 || header.length > bytesReceived || header.length > 4095) return {}; // Invalid as per spec

    auto packetEnd = begin + header.length;
    LOG(logger::DEBUG,
        "\n"
        "Header--\n"
        ":: Code:   {:d}\n"
        ":: ID:     {:d}\n"
        ":: Length: {:d}",
        header.code,
        header.id,
        header.length);
    begin += radius::Header::SIZE;

    // Prepare the cache action and data
    auto action = Action::DO_NOTHING;
    std::optional<std::string> key;
    std::optional<std::string> value;

    // Slide through the attributes
    LOG(logger::DEBUG, "Start attribute iteration");
    while (begin < end && begin < packetEnd) {
      auto attribute = radius::Attribute::extract(begin, end);

      if (attribute.length < 2) {
        LOG(logger::INFO, "Invalid attribute size found. Discarding packet");
        return {};
      }

      auto valueBegin = begin + radius::Attribute::SIZE;

      if (attribute.length > radius::Attribute::SIZE) {
        LOG(logger::DEBUG,
            "\n"
            "Attribute--\n"
            ":: Type:   {:d}\n"
            ":: Length: {:d}\n"
            ":: Value:  {:s}",
            attribute.type,
            attribute.length,
            radius::ValueReader::getString(valueBegin, end, begin + attribute.length));
      } else {
        LOG(logger::DEBUG,
            "\n"
            "Attribute--\n"
            ":: Type:   {:d}\n"
            ":: Length: {:d}",
            attribute.type,
            attribute.length);
      }

      switch (attribute.type) {

        case radius::Attribute::ACCT_STATUS_TYPE:
          if ((action = extractAction(valueBegin, end)) == Action::DO_NOTHING) {
            LOG(logger::INFO, "Got action DO_NOTHING. Breaking away");
            return {}; // Free the buffer stack and callback ASAP
          }

          LOG(logger::DEBUG, "Got action {:s}", action == Action::STORE ? "STORE" : "REMOVE");
          break;

        case radius::Attribute::FRAMED_IP_ADDRESS:
          key = std::make_optional(radius::ValueReader::getAddress(valueBegin, end).ip);

          LOG(logger::DEBUG, "Key = {:s}", *key);
          break;

        case radius::Attribute::USER_NAME:
          value = std::make_optional(radius::ValueReader::getString(valueBegin, end, begin + attribute.length));

          if (mFilter.contains(std::stoull(*value))) {
            // User opted-out; Free the buffer stack and callback ASAP
            return {Action::FILTER, std::move(key), std::move(value)};
          }

          LOG(logger::DEBUG, "Value = {:s}", *value);
          break;
      }

      if (key && value && action != Action::DO_NOTHING) {
        LOG(logger::DEBUG, "Got all fields. Breaking loop");
        break; // Break the loop ASAP
      }
      begin += attribute.length;
    }

    if (!key || !value) {
      LOG(logger::INFO, "Missing fields. Breaking away");
      return {}; // Free the buffer stack and callback ASAP
    }

    return {action, std::move(key), std::move(value)};
  }
};
