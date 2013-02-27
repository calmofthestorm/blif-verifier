#include <iostream>
#include <fstream>

#include "blif.h"

int main() {
  std::ifstream is("and16.blif");
  blifverifier::BLIF a(is);
  a.writeEvaluator(std::cout, "bee");
}
