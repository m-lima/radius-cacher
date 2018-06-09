//
// Created by Marcelo Lima on 6/9/18.
//

#pragma once

struct Config {
  const unsigned short port = 1813;
  const unsigned short threadPoolSize = 8;
  const std::string key = "";
  const std::string value = "";

  Config() {};
};
