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
  blifverifier::BLIF circuit0(std::ifstream(argv[++i]));
  blifverifier::BLIF circuit1(std::ifstream(argv[++i]));
  std::ofstream outfile(argv[++i]);

  // Write out the generated code.
}
