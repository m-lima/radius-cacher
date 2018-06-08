//
// Created by Marcelo Lima on 8/6/18.
//

#include "server.hpp"

using boostUdp = boost::asio::ip::udp;

namespace {
  static void receive(boostUdp::socket & socket) {}
}

Server::Server(boost::asio::io_service & ioService, unsigned short port)
  : mSocket{ioService, boostUdp::endpoint{boostUdp::v4(), port}} {
    receive(mSocket);
}

