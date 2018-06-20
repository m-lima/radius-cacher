//
// Created by Marcelo Lima on 13/06/2018.
//

#pragma once

#include <vector>
#include <array>
#include <string>

#include <chrono>

#include "config.hpp"

class Filter {
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

  std::array<std::vector<std::uint64_t>, 2> mFilters;
  CurrentVector mCurrent;
  const std::string mFilePath;

  const std::chrono::minutes mRefreshMinutes;

  void reload();
  void reloadLoop();

public:

  Filter(const Config::Server & config);
  bool contains(std::uint64_t value) const;

  ~Filter() = default;
  Filter(const Filter &) = delete;
  Filter(Filter &&) = delete;
  void operator=(const Filter &) = delete;

};


