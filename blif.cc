#include <algorithm>
#include <cassert>
#include <istream>
#include <iterator>
#include <ostream>
#include <sstream>
#include <stack>
#include <vector>

#include "blif.h"
#include "error.h"
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
  if (!input) {
    throw BadInputStreamError();
  }

  bool read_model = false;
  bool read_inputs = false;
  bool read_outputs = false;

  // Need this to handle validation -- outputs are a special case but we don't
  // know it when reading their truth table. Once we're done reading the file
  // we can discard this, however.
  unordered_set<string> outputs;

  Tokenizer::LineTokenReader reader(input);

  auto tokens = reader.readLine();
  while (reader.isGood() && tokens[0] != TOKENS::END) {
    auto tok = tokens.begin();
    auto section = *tok++;

    // Allow at most one model name.
    if (section == TOKENS::MODEL) {
      if (read_model) {
        throw DuplicateBlockError(reader.getRawLineNumber(), "model");
      }
      read_model = true;
      // De-tokenize the model name (since they can have spaces)
      while (tok != tokens.end()) {
        mModel += *tok++ + " ";
      }
    }

    // Allow at most one inputs section.
    else if (section == TOKENS::INPUTS) {
      if (read_inputs) {
        throw DuplicateBlockError(reader.getRawLineNumber(), "inputs");
      }
      read_inputs = true;
      int index = 0;
      while (tok != tokens.end()) {
        mPrimaryInputs.push_back(*tok);
        mTruthTables[registerLiteral(*tok, "inputs", index++)] = TruthTable();
        ++tok;
      }
    }

    // Allow at most one outputs section.
    else if (section == TOKENS::OUTPUTS) {
      if (read_outputs) {
        throw DuplicateBlockError(reader.getRawLineNumber(), "outputs");
      }
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
      if (!read_inputs) {
        throw NamesBlockBeforeHeadersError(reader.getRawLineNumber(), "inputs");
      } else if (!read_outputs) {
        throw NamesBlockBeforeHeadersError(reader.getRawLineNumber(), "outputs");
      } else if (!read_model) {
        throw NamesBlockBeforeHeadersError(reader.getRawLineNumber(), "model");
      }

      string name = registerLiteral(*(tokens.end() - 1));
      if (mTruthTables.find(name) != mTruthTables.end()) {
        throw DuplicateTruthTableError(reader.getRawLineNumber(), *(tokens.end() - 1));
      }

      auto kind = (outputs.find(name) != outputs.end() ?
                   TruthTable::TTKind::OUTPUT :
                   TruthTable::TTKind::NORMAL);

      vector<string> literals;
      while (tok != tokens.end() - 1) {
        literals.push_back(registerLiteral(*tok++));
      }
      mTruthTables[name] = TruthTable(reader, std::move(literals), kind);

      // TODO: would be nice to verify the consistency of the input cover, or
      //       provide an option to if it's algorithmically expensive.

      // Last remaining valid token.
      } else {
        if (section != TOKENS::END) {
          throw UnrecognizedSectionError(reader.getRawLineNumber(),
                                         section.c_str());
        }
      }

      tokens = reader.readLine();
    }

  // Verify the BLIF's validity for, eg, undefined names.

  // Ensure all dependencies of this truth table are in the map.
  for (const auto& tt : mTruthTables) {
    auto inputs = tt.second;
    for (const auto& input : tt.second.getInputs()) {
      if (mTruthTables.find(input) == mTruthTables.end()) {
        throw MissingLogicDependencyError(mLiteralsReverse[tt.first].c_str(),
                                          mLiteralsReverse[input].c_str());
      }
    }
  }

  // Ensure all primary outputs are defined.
  for (const auto& po : mPrimaryOutputs) {
    if (mTruthTables.find(registerLiteral(po)) == mTruthTables.end()) {
      throw UndefinedPrimaryOutputError(po.c_str());
    }
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

string BLIF::registerLiteral(const string& lit, const string& arrayName,
                             int arrayIndex) {
  std::ostringstream sstr;
  sstr << arrayName << "[" << arrayIndex << "]";
  assert(mLiterals.find(sstr.str()) == mLiterals.end());
  mLiterals[lit] = sstr.str();
  mLiteralsReverse[sstr.str()] = lit;
  return mLiterals[lit];
}

string BLIF::registerLiteral(const string& lit) {
  if (mLiterals.find(lit) == mLiterals.end()) {
    std::ostringstream sstr;
    sstr << "node" << mNextLiteralIndex++;
    mLiterals[lit] = sstr.str();
    mLiterals[sstr.str()] = lit;
  }
  return mLiterals[lit];
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
