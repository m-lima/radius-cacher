//
// Created by Marcelo Lima on 13/06/2018.
//

#include "consent_filter.hpp"

#include <fstream>
#include <regex>

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "logger.hpp"

namespace {
  const std::regex REGEX{("\"([[:digit:]]+)\"")};
}

ConsentFilter::ConsentFilter(const Config::Server & config)
    : mConsentFilePath{config.consentFile} {
  reload();

//  boost::asio::io_service ioService;
//
//  boost::asio::deadline_timer reloader(ioService, boost::posix_time::minutes(config.consentRefreshMinutes));
//  reloader.async_wait(&reload);
}

bool ConsentFilter::contains(std::uint64_t value) const {
  return std::binary_search(mConsents[mCurrent].cbegin(),
                            mConsents[mCurrent].cend(),
                            value);
}

void ConsentFilter::reload() {
  std::ifstream stream(mConsentFilePath);

  if (!stream.is_open()) {
    LOG(logger::ERROR, "ConsentFilter::reload: could not load configuration file \"{:s}\".", mConsentFilePath);
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
          LOG(logger::WARN, "ConsentFilter::reload: failed to parse value {:s}: {}",
                                        match[1],
                                        ex.what());
        }
      }
    }
  } catch (const std::exception & ex) {
    LOG(logger::WARN, "ConsentFilter::reload: exception while reloadin consent: {}", ex.what());
  }

  std::sort(mConsents[!mCurrent].begin(), mConsents[!mCurrent].end());
  mCurrent.swap();

  LOG(logger::INFO, "Enabled new filter");
  for (const auto v : mConsents[mCurrent]) {
    logger::println<logger::DEBUG>("Filtering {:d}", v);
  }
}

