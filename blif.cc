#include <algorithm>
#include <cassert>
#include <istream>
#include <iterator>
#include <ostream>
#include <sstream>
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

blifverifier::BLIF::BLIF() {
  /*
  istringstream test("Hello world #how are you?");
  auto res = readLineAsTokens(test);
  std::copy(res.begin(), res.end(), std::ostream_iterator<string>(std::cout));
  */
}

blifverifier::BLIF::BLIF(istream& input) {
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
      std::copy(tok, tokens.end(),
                std::inserter(mPrimaryInputs, mPrimaryInputs.end()));
    }

    // Allow at most one outputs section.
    else if (section == TOKENS::OUTPUTS) {
      assert(!read_outputs);
      read_outputs = true;
      std::copy(tok, tokens.end(),
                std::inserter(mPrimaryOutputs, mPrimaryOutputs.end()));
    }

    // All names sections must be unique, and all outputs need one.
    else if (section == TOKENS::NAMES) {
      // Verify we are ready for these and the uniqueness of the name.
      assert(read_inputs && read_outputs && read_model);
      string name = *(tokens.end() - 1);
      assert(mTruthTables.find(name) == mTruthTables.end());

      // Parse the truth table.
      // Save the inputs.
      TruthTable tt;
      std::copy(tok, tokens.end() - 1, std::back_inserter(tt.inputs));

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
