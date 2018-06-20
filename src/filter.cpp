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

Filter::Filter(const Config::Server & config)
    : mFilePath{config.filterFile},
      mRefreshMinutes{config.filterRefreshMinutes} {
  reloadLoop();
}

bool Filter::contains(std::uint64_t value) const {
  return std::binary_search(mFilters[mCurrent].cbegin(),
                            mFilters[mCurrent].cend(),
                            value);
}

void Filter::reloadLoop() {
  std::thread t{[this]() {
    reload();

    LOG(logger::LOG,
        "Filter::reloadLoop: Scheduling new filter reload for {:d} minutes from now",
        mRefreshMinutes.count());
    std::this_thread::sleep_for(mRefreshMinutes);

    reloadLoop();
  }};
  t.detach();
}

void Filter::reload() {
  LOG(logger::INFO, "Filter::reload: reloading");

  std::ifstream stream(mFilePath);

  if (!stream.is_open()) {
    LOG(logger::ERROR, "Filter::reload: could not load filter file \"{:s}\"", mFilePath);
  }

  mFilters[!mCurrent].clear();

  std::string buffer;
  try {
    while (std::getline(stream, buffer)) {
      std::smatch match;
      if (std::regex_search(buffer, match, REGEX)) {
        try {
          mFilters[!mCurrent].emplace_back(std::stoull(match[1]));
        } catch (const std::exception & ex) {
          LOG(logger::WARN, "Filter::reload: failed to parse value {:s}: {}", match[1], ex.what());
        }
      }
    }
  } catch (const std::exception & ex) {
    LOG(logger::WARN, "Filter::reload: exception while reloading filter: {}", ex.what());
  }

  std::sort(mFilters[!mCurrent].begin(), mFilters[!mCurrent].end());
  mCurrent.swap();

  LOG(logger::LOG, "Filter::reload: Enabled new filter with {:d} entries", mFilters[mCurrent].size());
  for (const auto value : mFilters[mCurrent]) {
    LOG(logger::INFO, "Filter::reload: Filtering {:d}", value);
  }
}

