#include <algorithm>
#include <cassert>
#include <istream>
#include <iterator>
#include <ostream>
#include <sstream>
#include <stack>
#include <vector>

#include "truthtable.h"

#include <iostream>

using std::istream;
using std::istringstream;
using std::ostream;
using std::string;
using std::vector;
using std::unordered_set;

using blifverifier::TruthTable;
using blifverifier::TruthTableEntry;

TruthTable::TruthTable()
  : mKind(TTKind::NORMAL) { }

TruthTable::TruthTable(TruthTable::TTKind kind)
  : mKind(kind) { }

TruthTableEntry::TruthTableEntry(const std::string& inputs, char output)
  : mInputs(inputs), mOutput(output) { }

char TruthTableEntry::getOutput() const {
  return mOutput;
}

// TODO: if BLIF truthtable contains a contradiction it will be resolved silently
// as true.
void TruthTableEntry::generateCode(ostream& out,
                                                 const vector<string>& input_names) const {
  out << "(";
  for (decltype(mInputs)::size_type i = 0; i < mInputs.size(); ++i) {
    if (mInputs[i] != TOKENS::NC) {
      if (mInputs[i] == TOKENS::ZERO) {
        out << "!";
      }
      out << input_names[i] << " && ";
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
        out << " || ";
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
