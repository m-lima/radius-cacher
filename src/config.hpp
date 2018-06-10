//
// Created by Marcelo Lima on 09/06/2018.
//

#pragma once

namespace config {
  struct Server {
    const unsigned short port = 1813;
    const unsigned short threadPoolSize = 8;
    const std::string key = "";
    const std::string value = "";

    Config() {};
  };

  struct Cache {
    const std::string host = "localhost";
    const unsigned short port = 11211;
    const time_t ttl = 5;
    const bool noReply = true;
  };
}

