//
// Created by Marcelo Lima on 09/06/2018.
//

#pragma once

#include <string>

struct Config {
  struct Server {
    const unsigned short port;
    const unsigned short threadPoolSize;
    const bool singleCore;
    const std::string key;
    const std::string value;
    const std::string filterFile;
    const unsigned short filterRefreshMinutes;

    static Server load(const std::string & path);

    Server(const unsigned short port,
           const unsigned short threadPoolSize,
           const bool singleCore,
           std::string key,
           std::string value,
           std::string filterFile,
           const unsigned short filterRefreshMinutes)
        : port{port},
          threadPoolSize{threadPoolSize},
          singleCore{singleCore},
          key{std::move(key)},
          value{std::move(value)},
          filterFile{std::move(filterFile)},
          filterRefreshMinutes{filterRefreshMinutes} {}
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

