//
// Created by Marcelo Lima on 6/9/18.
//

#pragma once

#include <string>
#include <vector>

#include <libmemcached/memcached.hpp>

class Cacher {
public:
  Cacher(const std::string & config)
      : memcache{config} {}

  inline void push(const std::string & key, const std::string & value) {
    memcache.add(key, std::vector<char>{value.cbegin(), value.cend()});
  }

  inline void remove(const std::string & key) {
    memcache.remove(key);
  }

private:
  memcache::Memcache memcache;
};
