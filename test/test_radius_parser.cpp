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

class Parser : public ::testing::Test {
public:
  RadiusParser parser{"res/test/filter.txt", std::chrono::minutes{0}};
  std::array<std::uint8_t, 128> buffer{};
};

TEST_F(Parser, empty_buffer) {
  std::array<std::uint8_t, 0> localBuffer{};
  ASSERT_ANY_THROW(parser(64, localBuffer.begin(), localBuffer.end()));
}

TEST_F(Parser, no_bytes_read) {
  std::array<std::uint8_t, 4> localBuffer{};
  ASSERT_ANY_THROW(parser(0, localBuffer.begin(), localBuffer.end()));
}

TEST_F(Parser, invalid_bytes_read) {
  std::array<std::uint8_t, 4> localBuffer{};
  ASSERT_ANY_THROW(parser(64, localBuffer.begin(), localBuffer.end()));
}

TEST_F(Parser, reject_packets_that_are_not_request) {
  auto ptr = addHeader(buffer.begin(), radius::Header::RESPONSE);
  auto action = parser(closePacket(buffer.begin(), ptr), buffer.begin(), buffer.end());

  ASSERT_EQ(Action::DO_NOTHING, action.action);
}

TEST_F(Parser, missing_value) {
  auto ptr = addHeader(buffer.begin(), radius::Header::REQUEST);
  ptr = addAttribute(ptr, radius::Attribute::ACCT_STATUS_TYPE, radius::StatusType::START);
  ptr = addAttribute(ptr, radius::Attribute::FRAMED_IP_ADDRESS, std::array<std::uint8_t, 4>{192, 168, 10, 22});

  auto action = parser(closePacket(buffer.begin(), ptr), buffer.begin(), buffer.end());
  ASSERT_EQ(Action::DO_NOTHING, action.action);
}

TEST_F(Parser, missing_key) {
  auto ptr = addHeader(buffer.begin(), radius::Header::REQUEST);
  ptr = addAttribute(ptr, radius::Attribute::ACCT_STATUS_TYPE, radius::StatusType::START);
  ptr = addAttribute(ptr, radius::Attribute::USER_NAME, "987654321");

  auto action = parser(closePacket(buffer.begin(), ptr), buffer.begin(), buffer.end());
  ASSERT_EQ(Action::DO_NOTHING, action.action);
}

TEST_F(Parser, start) {
  auto ptr = addHeader(buffer.begin(), radius::Header::REQUEST);
  ptr = addAttribute(ptr, radius::Attribute::ACCT_STATUS_TYPE, radius::StatusType::START);
  ptr = addAttribute(ptr, radius::Attribute::FRAMED_IP_ADDRESS, std::array<std::uint8_t, 4>{192, 168, 10, 22});
  ptr = addAttribute(ptr, radius::Attribute::USER_NAME, "987654321");

  auto action = parser(closePacket(buffer.begin(), ptr), buffer.begin(), buffer.end());
  ASSERT_EQ(Action::STORE, action.action);
  ASSERT_EQ("192.168.10.22", *action.key);
  ASSERT_EQ("987654321", *action.value);
}

TEST_F(Parser, update) {
  auto ptr = addHeader(buffer.begin(), radius::Header::REQUEST);
  ptr = addAttribute(ptr, radius::Attribute::ACCT_STATUS_TYPE, radius::StatusType::UPDATE);
  ptr = addAttribute(ptr, radius::Attribute::FRAMED_IP_ADDRESS, std::array<std::uint8_t, 4>{192, 168, 10, 22});
  ptr = addAttribute(ptr, radius::Attribute::USER_NAME, "987654321");

  auto action = parser(closePacket(buffer.begin(), ptr), buffer.begin(), buffer.end());
  ASSERT_EQ(Action::STORE, action.action);
  ASSERT_EQ("192.168.10.22", *action.key);
  ASSERT_EQ("987654321", *action.value);
}

TEST_F(Parser, deletion) {
  auto ptr = addHeader(buffer.begin(), radius::Header::REQUEST);
  ptr = addAttribute(ptr, radius::Attribute::ACCT_STATUS_TYPE, radius::StatusType::STOP);
  ptr = addAttribute(ptr, radius::Attribute::FRAMED_IP_ADDRESS, std::array<std::uint8_t, 4>{192, 168, 10, 22});
  ptr = addAttribute(ptr, radius::Attribute::USER_NAME, "987654321");

  auto action = parser(closePacket(buffer.begin(), ptr), buffer.begin(), buffer.end());
  ASSERT_EQ(Action::REMOVE, action.action);
  ASSERT_EQ("192.168.10.22", *action.key);
  ASSERT_EQ("987654321", *action.value);
}

TEST_F(Parser, filtering) {
  auto ptr = addHeader(buffer.begin(), radius::Header::REQUEST);
  ptr = addAttribute(ptr, radius::Attribute::ACCT_STATUS_TYPE, radius::StatusType::START);
  ptr = addAttribute(ptr, radius::Attribute::USER_NAME, "1234567890123456");
  ptr = addAttribute(ptr, radius::Attribute::FRAMED_IP_ADDRESS, std::array<std::uint8_t, 4>{192, 168, 10, 22});

  auto action = parser(closePacket(buffer.begin(), ptr), buffer.begin(), buffer.end());
  ASSERT_EQ(Action::FILTER, action.action);
  ASSERT_EQ("1234567890123456", *action.value);
  ASSERT_FALSE(action.key);
}

TEST_F(Parser, buffer_overflow) {
  auto ptr = addHeader(buffer.begin(), radius::Header::REQUEST);
  ptr = addAttribute(ptr, radius::Attribute::ACCT_STATUS_TYPE, radius::StatusType::START);
  ptr = addAttribute(ptr, radius::Attribute::USER_NAME, "987654321");
  buffer[27] += 10;

  auto action = parser(closePacket(buffer.begin(), ptr) + 10, buffer.begin(), buffer.end());
  ASSERT_EQ(Action::DO_NOTHING, action.action);
}

TEST_F(Parser, corrupted_data) {
  auto ptr = addHeader(buffer.begin(), radius::Header::REQUEST);
  ptr = addAttribute(ptr, radius::Attribute::ACCT_STATUS_TYPE, radius::StatusType::START);
  ptr = addAttribute(ptr, radius::Attribute::USER_NAME, "987654321");
  ptr = addAttribute(ptr, radius::Attribute::FRAMED_IP_ADDRESS, std::array<std::uint8_t, 4>{192, 168, 10, 22});
  buffer[27] -= 4;

  auto action = parser(closePacket(buffer.begin(), ptr), buffer.begin(), buffer.end());
  ASSERT_EQ(Action::DO_NOTHING, action.action);
}
