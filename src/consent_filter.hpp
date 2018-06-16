//
// Created by Marcelo Lima on 13/06/2018.
//

#pragma once

#include <vector>
#include <array>
#include <string>

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "config.hpp"

class ConsentFilter {
private:

  struct CurrentVector {
    bool flag = false;

    operator std::size_t () const {
      return flag ? 1 : 0;
    }

    std::size_t operator ! () const {
      return flag ? 0 : 1;
    }

    void swap() {
      flag = !flag;
    }
  };

  std::array<std::vector<std::uint64_t>, 2> mConsents;
  CurrentVector mCurrent;
  const std::string mConsentFilePath;
//  boost::asio::deadline_timer mReloader;

  void reload();

public:

  ConsentFilter(const Config::Server & config);
  bool contains(std::uint64_t value) const;

  ~ConsentFilter() = default;
  ConsentFilter(const ConsentFilter &) = delete;
  ConsentFilter(ConsentFilter &&) = delete;
  void operator=(const ConsentFilter &) = delete;

};


