#include <algorithm>
#include <cassert>
#include <istream>
#include <iterator>
#include <ostream>
#include <sstream>
#include <stack>
#include <vector>

#include "blif.h"

#include <iostream>

using std::istream;
using std::istringstream;
using std::ostream;
using std::string;
using std::vector;

namespace {
  namespace TOKENS {
    const std::string MODEL = ".model";
    const std::string INPUTS = ".inputs";
    const std::string OUTPUTS = ".outputs";
    const std::string NAMES = ".names";
    const std::string END = ".end";
    const char ZERO = '0';
    const char ONE = '1';
    const char NC = '-';
  };
}

void blifverifier::TruthTableEntry::generateCode(ostream& out,
                                                 const vector<string>& input_names) const {
  out << "(";
  for (decltype(inputs)::size_type i = 0; i < inputs.size(); ++i) {
    if (inputs[i] != TOKENS::NC) {
      if (inputs[i] == TOKENS::ZERO) {
        out << "!";
      }
      out << input_names[i] << " && ";
    }
  }
  out << " true)";
}

void blifverifier::TruthTable::generateCode(ostream& out) const {
  out << "(";
  for (const auto& entry : entries) {
    if (entry.output == TOKENS::ONE) {
      entry.generateCode(out, inputs);
      out << " || ";
    }
  }
  out << " false)";
}

blifverifier::BLIF::BLIF(istream&& input)
  : BLIF(input) {}

blifverifier::BLIF::BLIF(istream& input)
  : mNextLiteralIndex(0) {
  assert(input); // TODO all this should be exceptions

  bool read_model = false;
  bool read_inputs = false;
  bool read_outputs = false;

  auto tokens = readLineAsTokens(input);
  while (input && !tokens.empty() && tokens[0] != TOKENS::END) {
    auto tok = tokens.begin();
    auto section = *tok++;

    // This is so we can "push back" when we read one line too far in the .names
    // sections.
    bool read_next_line = true;

    // Allow at most one model name.
    if (section == TOKENS::MODEL) {
      assert(!read_model);
      read_model = true;
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
        // Replace all user names with nicknames to deal with conflicts cleanly.
        mPrimaryInputs.insert(registerLiteral(*tok++, "inputs", index++));
      }
    }

    // Allow at most one outputs section.
    else if (section == TOKENS::OUTPUTS) {
      assert(!read_outputs);
      read_outputs = true;
      int index = 0;
      while (tok != tokens.end()) {
        // Replace all user names with nicknames to deal with conflicts cleanly.
        mPrimaryOutputs.insert(registerLiteral(*tok++, "outputs", index++));
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
      TruthTable tt;
      while (tok != tokens.end() - 1) {
        tt.inputs.push_back(registerLiteral(*tok++));
      }

      // Keep reading logic lines until we hit something else, and push it back.
      do {
        tokens = readLineAsTokens(input);
        if (tokens.size() == 2 && isValidTTEntry(tokens[0], tt.inputs.size()) &&
            tokens[0].size() == tt.inputs.size() && tokens[1].size() == 1 &&
            (tokens[1][0] == TOKENS::ZERO || tokens[1][0] == TOKENS::ONE)) {
          // Is a valid logic line.
          TruthTableEntry tte{tokens[0], tokens[1][0]};
          tt.entries.push_back(tte);
        } else {
          // Either another section or an error.
          read_next_line = false;
        }
      } while (read_next_line);
      mTruthTables[name] = tt;

      // TODO: would be nice to verify the consistency of the input cover.

      // Last remaining valid token.
      } else {
        assert(section == TOKENS::END);
      }

      if (read_next_line) {
        tokens = readLineAsTokens(input);
      }
    }

  // Verify the BLIF's validity for, eg, undefined names.

  // Ensure all dependencies of this truth table are in the map.
  for (const auto& tt : mTruthTables) {
    auto inputs = tt.second;
    for (const auto& input : tt.second.inputs) {
      assert(mPrimaryInputs.find(input) != mPrimaryInputs.end() ||
             mTruthTables.find(input) != mTruthTables.end());
    }
  }

  // Ensure all primary outputs are defined.
  for (const auto& po : mPrimaryOutputs) {
    assert(mTruthTables.find(po) != mTruthTables.end());
  }
}

string blifverifier::BLIF::registerLiteral(const string& lit,
                                           const string& arrayName,
                                           int arrayIndex) {
  // TODO: throw
  std::ostringstream sstr;
  sstr << arrayName << "[" << arrayIndex << "]";
  assert(mLiterals.find(sstr.str()) == mLiterals.end());
  mLiterals[lit] = sstr.str();
  return mLiterals[lit] = sstr.str();
}

string blifverifier::BLIF::registerLiteral(const string& lit) {
  // TODO: throw
  if (mLiterals.find(lit) == mLiterals.end()) {
    std::ostringstream sstr;
    sstr << "node" << mNextLiteralIndex++;
    mLiterals[lit] = sstr.str();
  }
  return mLiterals[lit];
}

bool blifverifier::BLIF::isValidTTEntry(const string& line, int num_entries) {
  if (num_entries != line.size()) {
    return false;
  }

  for (const auto& c : line) {
    if (c != TOKENS::ZERO && c != TOKENS::ONE && c != TOKENS::NC) {
      return false;
    }
  }

  return true;
}

vector<string> blifverifier::BLIF::readLineAsTokens(istream& input) {
  string line;
  vector<string> result;

  // Read until we fail to read a line or get a non-empty line. Lines with just
  // whitespace or a comment are ignored as empty.
  while (result.empty()) {
    // Read failure. Return empty result.
    if (!std::getline(input, line)) {
      return result;
    }

    // Tokenize the line and store the tokens in the vector.
    istringstream tokenizer(line);
    std::copy(std::istream_iterator<string>(tokenizer),
              std::istream_iterator<string>(),
              std::back_inserter(result));


    // Remove any comments (tokens that begin with #) and any tokens after them
    // if a comment is found.
    auto comment = std::find_if(result.begin(), result.end(),
                                [](const string& tok){return tok[0] == '#';});
    result.erase(comment, result.end());
  }
  return result;
}

void blifverifier::BLIF::writeEvaluator(std::ostream& output, const string& fxn_name) const {
  output << "void " << fxn_name << "(size_t inputs[numInputs],"
         << " size_t outputs[numOutputs]) {\n";

  // We need to write out the assignments in topologically sorted order to
  // ensure that all dependencies are met. BLIF does not require the circuit
  // be so sorted.
  std::unordered_set<std::string> ordered(mPrimaryInputs.begin(), mPrimaryInputs.end());
  for (const auto& gate : mTruthTables) {
    if (ordered.find(gate.first) == ordered.end()) {
      std::stack<string> todo;
      todo.push(gate.first);
      while (!todo.empty()) {
        string top_name = todo.top();
        todo.pop();
        bool ready = true;
        const TruthTable& top = mTruthTables.find(top_name)->second;
        for (const auto& dependency : top.inputs) {
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
          output << "size_t " << top_name << " = ";
          top.generateCode(output);
          output << "; // strategy: naive\n";
        }
      }
    }
  }

  output << "}\n";
}
