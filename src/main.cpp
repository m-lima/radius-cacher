//
// Created by Marcelo Lima on 08/06/2018.
//

#include <string>
#include <vector>
#include <thread>

#include <mfl/out.hpp>
#include <mfl/args.hpp>

#include "server.hpp"
#include "radius_parser.hpp"

/**
 * Prints the usage for the application
 *
 * @param appPath the raw argv[0] value for the executable call
 * @param file which file to print to
 */
void printUsage(std::FILE * file = stdout) {
  mfl::out::println(file, "Usage for radius-cacher:");
  mfl::out::println(file, "radius-cacher [-s SERVER_CONFIG] [-m CACHE_CONFIG]");
  mfl::out::println(file, "  {:<15s}{:s}",
                    "SERVER_CONFIG",
                    "Server configuration file (default: /etc/radius-cacher/server.conf)");
  mfl::out::println(file, "  {:<15s}{:s}",
                    "CACHE_CONFIG",
                    "cache configuration file (default: /etc/radius-cacher/cache.conf)");
  mfl::out::println(file, "");

  mfl::out::println(file, "Usage for help:");
  mfl::out::println(file, "radius-cacher -h");
}

int main(int argc, char * argv[]) {
  if (mfl::args::findOption(argv, argv + argc, "-h")) {
    printUsage();
    return 0;
  }

  auto serverConfig = std::string{"/etc/radius-cacher/server.conf"};
  auto cacheConfig = std::string{"/etc/radius-cacher/cache.conf"};

  try {
    auto aServerConfig = mfl::args::extractOption(argv, argv + argc, "-s");
    if (aServerConfig) {
      serverConfig = std::string{aServerConfig};
    }

    auto aCacheConfig = mfl::args::extractOption(argv, argv + argc, "-m");
    if (aCacheConfig) {
      cacheConfig = std::string{aCacheConfig};
    }

  } catch (const std::exception & ex) {
    LOG(logger::FATAL, "main: error parsing arguments: {:s}", ex.what());
    printUsage(stderr);
    return -1;
  }

  try {
    Config config{serverConfig, cacheConfig};
    LOG(logger::DEBUG, "main: configuration built");

    Server server;
    RadiusParser parser{config.server};
//    server.run(config, RadiusParser{});
    server.run(config, parser);

  } catch (const std::exception & ex) {
    LOG(logger::FATAL, "main: terminating due to exception: {}", ex.what());
    return -1;
  }

  return 0;
}
