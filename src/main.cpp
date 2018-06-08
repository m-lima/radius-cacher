//
// Created by Marcelo Lima on 8/6/18.
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

void printUsage(char * appPath, std::FILE * file = stdout) {
  auto appName = boost::filesystem::path{appPath}.stem().string();

  mfl::out::println(file, "Usage for {:s}:", appName);
  mfl::out::println(file, "{:s} [-p PORT] [-t THREAD_COUNT]", appName);
  mfl::out::println(file, "  {:<15s}{:s}", "PORT", "The port to listen (default: 1813)");
  mfl::out::println(file, "  {:<15s}{:s}", "THREAD_COUNT", "Number of threads to use (default: 8)");
  mfl::out::println(file, "");

  mfl::out::println(file, "Usage for help:");
  mfl::out::println(file, "{:s} -h", appName);
}

int main(int argc, char * argv[]) {
  if (mfl::args::findOption(argv, argv + argc, "-h")) {
    printUsage(argv[0]);
    return 0;
  }

  unsigned short port = 1813;
  unsigned short threadCount = 8;

  try {
    auto aPort = mfl::args::extractOption(argv, argv + argc, "-p");
    if (aPort) {
      int tPort = std::atoi(aPort);
      if (tPort < 1 || tPort > 65535) {
        throw std::runtime_error("port should be between 1 and 65535");
      }
      port = static_cast<unsigned short>(tPort);
    }

    auto aThreadCount = mfl::args::extractOption(argv, argv + argc, "-t");
    if (aThreadCount) {
      int tThreadCount = std::atoi(aThreadCount);
      if (tThreadCount < 1 || tThreadCount > 65535) {
        throw std::runtime_error("thread count should be between 1 and 65535");
      }
      threadCount = static_cast<unsigned short>(tThreadCount);
    }

  } catch (std::exception & ex) {
    mfl::out::println(stderr, "Error parsing arguments: {:s}", ex.what());
    mfl::out::println(stderr, "");
    printUsage(argv[0], stderr);
    return -1;
  }

  boost::asio::io_service ioService;

  Server server{ioService, port};

  std::vector<std::thread> threadPool;
  threadPool.reserve(threadCount);

  for(unsigned short i = 0; i < threadCount; ++i) {
    threadPool[i] = std::thread{[&ioService](){ ioService.run(); }};
  }

  for(unsigned short i = 0; i < threadCount; ++i) {
    threadPool[i].join();
  }

  return 0;
}

//std::string make_daytime_string()
//{
//  time_t now = time(0);
//  return ctime(&now);
//}
//
//int main()
//{
//  try
//  {
//    boost::asio::io_service io_service;
//
//    boost_ip::udp::acceptor acceptor(io_service,
//                                            boost_ip::udp::endpoint(boost_ip::udp::v4(), 1813));
//
//    for (;;)
//    {
//      boost::asio::ip::udp::socket socket(io_service);
//      acceptor.accept(socket);
//
//      std::string message = make_daytime_string();
//
//      boost::system::error_code ignored_error;
//      boost::asio::write(socket, boost::asio::buffer(message), ignored_error);
//    }
//  }
//  catch (std::exception& e)
//  {
//    mfl::out::println(stderr, "{}", e.what());
//  }
//
//  return 0;
//}