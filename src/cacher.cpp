//
// Created by Marcelo Lima on 09/06/2018.
//

#include <mfl/out.hpp>

#include "config.hpp"
#include "logger.hpp"
#include "cache.hpp"


int main() {
  try {
    Cache cache{config::Cache::load("/home/archer/cache.conf")};
  } catch (std::exception & ex) {
    logger::println<logger::FATAL>("main: terminating due to exception: {}", ex.what());
    return -1;
  }

  return 0;
}
