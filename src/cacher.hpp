//
// Created by Marcelo Lima on 09/06/2018.
//

#pragma once

#include <string>
#include <vector>

#include <libmemcached/memcached.hpp>
#include <fmt/format.h>

#include "config.hpp"

class Cacher {
public:
  inline void set(const std::string & key, const std::string & value) {
    mMemcache.set(key, std::vector<char>{value.cbegin(), value.cend()}, mTTL, 0);
  }

  inline void remove(const std::string & key) {
    mMemcache.remove(key);
  }

  auto get() {
    return &mMemcache;
  }

  Cacher(const config::Cache & config)
      : mMemcache{fmt::format("--SERVER={:s}:{:d} {:s} {:s}",
                              config.host,
                              config.port,
                              config.noReply ? "--NOREPLY" : "",
                              config.useBinary ? "--BINARY-PROTOCOL" : "")},
        mTTL{config.ttl} {}

  /**
   * Must declare since compiler will omit due to copy constructor deletion
   */
  ~Cacher() = default;

  /**
   * Delete copy constructor
   */
  Cacher(const Cacher &) = delete;

  /**
   * Delete copy
   */
  void operator=(const Cacher &) = delete;

private:
  memcache::Memcache mMemcache;
  time_t mTTL;
};
