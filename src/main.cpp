//
// Created by Marcelo Lima on 08/06/2018.
//

#include <string>
#include <vector>
#include <thread>
#include <stdexcept>

#include <mfl/out.hpp>
#include <mfl/args.hpp>

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

#include "server.hpp"
#include "config.hpp"
#include "logger.hpp"

/**
 * Prints the usage for the application
 *
 * @param appPath the raw argv[0] value for the executable call
 * @param file which file to print to
 */
void printUsage(char * appPath, std::FILE * file = stdout) {
  auto appName = boost::filesystem::path{appPath}.stem().string();

  mfl::out::println(file, "Usage for {:s}:", appName);
  mfl::out::println(file, "{:s} [-s SERVER_CONFIG] [-m MEMCACHED_CONFIG]", appName);
  mfl::out::println(file, "  {:<15s}{:s}", "SERVER_CONFIG", "Server configuration file (default: server.conf)");
  mfl::out::println(file, "  {:<15s}{:s}",
                    "MEMCACHED_CONFIG",
                    "Memcached configuration file (default: memcached.conf)");
  mfl::out::println(file, "");

  mfl::out::println(file, "Usage for help:");
  mfl::out::println(file, "{:s} -h", appName);
}

int main(int argc, char * argv[]) {
  if (mfl::args::findOption(argv, argv + argc, "-h")) {
    printUsage(argv[0]);
    return 0;
  }

  auto serverConfig = std::string{"server.conf"};
  auto memcachedConfig = std::string{"memcached.conf"};

  try {
    auto aServerConfig = mfl::args::extractOption(argv, argv + argc, "-s");
    if (aServerConfig) {
      serverConfig = std::string{aServerConfig};
    }

    auto aMemcachedConfig = mfl::args::extractOption(argv, argv + argc, "-m");
    if (aMemcachedConfig) {
      memcachedConfig = std::string{aMemcachedConfig};
    }

  } catch (std::exception & ex) {
    logger::errPrintln<logger::FATAL>("Error parsing arguments: {:s}", ex.what());
    logger::errPrintln<logger::FATAL>();
    printUsage(argv[0], stderr);
    return -1;
  }

  Config config;

  boost::asio::io_service ioService;

  Server server{ioService, config.port};

  if (config.threadPoolSize == 1) {
    logger::println<logger::LOG>("Listening on UDP {:d} on a single thread", config.port);
    ioService.run();
  } else {
    std::vector<std::thread> threadPool;
    threadPool.reserve(config.threadPoolSize);

    for (unsigned short i = 0; i < config.threadPoolSize; ++i) {
      threadPool[i] = std::thread{[&ioService]() { ioService.run(); }};
    }

    logger::println<logger::LOG>("Listening on UDP {:d} on {:d} threads", config.port, config.threadPoolSize);

    for (unsigned short i = 0; i < config.threadPoolSize; ++i) {
      threadPool[i].join();
    }
  }

  return 0;
}
