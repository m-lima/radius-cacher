//
// Created by Marcelo Lima on 09/06/2018.
//

#pragma once

#include <string>
#include <vector>

#include <libmemcached/memcached.hpp>

class Cacher {
public:
  Cacher(const std::string & config, const time_t ttl)
      : mMemcache{config},
        mTTL{ttl} {}

  inline void set(const std::string & key, const std::string & value) {
    mMemcache.set(key, std::vector<char>{value.cbegin(), value.cend()}, mTTL, 0);
  }

  inline void remove(const std::string & key) {
    mMemcache.remove(key);
  }

private:
  memcache::Memcache mMemcache;
  time_t mTTL;
};
