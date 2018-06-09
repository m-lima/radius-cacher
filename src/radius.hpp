//
// Created by Marcelo Lima on 09/06/2018.
//

#pragma once

#include <array>
#include <ctime>

#include <fmt/format.h>

/**
 * File based on Radius Accounting specification RFC-2866
 * Minimized for current use-case, however
 *
 * All Radius payloads shall come in a single UDP packet on port 1813 for accounting
 */
namespace radius {

  /**
   * Supported accounting requests
   */
  enum StatusType {
    START = 1,
    STOP = 2,
    UPDATE = 3
  };

  /**
   * The four possible value types for AVP as defined in the spec
   */
  enum ValueType {
    STRING,
    ADDRESS,
    UNSIGNED_INT,
    TIME,
  };

  /**
   * Header for the Radius packet
   *
   * Code: The type of packet: REQUEST / RESPONSE
   * Id: An ID to track replies
   * Length: Total length of the packet (including this header)
   * Authenticator: An MD5 of (Code+ID+Length+(16 * 0x00)+Attributes+Secret)
   *                Basically, the full packet is hashed toghether with the secret,
   *                but with 16 octets of zeros in place of the actual Authenticator
   */
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

    /**
     * Possible codes possible according to the spec
     */
    enum Code {
      REQUEST = 4,
      RESPONSE = 5
    };

    /**
     * 128-bit struct (16 octets) to hold the authenticator hash
     */
    struct Authenticator {
      std::uint64_t hi;
      std::uint64_t lo;
    };

    const std::uint8_t code;
    const std::uint8_t id;
    const std::uint16_t length;
    const Authenticator authenticator;

    /**
     * Does a copy to stack from the given buffer
     * Uses reinterpret_cast on the buffer before copying
     *
     * @tparam I the itarator for the buffer
     * @param begin the start of the buffer
     * @param end the end of the buffer
     * @return a parsed Radius packet header
     * @throws runtime_error buffer overflow
     */
    template <typename I>
    static auto extract(I begin, I end) {
      if (std::distance(begin, end) < static_cast<int>(sizeof(HeaderRaw))) {
        throw std::runtime_error("Header::extract: buffer overflow");
      }

      auto raw = reinterpret_cast<const HeaderRaw *>(begin);

      return Header(raw);
    }

  };

  /**
   * An AVP for the Radius packet
   *
   * Type: The type of packet; e.g. 40 = Acct-Status-Type
   * Length: Total length of the packet (including this preamble)
   */
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

    /**
     * Does a copy to stack from the given buffer
     * Uses reinterpret_cast on the buffer before copying
     *
     * @tparam I the itarator for the buffer
     * @param begin the start of the buffer
     * @param end the end of the buffer
     * @return a parsed AVP header
     * @throws runtime_error buffer overflow
     */
    template <typename I>
    static auto extract(I begin, I end) {
      if (std::distance(begin, end) < static_cast<int>(sizeof(AttributeRaw))) {
        throw std::runtime_error("Attribute::extract: buffer overflow");
      }

      auto raw = reinterpret_cast<const AttributeRaw *>(begin);

      return Attribute(raw);
    }
  };

  /**
   * A wrapper around IPv4 octets that exports a string format
   */
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

    /**
     * Does a copy to stack from the given buffer
     * Uses reinterpret_cast on the buffer before copying
     *
     * @tparam I the itarator for the buffer
     * @param begin the start of the buffer
     * @param end the end of the buffer
     * @return a parsed IPv4 value
     * @throws runtime_error buffer overflow
     */
    template <typename I>
    static auto extract(I begin, I end) {
      if (std::distance(begin, end) < static_cast<int>(sizeof(IPv4Raw))) {
        throw std::runtime_error("IPv4::extract: buffer overflow");
      }

      auto raw = reinterpret_cast<const IPv4Raw *>(begin);
      return IPv4(raw);
    }
  };

  /**
   * A reader for all attributes types allowed in the spec
   */
  struct ValueReader {

    /**
     * Parses and extracts a string from the buffer
     *
     * @tparam I the itarator for the buffer
     * @param begin the start of the buffer
     * @param end the end of the buffer
     * @param stringEnd the end of the string
     * @return a parsed string
     * @throws runtime_error buffer overflow
     * @throws runtime_error empty string
     * @throws runtime_error string larger than 253 bytes
     */
    template <typename I>
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

    /**
     * Parses and extracts an IPv4 object from the buffer
     *
     * @tparam I the itarator for the buffer
     * @param begin the start of the buffer
     * @param end the end of the buffer
     * @return a parsed IPv4 value
     * @throws runtime_error buffer overflow
     */
    template <typename I>
    static auto getAddress(I begin, I end) {
      if (std::distance(begin, end) < static_cast<int>(sizeof(IPv4::IPv4Raw))) {
        throw std::runtime_error("ValueReader::getAddress: buffer overflow");
      }

      return IPv4::extract(begin, end);
    }

    /**
     * Parses and extracts a 32bit unsigned integer from the buffer
     *
     * @tparam I the itarator for the buffer
     * @param begin the start of the buffer
     * @param end the end of the buffer
     * @return a parsed 32bit unsigned integer value
     * @throws runtime_error buffer overflow
     */
    template <typename I>
    static std::uint32_t getUnsignedInt(I begin, I end) {
      if (std::distance(begin, end) < static_cast<int>(sizeof(std::uint32_t))) {
        throw std::runtime_error("ValueReader::getAddress: buffer overflow");
      }

      return (begin[0] << 24)
             | (begin[1] << 16)
             | (begin[2] << 8)
             | (begin[3]);
    }

    /**
     * Parses and extracts the time based on the number of seconds since 00:00:00 UTC, Jan 1, 1970
     *
     * @tparam I the itarator for the buffer
     * @param begin the start of the buffer
     * @param end the end of the buffer
     * @return a parsed time_t object
     * @throws runtime_error buffer overflow
     */
    template <typename I>
    static auto getTime(I begin, I end) {
      return std::time_t{getUnsignedInt(begin, end)};
    }
  };

}

