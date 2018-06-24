//
// Created by Marcelo Lima on 13/06/2018.
//

#pragma once

#include <vector>
#include <array>
#include <string>

#include <chrono>

class Filter {
private:

  // For testing
  friend class FilterTester;

  std::size_t mCurrent{0};
  std::array<std::vector<std::uint64_t>, 2> mFilters{};
  const std::string mFilePath;
  const std::chrono::seconds mRefreshSeconds;

  void reload();
  void reloadLoop();

public:

  Filter(std::string filePath, std::chrono::seconds refreshSeconds);
  Filter(std::string filePath, std::chrono::minutes refreshMinutes)
      : Filter{std::move(filePath), std::chrono::duration_cast<std::chrono::seconds>(refreshMinutes)} {};

  bool contains(std::uint64_t value) const;

  ~Filter() = default;
  Filter(const Filter &) = delete;
  Filter(Filter &&) = delete;
  void operator=(const Filter &) = delete;

};


