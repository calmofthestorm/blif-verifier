#include "blif.h"

#include <cassert>

#include <algorithm>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stack>
#include <string>
#include <vector>

#include "error.h"
#include "tokenizer.h"
#include "truthtable.h"

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
  unordered_set<int> outputs;

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
      while (tok != tokens.end()) {
        mPrimaryInputs.push_back(*tok);
        mTruthTables[registerLiteral(*tok)] = TruthTable();
        ++tok;
      }
    }

    // Allow at most one outputs section.
    else if (section == TOKENS::OUTPUTS) {
      if (read_outputs) {
        throw DuplicateBlockError(reader.getRawLineNumber(), "outputs");
      }
      read_outputs = true;
      while (tok != tokens.end()) {
        mPrimaryOutputs.push_back(*tok);
        outputs.insert(registerLiteral(*tok));
        ++tok;
      }
    }

    // All names sections must be unique, and all outputs need one.
    else if (section == TOKENS::NAMES) {
      // Verify we are ready for these and the uniqueness of the name.
      if (!read_inputs) {
        throw NamesBlockBeforeHeadersError(reader.getRawLineNumber(), "inputs");
      } else if (!read_outputs) {
        throw NamesBlockBeforeHeadersError(reader.getRawLineNumber(),
                                           "outputs");
      } else if (!read_model) {
        throw NamesBlockBeforeHeadersError(reader.getRawLineNumber(), "model");
      }

      int name = registerLiteral(*(tokens.end() - 1));
      if (mTruthTables.find(name) != mTruthTables.end()) {
        throw DuplicateTruthTableError(reader.getRawLineNumber(),
                                       *(tokens.end() - 1));
      }

      auto kind = (outputs.find(name) != outputs.end() ?
                   TruthTable::TTKind::OUTPUT :
                   TruthTable::TTKind::NORMAL);

      vector<int> literals;
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

int BLIF::registerLiteral(const string& lit) {
  if (mLiterals.find(lit) == mLiterals.end()) {
    mLiterals[lit] = mNextLiteralIndex;
    mLiteralsReverse[mNextLiteralIndex] = lit;
    ++mNextLiteralIndex;
  }
  return mLiterals[lit];
}

void BLIF::writeEvaluator(std::ostream& output, const string& fxn_name) const {
  output << "void " << fxn_name << "(size_t inputs[numInputs],"
         << " size_t outputs[numOutputs]) {\n";

  // Generate node names for each gate
  std::unordered_map<int, string> nicknames;
  for (const auto& gate : mLiteralsReverse) {
    nicknames[gate.first] = "node" + std::to_string(gate.first); // keys only
  }

  // Special case primary inputs and outputs
  for (decltype(mPrimaryInputs)::size_type i = 0; i < mPrimaryInputs.size(); ++i) {
    assert(mLiterals.find(mPrimaryInputs[i]) != mLiterals.end());
    int key = mLiterals.find(mPrimaryInputs[i])->second;
    nicknames[key] = "inputs[" + std::to_string(i) + "]";
  }

  for (decltype(mPrimaryOutputs)::size_type i = 0; i < mPrimaryOutputs.size(); ++i) {
    assert(mLiterals.find(mPrimaryOutputs[i]) != mLiterals.end());
    int key = mLiterals.find(mPrimaryOutputs[i])->second;
    nicknames[key] = "outputs[" + std::to_string(i) + "]";
  }

  // We need to write out the assignments in topologically sorted order to
  // ensure that all dependencies are met. BLIF does not require the circuit
  // be so sorted.
  std::unordered_set<int> ordered;
  for (const auto& gate : mTruthTables) {
    if (ordered.find(gate.first) == ordered.end()) {
      std::stack<int> todo;
      std::unordered_set<int> visited;
      todo.push(gate.first);
      while (!todo.empty()) {
        int top_name = todo.top();
        todo.pop();
        if (ordered.find(top_name) == ordered.end()) {
          // Mark this node as visit started to catch cycles
          visited.insert(top_name);
          bool ready = true;
          const TruthTable& top = mTruthTables.find(top_name)->second;
          for (const auto& dependency : top.getInputs()) {
            if (ordered.find(dependency) == ordered.end()) {
              if (ready) {
                todo.push(top_name);
                ready = false;
              }

              // If we attempt to visit a node we've already visited, and
              // it has not been ordered, then there is a cycle.
              if (visited.find(dependency) == visited.end()) {
                todo.push(dependency);
              } else {
                throw CircularDependencyError(mLiteralsReverse.find(top_name)->second,
                                              mLiteralsReverse.find(dependency)->second);
              }
            }
          }
          if (ready) {
            ordered.insert(top_name);
            top.generateCode(top_name, output, nicknames);
          }
        }
      }
    }
  }

  output << "}\n";
}
}
