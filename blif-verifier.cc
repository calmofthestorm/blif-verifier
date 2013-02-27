#include <iostream>
#include <fstream>

#include "blif.h"

int main() {
  std::ifstream is("segfault.blif");
  blifverifier::BLIF a(is);
}
