//
// Created by Marcelo Lima on 09/06/2018.
//

#include "config.hpp"

#include <regex>
#include <fstream>
#include <cstdlib>

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
          logger::println<logger::DEBUG>("config::parse: found configuration in file: {:s} = {:s}",
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

  auto getShort(const std::string & key, const std::string & value) {
    auto asInt = std::stoi(value);
    if (asInt < 1 || asInt > 65535) {
      throw std::runtime_error(fmt::format("{:s} should be between 1 and 65535", key));
    }
    return static_cast<unsigned short>(asInt);
  }
  auto getShort(const std::smatch & match) {
    return getShort(match[1], match[2]);
  }

  auto getInt(const std::string & key, const std::string & value) {
    return std::stoi(value);
  }
  auto getInt(const std::smatch & match) {
    return getInt(match[1], match[2]);
  }

  auto getString(const std::string & key, const std::string & value) {
    if (value.empty()) {
      throw std::runtime_error(fmt::format("{:s} cannot be empty", key));
    }
    return value;
  }
  auto getString(const std::smatch & match) {
    return getString(match[1], match[2]);
  }

  auto getBool(const std::string & key, const std::string & value) {
    using namespace mfl::string::hash32;
    switch (hash(value)) {
      case "TRUE"_h: return true;
      case "FALSE"_h: return false;
      default: throw std::runtime_error(fmt::format("{:s} can take TRUE or FALSE only", key));
    }
  }
  auto getBool(const std::smatch & match) {
    return getBool(match[1], match[2]);
  }
}

Config::Server Config::Server::load(const std::string & path) {
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
    switch (hash(match[1])) {
      case "PORT"_h: port = getShort(match); break;
      case "THREAD_POOL_SIZE"_h: threadPoolSize = getShort(match); break;
      case "KEY"_h: key = getString(match); break;
      case "VALUE"_h: value = getString(match); break;
    }
  });

  char * env;
  env = std::getenv("RADIUS_PORT");
  if (env) port = getShort("PORT", env);

  env = std::getenv("RADIUS_THREAD_POOL_SIZE");
  if (env) threadPoolSize = getShort("THREAD_POOL_SIZE", env);

  env = std::getenv("RADIUS_KEY");
  if (env) key = getString("KEY", env);

  env = std::getenv("RADIUS_VALUE");
  if (env) value = getString("VALUE", env);

  logger::println<logger::LOG>("config::Server::load: configuring server with\n"
                               "{:s} = {}\n"
                               "{:s} = {}\n"
                               "{:s} = {}\n"
                               "{:s} = {}",
                               "PORT", port,
                               "THREAD_POOL_SIZE", threadPoolSize,
                               "KEY", key,
                               "VALUE", value);
  return {port, threadPoolSize, key, value};
}

Config::Cache Config::Cache::load(const std::string & path) {
  using namespace mfl::string::hash32;
  static const std::regex LINE_REGEX = std::regex("^[[:space:]]*"
                                                  "(HOST|PORT|TTL|NO_REPLY|USE_BINARY|TCP_KEEP_ALIVE)"
                                                  "[[:space:]]*=[[:space:]]*"
                                                  "(.+)"
                                                  "[[:space:]]*$");

  std::string host = "localhost";
  unsigned short port = 11211;
  time_t ttl = 5400;
  bool noReply = true;
  bool useBinary = true;
  bool tcpKeepAlive = true;

  parse(path, LINE_REGEX, [&](const std::smatch & match) {
    switch (hash(match[1])) {
      case "HOST"_h: host = getString(match); break;
      case "PORT"_h: port = getShort(match); break;
      case "TTL"_h: ttl = getInt(match); break;
      case "NO_REPLY"_h: noReply = getBool(match); break;
      case "USE_BINARY"_h: useBinary = getBool(match); break;
      case "TCP_KEEP_ALIVE"_h: tcpKeepAlive = getBool(match); break;
    }
  });

  char * env;
  env = std::getenv("RADIUS_CACHE_HOST");
  if (env) host = getString("HOST", env);

  env = std::getenv("RADIUS_CACHE_PORT");
  if (env) port = getShort("PORT", env);

  env = std::getenv("RADIUS_CACHE_TTL");
  if (env) ttl = getInt("TTL", env);

  env = std::getenv("RADIUS_CACHE_NO_REPLY");
  if (env) noReply = getBool("NO_REPLY", env);

  env = std::getenv("RADIUS_CACHE_USE_BINARY");
  if (env) useBinary = getBool("USE_BINARY", env);

  env = std::getenv("RADIUS_CACHE_TCP_KEEP_ALIVE");
  if (env) tcpKeepAlive = getBool("TCP_KEEP_ALIVE", env);

  logger::println<logger::LOG>("config::Server::load: configuring cache with\n"
                               "{:s} = {}\n"
                               "{:s} = {}\n"
                               "{:s} = {}\n"
                               "{:s} = {}\n"
                               "{:s} = {}\n"
                               "{:s} = {}",
                               "HOST", host,
                               "PORT", port,
                               "TTL", ttl,
                               "NO_REPLY", noReply,
                               "USE_BINARY", useBinary,
                               "TCP_KEEP_ALIVE", tcpKeepAlive);

  return {host, port, ttl, noReply, useBinary, tcpKeepAlive};
}
