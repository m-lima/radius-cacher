//
// Created by Marcelo Lima on 13/06/2018.
//

#include <gtest/gtest.h>

#include "../src/logger.hpp"

namespace logger {
  Level verboseLevel = logger::LOG;
}

int main(int argc, char * argv[]) {
  try {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
  } catch (...) {}

  return -1;
}
