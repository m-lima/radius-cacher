#include <gtest/gtest.h>

#include "../src/radius_parser.hpp"

namespace {

  template <typename I>
  auto addHeader(I it, std::uint8_t code) {
    *(it++) = code;
    *(it++) = 0;
    *(it++) = 0;
    *(it++) = 0;
    it += 16; // No authenticator

    return it;
  }

  template <typename I, std::size_t N>
  auto addAttribute(I it,
                    std::uint8_t type,
                    std::array<std::uint8_t, N> && data) {
    *(it++) = type;
    *(it++) = 2 + N;
    std::copy(data.begin(), data.end(), it);
    it += N;

    return it;
  }

  template <typename I>
  auto addAttribute(I it,
                    std::uint8_t type,
                    const std::string && string) {
    *(it++) = type;
    *(it++) = 2 + string.length();
    std::copy(string.cbegin(), string.cend(), it);
    it += string.length();

    return it;
  }

  template <typename I>
  auto addAttribute(I it, std::uint8_t type, std::uint32_t data) {
    *(it++) = type;
    *(it++) = 6;
    *(it++) = static_cast<std::uint8_t>((data >> 24u) & 0xFF);
    *(it++) = static_cast<std::uint8_t>((data >> 16u) & 0xFF);
    *(it++) = static_cast<std::uint8_t>((data >> 8u) & 0xFF);
    *(it++) = static_cast<std::uint8_t>(data & 0xFF);

    return it;
  }

  template <typename I>
  auto closePacket(I begin, I it) {
    auto length = static_cast<std::size_t>(std::distance(begin, it));
    *(begin + 2) = (length >> 8u) & 0xFF;
    *(begin + 3) = length & 0xFF;
    return length;
  }
}

TEST(RadiusParser, empty_buffer) {
  RadiusParser parser{"", std::chrono::minutes{0}};

  std::array<std::uint8_t, 0> buffer{};
  ASSERT_ANY_THROW(parser(64, buffer.begin(), buffer.end()));
}

TEST(RadiusParser, no_bytes_read) {
  RadiusParser parser{"", std::chrono::minutes{0}};

  std::array<std::uint8_t, 4> buffer{};
  ASSERT_ANY_THROW(parser(0, buffer.begin(), buffer.end()));
}

TEST(RadiusParser, invalid_bytes_read) {
  RadiusParser parser{"", std::chrono::minutes{0}};

  std::array<std::uint8_t, 4> buffer{};
  ASSERT_ANY_THROW(parser(64, buffer.begin(), buffer.end()));
}

TEST(RadiusParser, reject_packets_that_are_not_request) {
  RadiusParser parser{"", std::chrono::minutes{0}};

  std::array<std::uint8_t, 64> buffer{};

  auto ptr = addHeader(buffer.begin(), radius::Header::RESPONSE);
  auto action = parser(closePacket(buffer.begin(), ptr), buffer.begin(), buffer.end());

  ASSERT_EQ(Action::DO_NOTHING, action.action);
}

TEST(RadiusParser, missing_value) {
  RadiusParser parser{"", std::chrono::minutes{0}};

  std::array<std::uint8_t, 128> buffer{};

  auto ptr = addHeader(buffer.begin(), radius::Header::REQUEST);
  ptr = addAttribute(ptr, radius::Attribute::ACCT_STATUS_TYPE, radius::StatusType::START);
  ptr = addAttribute(ptr, radius::Attribute::FRAMED_IP_ADDRESS, std::array<std::uint8_t, 4>{192, 168, 10, 22});

  auto action = parser(closePacket(buffer.begin(), ptr), buffer.begin(), buffer.end());
  ASSERT_EQ(Action::DO_NOTHING, action.action);
}

TEST(RadiusParser, missing_key) {
  RadiusParser parser{"", std::chrono::minutes{0}};

  std::array<std::uint8_t, 128> buffer{};

  auto ptr = addHeader(buffer.begin(), radius::Header::REQUEST);
  ptr = addAttribute(ptr, radius::Attribute::ACCT_STATUS_TYPE, radius::StatusType::START);
  ptr = addAttribute(ptr, radius::Attribute::USER_NAME, "987654321");

  auto action = parser(closePacket(buffer.begin(), ptr), buffer.begin(), buffer.end());
  ASSERT_EQ(Action::DO_NOTHING, action.action);
}

TEST(RadiusParser, start) {
  RadiusParser parser{"", std::chrono::minutes{0}};

  std::array<std::uint8_t, 128> buffer{};

  auto ptr = addHeader(buffer.begin(), radius::Header::REQUEST);
  ptr = addAttribute(ptr, radius::Attribute::ACCT_STATUS_TYPE, radius::StatusType::START);
  ptr = addAttribute(ptr, radius::Attribute::FRAMED_IP_ADDRESS, std::array<std::uint8_t, 4>{192, 168, 10, 22});
  ptr = addAttribute(ptr, radius::Attribute::USER_NAME, "987654321");

  auto action = parser(closePacket(buffer.begin(), ptr), buffer.begin(), buffer.end());
  ASSERT_EQ(Action::STORE, action.action);
  ASSERT_EQ("192.168.10.22", *action.key);
  ASSERT_EQ("987654321", *action.value);
}

TEST(RadiusParser, update) {
  RadiusParser parser{"", std::chrono::minutes{0}};

  std::array<std::uint8_t, 128> buffer{};

  auto ptr = addHeader(buffer.begin(), radius::Header::REQUEST);
  ptr = addAttribute(ptr, radius::Attribute::ACCT_STATUS_TYPE, radius::StatusType::UPDATE);
  ptr = addAttribute(ptr, radius::Attribute::FRAMED_IP_ADDRESS, std::array<std::uint8_t, 4>{192, 168, 10, 22});
  ptr = addAttribute(ptr, radius::Attribute::USER_NAME, "987654321");

  auto action = parser(closePacket(buffer.begin(), ptr), buffer.begin(), buffer.end());
  ASSERT_EQ(Action::STORE, action.action);
  ASSERT_EQ("192.168.10.22", *action.key);
  ASSERT_EQ("987654321", *action.value);
}

TEST(RadiusParser, deletion) {
  RadiusParser parser{"", std::chrono::minutes{0}};

  std::array<std::uint8_t, 128> buffer{};

  auto ptr = addHeader(buffer.begin(), radius::Header::REQUEST);
  ptr = addAttribute(ptr, radius::Attribute::ACCT_STATUS_TYPE, radius::StatusType::STOP);
  ptr = addAttribute(ptr, radius::Attribute::FRAMED_IP_ADDRESS, std::array<std::uint8_t, 4>{192, 168, 10, 22});
  ptr = addAttribute(ptr, radius::Attribute::USER_NAME, "987654321");

  auto action = parser(closePacket(buffer.begin(), ptr), buffer.begin(), buffer.end());
  ASSERT_EQ(Action::REMOVE, action.action);
  ASSERT_EQ("192.168.10.22", *action.key);
  ASSERT_EQ("987654321", *action.value);
}

TEST(RadiusParser, filtering) {
  RadiusParser parser{"res/test/filter.txt", std::chrono::minutes{0}};

  std::array<std::uint8_t, 128> buffer{};

  auto ptr = addHeader(buffer.begin(), radius::Header::REQUEST);
  ptr = addAttribute(ptr, radius::Attribute::ACCT_STATUS_TYPE, radius::StatusType::START);
  ptr = addAttribute(ptr, radius::Attribute::USER_NAME, "1234567890123456");
  ptr = addAttribute(ptr, radius::Attribute::FRAMED_IP_ADDRESS, std::array<std::uint8_t, 4>{192, 168, 10, 22});

  auto action = parser(closePacket(buffer.begin(), ptr), buffer.begin(), buffer.end());
  ASSERT_EQ(Action::FILTER, action.action);
  ASSERT_EQ("1234567890123456", *action.value);
  ASSERT_FALSE(action.key);
}

TEST(RadiusParser, buffer_overflow) {
  RadiusParser parser{"", std::chrono::minutes{0}};

  std::array<std::uint8_t, 128> buffer{};

  auto ptr = addHeader(buffer.begin(), radius::Header::REQUEST);
  ptr = addAttribute(ptr, radius::Attribute::ACCT_STATUS_TYPE, radius::StatusType::START);
  ptr = addAttribute(ptr, radius::Attribute::USER_NAME, "987654321");
  buffer[27] += 10;

  auto action = parser(closePacket(buffer.begin(), ptr) + 10, buffer.begin(), buffer.end());
  ASSERT_EQ(Action::DO_NOTHING, action.action);
}

TEST(RadiusParser, corrupted_data) {
  RadiusParser parser{"", std::chrono::minutes{0}};

  std::array<std::uint8_t, 128> buffer{};

  auto ptr = addHeader(buffer.begin(), radius::Header::REQUEST);
  ptr = addAttribute(ptr, radius::Attribute::ACCT_STATUS_TYPE, radius::StatusType::START);
  ptr = addAttribute(ptr, radius::Attribute::USER_NAME, "987654321");
  ptr = addAttribute(ptr, radius::Attribute::FRAMED_IP_ADDRESS, std::array<std::uint8_t, 4>{192, 168, 10, 22});
  buffer[27] -= 4;

  auto action = parser(closePacket(buffer.begin(), ptr), buffer.begin(), buffer.end());
  ASSERT_EQ(Action::DO_NOTHING, action.action);
}
