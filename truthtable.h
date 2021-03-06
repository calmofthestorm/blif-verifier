#ifndef BLIF_VERIFIER_TRUTHTABLE_H
#define BLIF_VERIFIER_TRUTHTABLE_H

#include <iosfwd>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>

#include "error.h"

namespace Tokenizer {
  class LineTokenReader;
}

namespace blifverifier {

// TODO: clean this up
namespace TOKENS {
  const std::string MODEL = ".model";
  const std::string INPUTS = ".inputs";
  const std::string OUTPUTS = ".outputs";
  const std::string NAMES = ".names";
  const std::string END = ".end";
  const char ZERO = '0';
  const char ONE = '1';
  const char NC = '-';
}

class TruthTableEntry {
  public:
    TruthTableEntry();
    TruthTableEntry(const std::string& inputs, char output);

    char getOutput() const;

    // Represents the entry as a C logic expression.
    void generateCode(std::ostream& out,
                      const std::vector<int>& input_names,
                      const std::unordered_map<int, std::string>& nicknames) const;

  private:
    std::string mInputs;
    char mOutput;
};

class TruthTable {
  public:
    // Special kinds of truth tables.
    enum class TTKind {INPUT, OUTPUT, NORMAL};

    TruthTable();  // Default -- primary input
    TruthTable(Tokenizer::LineTokenReader& reader,
               std::vector<int>&& inputs, TTKind kind);

    // Add an input to the truth table (not valid for inputs)
    void addInput(int input);

    // Add an entry to the truth table
    void addEntry(const TruthTableEntry& entry);

    const std::vector<int>& getInputs() const;

    // Represents the entire truth table as a C logic expression.
    void generateCode(int, std::ostream& out,
                      const std::unordered_map<int, std::string>& nicknames) const;

  private:
    // Verifies a truth table line.
    static bool isValidTTEntry(const std::string& line, int num_entries);

    // (Translated) names of the inputs to the truth table.
    std::vector<int> mInputs;

    // All entries for the circuit that have defined output.
    std::vector<TruthTableEntry> mEntries;

    // Primary inputs and outputs require special treatment for code generation.
    TTKind mKind;
};

}  // namespace blifverifier

#endif
