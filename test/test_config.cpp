#include <gtest/gtest.h>

#include <mutex>

#include "../src/config.hpp"

namespace {
  std::mutex serverMutex;
  std::mutex cacheMutex;
}

TEST(Config, get_short) {
  std::unique_lock<std::mutex> lock(serverMutex);

  setenv("RADIUS_PORT", "abc", true);
  ASSERT_ANY_THROW(Config::Server::load(""));
  setenv("RADIUS_PORT", "0", true);
  ASSERT_ANY_THROW(Config::Server::load(""));
  setenv("RADIUS_PORT", "-2", true);
  ASSERT_ANY_THROW(Config::Server::load(""));
  setenv("RADIUS_PORT", "99999", true);
  ASSERT_ANY_THROW(Config::Server::load(""));
  unsetenv("RADIUS_PORT");
}

TEST(Config, get_int) {
  std::unique_lock<std::mutex> lock(cacheMutex);

  setenv("RADIUS_CACHE_TTL", "abc", true);
  ASSERT_ANY_THROW(Config::Cache::load(""));
  setenv("RADIUS_CACHE_TTL", "298347502938475", true);
  ASSERT_ANY_THROW(Config::Cache::load(""));
  unsetenv("RADIUS_CACHE_TTL");
}

TEST(Config, get_string) {
  std::unique_lock<std::mutex> lock(serverMutex);

  setenv("RADIUS_KEY", "", true);
  ASSERT_ANY_THROW(Config::Server::load(""));
  unsetenv("RADIUS_KEY");
}

TEST(Config, get_bool) {
  std::unique_lock<std::mutex> lock(serverMutex);

  setenv("RADIUS_SINGLE_CORE", "abc", true);
  ASSERT_ANY_THROW(Config::Server::load(""));
  setenv("RADIUS_SINGLE_CORE", "1", true);
  ASSERT_ANY_THROW(Config::Server::load(""));
  setenv("RADIUS_SINGLE_CORE", "true", true);
  ASSERT_ANY_THROW(Config::Server::load(""));
  unsetenv("RADIUS_SINGLE_CORE");
}

TEST(Config_Server, no_file_no_filter_loads_defaults) {
  auto server = Config::Server::load("");

  ASSERT_EQ(1813, server.port);
  ASSERT_EQ(1, server.threadPoolSize);
  ASSERT_EQ(true, server.singleCore);
  ASSERT_EQ("FRAMED_IP_ADDRESS", server.key);
  ASSERT_EQ("USER_NAME", server.value);
  ASSERT_EQ("/etc/radius-cacher/filter.txt", server.filterFile);
  ASSERT_EQ(std::chrono::minutes{720}, server.filterRefreshMinutes);
}

TEST(Config_Server, file_loads_properly) {
  auto server = Config::Server::load("res/test/server.cfg");

  ASSERT_EQ(987, server.port);
  ASSERT_EQ(654, server.threadPoolSize);
  ASSERT_EQ(false, server.singleCore);
  ASSERT_EQ("yekyekyek", server.key);
  ASSERT_EQ("lavlavlav", server.value);
  ASSERT_EQ("my_lame_file", server.filterFile);
  ASSERT_EQ(std::chrono::minutes{5588}, server.filterRefreshMinutes);
}

TEST(Config_Server, env_vars_loads_properly) {
  std::unique_lock<std::mutex> lock(serverMutex);

  setenv("RADIUS_PORT", "1234", true);
  setenv("RADIUS_THREAD_POOL_SIZE", "5678", true);
  setenv("RADIUS_SINGLE_CORE", "FALSE", true);
  setenv("RADIUS_KEY", "keykeykey", true);
  setenv("RADIUS_VALUE", "valvalval", true);
  setenv("RADIUS_FILTER_FILE", "my_super_file", true);
  setenv("RADIUS_FILTER_REFRESH_MINUTES", "8855", true);

  auto server = Config::Server::load("");

  unsetenv("RADIUS_PORT");
  unsetenv("RADIUS_THREAD_POOL_SIZE");
  unsetenv("RADIUS_SINGLE_CORE");
  unsetenv("RADIUS_KEY");
  unsetenv("RADIUS_VALUE");
  unsetenv("RADIUS_FILTER_FILE");
  unsetenv("RADIUS_FILTER_REFRESH_MINUTES");

  ASSERT_EQ(1234, server.port);
  ASSERT_EQ(5678, server.threadPoolSize);
  ASSERT_EQ(false, server.singleCore);
  ASSERT_EQ("keykeykey", server.key);
  ASSERT_EQ("valvalval", server.value);
  ASSERT_EQ("my_super_file", server.filterFile);
  ASSERT_EQ(std::chrono::minutes{8855}, server.filterRefreshMinutes);
}

TEST(Config_Server, env_vars_overloads_file) {
  std::unique_lock<std::mutex> lock(serverMutex);

  setenv("RADIUS_PORT", "1234", true);
  setenv("RADIUS_THREAD_POOL_SIZE", "5678", true);
  setenv("RADIUS_FILTER_FILE", "my_super_file", true);

  auto server = Config::Server::load("res/test/server.cfg");

  unsetenv("RADIUS_PORT");
  unsetenv("RADIUS_THREAD_POOL_SIZE");
  unsetenv("RADIUS_FILTER_FILE");

  ASSERT_EQ(1234, server.port);
  ASSERT_EQ(5678, server.threadPoolSize);
  ASSERT_EQ(false, server.singleCore);
  ASSERT_EQ("yekyekyek", server.key);
  ASSERT_EQ("lavlavlav", server.value);
  ASSERT_EQ("my_super_file", server.filterFile);
  ASSERT_EQ(std::chrono::minutes{5588}, server.filterRefreshMinutes);
}

TEST(Config_Cache, test_no_file_no_env_loads_default) {
  auto cache = Config::Cache::load("");

  ASSERT_EQ("localhost", cache.host);
  ASSERT_EQ(11211, cache.port);
  ASSERT_EQ(5400, cache.ttl);
  ASSERT_EQ(true, cache.noReply);
  ASSERT_EQ(true, cache.useBinary);
  ASSERT_EQ(true, cache.tcpKeepAlive);
}

TEST(Config_Cache, file_loads_properly) {
  auto cache = Config::Cache::load("res/test/cache.cfg");

  ASSERT_EQ("my_awesome_host", cache.host);
  ASSERT_EQ(4321, cache.port);
  ASSERT_EQ(9876, cache.ttl);
  ASSERT_EQ(false, cache.noReply);
  ASSERT_EQ(false, cache.useBinary);
  ASSERT_EQ(false, cache.tcpKeepAlive);
}

TEST(Config_Cache, env_vars_loads_properly) {
  std::unique_lock<std::mutex> lock(cacheMutex);

  setenv("RADIUS_CACHE_HOST", "my_lame_host", true);
  setenv("RADIUS_CACHE_PORT", "5432", true);
  setenv("RADIUS_CACHE_TTL", "5678", true);
  setenv("RADIUS_CACHE_NO_REPLY", "FALSE", true);
  setenv("RADIUS_CACHE_USE_BINARY", "FALSE", true);
  setenv("RADIUS_CACHE_TCP_KEEP_ALIVE", "FALSE", true);

  auto cache = Config::Cache::load("");

  unsetenv("RADIUS_CACHE_HOST");
  unsetenv("RADIUS_CACHE_PORT");
  unsetenv("RADIUS_CACHE_TTL");
  unsetenv("RADIUS_CACHE_NO_REPLY");
  unsetenv("RADIUS_CACHE_USE_BINARY");
  unsetenv("RADIUS_CACHE_TCP_KEEP_ALIVE");

  ASSERT_EQ("my_lame_host", cache.host);
  ASSERT_EQ(5432, cache.port);
  ASSERT_EQ(5678, cache.ttl);
  ASSERT_EQ(false, cache.noReply);
  ASSERT_EQ(false, cache.useBinary);
  ASSERT_EQ(false, cache.tcpKeepAlive);
}

TEST(Config_Cache, env_vars_overloads_file) {
  std::unique_lock<std::mutex> lock(cacheMutex);

  setenv("RADIUS_CACHE_HOST", "my_lame_host", true);
  setenv("RADIUS_CACHE_TTL", "5678", true);

  auto cache = Config::Cache::load("res/test/cache.cfg");

  unsetenv("RADIUS_CACHE_HOST");
  unsetenv("RADIUS_CACHE_TTL");

  ASSERT_EQ("my_lame_host", cache.host);
  ASSERT_EQ(4321, cache.port);
  ASSERT_EQ(5678, cache.ttl);
  ASSERT_EQ(false, cache.noReply);
  ASSERT_EQ(false, cache.useBinary);
  ASSERT_EQ(false, cache.tcpKeepAlive);
}
