#ifndef BLIF_VERIFIER_TRUTHTABLE_H
#define BLIF_VERIFIER_TRUTHTABLE_H

#include <iosfwd>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace blifverifier {

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

class TruthTableEntry {
  public:
    TruthTableEntry();
    TruthTableEntry(const std::string& inputs, char output);

    char getOutput() const;

    // Represents the entry as a C logic expression.
    void generateCode(std::ostream& out,
                      const std::vector<std::string>& input_names) const;

  private:
    std::string mInputs;
    char mOutput;
};

class TruthTable {
  public:
    // Special kinds of truth tables.
    enum class TTKind {INPUT, OUTPUT, NORMAL};

    TruthTable();
    TruthTable(TTKind kind);

    // Add an input to the truth table (not valid for inputs)
    void addInput(const std::string& input);

    // Add an entry to the truth table
    void addEntry(const TruthTableEntry& entry);

    const std::vector<std::string>& getInputs() const;    

    // Represents the entire truth table as a C logic expression.
    void generateCode(const std::string& name, std::ostream& out) const;

  private:
    // (Translated) names of the inputs to the truth table.
    std::vector<std::string> mInputs;

    // All entries for the circuit that have defined output.
    std::vector<TruthTableEntry> mEntries;

    // Primary inputs and outputs require special treatment for code generation.
    TTKind mKind;
};

} // namespace blifverifier

#endif
