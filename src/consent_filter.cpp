//
// Created by Marcelo Lima on 13/06/2018.
//

#include "consent_filter.hpp"

#include <fstream>
#include <regex>
#include <thread>

#include "logger.hpp"

namespace {
  const std::regex REGEX{("\"([[:digit:]]+)\"")};
}

ConsentFilter::ConsentFilter(const Config::Server & config)
    : mConsentFilePath{config.consentFile},
      mRefreshMinutes{config.consentRefreshMinutes} {
  reloadLoop();
}

bool ConsentFilter::contains(std::uint64_t value) const {
  return std::binary_search(mConsents[mCurrent].cbegin(),
                            mConsents[mCurrent].cend(),
                            value);
}

void ConsentFilter::reloadLoop() {
  std::thread t{[this]() {
    reload();

    LOG(logger::INFO,
        "ConsentFilter::reloadLoop: Scheduling new consent reload for {:d} minutes from now",
        mRefreshMinutes.count());
    std::this_thread::sleep_for(mRefreshMinutes);

    reloadLoop();
  }};
  t.detach();
}

void ConsentFilter::reload() {
  LOG(logger::DEBUG, "ConsentFilter::reload: reloading");

  std::ifstream stream(mConsentFilePath);

  if (!stream.is_open()) {
    LOG(logger::ERROR, "ConsentFilter::reload: could not load configuration file \"{:s}\"", mConsentFilePath);
  }

  mConsents[!mCurrent].clear();

  std::string buffer;
  try {
    while (std::getline(stream, buffer)) {
      std::smatch match;
      if (std::regex_match(buffer, match, REGEX)) {
        try {
          mConsents[!mCurrent].emplace_back(std::stoll(match[1]));
        } catch (const std::exception & ex) {
          LOG(logger::WARN, "ConsentFilter::reload: failed to parse value {:s}: {}", match[1], ex.what());
        }
      }
    }
  } catch (const std::exception & ex) {
    LOG(logger::WARN, "ConsentFilter::reload: exception while reloading consent: {}", ex.what());
  }

  std::sort(mConsents[!mCurrent].begin(), mConsents[!mCurrent].end());
  mCurrent.swap();

  LOG(logger::INFO, "ConsentFilter::reload: Enabled new filter");
  for (const auto v : mConsents[mCurrent]) {
    logger::println<logger::INFO>("Filtering {:d}", v);
  }
}

