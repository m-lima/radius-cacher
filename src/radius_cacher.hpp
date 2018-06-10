//
// Created by Marcelo Lima on 10/06/2018.
//

#pragma once

#include <cstddef>

class RadiusCacher {
  template <typename E, typename I>
  void parse(const E &,
             std::size_t bytesReceived,
             I begin,
             I end);
};


