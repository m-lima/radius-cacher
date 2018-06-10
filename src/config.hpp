//
// Created by Marcelo Lima on 09/06/2018.
//

#pragma once

#include <string>

namespace config {
  struct Server {
    const unsigned short port;
    const unsigned short threadPoolSize;
    const std::string key;
    const std::string value;

    static Server load(const std::string & path);

    Server(const unsigned short port,
           const unsigned short threadPoolSize,
           std::string key,
           std::string value)
        : port{port},
          threadPoolSize{threadPoolSize},
          key{std::move(key)},
          value{std::move(value)} {}
  };

  struct Cache {
    const std::string host;
    const unsigned short port;
    const time_t ttl;
    const bool noReply;
    const bool useBinary;

    static Cache load(const std::string & path);

    Cache(std::string host,
          const unsigned short port,
          const time_t ttl,
          const bool noReply,
          const bool useBinary)
        : host{std::move(host)},
          port{port},
          ttl{ttl},
          noReply{noReply},
          useBinary{useBinary} {}

  };
}

