//
// Created by Marcelo Lima on 10/06/2018.
//

#pragma once

#include "cache.hpp"

class RadiusCacher {
public:
  RadiusCacher(config::Cache cache)
      : mCache{cache} {};

  template <typename E, typename I>
  void operator()(const E &,
                  std::size_t bytesReceived,
                  I begin,
                  I end);
private:
  const Cache mCache;
};


