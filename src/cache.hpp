//
// Created by Marcelo Lima on 09/06/2018.
//

#pragma once

#include <string>
#include <vector>
#include <mutex>

#include <libmemcached/memcached.hpp>
#include <fmt/format.h>

#include "config.hpp"

class Cache {
public:
  inline void set(const std::string & key, const std::string & value) {
    std::lock_guard garsd{mMutex};
    mMemcache.set(key, std::vector<char>{value.cbegin(), value.cend()}, mTTL, 0);
  }

  inline void remove(const std::string & key) {
    mMemcache.remove(key);
  }

  Cache(const config::Cache & config)
      : mMemcache{fmt::format("--SERVER={:s}:{:d} {:s} {:s} {:s}",
                              config.host,
                              config.port,
                              config.noReply ? "--NOREPLY" : "",
                              config.useBinary ? "--BINARY-PROTOCOL" : "",
                              config.tcpKeepAlive ? "--TCP-KEEPALIVE" : "")},
        mTTL{config.ttl} {
#ifdef RC_DISABLE_CACHE_OPERATIONS
    logger::println<logger::WARN>("Cache: cache operations disabled");
#endif
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
   * Delete copy
   */
  void operator=(const Cache &) = delete;

private:
  memcache::Memcache mMemcache;
  time_t mTTL;
  std::mutex mMutex;
};
