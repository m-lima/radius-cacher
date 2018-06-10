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

  auto serverConfigFile = std::string{"server.conf"};
  auto memcachedConfigFile = std::string{"memcached.conf"};

  try {
    auto aServerConfig = mfl::args::extractOption(argv, argv + argc, "-s");
    if (aServerConfig) {
      serverConfigFile = std::string{aServerConfig};
    }

    auto aMemcachedConfig = mfl::args::extractOption(argv, argv + argc, "-m");
    if (aMemcachedConfig) {
      memcachedConfigFile = std::string{aMemcachedConfig};
    }

  } catch (std::exception & ex) {
    logger::errPrintln<logger::FATAL>("main: error parsing arguments: {:s}", ex.what());
    printUsage(argv[0], stderr);
    return -1;
  }

  try {
    Server server{config::Server::load(serverConfigFile)};
    server.run();
  } catch (std::exception & ex) {
    logger::println<logger::FATAL>("main: terminating due to exception: {}", ex.what());
    return -1;
  }

  return 0;
}
