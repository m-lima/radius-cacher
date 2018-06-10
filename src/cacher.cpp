//
// Created by Marcelo Lima on 09/06/2018.
//

#include "cacher.hpp"

int main() {
  Cacher cacher{"--SERVER=localhost", 5};
  cacher.set("yo", "man");
  return 0;
}
