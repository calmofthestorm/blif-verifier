#include <algorithm>
#include <cassert>
#include <istream>
#include <iterator>
#include <ostream>
#include <sstream>
#include <stack>
#include <vector>

#include "blif.h"
#include "truthtable.h"
#include "tokenizer.h"

#include <iostream>

namespace blifverifier {

using std::istream;
using std::istringstream;
using std::ostream;
using std::string;
using std::vector;
using std::unordered_set;

BLIF::BLIF(istream&& input)
  : BLIF(input) {}

BLIF::BLIF(istream& input)
  : mNextLiteralIndex(0) {
  assert(input); // TODO all this should be exceptions

  bool read_model = false;
  bool read_inputs = false;
  bool read_outputs = false;

  // Need this to handle validation -- outputs are a special case but we don't
  // know it when reading their truth table.
  unordered_set<string> outputs;

  Tokenizer::LineTokenReader reader(input);

  auto tokens = reader.readLine();
  while (reader.isGood() && tokens[0] != TOKENS::END) {
    auto tok = tokens.begin();
    auto section = *tok++;

    // Allow at most one model name.
    if (section == TOKENS::MODEL) {
      assert(!read_model);
      read_model = true;
      // De-tokenize the model name (since they can have spaces)
      while (tok != tokens.end()) {
        mModel += *tok++ + " ";
      }
    }

    // Allow at most one inputs section.
    else if (section == TOKENS::INPUTS) {
      assert(!read_inputs);
      read_inputs = true;
      int index = 0;
      while (tok != tokens.end()) {
        mPrimaryInputs.push_back(*tok);
        TruthTable tt(TruthTable::TTKind::INPUT);
        mTruthTables[registerLiteral(*tok, "inputs", index++)] = tt;
        ++tok;
      }
    }

    // Allow at most one outputs section.
    else if (section == TOKENS::OUTPUTS) {
      assert(!read_outputs);
      read_outputs = true;
      int index = 0;
      while (tok != tokens.end()) {
        mPrimaryOutputs.push_back(*tok);
        outputs.insert(registerLiteral(*tok, "outputs", index++));
        ++tok;
      }
    }

    // All names sections must be unique, and all outputs need one.
    else if (section == TOKENS::NAMES) {
      // Verify we are ready for these and the uniqueness of the name.
      assert(read_inputs && read_outputs && read_model);
      string name = registerLiteral(*(tokens.end() - 1));
      assert(mTruthTables.find(name) == mTruthTables.end());

      // Parse the truth table.
      // Save the inputs.
      auto kind = (outputs.find(name) != outputs.end() ?
                   TruthTable::TTKind::OUTPUT :
                   TruthTable::TTKind::NORMAL);
      TruthTable tt(kind);
      while (tok != tokens.end() - 1) {
        tt.addInput(registerLiteral(*tok++));
      }

      // Keep reading logic lines until we hit something else, and push it back.
      bool done = false;
      do {
        tokens = reader.readLine();
        if (tokens.size() == 2 &&
            isValidTTEntry(tokens[0], tt.getInputs().size()) &&
            tokens[0].size() == tt.getInputs().size() && tokens[1].size() == 1 &&
            (tokens[1][0] == TOKENS::ZERO || tokens[1][0] == TOKENS::ONE)) {
          // Is a valid logic line.
          TruthTableEntry tte(tokens[0], tokens[1][0]);
          tt.addEntry(tte);
        } else {
          // Either another section or an error.
          reader.putBack(std::move(tokens));
          done = true;
        }
      } while (!done);
      mTruthTables[name] = tt;

      // TODO: would be nice to verify the consistency of the input cover.

      // Last remaining valid token.
      } else {
        assert(section == TOKENS::END);
      }

      tokens = reader.readLine();
    }

  // Verify the BLIF's validity for, eg, undefined names.

  // Ensure all dependencies of this truth table are in the map.
  for (const auto& tt : mTruthTables) {
    auto inputs = tt.second;
    for (const auto& input : tt.second.getInputs()) {
      //TODO exceptions
      assert(mTruthTables.find(input) != mTruthTables.end());
    }
  }

  // Ensure all primary outputs are defined.
  for (const auto& po : mPrimaryOutputs) {
    assert(mTruthTables.find(registerLiteral(po)) != mTruthTables.end());
  }
}

bool BLIF::triviallyNotEquivalent(const BLIF& other, ostream& warn) const {
  if (other.mPrimaryInputs.size() != mPrimaryInputs.size()) {
    warn << "WARNING: Circuits not equivalent; different number of inputs.\n";
    return true;
  }
  
  if (other.mPrimaryInputs != mPrimaryInputs) {
    warn << "WARNING: Circuits not equivalent; input names do not match.\n";
    return true;
  }

  if (other.mPrimaryOutputs.size() != mPrimaryOutputs.size()) {
    warn << "WARNING: Circuits not equivalent; different number of outputs.\n";
    return true;
  }
  
  if (other.mPrimaryOutputs != mPrimaryOutputs) {
    warn << "WARNING: Circuits not equivalent; output names do not match.\n";
    return true;
  }

  return false;
}

const std::vector<std::string>& BLIF::getPrimaryInputs() const {
  return mPrimaryInputs;
}

const std::vector<std::string>& BLIF::getPrimaryOutputs() const {
  return mPrimaryOutputs;
}

string BLIF::registerLiteral(const string& lit,
                                           const string& arrayName,
                                           int arrayIndex) {
  // TODO: throw
  std::ostringstream sstr;
  sstr << arrayName << "[" << arrayIndex << "]";
  assert(mLiterals.find(sstr.str()) == mLiterals.end());
  mLiterals[lit] = sstr.str();
  mLiteralsReverse[sstr.str()] = lit;
  return mLiterals[lit];
}

string BLIF::registerLiteral(const string& lit) {
  // TODO: throw
  if (mLiterals.find(lit) == mLiterals.end()) {
    std::ostringstream sstr;
    sstr << "node" << mNextLiteralIndex++;
    mLiterals[lit] = sstr.str();
    mLiterals[sstr.str()] = lit;
  }
  return mLiterals[lit];
}

bool BLIF::isValidTTEntry(const string& line, int num_entries) {
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

void BLIF::writeEvaluator(std::ostream& output, const string& fxn_name) const {
  output << "void " << fxn_name << "(size_t inputs[numInputs],"
         << " size_t outputs[numOutputs]) {\n";

  // We need to write out the assignments in topologically sorted order to
  // ensure that all dependencies are met. BLIF does not require the circuit
  // be so sorted.
  std::unordered_set<std::string> ordered;
  for (const auto& gate : mTruthTables) {
    if (ordered.find(gate.first) == ordered.end()) {
      std::stack<string> todo;
      todo.push(gate.first);
      while (!todo.empty()) {
        string top_name = todo.top();
        todo.pop();
        bool ready = true;
        const TruthTable& top = mTruthTables.find(top_name)->second;
        for (const auto& dependency : top.getInputs()) {
          if (ordered.find(dependency) == ordered.end()) {
            if (ready == true) {
              todo.push(top_name);
              ready = false;
            }
            todo.push(dependency);
          }
        }
        if (ready) {
          ordered.insert(top_name);
          top.generateCode(top_name, output);
        }
      }
    }
  }

  output << "}\n";
}
}
