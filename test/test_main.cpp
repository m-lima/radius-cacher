//
// Created by Marcelo Lima on 13/06/2018.
//

#include <gtest/gtest.h>
#include <mfl/args.hpp>

#include "../src/logger.hpp"

namespace logger {
  Level verboseLevel = logger::NONE;
}

int main(int argc, char * argv[]) {

  auto aVerboseLevel = mfl::args::extractOption(argv, argv + argc, "-v");
  if (aVerboseLevel) {
    if (!logger::setVerboseLevel(aVerboseLevel)) {
      LOG(logger::FATAL, "Invalid verbose level set: {:s}", aVerboseLevel);
      return -1;
    }
  }

  try {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
  } catch (...) {}

  return -1;
}
