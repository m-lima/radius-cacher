//
// Created by Marcelo Lima on 09/06/2018.
//

#pragma once

#include <string>

struct Config {
  struct Server {
    const unsigned short port;
    const unsigned short threadPoolSize;
    const std::string key;
    const std::string value;
    const std::string consentFile;
    const unsigned short consentRefreshMinutes;

    static Server load(const std::string & path);

    Server(const unsigned short port,
           const unsigned short threadPoolSize,
           std::string key,
           std::string value,
           std::string consentFile,
           const unsigned short consentRefreshMinutes)
        : port{port},
          threadPoolSize{threadPoolSize},
          key{std::move(key)},
          value{std::move(value)},
          consentFile{std::move(consentFile)},
          consentRefreshMinutes{consentRefreshMinutes} {}
  };

  struct Cache {
    const std::string host;
    const unsigned short port;
    const time_t ttl;
    const bool noReply;
    const bool useBinary;
    const bool tcpKeepAlive;

    static Cache load(const std::string & path);

    Cache(std::string host,
          const unsigned short port,
          const time_t ttl,
          const bool noReply,
          const bool useBinary,
          const bool tcpKeepAlive)
        : host{std::move(host)},
          port{port},
          ttl{ttl},
          noReply{noReply},
          useBinary{useBinary},
          tcpKeepAlive{tcpKeepAlive} {}

  };

  Config(const std::string & serverConfigPath, const std::string & cacheConfigPath)
      : server{Server::load(serverConfigPath)},
        cache{Cache::load(cacheConfigPath)} {}

  const Server server;
  const Cache cache;
};

