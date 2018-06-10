//
// Created by Marcelo Lima on 09/06/2018.
//

#include "cacher.hpp"

#include <mfl/out.hpp>

#include "config.hpp"
#include "logger.hpp"

int main() {
  try {
    Cacher cacher{config::Cache::load("/home/archer/cache.conf")};
    auto mem = cacher.get();

    mfl::out::println("Binary: {}", mem->getBehavior(MEMCACHED_BEHAVIOR_BINARY_PROTOCOL));
    mfl::out::println("ConnTimeOut: {}", mem->getBehavior(MEMCACHED_BEHAVIOR_CONNECT_TIMEOUT));
    mfl::out::println("NoBlock: {}", mem->getBehavior(MEMCACHED_BEHAVIOR_NO_BLOCK));
    mfl::out::println("TCP-keepalive: {}", mem->getBehavior(MEMCACHED_BEHAVIOR_TCP_KEEPALIVE));
    mfl::out::println("TCP-keepidle: {}", mem->getBehavior(MEMCACHED_BEHAVIOR_TCP_KEEPIDLE));
    mfl::out::println("TCP-nodelay: {}", mem->getBehavior(MEMCACHED_BEHAVIOR_TCP_NODELAY));
  } catch (std::exception & ex) {
    logger::println<logger::FATAL>("main: terminating due to exception: {}", ex.what());
    return -1;
  }

  return 0;
}
