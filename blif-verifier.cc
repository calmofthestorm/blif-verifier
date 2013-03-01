#include <iostream>
#include <map>
#include <vector>
#include <fstream>

#include "blif.h"

blifverifier::BLIF loadCircuit(const char* const filename);

int main(int argc, char* argv[]) {
  if (argc != 4) {
    std::cout << "Usage: " << argv[0]
              << "  circuit1.blif circuit2.blif output.c" << std::endl;
    return 1;
  }

  // Load all data.
  int i = 0;
  auto circuit0 = loadCircuit(argv[++i]);
  auto circuit1 = loadCircuit(argv[++i]);
  std::ofstream outfile(argv[++i]);

  // Write out the generated code.
}

blifverifier::BLIF loadCircuit(const char* const filename) {
  std::ifstream is(filename);
  blifverifier::BLIF circuit(is);
  std::cout << sizeof(decltype(circuit));
  is.close();
  return circuit;
}
