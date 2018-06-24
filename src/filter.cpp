//
// Created by Marcelo Lima on 13/06/2018.
//

#include "filter.hpp"

#include <fstream>
#include <regex>
#include <thread>

#include "logger.hpp"

namespace {
  const std::regex REGEX{("([[:digit:]]+)")};
}

Filter::Filter(std::string path, std::chrono::seconds refreshSeconds)
    : mFilePath{std::move(path)},
      mRefreshSeconds{refreshSeconds} {
  reload();
  if (mRefreshSeconds.count() > 0) {
    reloadLoop();
  }
}

bool Filter::contains(std::uint64_t value) const {
  auto index = mCurrent;
  return std::binary_search(mFilters[index].cbegin(),
                            mFilters[index].cend(),
                            value);
}

void Filter::reloadLoop() {
  std::thread t{[this]() {
    {
      auto nextTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now() + mRefreshSeconds);
      LOG(logger::LOG,
          "Filter::reloadLoop: Scheduling new filter reload at {:%F %T}",
          *std::localtime(&nextTime));
      std::this_thread::sleep_for(mRefreshSeconds);
    }

    reload();
    reloadLoop();
  }};
  t.detach();
}

void Filter::reload() {
  LOG(logger::INFO, "Filter::reload: reloading");

  std::ifstream stream(mFilePath);

  if (!stream.is_open()) {
    LOG(logger::ERROR, "Filter::reload: could not load filter file \"{:s}\"", mFilePath);
    return;
  }

  auto index = 1 - mCurrent;
  mFilters[index].clear();

  std::string buffer;
  try {
    while (std::getline(stream, buffer)) {
      std::smatch match;
      if (std::regex_search(buffer, match, REGEX)) {
        try {
          mFilters[index].emplace_back(std::stoull(match[1]));
        } catch (const std::exception & ex) {
          LOG(logger::WARN, "Filter::reload: failed to parse value {:s}: {}", match[1], ex.what());
        }
      }
    }
  } catch (const std::exception & ex) {
    LOG(logger::WARN, "Filter::reload: exception while reloading filter: {}", ex.what());
  }

  std::sort(mFilters[index].begin(), mFilters[index].end());

  mCurrent = index;

  LOG(logger::LOG, "Filter::reload: Enabled new filter with {:d} entries", mFilters[index].size());
  for (const auto value : mFilters[index]) {
    LOG(logger::INFO, "Filter::reload: Filtering {:d}", value);
  }
}

