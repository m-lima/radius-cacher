//
// Created by Marcelo Lima on 09/06/2018.
//

#pragma once

#include <string>
#include <vector>

#include <libmemcached/memcached.hpp>
#include <fmt/format.h>

#include "config.hpp"

class Cache {
public:
  inline void set(const std::string & key, const std::string & value) {
    mMemcache.set(key, std::vector<char>{value.cbegin(), value.cend()}, mTTL, 0);
  }

  inline void remove(const std::string & key) {
    mMemcache.remove(key);
  }

  explicit Cache(const Config::Cache & config)
      : mMemcache{fmt::format("--SERVER={:s}:{:d} {:s} {:s} {:s}",
                              config.host,
                              config.port,
                              config.noReply ? "--NOREPLY" : "",
                              config.useBinary ? "--BINARY-PROTOCOL" : "",
                              config.tcpKeepAlive ? "--TCP-KEEPALIVE" : "")},
        mTTL{config.ttl} {
  }

  /**
   * Must declare since compiler will omit due to copy constructor deletion
   */
  ~Cache() = default;

  /**
   * Delete copy constructor
   */
  Cache(const Cache &) = delete;

  /**
   * Allow move constructor
   */
  Cache(Cache &&) = default;

  /**
   * Delete copy
   */
  void operator=(const Cache &) = delete;

private:
  memcache::Memcache mMemcache;
  time_t mTTL;
};
