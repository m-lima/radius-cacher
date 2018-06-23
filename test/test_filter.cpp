#include <gtest/gtest.h>

#include "../src/filter.hpp"

struct FilterTester {
  Filter filter;

  FilterTester(std::string path, unsigned short seconds = 0)
      : filter{std::move(path), std::chrono::seconds{seconds}} {}

  auto inline getFilterSizer() const {
    return filter.mFilters[filter.mCurrent].size();
  }

  void inline setFilePath(const std::string & path) {
    auto * targetPath = (std::string *)&(filter.mFilePath);
    *targetPath = path;
  }

  void inline setTimer(const std::string & path) {
    auto * targetPath = (std::string *)&(filter.mFilePath);
    *targetPath = path;
  }

  void inline reload() {
    filter.reload();
  }
};

TEST(Filter, no_file_loads_empty) {
  FilterTester tester("");
  ASSERT_EQ(0, tester.getFilterSizer());
}

TEST(Filter, filter_loads_properly) {
  FilterTester tester("res/test/filter.txt");

  ASSERT_EQ(4, tester.getFilterSizer());
  ASSERT_TRUE(tester.filter.contains(123));
  ASSERT_TRUE(tester.filter.contains(1234567890123456));
  ASSERT_TRUE(tester.filter.contains(567));
  ASSERT_TRUE(tester.filter.contains(345));
}

TEST(Filter, filter_reloads_properly) {
  FilterTester tester("res/test/filter.txt");

  ASSERT_EQ(4, tester.getFilterSizer());
  ASSERT_TRUE(tester.filter.contains(123));
  ASSERT_TRUE(tester.filter.contains(1234567890123456));
  ASSERT_TRUE(tester.filter.contains(567));
  ASSERT_TRUE(tester.filter.contains(345));

  tester.setFilePath("res/test/filter2.txt");
  tester.reload();

  ASSERT_EQ(4, tester.getFilterSizer());
  ASSERT_TRUE(tester.filter.contains(9123));
  ASSERT_TRUE(tester.filter.contains(91234567890123456));
  ASSERT_TRUE(tester.filter.contains(9567));
  ASSERT_TRUE(tester.filter.contains(9345));
}

TEST(Filter, filter_reload_failure_should_not_clear_filter) {
  FilterTester tester("res/test/filter.txt");

  ASSERT_EQ(4, tester.getFilterSizer());
  ASSERT_TRUE(tester.filter.contains(123));
  ASSERT_TRUE(tester.filter.contains(1234567890123456));
  ASSERT_TRUE(tester.filter.contains(567));
  ASSERT_TRUE(tester.filter.contains(345));

  tester.setFilePath("");
  tester.reload();

  ASSERT_EQ(4, tester.getFilterSizer());
  ASSERT_TRUE(tester.filter.contains(123));
  ASSERT_TRUE(tester.filter.contains(1234567890123456));
  ASSERT_TRUE(tester.filter.contains(567));
  ASSERT_TRUE(tester.filter.contains(345));
}

TEST(Filter, filter_reloader_works_properly) {
  FilterTester tester("res/test/filter.txt", 1);
  tester.setFilePath("res/test/filter2.txt");

  ASSERT_EQ(4, tester.getFilterSizer());
  ASSERT_TRUE(tester.filter.contains(123));
  ASSERT_TRUE(tester.filter.contains(1234567890123456));
  ASSERT_TRUE(tester.filter.contains(567));
  ASSERT_TRUE(tester.filter.contains(345));

  sleep(2);

  ASSERT_EQ(4, tester.getFilterSizer());
  ASSERT_TRUE(tester.filter.contains(9123));
  ASSERT_TRUE(tester.filter.contains(91234567890123456));
  ASSERT_TRUE(tester.filter.contains(9567));
  ASSERT_TRUE(tester.filter.contains(9345));
}
