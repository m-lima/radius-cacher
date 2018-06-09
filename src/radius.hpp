//
// Created by Marcelo Lima on 6/9/18.
//

#pragma once

#include <array>
#include <ctime>

#include <fmt/format.h>

/**
 * File based on Radius Accounting specification RFC-2866
 * Minimized for current use-case, however
 */
namespace radius {

  enum StatusType {
    START = 1,
    STOP = 2,
    UPDATE = 3
  };

  class Header {
#pragma pack(push, 1)
    struct HeaderRaw {
      std::uint8_t code;
      std::uint8_t id;
      std::array<std::uint8_t, 2> length;
      std::array<std::uint8_t, 16> authenticator;
    };
#pragma pop()

    Header(const HeaderRaw * raw)
        : code{raw->code},
          id{raw->id},
          length{static_cast<std::uint16_t>((raw->length[0] << 8u) | (raw->length[1]))},
          authenticator{} {};

  public:
    static constexpr auto SIZE = sizeof(HeaderRaw);

    enum Code {
      REQUEST = 4,
      RESPONSE = 5
    };

    struct Authenticator {
      std::uint64_t hi;
      std::uint64_t lo;
    };

    const std::uint8_t code;
    const std::uint8_t id;
    const std::uint16_t length;
    const Authenticator authenticator;

    template<typename I>
    static auto extract(I begin, I end) {
      if (std::distance(begin, end) < static_cast<int>(sizeof(HeaderRaw))) {
        throw std::runtime_error("Header::extract: buffer overflow");
      }

      auto raw = reinterpret_cast<const HeaderRaw *>(begin);

      return Header(raw);
    }

  };

  class Attribute {
#pragma pack(push, 1)
    struct AttributeRaw {
      std::uint8_t type;
      std::uint8_t length;
    };
#pragma pop()

    Attribute(const AttributeRaw * raw)
        : type{raw->type},
          length{raw->length} {}

  public:
    static constexpr auto SIZE = sizeof(AttributeRaw);

    enum Type {
      ACCT_STATUS_TYPE = 40,
      USER_NAME = 1,
      FRAMED_IP_ADDRESS = 8
    };

    const std::uint8_t type;
    const std::uint8_t length;

    template<typename I>
    static auto extract(I begin, I end) {
      if (std::distance(begin, end) < static_cast<int>(sizeof(AttributeRaw))) {
        throw std::runtime_error("Attribute::extract: buffer overflow");
      }

      auto raw = reinterpret_cast<const AttributeRaw *>(begin);

      return Attribute(raw);
    }
  };

  struct ValueReader;

  class IPv4 {
    friend struct ValueReader;
#pragma pack(push, 1)
    struct IPv4Raw {
      std::array<std::uint8_t, 4> octets;
    };
#pragma pack(pop)

    IPv4(const IPv4Raw * raw)
        : ip{fmt::format("{:d}.{:d}.{:d}.{:d}",
                         raw->octets[0],
                         raw->octets[1],
                         raw->octets[2],
                         raw->octets[3])},
          octets{raw->octets} {}

  public:
    static constexpr auto SIZE = sizeof(IPv4Raw);

    const std::string ip;
    std::array<std::uint8_t, 4> octets;

    template<typename I>
    static auto extract(I begin, I end) {
      if (std::distance(begin, end) < static_cast<int>(sizeof(IPv4Raw))) {
        throw std::runtime_error("IPv4::extract: buffer overflow");
      }

      auto raw = reinterpret_cast<const IPv4Raw *>(begin);
      return IPv4(raw);
    }
  };

  // The only types for attributes allowed in the spec
  struct ValueReader {

    template<typename I>
    static auto getString(I begin, I end, I stringEnd) {
      if (stringEnd > end) {
        throw std::runtime_error("ValueReader::getString: buffer overflow");
      }

      auto distance = std::distance(begin, stringEnd);
      // Spec does not allow empty strings
      if (distance < 1) {
        throw std::runtime_error("ValueReader::getString: string values cannot have zero bytes");
      }
      // Spec does not allow strings bigger than 253 B
      if (distance > 253) {
        throw std::runtime_error("ValueReader::getString: string values cannot exceed 253 bytes");
      }

      return std::string{begin, stringEnd};
    }

    template<typename I>
    static auto getAddress(I begin, I end) {
      if (std::distance(begin, end) < static_cast<int>(sizeof(IPv4::IPv4Raw))) {
        throw std::runtime_error("ValueReader::getAddress: buffer overflow");
      }

      return IPv4::extract(begin, end);
    }

    template<typename I>
    static std::uint32_t getUnsignedInt(I begin, I end) {
      if (std::distance(begin, end) < static_cast<int>(sizeof(std::uint32_t))) {
        throw std::runtime_error("ValueReader::getAddress: buffer overflow");
      }

      return (begin[0] << 24)
             | (begin[1] << 16)
             | (begin[2] << 8)
             | (begin[3]);
    }

    template<typename I>
    static auto getTime(I begin, I end) {
      return std::time_t{getUnsignedInt(begin, end)};
    }
  };

}

