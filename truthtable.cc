#include "truthtable.h"

#include <cassert>

#include <algorithm>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stack>
#include <string>
#include <vector>

#include "tokenizer.h"

using std::istream;
using std::istringstream;
using std::ostream;
using std::string;
using std::vector;
using std::unordered_set;

namespace blifverifier {

TruthTable::TruthTable()
  : mKind(TTKind::INPUT) { }

bool TruthTable::isValidTTEntry(const string& line, int num_entries) {
  if (num_entries != static_cast<int>(line.size())) {
    return false;
  }

  for (const auto& c : line) {
    if (c != TOKENS::ZERO && c != TOKENS::ONE && c != TOKENS::NC) {
      return false;
    }
  }

  return true;
}

TruthTable::TruthTable(Tokenizer::LineTokenReader& reader,
                       vector<string>&& inputs, TTKind kind)
  : mInputs(inputs), mKind(kind) {
  // Parse the truth table.
  // Keep reading logic lines until we hit something else, and push it back.
  while (reader.isGood()) {
    auto tokens = reader.readLine();
    // TODO: clean this.
    if (tokens.size() == 1 && inputs.size() == 0 &&
         (tokens[0][0] == TOKENS::ZERO || tokens[0][0] == TOKENS::ONE ||
          tokens[0][0] == TOKENS::NC)) {
      TruthTableEntry tte("", tokens[0][0]);
      addEntry(tte);
    } else if (tokens.size() == 2 &&
               isValidTTEntry(tokens[0], inputs.size()) &&
               tokens[0].size() == inputs.size() && tokens[1].size() == 1 &&
               (tokens[1][0] == TOKENS::ZERO || tokens[1][0] == TOKENS::ONE ||
                tokens[1][0] == TOKENS::NC)) {
      // Is a valid logic line.
      TruthTableEntry tte(tokens[0], tokens[1][0]);
      addEntry(tte);
    } else {
      // Read a non-TT line. Push it back and finish.
      reader.putBack(std::move(tokens));
      return;
    }
  }
  // should never get here.
  assert(false);
}

TruthTableEntry::TruthTableEntry(const std::string& inputs, char output)
  : mInputs(inputs), mOutput(output) { }

char TruthTableEntry::getOutput() const {
  return mOutput;
}

// TODO: if BLIF truthtable contains a contradiction it will be resolved
//       silently as true. This should raise an error as the file is illegal.
void TruthTableEntry::generateCode(ostream& out,
                                   const vector<string>& input_names) const {
  out << "(";
  for (decltype(mInputs)::size_type i = 0; i < mInputs.size(); ++i) {
    if (mInputs[i] != TOKENS::NC) {
      if (mInputs[i] == TOKENS::ZERO) {
        out << "~";
      }
      out << input_names[i] << " & ";
    }
  }
  out << " -1)";
}

void TruthTable::generateCode(const string& name, ostream& out) const {
  if (mKind != TruthTable::TTKind::INPUT) {
    if (mKind == TruthTable::TTKind::NORMAL) {
      out << "size_t ";
    }
    out << name << " = ";
    out << "(";
    for (const auto& entry : mEntries) {
      if (entry.getOutput() == TOKENS::ONE) {
        entry.generateCode(out, mInputs);
        out << " | ";
      }
    }
    out << " 0); // strategy: naive\n";
  }
}

void TruthTable::addInput(const std::string& input) {
  assert(mKind != TTKind::INPUT);
  mInputs.push_back(input);
}

void TruthTable::addEntry(const TruthTableEntry& entry) {
  assert(mKind != TTKind::INPUT);
  mEntries.push_back(entry);
}

const vector<string>& TruthTable::getInputs() const {
  return mInputs;
}

}  // namespace blifverifier
