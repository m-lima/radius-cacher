//
// Created by Marcelo Lima on 09/06/2018.
//

#include "config.hpp"

#include <regex>
#include <fstream>

#include <mfl/string.hpp>

#include "logger.hpp"

namespace {

  template <typename C>
  void parse(const std::string & path, const std::regex & regex, C callback) {
    std::ifstream stream{path};

    if (!stream.is_open()) {
      logger::println<logger::ERROR>("config::parse: could not load configuration file \"{:s}\"."
                                     " Using default configuration", path);
      return;
    }

    std::string buffer;

    try {
      while (std::getline(stream, buffer)) {
        std::smatch match;
        if (std::regex_match(buffer, match, regex)) {
          logger::println<logger::DEBUG>("config::parse: found configuration: {:s} = {:s}",
                                         match[1],
                                         match[2]);
          callback(match);
        }
      }
    } catch (std::exception & ex) {
      logger::println<logger::FATAL>("config::parse: configuration file \"{:s}\" is invalid: {}",
                                     path,
                                     ex.what());
      throw std::runtime_error{ex.what()};
    }
  }

  auto getShort(const std::smatch match) {
    auto value = std::stoi(match[2]);
    if (value < 1 || value > 65535) {
      throw std::runtime_error(fmt::format("{:s} should be between 1 and 65535", match[1]));
    }
    return static_cast<unsigned short>(value);
  }

  auto getBool(const std::smatch match) {
    using namespace mfl::string::hash32;
    switch(hash(match[2])) {
      case "TRUE"_f32: return true;
      case "FALSE"_f32: return false;
      default: throw std::runtime_error(fmt::format("{:s} can take TRUE or FALSE only", match[1]));
    }
  }
}

namespace config {

  Server Server::load(const std::string & path) {
    using namespace mfl::string::hash32;
    static const std::regex LINE_REGEX = std::regex("^[[:space:]]*"
                                                    "(PORT|THREAD_POOL_SIZE|KEY|VALUE)"
                                                    "[[:space:]]*=[[:space:]]*"
                                                    "(.+)"
                                                    "[[:space:]]*$");

    unsigned short port = 1813;
    unsigned short threadPoolSize = 8;
    std::string key = "FRAMED_IP_ADDRESS";
    std::string value = "USER_NAME";

    parse(path, LINE_REGEX, [&](const std::smatch & match) {
      switch(hash(match[1])) {
        case "PORT"_f32:
          port = getShort(match);
          break;
        case "THREAD_POOL_SIZE"_f32:
          threadPoolSize = getShort(match);
          break;
        case "KEY"_f32:
          key = match[2];
          break;
        case "VALUE"_f32:
          value = match[2];
          break;
      }
    });

    logger::println<logger::INFO>("config::Server::load: configuring server with\n"
                                  "{:s} = {}\n"
                                  "{:s} = {}\n"
                                  "{:s} = {}\n"
                                  "{:s} = {}",
                                  "PORT", port,
                                  "THREAD_POOL_SIZE", threadPoolSize,
                                  "KEY", key,
                                  "VALUE", value);
    return Server{port, threadPoolSize, key, value};
  }

  Cache Cache::load(const std::string & path) {
    using namespace mfl::string::hash32;
    static const std::regex LINE_REGEX = std::regex("^[[:space:]]*"
                                                    "(HOST|PORT|TTL|NO_REPLY|USE_BINARY)"
                                                    "[[:space:]]*=[[:space:]]*"
                                                    "(.+)"
                                                    "[[:space:]]*$");

    std::string host = "localhost";
    unsigned short port = 11211;
    time_t ttl = 5;
    bool noReply = true;
    bool useBinary = true;

    parse(path, LINE_REGEX, [&](const std::smatch & match) {
      switch(hash(match[1])) {
        case "HOST"_f32:
          host = match[2];
          break;
        case "PORT"_f32:
          port = getShort(match);
          break;
        case "TTL"_f32:
          ttl = std::stoi(match[2]);
          break;
        case "NO_REPLY"_f32:
          noReply = getBool(match);
          break;
        case "USE_BINARY"_f32:
          useBinary = getBool(match);
          break;
      }
    });

    logger::println<logger::INFO>("config::Server::load: configuring cache with\n"
                                  "{:s} = {}\n"
                                  "{:s} = {}\n"
                                  "{:s} = {}\n"
                                  "{:s} = {}\n"
                                  "{:s} = {}",
                                  "HOST", host,
                                  "PORT", port,
                                  "TTL", ttl,
                                  "NO_REPLY", noReply,
                                  "USE_BINARY", useBinary);

    return Cache{host, port, ttl, noReply, useBinary};
  }
}
