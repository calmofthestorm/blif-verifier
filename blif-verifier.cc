#include <cassert>
#include <iostream>
#include <iterator>
#include <map>
#include <vector>
#include <fstream>

#include "blif.h"

using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::ostream;
using std::string;

using blifverifier::BLIF;

void writeArray(const std::vector<string>& data, ostream& out) {
  if (data.size() == 0) {
    out << "{}";
    return;
  } else if (data.size() == 1) {
    out << "{\"" << *data.begin() << "\"}";
    return;
  }
  out << "{\"";

  bool first = true;
  for (const auto& pi : data) {
    if (!first) {
      out << "\", \"";
    }
    first = false;
    out << pi;
  }

  out << "\"}";
}

int main(int argc, char* argv[]) {
  if (argc != 4) {
    cout << "Usage: " << argv[0]
         << " circuit1.blif circuit2.blif output.c" << endl;
    return 1;
  }

  // Load all data.
  int i = 0;
  BLIF circuit0(ifstream(argv[++i]));
  BLIF circuit1(ifstream(argv[++i]));
  ofstream outfile(argv[++i]);

  // Check if the two circuits are not possibly equivalent (different inputs,
  // etc).
  if (circuit0.triviallyNotEquivalent(circuit1, cout)) {
    return 2;
  }

  // Libraries
  outfile << "#include <stdlib.h>\n#include <stdarg.h>\n#include <stdio.h>\n\n";

  // Verifier common info
  outfile << "const size_t numInputs = " << circuit0.getPrimaryInputs().size()
          << ";\nconst size_t numOutputs = " << circuit0.getPrimaryOutputs().size()
          << ";\n";

  //TODO: handle more gracefully
  assert(circuit0.getPrimaryInputs().size() > 0);

  // Primary input names
  outfile << "const char* const INPUT_NAMES[] = ";
  writeArray(circuit0.getPrimaryInputs(), outfile);
  outfile << ";\n";

  // Primary output names
  outfile << "const char* const OUTPUT_NAMES[] = ";
  writeArray(circuit0.getPrimaryOutputs(), outfile);
  outfile << ";\n";

  // Write the circuit functions
  circuit0.writeEvaluator(outfile, "circuit0");
  circuit1.writeEvaluator(outfile, "circuit1");
}
