#ifndef BLIF_VERIFIER_BLIF_H

#include <iosfwd>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace blifverifier {

struct TruthTableEntry {
  std::string inputs;
  char output;

  // Represents the entry as a C logic expression.
  void generateCode(std::ostream& out, const std::vector<std::string>& input_names) const;
};

struct TruthTable {
  std::vector<std::string> inputs;
  std::vector<TruthTableEntry> entries;

  // Represents the entire truth table as a C logic expression.
  void generateCode(std::ostream& out) const;
};

//TODO: use exceptions
class BLIF {
  public:
    // Constructs an empty BLIF (no inputs, outputs, names, logic, etc).
    BLIF();

    // Reads a BLIF from a stream.
    BLIF(std::istream& input);

    // Writes out a C function that implements the BLIF's logic to the
    // provided ostream.
    void writeEvaluator(std::ostream& output, const std::string& fxn_name) const;

    // Returns a vector of the names of the node's inputs.
    const std::vector<bool>& getInputs() const;

  private:
    // Reads a single line from the stream, and returns a vector of strings.
    // Will strip comments denoted by #, and any lines with no tokens.
    static std::vector<std::string> readLineAsTokens(std::istream& input);

    // Verifies a truth table line.
    static bool isValidTTEntry(const std::string& line, int num_entries);

    // Generate a unique name for this user node (eg to mitigate naming conflicts)
    std::string registerLiteral(const std::string& lit);
    std::string registerLiteral(const std::string& lit,
                                      const std::string& arrayName,
                                      int arrayIndex);

    // Name of the model
    std::string mModel;

    // A vector of the primary inputs names.
    std::unordered_set<std::string> mPrimaryInputs;

    // A vector of the primary outputs names.
    std::unordered_set<std::string> mPrimaryOutputs;

    // Map from user supplied names to the ones we'll use.
    std::unordered_map<std::string, std::string> mLiterals;

    // Next integer to use for unique generated names.
    int mNextLiteralIndex;

    // Map from node names (truth tables) to the truth tables that represent them.
    std::unordered_map<std::string, TruthTable> mTruthTables;
};

} // namespace blifverifier

#endif // BLIF_VERIFIER_BLIF_H
